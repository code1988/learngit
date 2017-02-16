/*
 * hostapd / Initialization and configuration
 * Copyright (c) 2002-2009, Jouni Malinen <j@w1.fi>
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

#ifndef HOSTAPD_H
#define HOSTAPD_H

#include "common/defs.h"

struct wpa_driver_ops;
struct wpa_ctrl_dst;
struct radius_server_data;
struct upnp_wps_device_sm;
struct hapd_interfaces;
struct hostapd_data;
struct sta_info;
struct hostap_sta_driver_data;
struct ieee80211_ht_capabilities;
struct full_dynamic_vlan;

struct hostapd_probereq_cb {
	int (*cb)(void *ctx, const u8 *sa, const u8 *ie, size_t ie_len);
	void *ctx;
};

#define HOSTAPD_RATE_BASIC 0x00000001

struct hostapd_rate_data {
	int rate; /* rate in 100 kbps */
	int flags; /* HOSTAPD_RATE_ flags */
};

struct hostapd_frame_info {
	u32 channel;
	u32 datarate;
	u32 ssi_signal;
};

// hostapd模式下才使用到的驱动程序API集合
struct hostapd_driver_ops {
	int (*set_ap_wps_ie)(struct hostapd_data *hapd);                        // hostapd_set_ap_wps_ie
	int (*send_mgmt_frame)(struct hostapd_data *hapd, const void *msg,      
			       size_t len);     // hostapd_send_mgmt_frame
	int (*send_eapol)(struct hostapd_data *hapd, const u8 *addr,
			  const u8 *data, size_t data_len, int encrypt);                // hostapd_send_eapol
	int (*set_authorized)(struct hostapd_data *hapd, struct sta_info *sta,
			      int authorized);  // hostapd_set_authorized
	int (*set_key)(const char *ifname, struct hostapd_data *hapd,
		       enum wpa_alg alg, const u8 *addr, int key_idx,
		       int set_tx, const u8 *seq, size_t seq_len,
		       const u8 *key, size_t key_len);                              // hostapd_set_key
	int (*read_sta_data)(struct hostapd_data *hapd,
			     struct hostap_sta_driver_data *data,
			     const u8 *addr);   // hostapd_read_sta_data
	int (*sta_clear_stats)(struct hostapd_data *hapd, const u8 *addr);      // hostapd_sta_clear_stats
	int (*set_sta_flags)(struct hostapd_data *hapd, struct sta_info *sta);  // hostapd_set_sta_flags
	int (*set_drv_ieee8021x)(struct hostapd_data *hapd, const char *ifname, 
				 int enabled);      // hostapd_set_drv_ieee8021x
	int (*set_radius_acl_auth)(struct hostapd_data *hapd,
				   const u8 *mac, int accepted,
				   u32 session_timeout);                                    // hostapd_set_radius_acl_auth
	int (*set_radius_acl_expire)(struct hostapd_data *hapd,
				     const u8 *mac);                                        // hostapd_set_radius_acl_expire
	int (*set_bss_params)(struct hostapd_data *hapd, int use_protection);   // hostapd_set_bss_params 
	int (*set_beacon)(struct hostapd_data *hapd,
			  const u8 *head, size_t head_len,
			  const u8 *tail, size_t tail_len, int dtim_period,
			  int beacon_int);      // hostapd_set_beacon
	int (*vlan_if_add)(struct hostapd_data *hapd, const char *ifname);      // hostapd_vlan_if_add
	int (*vlan_if_remove)(struct hostapd_data *hapd, const char *ifname);   // hostapd_vlan_if_remove
	int (*set_wds_sta)(struct hostapd_data *hapd, const u8 *addr, int aid,  
			   int val);            // hostapd_set_wds_sta
	int (*set_sta_vlan)(const char *ifname, struct hostapd_data *hapd,
			    const u8 *addr, int vlan_id);                               // hostapd_set_sta_vlan
	int (*get_inact_sec)(struct hostapd_data *hapd, const u8 *addr);        // hostapd_get_inact_sec
	int (*sta_deauth)(struct hostapd_data *hapd, const u8 *addr,
			  int reason);          // hostapd_sta_deauth
	int (*sta_disassoc)(struct hostapd_data *hapd, const u8 *addr,
			    int reason);        // hostapd_sta_disassoc
	int (*sta_add)(struct hostapd_data *hapd,
		       const u8 *addr, u16 aid, u16 capability,
		       const u8 *supp_rates, size_t supp_rates_len,
		       u16 listen_interval,
		       const struct ieee80211_ht_capabilities *ht_capab);           // hostapd_sta_add
	int (*sta_remove)(struct hostapd_data *hapd, const u8 *addr);           // hostapd_sta_remove
	int (*set_countermeasures)(struct hostapd_data *hapd, int enabled);     // hostapd_set_countermeasures
};

/**
 * struct hostapd_data - hostapd per-BSS data structure
 * 描述了每个bss
 */
struct hostapd_data {
	struct hostapd_iface *iface;
	struct hostapd_config *iconf;
	struct hostapd_bss_config *conf;    // 保存此bss的配置信息
	int interface_added; /* virtual interface added for this BSS */

	u8 own_addr[ETH_ALEN];              // 保存此bss的BSSID（估计就是mac）

	int num_sta; /* number of entries in sta_list 链表形式站表中入口数量*/
	struct sta_info *sta_list; /* STA info list head 站表链表头*/
#define STA_HASH_SIZE 256
#define STA_HASH(sta) (sta[5])
	struct sta_info *sta_hash[STA_HASH_SIZE];   // 拉链式hash表，记录了所有站表入口

	/*
	 * Bitfield for indicating which AIDs are allocated. Only AID values
	 * 1-2007 are used and as such, the bit at index 0 corresponds to AID
	 * 1.
	 */
#define AID_WORDS ((2008 + 31) / 32)
	u32 sta_aid[AID_WORDS];

	const struct wpa_driver_ops *driver;    // 指向此bss所使用的驱动程序API接口集合
	void *drv_priv;                         // 指向此bss所使用的驱动程序控制块
	struct hostapd_driver_ops drv;          // hostapd模式才会使用到的驱动程序API集合

	void (*new_assoc_sta_cb)(struct hostapd_data *hapd,
				 struct sta_info *sta, int reassoc);    // 指向一个通知函数，用于通知有新的站表元素关联到AP

	void *msg_ctx; /* ctx for wpa_msg() calls */

	struct radius_client_data *radius;      // radius客户端控制块
	u32 acct_session_id_hi, acct_session_id_lo;

	struct iapp_data *iapp;

	struct hostapd_cached_radius_acl *acl_cache;
	struct hostapd_acl_query_data *acl_queries;

	struct wpa_authenticator *wpa_auth;
	struct eapol_authenticator *eapol_auth;

	struct rsn_preauth_interface *preauth_iface;
	time_t michael_mic_failure;
	int michael_mic_failures;
	int tkip_countermeasures;

	int ctrl_sock;
	struct wpa_ctrl_dst *ctrl_dst;

	void *ssl_ctx;
	void *eap_sim_db_priv;
	struct radius_server_data *radius_srv;

	int parameter_set_count;

#ifdef CONFIG_FULL_DYNAMIC_VLAN
	struct full_dynamic_vlan *full_dynamic_vlan;
#endif /* CONFIG_FULL_DYNAMIC_VLAN */

	struct l2_packet_data *l2;
	struct wps_context *wps;

	struct wpabuf *wps_beacon_ie;
	struct wpabuf *wps_probe_resp_ie;
#ifdef CONFIG_WPS
	unsigned int ap_pin_failures;
	struct upnp_wps_device_sm *wps_upnp;
	unsigned int ap_pin_lockout_time;
#endif /* CONFIG_WPS */

	struct hostapd_probereq_cb *probereq_cb;
	size_t num_probereq_cb;

	void (*public_action_cb)(void *ctx, const u8 *buf, size_t len,
				 int freq);
	void *public_action_cb_ctx;

	void (*wps_reg_success_cb)(void *ctx, const u8 *mac_addr,
				   const u8 *uuid_e);
	void *wps_reg_success_cb_ctx;
};


/**
 * struct hostapd_iface - hostapd per-interface data structure
 * 每个物理接口的控制块
 */
struct hostapd_iface {
	struct hapd_interfaces *interfaces; // 指向父结构，也就是接口集合的指针
	void *owner;
	int (*reload_config)(struct hostapd_iface *iface);
	struct hostapd_config * (*config_read_cb)(const char *config_fname);
	char *config_fname;                 // 保存了命令行输入的配置文件名
	struct hostapd_config *conf;        // 根据配置文件生成的配置控制块

	size_t num_bss;                     // 接口中bss数量，0.7.3版本这个值固定为1
	struct hostapd_data **bss;          // bss列表，描述各bss的配置

	int num_ap; /* number of entries in ap_list */
	struct ap_info *ap_list; /* AP info list head */
	struct ap_info *ap_hash[STA_HASH_SIZE];
	struct ap_info *ap_iter_list;

	struct hostapd_hw_modes *hw_features;
	int num_hw_features;
	struct hostapd_hw_modes *current_mode;
	/* Rates that are currently used (i.e., filtered copy of
	 * current_mode->channels */
	int num_rates;
	struct hostapd_rate_data *current_rates;
	int freq;

	u16 hw_flags;

	/* Number of associated Non-ERP stations (i.e., stations using 802.11b
	 * in 802.11g BSS) */
	int num_sta_non_erp;

	/* Number of associated stations that do not support Short Slot Time */
	int num_sta_no_short_slot_time;

	/* Number of associated stations that do not support Short Preamble */
	int num_sta_no_short_preamble;

	int olbc; /* Overlapping Legacy BSS Condition */

	/* Number of HT associated stations that do not support greenfield */
	int num_sta_ht_no_gf;

	/* Number of associated non-HT stations */
	int num_sta_no_ht;

	/* Number of HT associated stations 20 MHz */
	int num_sta_ht_20mhz;

	/* Overlapping BSS information */
	int olbc_ht;

	u16 ht_op_mode;
	void (*scan_cb)(struct hostapd_iface *iface);

	int (*ctrl_iface_init)(struct hostapd_data *hapd);
	void (*ctrl_iface_deinit)(struct hostapd_data *hapd);

	int (*for_each_interface)(struct hapd_interfaces *interfaces,
				  int (*cb)(struct hostapd_iface *iface,
					    void *ctx), void *ctx);
};

/* hostapd.c */
int hostapd_reload_config(struct hostapd_iface *iface);
struct hostapd_data *
hostapd_alloc_bss_data(struct hostapd_iface *hapd_iface,
		       struct hostapd_config *conf,
		       struct hostapd_bss_config *bss);
int hostapd_setup_interface(struct hostapd_iface *iface);
int hostapd_setup_interface_complete(struct hostapd_iface *iface, int err);
void hostapd_interface_deinit(struct hostapd_iface *iface);
void hostapd_interface_free(struct hostapd_iface *iface);
void hostapd_new_assoc_sta(struct hostapd_data *hapd, struct sta_info *sta,
			   int reassoc);

/* utils.c */
int hostapd_register_probereq_cb(struct hostapd_data *hapd,
				 int (*cb)(void *ctx, const u8 *sa,
					   const u8 *ie, size_t ie_len),
				 void *ctx);
void hostapd_prune_associations(struct hostapd_data *hapd, const u8 *addr);

/* drv_callbacks.c (TODO: move to somewhere else?) */
int hostapd_notif_assoc(struct hostapd_data *hapd, const u8 *addr,
			const u8 *ie, size_t ielen);
void hostapd_notif_disassoc(struct hostapd_data *hapd, const u8 *addr);

#endif /* HOSTAPD_H */