/*
 * hostapd / EAP Full Authenticator state machine (RFC 4137)
 * Copyright (c) 2004-2007, Jouni Malinen <j@w1.fi>
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

#ifndef EAP_H
#define EAP_H

#include "common/defs.h"
#include "eap_common/eap_defs.h"
#include "eap_server/eap_methods.h"
#include "wpabuf.h"

struct eap_sm;

#define EAP_MAX_METHODS 8

#define EAP_TTLS_AUTH_PAP 1
#define EAP_TTLS_AUTH_CHAP 2
#define EAP_TTLS_AUTH_MSCHAP 4
#define EAP_TTLS_AUTH_MSCHAPV2 8

// eap层记录的用户信息管理块
struct eap_user {
	struct {
		int vendor;
		u32 method;
	} methods[EAP_MAX_METHODS];
	u8 *password;
	size_t password_len;
	int password_hash; /* whether password is hashed with
			    * nt_password_hash() */
	int phase2;
	int force_version;
	int ttls_auth; /* bitfield of
			* EAP_TTLS_AUTH_{PAP,CHAP,MSCHAP,MSCHAPV2} */
};

// 认证者eap<-->eapol交互的接口
struct eap_eapol_interface {
	/* Lower layer to full authenticator variables */
    // 以下变量用于EAPOL层->EAP层交互
	Boolean eapResp;    // EAPOL层 BE_AUTH SM 进入RESPONSE状态时设置TRUE，EAP层AUTH SM进入SEND_REQUEST/DISCARD状态时设置FALSE
	struct wpabuf *eapRespData; // 此buffer由EAPOL层填充，同时设置eapResp，然后递交EAP层处理
	Boolean portEnabled;    // 此标志同时影响EAPOL层SM和EAP层SM，意味着该port是否可操作，开始802.1X认证时被设置TRUE，认证失败时设置FALSE(或者删除接口时)
	int retransWhile;       // EAP层发送的REQUEST*报文重传定时器，由EAPOL层PORT_TIMER SM触发超时，由EAP进入IDLE/IDLE2时层开启 (并且只会在这两种状态下有用)
	Boolean eapRestart;     // EAPOL层 AUTH_PAE SM进入RESTART状态时设置TRUE，EAP层AUTH SM进入INITIALIZE状态时设置FALSE
	int eapSRTT;
	int eapRTTVAR;

	/* Full authenticator to lower layer variables */
    // 以下变量用于EAP层->EAPOL层交互
	Boolean eapReq;     // EAP层AUTH SM进入RETRANSMIT/REQUEST状态时设置TRUE并填充eapReqData，EAPOL层BE_AUTH SM进入REQUEST状态时设置FALSE
	Boolean eapNoReq;   // EAP层AUTH SM在没有eap-resp需要发送给请求者时设置TRUE，EAPOL层BE_AUTH SM进入INITIALIZE/RESPONCE/IGNORE状态时设置FALSE
	Boolean eapSuccess; // 此标志由EAP层管理，EAP层AUTH SM进入SUCCESS状态时设置TRUE并填充eapReqData，进入INITIALIZE状态时设置FALSE 
	Boolean eapFail;    // 此标志由EAP层管理，EAP层AUTH SM进入FAIL状态时设置TRUE并填充eapReqData，进入INITIALIZE状态时设置FALSE
	Boolean eapTimeout; // 此标志由EAP层管理，EAP层AUTH SM进入TIMEOUT状态时设置TRUE，进入INITIALIZE状态时设置FALSE
	struct wpabuf *eapReqData;  // 此buffer由EAP层填充，同时设置eapReq/eapSuccess/eapFail，然后递交给EAPOL层处理 
	u8 *eapKeyData;             // 此标志由EAP层管理，存储了KEY数据（802.1X不用）
	size_t eapKeyDataLen;       // 此标志由EAP层管理，存储了KEY数据长度（802.1X不用）
	Boolean eapKeyAvailable; /* called keyAvailable in IEEE 802.1X-2004 */

	/* AAA interface to full authenticator variables */
    // 以下变量用于AAA层->EAP层交互
	Boolean aaaEapReq;      // AAA层收到一个类型为Access-Challenge的radius报文时设置TRUE，EAP层根据此标志判断是否进入AAA_RESPONSE状态
	Boolean aaaEapNoReq;    // AAA层从收到的radius报文中提取EAP数据失败时设置TRUE，EAP层根据此标志判断是否进入DISCARD2状态
	Boolean aaaSuccess;     // AAA层收到一个类型为Access-Accept的radius报文时设置TRUE，EAP层根据此标志判断是否进入SUCCESS2状态
	Boolean aaaFail;        // AAA层收到一个类型为Access-Reject的radius报文时设置TRUE，EAP层根据此标志判断是否进入FAILURE2状态
	struct wpabuf *aaaEapReqData;   // 此buffer由AAA层填充，同时设置aaaEapReq/aaaSuccess/aaaFail，然后递交给EAP层处理
	u8 *aaaEapKeyData;
	size_t aaaEapKeyDataLen;
	Boolean aaaEapKeyAvailable;
	int aaaMethodTimeout;   // EAP报文的重传超时值，这个值由radius服务器提供，却用于NAS->suppliant之间

	/* Full authenticator to AAA interface variables */
    // 以下变量用于EAP层->AAA层交互
	Boolean aaaEapResp;             // EAP层AUTH SM进入AAA_IDLE时设置TRUE并填充aaaEapRespData，AAA层发送数据并设置FALSE
	struct wpabuf *aaaEapRespData;  // 此标志有EAP层填充(数据来源通常就是eapReapData转储)，同时设置aaaEapResp，然后递交给AAA层处理
	/* aaaIdentity -> eap_get_identity() */
	Boolean aaaTimeout;
};

// eap层需要用到的eapol层回调函数
struct eapol_callbacks {
	int (*get_eap_user)(void *ctx, const u8 *identity, size_t identity_len,
			    int phase2, struct eap_user *user);
	const char * (*get_eap_req_id_text)(void *ctx, size_t *len);
};

struct eap_config {
	void *ssl_ctx;
	void *msg_ctx;
	void *eap_sim_db_priv;
	Boolean backend_auth;
	int eap_server;
	u16 pwd_group;
	u8 *pac_opaque_encr_key;
	u8 *eap_fast_a_id;
	size_t eap_fast_a_id_len;
	char *eap_fast_a_id_info;
	int eap_fast_prov;
	int pac_key_lifetime;
	int pac_key_refresh_time;
	int eap_sim_aka_result_ind;
	int tnc;
	struct wps_context *wps;
	const struct wpabuf *assoc_wps_ie;
	const struct wpabuf *assoc_p2p_ie;
	const u8 *peer_addr;
	int fragment_size;

	int pbc_in_m1;
};


struct eap_sm * eap_server_sm_init(void *eapol_ctx,
				   struct eapol_callbacks *eapol_cb,
				   struct eap_config *eap_conf);
void eap_server_sm_deinit(struct eap_sm *sm);
int eap_server_sm_step(struct eap_sm *sm);
void eap_sm_notify_cached(struct eap_sm *sm);
void eap_sm_pending_cb(struct eap_sm *sm);
int eap_sm_method_pending(struct eap_sm *sm);
const u8 * eap_get_identity(struct eap_sm *sm, size_t *len);
struct eap_eapol_interface * eap_get_interface(struct eap_sm *sm);
void eap_server_clear_identity(struct eap_sm *sm);

#endif /* EAP_H */
