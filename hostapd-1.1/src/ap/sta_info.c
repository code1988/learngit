/*
 * hostapd / Station table
 * Copyright (c) 2002-2011, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "utils/includes.h"

#include "utils/common.h"
#include "utils/eloop.h"
#include "common/ieee802_11_defs.h"
#include "common/wpa_ctrl.h"
#include "radius/radius.h"
#include "radius/radius_client.h"
#include "drivers/driver.h"
#include "p2p/p2p.h"
#include "hostapd.h"
#include "accounting.h"
#include "ieee802_1x.h"
#include "ieee802_11.h"
#include "wpa_auth.h"
#include "preauth_auth.h"
#include "ap_config.h"
#include "beacon.h"
#include "ap_mlme.h"
#include "vlan_init.h"
#include "p2p_hostapd.h"
#include "ap_drv_ops.h"
#include "sta_info.h"

static void ap_sta_remove_in_other_bss(struct hostapd_data *hapd,
				       struct sta_info *sta);
static void ap_handle_session_timer(void *eloop_ctx, void *timeout_ctx);
static void ap_sta_deauth_cb_timeout(void *eloop_ctx, void *timeout_ctx);
static void ap_sta_disassoc_cb_timeout(void *eloop_ctx, void *timeout_ctx);
#ifdef CONFIG_IEEE80211W
static void ap_sa_query_timer(void *eloop_ctx, void *timeout_ctx);
#endif /* CONFIG_IEEE80211W */
static int ap_sta_remove(struct hostapd_data *hapd, struct sta_info *sta);

// 遍历sta链表，调用传入的cb
int ap_for_each_sta(struct hostapd_data *hapd,
		    int (*cb)(struct hostapd_data *hapd, struct sta_info *sta,
			      void *ctx),
		    void *ctx)
{
	struct sta_info *sta;

	for (sta = hapd->sta_list; sta; sta = sta->next) {
		if (cb(hapd, sta, ctx))
			return 1;
	}

	return 0;
}

// 在当前bss上的hash表中查找符合的入口sta_info（只在当前bss上找）
struct sta_info * ap_get_sta(struct hostapd_data *hapd, const u8 *sta)
{
	struct sta_info *s;

	s = hapd->sta_hash[STA_HASH(sta)];
	while (s != NULL && os_memcmp(s->addr, sta, 6) != 0)
		s = s->hnext;
	return s;
}


static void ap_sta_list_del(struct hostapd_data *hapd, struct sta_info *sta)
{
	struct sta_info *tmp;

	if (hapd->sta_list == sta) {
		hapd->sta_list = sta->next;
		return;
	}

	tmp = hapd->sta_list;
	while (tmp != NULL && tmp->next != sta)
		tmp = tmp->next;
	if (tmp == NULL) {
		wpa_printf(MSG_DEBUG, "Could not remove STA " MACSTR " from "
			   "list.", MAC2STR(sta->addr));
	} else
		tmp->next = sta->next;
}

// 将一个入口元素插入hash表(插入指定槽位链表的头部) 
void ap_sta_hash_add(struct hostapd_data *hapd, struct sta_info *sta)
{
	sta->hnext = hapd->sta_hash[STA_HASH(sta->addr)];
	hapd->sta_hash[STA_HASH(sta->addr)] = sta;
}


static void ap_sta_hash_del(struct hostapd_data *hapd, struct sta_info *sta)
{
	struct sta_info *s;

	s = hapd->sta_hash[STA_HASH(sta->addr)];
	if (s == NULL) return;
	if (os_memcmp(s->addr, sta->addr, 6) == 0) {
		hapd->sta_hash[STA_HASH(sta->addr)] = s->hnext;
		return;
	}

	while (s->hnext != NULL &&
	       os_memcmp(s->hnext->addr, sta->addr, ETH_ALEN) != 0)
		s = s->hnext;
	if (s->hnext != NULL)
		s->hnext = s->hnext->hnext;
	else
		wpa_printf(MSG_DEBUG, "AP: could not remove STA " MACSTR
			   " from hash table", MAC2STR(sta->addr));
}

// 释放当前bss上指定sta的所有资源
void ap_free_sta(struct hostapd_data *hapd, struct sta_info *sta)
{
	int set_beacon = 0;

    // 停止对该sta计费
	accounting_sta_stop(hapd, sta);

	/* just in case 
     * 停止对该sta的授权
     * */
	ap_sta_set_authorized(hapd, sta, 0);

	if (sta->flags & WLAN_STA_WDS)
		hostapd_set_wds_sta(hapd, sta->addr, sta->aid, 0);

	if (!(sta->flags & WLAN_STA_PREAUTH))
		hostapd_drv_sta_remove(hapd, sta->addr);

	ap_sta_hash_del(hapd, sta);
	ap_sta_list_del(hapd, sta);

	if (sta->aid > 0)
		hapd->sta_aid[(sta->aid - 1) / 32] &=
			~BIT((sta->aid - 1) % 32);

	hapd->num_sta--;
	if (sta->nonerp_set) {
		sta->nonerp_set = 0;
		hapd->iface->num_sta_non_erp--;
		if (hapd->iface->num_sta_non_erp == 0)
			set_beacon++;
	}

	if (sta->no_short_slot_time_set) {
		sta->no_short_slot_time_set = 0;
		hapd->iface->num_sta_no_short_slot_time--;
		if (hapd->iface->current_mode->mode == HOSTAPD_MODE_IEEE80211G
		    && hapd->iface->num_sta_no_short_slot_time == 0)
			set_beacon++;
	}

	if (sta->no_short_preamble_set) {
		sta->no_short_preamble_set = 0;
		hapd->iface->num_sta_no_short_preamble--;
		if (hapd->iface->current_mode->mode == HOSTAPD_MODE_IEEE80211G
		    && hapd->iface->num_sta_no_short_preamble == 0)
			set_beacon++;
	}

	if (sta->no_ht_gf_set) {
		sta->no_ht_gf_set = 0;
		hapd->iface->num_sta_ht_no_gf--;
	}

	if (sta->no_ht_set) {
		sta->no_ht_set = 0;
		hapd->iface->num_sta_no_ht--;
	}

	if (sta->ht_20mhz_set) {
		sta->ht_20mhz_set = 0;
		hapd->iface->num_sta_ht_20mhz--;
	}

#ifdef CONFIG_P2P
	if (sta->no_p2p_set) {
		sta->no_p2p_set = 0;
		hapd->num_sta_no_p2p--;
		if (hapd->num_sta_no_p2p == 0)
			hostapd_p2p_non_p2p_sta_disconnected(hapd);
	}
#endif /* CONFIG_P2P */

#if defined(NEED_AP_MLME) && defined(CONFIG_IEEE80211N)
	if (hostapd_ht_operation_update(hapd->iface) > 0)
		set_beacon++;
#endif /* NEED_AP_MLME && CONFIG_IEEE80211N */

	if (set_beacon)
		ieee802_11_set_beacons(hapd->iface);

    // 关闭所有4个定时器
	eloop_cancel_timeout(ap_handle_timer, hapd, sta);
	eloop_cancel_timeout(ap_handle_session_timer, hapd, sta);
	eloop_cancel_timeout(ap_sta_deauth_cb_timeout, hapd, sta);
	eloop_cancel_timeout(ap_sta_disassoc_cb_timeout, hapd, sta);

    // 注销该sta上的802.1x相关内容
	ieee802_1x_free_station(sta);
	wpa_auth_sta_deinit(sta->wpa_sm);
	rsn_preauth_free_station(hapd, sta);
#ifndef CONFIG_NO_RADIUS
	radius_client_flush_auth(hapd->radius, sta->addr);
#endif /* CONFIG_NO_RADIUS */

	os_free(sta->last_assoc_req);
	os_free(sta->challenge);

#ifdef CONFIG_IEEE80211W
	os_free(sta->sa_query_trans_id);
	eloop_cancel_timeout(ap_sa_query_timer, hapd, sta);
#endif /* CONFIG_IEEE80211W */

#ifdef CONFIG_P2P
	p2p_group_notif_disassoc(hapd->p2p_group, sta->addr);
#endif /* CONFIG_P2P */

	wpabuf_free(sta->wps_ie);
	wpabuf_free(sta->p2p_ie);

	os_free(sta->ht_capabilities);

	os_free(sta);
}

// 释放该bss上的所有sta
void hostapd_free_stas(struct hostapd_data *hapd)
{
	struct sta_info *sta, *prev;

	sta = hapd->sta_list;

    // 遍历该bss上的sta
	while (sta) {
		prev = sta;
		if (sta->flags & WLAN_STA_AUTH) {
			mlme_deauthenticate_indication(
				hapd, sta, WLAN_REASON_UNSPECIFIED);
		}
		sta = sta->next;
		wpa_printf(MSG_DEBUG, "Removing station " MACSTR,
			   MAC2STR(prev->addr));
        // 不经老化，直接释放每个sta
		ap_free_sta(hapd, prev);
	}
}


/**
 * ap_handle_timer - Per STA timer handler
 * @eloop_ctx: struct hostapd_data *
 * @timeout_ctx: struct sta_info *
 * sta老化定时器超时处理函数(执行该回调时deauth和disassoc定时器已经都执行完毕)
 *
 * This function is called to check station activity and to remove inactive
 * stations.
 */
void ap_handle_timer(void *eloop_ctx, void *timeout_ctx)
{
	struct hostapd_data *hapd = eloop_ctx;
	struct sta_info *sta = timeout_ctx;
	unsigned long next_time = 0;

    // 如果老化原因是STA_REMOVE，在这里就是直接释放该sta资源
	if (sta->timeout_next == STA_REMOVE) {
		hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_INFO, "deauthenticated due to "
			       "local deauth request");
		ap_free_sta(hapd, sta);
		return;
	}

	if ((sta->flags & WLAN_STA_ASSOC) &&
	    (sta->timeout_next == STA_NULLFUNC ||
	     sta->timeout_next == STA_DISASSOC)) {
		int inactive_sec;
		inactive_sec = hostapd_drv_get_inact_sec(hapd, sta->addr);
		if (inactive_sec == -1) {
			wpa_msg(hapd->msg_ctx, MSG_DEBUG,
				"Check inactivity: Could not "
				"get station info from kernel driver for "
				MACSTR, MAC2STR(sta->addr));
			/*
			 * The driver may not support this functionality.
			 * Anyway, try again after the next inactivity timeout,
			 * but do not disconnect the station now.
			 */
			next_time = hapd->conf->ap_max_inactivity;
		} else if (inactive_sec < hapd->conf->ap_max_inactivity &&
			   sta->flags & WLAN_STA_ASSOC) {
			/* station activity detected; reset timeout state */
			wpa_msg(hapd->msg_ctx, MSG_DEBUG,
				"Station " MACSTR " has been active %is ago",
				MAC2STR(sta->addr), inactive_sec);
			sta->timeout_next = STA_NULLFUNC;
			next_time = hapd->conf->ap_max_inactivity -
				inactive_sec;
		} else {
			wpa_msg(hapd->msg_ctx, MSG_DEBUG,
				"Station " MACSTR " has been "
				"inactive too long: %d sec, max allowed: %d",
				MAC2STR(sta->addr), inactive_sec,
				hapd->conf->ap_max_inactivity);
		}
	}

	if ((sta->flags & WLAN_STA_ASSOC) &&
	    sta->timeout_next == STA_DISASSOC &&
	    !(sta->flags & WLAN_STA_PENDING_POLL)) {
		wpa_msg(hapd->msg_ctx, MSG_DEBUG, "Station " MACSTR
			" has ACKed data poll", MAC2STR(sta->addr));
		/* data nullfunc frame poll did not produce TX errors; assume
		 * station ACKed it */
		sta->timeout_next = STA_NULLFUNC;
		next_time = hapd->conf->ap_max_inactivity;
	}

	if (next_time) {
		eloop_register_timeout(next_time, 0, ap_handle_timer, hapd,
				       sta);
		return;
	}

	if (sta->timeout_next == STA_NULLFUNC &&
	    (sta->flags & WLAN_STA_ASSOC)) {
		wpa_printf(MSG_DEBUG, "  Polling STA");
		sta->flags |= WLAN_STA_PENDING_POLL;
		hostapd_drv_poll_client(hapd, hapd->own_addr, sta->addr,
					sta->flags & WLAN_STA_WMM);
	} else if (sta->timeout_next != STA_REMOVE) {
		int deauth = sta->timeout_next == STA_DEAUTH;

		wpa_printf(MSG_DEBUG, "Sending %s info to STA " MACSTR,
			   deauth ? "deauthentication" : "disassociation",
			   MAC2STR(sta->addr));

		if (deauth) {
			hostapd_drv_sta_deauth(
				hapd, sta->addr,
				WLAN_REASON_PREV_AUTH_NOT_VALID);
		} else {
			hostapd_drv_sta_disassoc(
				hapd, sta->addr,
				WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY);
		}
	}

	switch (sta->timeout_next) {
	case STA_NULLFUNC:
		sta->timeout_next = STA_DISASSOC;
		eloop_register_timeout(AP_DISASSOC_DELAY, 0, ap_handle_timer,
				       hapd, sta);
		break;
	case STA_DISASSOC:
		ap_sta_set_authorized(hapd, sta, 0);
		sta->flags &= ~WLAN_STA_ASSOC;
		ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
		if (!sta->acct_terminate_cause)
			sta->acct_terminate_cause =
				RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT;
		accounting_sta_stop(hapd, sta);
		ieee802_1x_free_station(sta);
		hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_INFO, "disassociated due to "
			       "inactivity");
		sta->timeout_next = STA_DEAUTH;
		eloop_register_timeout(AP_DEAUTH_DELAY, 0, ap_handle_timer,
				       hapd, sta);
		mlme_disassociate_indication(
			hapd, sta, WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY);
		break;
	case STA_DEAUTH:
	case STA_REMOVE:
		hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_INFO, "deauthenticated due to "
			       "inactivity");
		if (!sta->acct_terminate_cause)
			sta->acct_terminate_cause =
				RADIUS_ACCT_TERMINATE_CAUSE_IDLE_TIMEOUT;
		mlme_deauthenticate_indication(
			hapd, sta,
			WLAN_REASON_PREV_AUTH_NOT_VALID);
		ap_free_sta(hapd, sta);
		break;
	}
}

// 对应每个用户认证通过后提供服务的超时处理函数，超时后意味着该用户将被踢下线
static void ap_handle_session_timer(void *eloop_ctx, void *timeout_ctx)
{
	struct hostapd_data *hapd = eloop_ctx;
	struct sta_info *sta = timeout_ctx;
	u8 addr[ETH_ALEN];

	if (!(sta->flags & WLAN_STA_AUTH))
		return;

	mlme_deauthenticate_indication(hapd, sta,
				       WLAN_REASON_PREV_AUTH_NOT_VALID);
	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
		       HOSTAPD_LEVEL_INFO, "deauthenticated due to "
		       "session timeout");
	sta->acct_terminate_cause =
		RADIUS_ACCT_TERMINATE_CAUSE_SESSION_TIMEOUT;
	os_memcpy(addr, sta->addr, ETH_ALEN);
    // 由于服务超时，所以不经老化，直接释放该sta资源
	ap_free_sta(hapd, sta);
	hostapd_drv_sta_deauth(hapd, addr, WLAN_REASON_PREV_AUTH_NOT_VALID);
}

// 更新为指定sta提供服务的剩余时间
void ap_sta_session_timeout(struct hostapd_data *hapd, struct sta_info *sta,
			    u32 session_timeout)
{
	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
		       HOSTAPD_LEVEL_DEBUG, "setting session timeout to %d "
		       "seconds", session_timeout);
    // 实际在这里就是更新提供服务超时定时器时间 
	eloop_cancel_timeout(ap_handle_session_timer, hapd, sta);
	eloop_register_timeout(session_timeout, 0, ap_handle_session_timer,
			       hapd, sta);
}


void ap_sta_no_session_timeout(struct hostapd_data *hapd, struct sta_info *sta)
{
	eloop_cancel_timeout(ap_handle_session_timer, hapd, sta);
}

// 每收到一个新的mac都需要被记录进hash表和链表中
struct sta_info * ap_sta_add(struct hostapd_data *hapd, const u8 *addr)
{
	struct sta_info *sta;

    // 记录之前先确认hash表中是否已经存在该mac
	sta = ap_get_sta(hapd, addr);
	if (sta)
		return sta;

	wpa_printf(MSG_DEBUG, "  New STA");
    // 如果记录满了，0.7.3版本的处理方法是直接丢弃
	if (hapd->num_sta >= hapd->conf->max_num_sta) {
		/* FIX: might try to remove some old STAs first? */
		wpa_printf(MSG_DEBUG, "no more room for new STAs (%d/%d)",
			   hapd->num_sta, hapd->conf->max_num_sta);
		return NULL;
	}

	sta = os_zalloc(sizeof(struct sta_info));
	if (sta == NULL) {
		wpa_printf(MSG_ERROR, "malloc failed");
		return NULL;
	}
	sta->acct_interim_interval = hapd->conf->acct_interim_interval;

	/* initialize STA info data */
    // 为每个新的站表元素注册老化时间
	eloop_register_timeout(hapd->conf->ap_max_inactivity, 0,
			       ap_handle_timer, hapd, sta);
	os_memcpy(sta->addr, addr, ETH_ALEN);
    // 插入链表 
	sta->next = hapd->sta_list;
	hapd->sta_list = sta;
	hapd->num_sta++;
    // 插入hash表
	ap_sta_hash_add(hapd, sta);
	sta->ssid = &hapd->conf->ssid;
    // 删除当前接口的其他bss中的该入口元素sta_info，也就是每个接口中的所有bss只能记录一个相同mac
	ap_sta_remove_in_other_bss(hapd, sta);

	return sta;
}

// 删除该sta(只被deauth和disassoc两个超时回调中调用)
static int ap_sta_remove(struct hostapd_data *hapd, struct sta_info *sta)
{
    // 清除eap<->eapol交互接口中的portEnabled标志，用于通知状态机不再激活
	ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);

	wpa_printf(MSG_DEBUG, "Removing STA " MACSTR " from kernel driver",
		   MAC2STR(sta->addr));
    // 调用驱动层的删除操作
	if (hostapd_drv_sta_remove(hapd, sta->addr) &&
	    sta->flags & WLAN_STA_ASSOC) {
		wpa_printf(MSG_DEBUG, "Could not remove station " MACSTR
			   " from kernel driver.", MAC2STR(sta->addr));
		return -1;
	}
	return 0;
}

// 删除当前接口的其他bss中的指定mac
static void ap_sta_remove_in_other_bss(struct hostapd_data *hapd,
				       struct sta_info *sta)
{
	struct hostapd_iface *iface = hapd->iface;
	size_t i;

	for (i = 0; i < iface->num_bss; i++) {
		struct hostapd_data *bss = iface->bss[i];
		struct sta_info *sta2;
		/* bss should always be set during operation, but it may be
		 * NULL during reconfiguration. Assume the STA is not
		 * associated to another BSS in that case to avoid NULL pointer
		 * dereferences. */
		if (bss == hapd || bss == NULL)
			continue;
		sta2 = ap_get_sta(bss, sta->addr);
		if (!sta2)
			continue;

        // 注销其他bss上相同mac的sta
		ap_sta_disconnect(bss, sta2, sta2->addr,
				  WLAN_REASON_PREV_AUTH_NOT_VALID);
	}
}

// sta的disassoc超时处理函数(被确保在该sta老化超时前执行)
static void ap_sta_disassoc_cb_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct hostapd_data *hapd = eloop_ctx;
	struct sta_info *sta = timeout_ctx;

    // 删除disassoc超时的sta
	ap_sta_remove(hapd, sta);
	mlme_disassociate_indication(hapd, sta, sta->disassoc_reason);
}

// 取消当前bss跟指定sta的联系
void ap_sta_disassociate(struct hostapd_data *hapd, struct sta_info *sta,
			 u16 reason)
{
	wpa_printf(MSG_DEBUG, "%s: disassociate STA " MACSTR,
		   hapd->conf->iface, MAC2STR(sta->addr));
	sta->flags &= ~WLAN_STA_ASSOC;
	ap_sta_set_authorized(hapd, sta, 0);
    // 注册sta的老化定时器（老化原因STA_DEAUTH）
	sta->timeout_next = STA_DEAUTH;
	eloop_cancel_timeout(ap_handle_timer, hapd, sta);
	eloop_register_timeout(AP_MAX_INACTIVITY_AFTER_DISASSOC, 0,
			       ap_handle_timer, hapd, sta);
    // 停止计费
	accounting_sta_stop(hapd, sta);
    // 释放sta上跟802.1x协议相关的数据块
	ieee802_1x_free_station(sta);

	sta->disassoc_reason = reason;
	sta->flags |= WLAN_STA_PENDING_DISASSOC_CB;
    // 注册sta的disassoc定时器
	eloop_cancel_timeout(ap_sta_disassoc_cb_timeout, hapd, sta);
	eloop_register_timeout(hapd->iface->drv_flags &
			       WPA_DRIVER_FLAGS_DEAUTH_TX_STATUS ? 2 : 0, 0,
			       ap_sta_disassoc_cb_timeout, hapd, sta);
}

// sta的deauth超时处理函数(被确保在该sta老化超时前执行)
static void ap_sta_deauth_cb_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct hostapd_data *hapd = eloop_ctx;
	struct sta_info *sta = timeout_ctx;

    // 删除deauth超时的sta
	ap_sta_remove(hapd, sta);
	mlme_deauthenticate_indication(hapd, sta, sta->deauth_reason);
}

// 注销当前bss上指定sta的认证通过状态
void ap_sta_deauthenticate(struct hostapd_data *hapd, struct sta_info *sta,
			   u16 reason)
{
	wpa_printf(MSG_DEBUG, "%s: deauthenticate STA " MACSTR,
		   hapd->conf->iface, MAC2STR(sta->addr));
	sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
	ap_sta_set_authorized(hapd, sta, 0);
	sta->timeout_next = STA_REMOVE;
    // 注册sta的老化定时器（老化原因STA_REMOVE）
	eloop_cancel_timeout(ap_handle_timer, hapd, sta);
	eloop_register_timeout(AP_MAX_INACTIVITY_AFTER_DEAUTH, 0,
			       ap_handle_timer, hapd, sta);
	accounting_sta_stop(hapd, sta);
	ieee802_1x_free_station(sta);

	sta->deauth_reason = reason;
	sta->flags |= WLAN_STA_PENDING_DEAUTH_CB;
    // 注册sta的deauth定时器
	eloop_cancel_timeout(ap_sta_deauth_cb_timeout, hapd, sta);
	eloop_register_timeout(hapd->iface->drv_flags &
			       WPA_DRIVER_FLAGS_DEAUTH_TX_STATUS ? 2 : 0, 0,
			       ap_sta_deauth_cb_timeout, hapd, sta);
}

// 为该sta绑定一个新的vlan
int ap_sta_bind_vlan(struct hostapd_data *hapd, struct sta_info *sta,
		     int old_vlanid)
{
#ifndef CONFIG_NO_VLAN
	const char *iface;
	struct hostapd_vlan *vlan = NULL;
	int ret;

	/*
	 * Do not proceed furthur if the vlan id remains same. We do not want
	 * duplicate dynamic vlan entries.
     * 如果新老vlan id相同，直接退出，省得重复操作
	 */
	if (sta->vlan_id == old_vlanid)
		return 0;

	/*
	 * During 1x reauth, if the vlan id changes, then remove the old id and
	 * proceed furthur to add the new one.
     * 动态删除当前bss中记录的old_vlanid
	 */
	if (old_vlanid > 0)
		vlan_remove_dynamic(hapd, old_vlanid);

	iface = hapd->conf->iface;
	if (sta->ssid->vlan[0])
		iface = sta->ssid->vlan;

	if (sta->ssid->dynamic_vlan == DYNAMIC_VLAN_DISABLED)
		sta->vlan_id = 0;
	else if (sta->vlan_id > 0) {
        // 如果新的vlan_id有效，则遍历该bss的vlan链表，查找vlan id相同或者通配符类型的节点
		vlan = hapd->conf->vlan;
		while (vlan) {
			if (vlan->vlan_id == sta->vlan_id ||
			    vlan->vlan_id == VLAN_ID_WILDCARD) {
				iface = vlan->ifname;
				break;
			}
			vlan = vlan->next;
		}
	}

	if (sta->vlan_id > 0 && vlan == NULL) {
        // 如果新的vlan_id有效，但是没有找到匹配的vlan节点，则返回失败
		hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_DEBUG, "could not find VLAN for "
			       "binding station to (vlan_id=%d)",
			       sta->vlan_id);
		return -1;
	} else if (sta->vlan_id > 0 && vlan->vlan_id == VLAN_ID_WILDCARD) {
        // 如果新的vlan_id有效，并且匹配到的vlan节点是通配符类型
        // 在指定bss中动态添加指定的vlan
		vlan = vlan_add_dynamic(hapd, vlan, sta->vlan_id);
		if (vlan == NULL) {
			hostapd_logger(hapd, sta->addr,
				       HOSTAPD_MODULE_IEEE80211,
				       HOSTAPD_LEVEL_DEBUG, "could not add "
				       "dynamic VLAN interface for vlan_id=%d",
				       sta->vlan_id);
			return -1;
		}

		iface = vlan->ifname;
		if (vlan_setup_encryption_dyn(hapd, sta->ssid, iface) != 0) {
			hostapd_logger(hapd, sta->addr,
				       HOSTAPD_MODULE_IEEE80211,
				       HOSTAPD_LEVEL_DEBUG, "could not "
				       "configure encryption for dynamic VLAN "
				       "interface for vlan_id=%d",
				       sta->vlan_id);
		}

		hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_DEBUG, "added new dynamic VLAN "
			       "interface '%s'", iface);
	} else if (vlan && vlan->vlan_id == sta->vlan_id) {
        // 如果匹配到的vlan节点和新的vlan有相同的vlan id
		if (sta->vlan_id > 0) {
            // 如果新的vlan又是有效的，意味着有多个sta加入到同一个vlan中
			vlan->dynamic_vlan++;
			hostapd_logger(hapd, sta->addr,
				       HOSTAPD_MODULE_IEEE80211,
				       HOSTAPD_LEVEL_DEBUG, "updated existing "
				       "dynamic VLAN interface '%s'", iface);
		}

		/*
		 * Update encryption configuration for statically generated
		 * VLAN interface. This is only used for static WEP
		 * configuration for the case where hostapd did not yet know
		 * which keys are to be used when the interface was added.
		 */
		if (vlan_setup_encryption_dyn(hapd, sta->ssid, iface) != 0) {
			hostapd_logger(hapd, sta->addr,
				       HOSTAPD_MODULE_IEEE80211,
				       HOSTAPD_LEVEL_DEBUG, "could not "
				       "configure encryption for VLAN "
				       "interface for vlan_id=%d",
				       sta->vlan_id);
		}
	}

	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
		       HOSTAPD_LEVEL_DEBUG, "binding station to interface "
		       "'%s'", iface);

	if (wpa_auth_sta_set_vlan(sta->wpa_sm, sta->vlan_id) < 0)
		wpa_printf(MSG_INFO, "Failed to update VLAN-ID for WPA");

    // 将该sta绑定到对应接口的对应vlan上
	ret = hostapd_drv_set_sta_vlan(iface, hapd, sta->addr, sta->vlan_id);
	if (ret < 0) {
		hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_DEBUG, "could not bind the STA "
			       "entry to vlan_id=%d", sta->vlan_id);
	}
	return ret;
#else /* CONFIG_NO_VLAN */
	return 0;
#endif /* CONFIG_NO_VLAN */
}


#ifdef CONFIG_IEEE80211W

int ap_check_sa_query_timeout(struct hostapd_data *hapd, struct sta_info *sta)
{
	u32 tu;
	struct os_time now, passed;
	os_get_time(&now);
	os_time_sub(&now, &sta->sa_query_start, &passed);
	tu = (passed.sec * 1000000 + passed.usec) / 1024;
	if (hapd->conf->assoc_sa_query_max_timeout < tu) {
		hostapd_logger(hapd, sta->addr,
			       HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_DEBUG,
			       "association SA Query timed out");
		sta->sa_query_timed_out = 1;
		os_free(sta->sa_query_trans_id);
		sta->sa_query_trans_id = NULL;
		sta->sa_query_count = 0;
		eloop_cancel_timeout(ap_sa_query_timer, hapd, sta);
		return 1;
	}

	return 0;
}


static void ap_sa_query_timer(void *eloop_ctx, void *timeout_ctx)
{
	struct hostapd_data *hapd = eloop_ctx;
	struct sta_info *sta = timeout_ctx;
	unsigned int timeout, sec, usec;
	u8 *trans_id, *nbuf;

	if (sta->sa_query_count > 0 &&
	    ap_check_sa_query_timeout(hapd, sta))
		return;

	nbuf = os_realloc(sta->sa_query_trans_id,
			  (sta->sa_query_count + 1) * WLAN_SA_QUERY_TR_ID_LEN);
	if (nbuf == NULL)
		return;
	if (sta->sa_query_count == 0) {
		/* Starting a new SA Query procedure */
		os_get_time(&sta->sa_query_start);
	}
	trans_id = nbuf + sta->sa_query_count * WLAN_SA_QUERY_TR_ID_LEN;
	sta->sa_query_trans_id = nbuf;
	sta->sa_query_count++;

	os_get_random(trans_id, WLAN_SA_QUERY_TR_ID_LEN);

	timeout = hapd->conf->assoc_sa_query_retry_timeout;
	sec = ((timeout / 1000) * 1024) / 1000;
	usec = (timeout % 1000) * 1024;
	eloop_register_timeout(sec, usec, ap_sa_query_timer, hapd, sta);

	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
		       HOSTAPD_LEVEL_DEBUG,
		       "association SA Query attempt %d", sta->sa_query_count);

#ifdef NEED_AP_MLME
	ieee802_11_send_sa_query_req(hapd, sta->addr, trans_id);
#endif /* NEED_AP_MLME */
}


void ap_sta_start_sa_query(struct hostapd_data *hapd, struct sta_info *sta)
{
	ap_sa_query_timer(hapd, sta);
}


void ap_sta_stop_sa_query(struct hostapd_data *hapd, struct sta_info *sta)
{
	eloop_cancel_timeout(ap_sa_query_timer, hapd, sta);
	os_free(sta->sa_query_trans_id);
	sta->sa_query_trans_id = NULL;
	sta->sa_query_count = 0;
}

#endif /* CONFIG_IEEE80211W */

// 执行对sta的授权/非授权操作（实际该函数不做任何事）
void ap_sta_set_authorized(struct hostapd_data *hapd, struct sta_info *sta,
			   int authorized)
{
	const u8 *dev_addr = NULL;
	if (!!authorized == !!(sta->flags & WLAN_STA_AUTHORIZED))
		return;

#ifdef CONFIG_P2P
	dev_addr = p2p_group_get_dev_addr(hapd->p2p_group, sta->addr);
#endif /* CONFIG_P2P */

	if (authorized) {
		if (dev_addr)
			wpa_msg(hapd->msg_ctx, MSG_INFO, AP_STA_CONNECTED
				MACSTR " p2p_dev_addr=" MACSTR,
				MAC2STR(sta->addr), MAC2STR(dev_addr));
		else
			wpa_msg(hapd->msg_ctx, MSG_INFO, AP_STA_CONNECTED
				MACSTR, MAC2STR(sta->addr));
		if (hapd->msg_ctx_parent &&
		    hapd->msg_ctx_parent != hapd->msg_ctx && dev_addr)
			wpa_msg(hapd->msg_ctx_parent, MSG_INFO,
				AP_STA_CONNECTED MACSTR " p2p_dev_addr="
				MACSTR,
				MAC2STR(sta->addr), MAC2STR(dev_addr));
		else if (hapd->msg_ctx_parent &&
			 hapd->msg_ctx_parent != hapd->msg_ctx)
			wpa_msg(hapd->msg_ctx_parent, MSG_INFO,
				AP_STA_CONNECTED MACSTR, MAC2STR(sta->addr));

		sta->flags |= WLAN_STA_AUTHORIZED;
	} else {
		if (dev_addr)
			wpa_msg(hapd->msg_ctx, MSG_INFO, AP_STA_DISCONNECTED
				MACSTR " p2p_dev_addr=" MACSTR,
				MAC2STR(sta->addr), MAC2STR(dev_addr));
		else
			wpa_msg(hapd->msg_ctx, MSG_INFO, AP_STA_DISCONNECTED
				MACSTR, MAC2STR(sta->addr));
		if (hapd->msg_ctx_parent &&
		    hapd->msg_ctx_parent != hapd->msg_ctx && dev_addr)
			wpa_msg(hapd->msg_ctx_parent, MSG_INFO,
				AP_STA_DISCONNECTED MACSTR " p2p_dev_addr="
				MACSTR, MAC2STR(sta->addr), MAC2STR(dev_addr));
		else if (hapd->msg_ctx_parent &&
			 hapd->msg_ctx_parent != hapd->msg_ctx)
			wpa_msg(hapd->msg_ctx_parent, MSG_INFO,
				AP_STA_DISCONNECTED MACSTR,
				MAC2STR(sta->addr));
		sta->flags &= ~WLAN_STA_AUTHORIZED;
	}

	if (hapd->sta_authorized_cb)
		hapd->sta_authorized_cb(hapd->sta_authorized_cb_ctx,
					sta->addr, authorized, dev_addr);
}

// 注销当前bss上添加的指定sta（异步过程）
void ap_sta_disconnect(struct hostapd_data *hapd, struct sta_info *sta,
		       const u8 *addr, u16 reason)
{

	if (sta == NULL && addr)
		sta = ap_get_sta(hapd, addr);

    // 调用驱动层的deauth操作
	if (addr)
		hostapd_drv_sta_deauth(hapd, addr, reason);

	if (sta == NULL)
		return;
	ap_sta_set_authorized(hapd, sta, 0);
    // wpa状态机中设置WPA_DEAUTH，略过
	wpa_auth_sm_event(sta->wpa_sm, WPA_DEAUTH);
    // 清除eap<->eapol交互接口中的portEnabled标志，用于通知状态机不再激活
	ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
	sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
    // 注册sta老化定时器(设置老化原因STA_REMOVE)
	eloop_cancel_timeout(ap_handle_timer, hapd, sta);
	eloop_register_timeout(AP_MAX_INACTIVITY_AFTER_DEAUTH, 0,
			       ap_handle_timer, hapd, sta);
	sta->timeout_next = STA_REMOVE;

	sta->deauth_reason = reason;
	sta->flags |= WLAN_STA_PENDING_DEAUTH_CB;
    // 注册sta的deauth定时器
	eloop_cancel_timeout(ap_sta_deauth_cb_timeout, hapd, sta);
	eloop_register_timeout(hapd->iface->drv_flags &
			       WPA_DRIVER_FLAGS_DEAUTH_TX_STATUS ? 2 : 0, 0,
			       ap_sta_deauth_cb_timeout, hapd, sta);
}


void ap_sta_deauth_cb(struct hostapd_data *hapd, struct sta_info *sta)
{
	if (!(sta->flags & WLAN_STA_PENDING_DEAUTH_CB)) {
		wpa_printf(MSG_DEBUG, "Ignore deauth cb for test frame");
		return;
	}
	sta->flags &= ~WLAN_STA_PENDING_DEAUTH_CB;
	eloop_cancel_timeout(ap_sta_deauth_cb_timeout, hapd, sta);
	ap_sta_deauth_cb_timeout(hapd, sta);
}


void ap_sta_disassoc_cb(struct hostapd_data *hapd, struct sta_info *sta)
{
	if (!(sta->flags & WLAN_STA_PENDING_DISASSOC_CB)) {
		wpa_printf(MSG_DEBUG, "Ignore disassoc cb for test frame");
		return;
	}
	sta->flags &= ~WLAN_STA_PENDING_DISASSOC_CB;
	eloop_cancel_timeout(ap_sta_disassoc_cb_timeout, hapd, sta);
	ap_sta_disassoc_cb_timeout(hapd, sta);
}
