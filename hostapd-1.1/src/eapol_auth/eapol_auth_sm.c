/*
 * IEEE 802.1X-2004 Authenticator - EAPOL state machine
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
 * 这里定义了认证者EAPOL层的状态机:
 *          Authenticator PAE state machine(下面简称AUTH_PAE SM)                - 认证者PAE状态机,负责和PACP相关的算法及处理工作
 *          Backend Authentication state machine(下面简称BE_AUTH SM)            - 后台认证状态机，实现最终认证工作（标准是建议这部分工作放到radius服务器）
 *          Reauthentication Timer state machine(下面简称REAUTH_TIMER SM)       - 重认证定时器状态机
 *          Authenticator Key Transmit state machine(下面简称AUTH_KEY_TX SM)    - 认证者key发送状态机，802.11环境下使用
 *          Key Receive state machine(下面简称KEY_RX SM)                        - 认证者key接收状态机，802.11环境下使用
 *          Controlled Directions state machine(下面简称CTRL_DIR SM)            - 受控方向状态机
 *          Port Timers state machine(下面简称PORT_TIMER SM)
 *
 * 备注： 1. 以上6个状态机定义全部来自IEEE 802.1X-2004 Chapter-8.2
 *        2. 本文件的6个状态机全部用于NAS，也就是说，本文件只会被NAS设备调用
 *        3. IEEE 802.1X-2004标准为NAS还定义了一个状态机Port Timers state machine(本文实现了它的功能，只是并未列为一个独立状态机)
 */

#include "includes.h"

#include "common.h"
#include "eloop.h"
#include "state_machine.h"
#include "common/eapol_common.h"
#include "eap_common/eap_defs.h"
#include "eap_common/eap_common.h"
#include "eap_server/eap.h"
#include "eapol_auth_sm.h"
#include "eapol_auth_sm_i.h"

#define STATE_MACHINE_DATA struct eapol_state_machine   // 状态机的管理块，这里EAPOL层6个状态机共用了这个管理块
#define STATE_MACHINE_DEBUG_PREFIX "IEEE 802.1X"        // 调试时本文件消息输出前缀
#define STATE_MACHINE_ADDR sm->addr                     // 调试时本文件消息输出附加MAC信息

static struct eapol_callbacks eapol_cb;

/* EAPOL state machines are described in IEEE Std 802.1X-2004, Chap. 8.2 */

// 调用驱动层对端口进行授权
#define setPortAuthorized() \
sm->eapol->cb.set_port_authorized(sm->eapol->conf.ctx, sm->sta, 1)
// 调用驱动层设置端口为未授权
#define setPortUnauthorized() \
sm->eapol->cb.set_port_authorized(sm->eapol->conf.ctx, sm->sta, 0)

/* procedures */
#define txCannedFail() eapol_auth_tx_canned_eap(sm, 0)      // 认证失败
#define txCannedSuccess() eapol_auth_tx_canned_eap(sm, 1)   // 认证成功
#define txReq() eapol_auth_tx_req(sm)       // eapol层发送帧的总接口
#define abortAuth() sm->eapol->cb.abort_auth(sm->eapol->conf.ctx, sm->sta)  // 清除认证过的痕迹
#define txKey() sm->eapol->cb.tx_key(sm->eapol->conf.ctx, sm->sta)
#define processKey() do { } while (0)


static void eapol_sm_step_run(struct eapol_state_machine *sm);
static void eapol_sm_step_cb(void *eloop_ctx, void *timeout_ctx);
static void eapol_auth_initialize(struct eapol_state_machine *sm);


static void eapol_auth_logger(struct eapol_authenticator *eapol,
			      const u8 *addr, eapol_logger_level level,
			      const char *txt)
{
	if (eapol->cb.logger == NULL)
		return;
	eapol->cb.logger(eapol->conf.ctx, addr, level, txt);
}


static void eapol_auth_vlogger(struct eapol_authenticator *eapol,
			       const u8 *addr, eapol_logger_level level,
			       const char *fmt, ...)
{
	char *format;
	int maxlen;
	va_list ap;

	if (eapol->cb.logger == NULL)
		return;

	maxlen = os_strlen(fmt) + 100;
	format = os_malloc(maxlen);
	if (!format)
		return;

	va_start(ap, fmt);
	vsnprintf(format, maxlen, fmt, ap);
	va_end(ap);

	eapol_auth_logger(eapol, addr, level, format);

	os_free(format);
}

/* eapol层发送认证成功/失败的总接口
 *
 * 备注：1.1版本中只被强制处于授权/强制处于未授权调用
 *       这里做了部分eap层的工作，即组eap包
 */
static void eapol_auth_tx_canned_eap(struct eapol_state_machine *sm,
				     int success)
{
	struct eap_hdr eap;

	os_memset(&eap, 0, sizeof(eap));

	eap.code = success ? EAP_CODE_SUCCESS : EAP_CODE_FAILURE;
	eap.identifier = ++sm->last_eap_id;
	eap.length = host_to_be16(sizeof(eap));

	eapol_auth_vlogger(sm->eapol, sm->addr, EAPOL_LOGGER_DEBUG,
			   "Sending canned EAP packet %s (identifier %d)",
			   success ? "SUCCESS" : "FAILURE", eap.identifier);
	sm->eapol->cb.eapol_send(sm->eapol->conf.ctx, sm->sta,
				 IEEE802_1X_TYPE_EAP_PACKET,
				 (u8 *) &eap, sizeof(eap));
	sm->dot1xAuthEapolFramesTx++;
}


// eapol层发送请求帧的总接口(当前1.1版本实际在这里混合了认证成功/失败数据)
static void eapol_auth_tx_req(struct eapol_state_machine *sm)
{
    // 检查EAP->EAPOL交互的eapReqData是否有效
	if (sm->eap_if->eapReqData == NULL ||
	    wpabuf_len(sm->eap_if->eapReqData) < sizeof(struct eap_hdr)) {
		eapol_auth_logger(sm->eapol, sm->addr,
				  EAPOL_LOGGER_DEBUG,
				  "TxReq called, but there is no EAP request "
				  "from authentication server");
		return;
	}

    // 检查是否启用了附加功能EAPOL_SM_WAIT_START
	if (sm->flags & EAPOL_SM_WAIT_START) {
		wpa_printf(MSG_DEBUG, "EAPOL: Drop EAPOL TX to " MACSTR
			   " while waiting for EAPOL-Start",
			   MAC2STR(sm->addr));
		return;
	}

	sm->last_eap_id = eap_get_id(sm->eap_if->eapReqData);   // 记录这个EAP报文的ID字段
	eapol_auth_vlogger(sm->eapol, sm->addr, EAPOL_LOGGER_DEBUG,
			   "Sending EAP Packet (identifier %d)",
			   sm->last_eap_id);
    // 发送承载了这个EAP包的EAPOL报文
	sm->eapol->cb.eapol_send(sm->eapol->conf.ctx, sm->sta,
				 IEEE802_1X_TYPE_EAP_PACKET,
				 wpabuf_head(sm->eap_if->eapReqData),
				 wpabuf_len(sm->eap_if->eapReqData));
    // 相应计数器累加
	sm->dot1xAuthEapolFramesTx++;
	if (eap_get_type(sm->eap_if->eapReqData) == EAP_TYPE_IDENTITY)
		sm->dot1xAuthEapolReqIdFramesTx++;
	else
		sm->dot1xAuthEapolReqFramesTx++;
}


/**
 * eapol_port_timers_tick - Port Timers state machine
 * @eloop_ctx: struct eapol_state_machine *
 * @timeout_ctx: Not used
 *
 * This statemachine is implemented as a function that will be called
 * once a second as a registered event loop timeout.
 * 1s进入一次，实现了PORT_TIMER SM
 */
static void eapol_port_timers_tick(void *eloop_ctx, void *timeout_ctx)
{
	struct eapol_state_machine *state = timeout_ctx;

    // aWhile每秒递减，直到超时
	if (state->aWhile > 0) {
		state->aWhile--;
		if (state->aWhile == 0) {
			wpa_printf(MSG_DEBUG, "IEEE 802.1X: " MACSTR
				   " - aWhile --> 0",
				   MAC2STR(state->addr));
		}
	}

    // quietWhile每秒递减，直到超时
	if (state->quietWhile > 0) {
		state->quietWhile--;
		if (state->quietWhile == 0) {
			wpa_printf(MSG_DEBUG, "IEEE 802.1X: " MACSTR
				   " - quietWhile --> 0",
				   MAC2STR(state->addr));
		}
	}

    // reAuthWhen每秒递减，直到超时
	if (state->reAuthWhen > 0) {
		state->reAuthWhen--;
		if (state->reAuthWhen == 0) {
			wpa_printf(MSG_DEBUG, "IEEE 802.1X: " MACSTR
				   " - reAuthWhen --> 0",
				   MAC2STR(state->addr));
		}
	}

    // EAPOL->EAP交互定时器retransWhile每秒递减，直到超时
	if (state->eap_if->retransWhile > 0) {
		state->eap_if->retransWhile--;
		if (state->eap_if->retransWhile == 0) {
			wpa_printf(MSG_DEBUG, "IEEE 802.1X: " MACSTR
				   " - (EAP) retransWhile --> 0",
				   MAC2STR(state->addr));
		}
	}

    // 运行状态机组自平衡流程
	eapol_sm_step_run(state);

    // 循环注册1s定时器
	eloop_register_timeout(1, 0, eapol_port_timers_tick, eloop_ctx, state);
}



/* Authenticator PAE state machine */

/* AUTH_PAE SM进入INITIALIZE状态的EA:
 *      将AUTH_PAE SM私有的端口授权模式初始化为Auto
 */
SM_STATE(AUTH_PAE, INITIALIZE)
{
	SM_ENTRY_MA(AUTH_PAE, INITIALIZE, auth_pae);
	sm->portMode = Auto;
}


/* AUTH_PAE SM进入DISCONNECTED状态的EA:
 *      1. 如果进入此EA的原因是收到了eapol-logoff报文，则根据旧状态累加相应的计数器
 *      2. 设置端口当前为未授权状态
 *      3. 重认证次数清0
 *      4. 清除eapolLogoff标志
 *      5. 如果不是从INITIALIZE进入DISCONNECTED,还需要执行eapol层的802.1X认证失败回调函数
 */
SM_STATE(AUTH_PAE, DISCONNECTED)
{
	int from_initialize = sm->auth_pae_state == AUTH_PAE_INITIALIZE;

	if (sm->eapolLogoff) {
		if (sm->auth_pae_state == AUTH_PAE_CONNECTING)
			sm->authEapLogoffsWhileConnecting++;
		else if (sm->auth_pae_state == AUTH_PAE_AUTHENTICATED)
			sm->authAuthEapLogoffWhileAuthenticated++;
	}

	SM_ENTRY_MA(AUTH_PAE, DISCONNECTED, auth_pae);

	sm->authPortStatus = Unauthorized;
	setPortUnauthorized();
	sm->reAuthCount = 0;
	sm->eapolLogoff = FALSE;
	if (!from_initialize) {
		sm->eapol->cb.finished(sm->eapol->conf.ctx, sm->sta, 0,
				       sm->flags & EAPOL_SM_PREAUTH);
	}
}


/* AUTH_PAE SM进入RESTART状态的EA:
 *      1. 如果进入此EA时的旧状态是AUTHENTICATED，则根据不同的进入原因累加相应的计数器
 *      2. 在和EAP交互接口中设置eapRestart标志位，用于通知EAP层状态机执行初始化操作
 */
SM_STATE(AUTH_PAE, RESTART)
{
	if (sm->auth_pae_state == AUTH_PAE_AUTHENTICATED) {
		if (sm->reAuthenticate)
			sm->authAuthReauthsWhileAuthenticated++;
		if (sm->eapolStart)
			sm->authAuthEapStartsWhileAuthenticated++;
		if (sm->eapolLogoff)
			sm->authAuthEapLogoffWhileAuthenticated++;
	}

	SM_ENTRY_MA(AUTH_PAE, RESTART, auth_pae);

	sm->eap_if->eapRestart = TRUE;
}


/* AUTH_PAE SM进入CONNECTING状态的EA:
 *      1. 如果进入此EA时的旧状态不是AUTHENTICATED，则累加计数器
 *      2. 清除重认证标志
 *      3. 重认证次数进行累加
 */
SM_STATE(AUTH_PAE, CONNECTING)
{
	if (sm->auth_pae_state != AUTH_PAE_CONNECTING)
		sm->authEntersConnecting++;

	SM_ENTRY_MA(AUTH_PAE, CONNECTING, auth_pae);

	sm->reAuthenticate = FALSE;
	sm->reAuthCount++;
}


/* AUTH_PAE SM进入HELD状态的EA:
 *      1. 如果进入此EA时的旧状态是AUTHENTICATING，并且authFail标志置位，则累加计数器
 *      2. 全局的端口当前状态设置为未授权
 *      3. 开启静默定时器，此时间里认证者不会接受任何请求者信息
 *      4. 清除eapolLogoff标志
 *      5. 执行eapol层的802.1X认证失败回调函数
 */
SM_STATE(AUTH_PAE, HELD)
{
	if (sm->auth_pae_state == AUTH_PAE_AUTHENTICATING && sm->authFail)
		sm->authAuthFailWhileAuthenticating++;

	SM_ENTRY_MA(AUTH_PAE, HELD, auth_pae);

	sm->authPortStatus = Unauthorized;
	setPortUnauthorized();
	sm->quietWhile = sm->quietPeriod;
	sm->eapolLogoff = FALSE;

	eapol_auth_vlogger(sm->eapol, sm->addr, EAPOL_LOGGER_WARNING,
			   "authentication failed - EAP type: %d (%s)",
			   sm->eap_type_authsrv,
			   eap_server_get_name(0, sm->eap_type_authsrv));
	if (sm->eap_type_authsrv != sm->eap_type_supp) {
		eapol_auth_vlogger(sm->eapol, sm->addr, EAPOL_LOGGER_INFO,
				   "Supplicant used different EAP type: "
				   "%d (%s)", sm->eap_type_supp,
				   eap_server_get_name(0, sm->eap_type_supp));
	}
	sm->eapol->cb.finished(sm->eapol->conf.ctx, sm->sta, 0,
			       sm->flags & EAPOL_SM_PREAUTH);
}


/* AUTH_PAE SM进入AUTHENTICATED状态的EA:
 *      1. 如果进入此EA时的旧状态是AUTHENTICATING，并且authSuccess标志置位，则累加计数器
 *      2. 全局的端口当前状态设置为已授权
 *      3. 重认证次数清0
 *      4. 执行eapol层的802.1X认证成功回调函数
 */
SM_STATE(AUTH_PAE, AUTHENTICATED)
{
	char *extra = "";

	if (sm->auth_pae_state == AUTH_PAE_AUTHENTICATING && sm->authSuccess)
		sm->authAuthSuccessesWhileAuthenticating++;
							
	SM_ENTRY_MA(AUTH_PAE, AUTHENTICATED, auth_pae);

	sm->authPortStatus = Authorized;
	setPortAuthorized();
	sm->reAuthCount = 0;
	if (sm->flags & EAPOL_SM_PREAUTH)
		extra = " (pre-authentication)";
	else if (sm->flags & EAPOL_SM_FROM_PMKSA_CACHE)
		extra = " (PMKSA cache)";
	eapol_auth_vlogger(sm->eapol, sm->addr, EAPOL_LOGGER_INFO,
			   "authenticated - EAP type: %d (%s)%s",
			   sm->eap_type_authsrv,
			   eap_server_get_name(0, sm->eap_type_authsrv),
			   extra);
	sm->eapol->cb.finished(sm->eapol->conf.ctx, sm->sta, 1,
			       sm->flags & EAPOL_SM_PREAUTH);
}


/* AUTH_PAE SM进入AUTHENTICATING状态的EA:
 *      1. 清除eapolStart标志
 *      2. 清除authSuccess标志
 *      3. 清除authFail标志
 *      4. 清除authTimeout标志
 *      5. authStart标志设置TRUE,用于通知BE_AUTH SM
 *      6. 清除keyRun标志
 *      7. 清除keyDone标志
 */
SM_STATE(AUTH_PAE, AUTHENTICATING)
{
	SM_ENTRY_MA(AUTH_PAE, AUTHENTICATING, auth_pae);

	sm->eapolStart = FALSE;
	sm->authSuccess = FALSE;
	sm->authFail = FALSE;
	sm->authTimeout = FALSE;
	sm->authStart = TRUE;
	sm->keyRun = FALSE;
	sm->keyDone = FALSE;
}


/* AUTH_PAE SM进入ABORTING状态的EA:
 *      1. 如果进入此EA时的旧状态是AUTHENTICATING，则根据不同的进入原因累加相应的计数器
 *      2. authAbort标志设置TRUE，用于通知BE_AUTH SM
 *      3. 清除keyRun标志
 *      4. 清除keyDone标志
 */
SM_STATE(AUTH_PAE, ABORTING)
{
	if (sm->auth_pae_state == AUTH_PAE_AUTHENTICATING) {
		if (sm->authTimeout)
			sm->authAuthTimeoutsWhileAuthenticating++;
		if (sm->eapolStart)
			sm->authAuthEapStartsWhileAuthenticating++;
		if (sm->eapolLogoff)
			sm->authAuthEapLogoffWhileAuthenticating++;
	}

	SM_ENTRY_MA(AUTH_PAE, ABORTING, auth_pae);

	sm->authAbort = TRUE;
	sm->keyRun = FALSE;
	sm->keyDone = FALSE;
}


// AUTH_PAE SM进入FORCE_AUTH状态的EA
SM_STATE(AUTH_PAE, FORCE_AUTH)
{
	SM_ENTRY_MA(AUTH_PAE, FORCE_AUTH, auth_pae);

	sm->authPortStatus = Authorized;
	setPortAuthorized();
	sm->portMode = ForceAuthorized;
	sm->eapolStart = FALSE;
	txCannedSuccess();
}


// AUTH_PAE SM进入FORCE_UNAUTH状态的EA
SM_STATE(AUTH_PAE, FORCE_UNAUTH)
{
	SM_ENTRY_MA(AUTH_PAE, FORCE_UNAUTH, auth_pae);

	sm->authPortStatus = Unauthorized;
	setPortUnauthorized();
	sm->portMode = ForceUnauthorized;
	sm->eapolStart = FALSE;
	txCannedFail();
}


// 检查AUTH_PAE SM的条件变量，然后根据情况进行状态切换
SM_STEP(AUTH_PAE)
{
    // 进入INITIALIZE条件(满足以下任何一条即可)：
    //  1. 受控端口的全局控制模式为Auto，但本状态机私有的端口模式不是Auto
    //  2. 全局的强制初始化标志被置位
    //  3. EAP->EAPOL交互标志portEnabled被清0
	if ((sm->portControl == Auto && sm->portMode != sm->portControl) ||
	    sm->initialize || !sm->eap_if->portEnabled)
		SM_ENTER_GLOBAL(AUTH_PAE, INITIALIZE);
    // 进入FORCE_AUTH条件(必须同时满足以下4条)：
    //  1. 受控端口的全局控制模式为ForceAuthorized
    //  2. 本状态机私有的端口模式不是ForceAuthorized
    //  3. 全局的强制初始化标志没有被置位
    //  4. EAP->EAPOL交互标志portEnabled被置位
	else if (sm->portControl == ForceAuthorized &&
		 sm->portMode != sm->portControl &&
		 !(sm->initialize || !sm->eap_if->portEnabled))
		SM_ENTER_GLOBAL(AUTH_PAE, FORCE_AUTH);
    // 进入FORCE_UNAUTH条件(必须同时满足以下4条)：
    //  1. 受控端口的全局控制模式为ForceUnauthorized
    //  2. 本状态机私有的端口模式不是ForceUnauthorized
    //  3. 全局的强制初始化标志没有被置位
    //  4. EAP->EAPOL交互标志portEnabled被置位
	else if (sm->portControl == ForceUnauthorized &&
		 sm->portMode != sm->portControl &&
		 !(sm->initialize || !sm->eap_if->portEnabled))
		SM_ENTER_GLOBAL(AUTH_PAE, FORCE_UNAUTH);
	else {
		switch (sm->auth_pae_state) {
		case AUTH_PAE_INITIALIZE:               // 当前处于INITIALIZE状态的话，无条件进入DISCONNECTED状态
			SM_ENTER(AUTH_PAE, DISCONNECTED);                                                                                                      
			break;                                                                                                                                 
		case AUTH_PAE_DISCONNECTED:             // 当前处于DISCONNECTED状态的话，无条件进入RESTART状态
			SM_ENTER(AUTH_PAE, RESTART);                                                                                                           
			break;                                                                                                                                 
		case AUTH_PAE_RESTART:                  // 当前处于RESTART状态的话，只有等待EAP层执行完初始化清除了eapRestart标志，才会进入CONNECTING状态
			if (!sm->eap_if->eapRestart)                                                                                                           
				SM_ENTER(AUTH_PAE, CONNECTING);                                                                                                    
			break;                                                                                                                                 
		case AUTH_PAE_HELD:                     // 当前处于HELD状态的话，只有等待静默定时器超时，才会进入RESTART状态
			if (sm->quietWhile == 0)
				SM_ENTER(AUTH_PAE, RESTART);
			break;
		case AUTH_PAE_CONNECTING:                                   // 当前处于CONNECTING状态的话，需要分为2种情况进行处理：
			if (sm->eapolLogoff || sm->reAuthCount > sm->reAuthMax) //      1. 如果收到eapol-logoff报文，或者重认证次数超过阈值，则进入DISCONNECTED状态
				SM_ENTER(AUTH_PAE, DISCONNECTED);                   
			else if ((sm->eap_if->eapReq &&                                                                                                              
				  sm->reAuthCount <= sm->reAuthMax) ||                                                                                                   
				 sm->eap_if->eapSuccess || sm->eap_if->eapFail)     //      2.1 如果EAP层设置了eapReq标志，并且重认证次数未超限，或者
				SM_ENTER(AUTH_PAE, AUTHENTICATING);                 //      2.2 如果EAP层设置了eapSuccess或eapFail标志，则进入AUTHENTICATING状态
			break;
		case AUTH_PAE_AUTHENTICATED:                        // 当前处于AUTHENTICATED状态的话，需要分为2种情况进行处理：
			if (sm->eapolStart || sm->reAuthenticate)       //      1.1 如果收到eapol-start报文，或者
				SM_ENTER(AUTH_PAE, RESTART);                //      1.2 如果REAUTH_TIMER SM设置了reAuthenticate标志，则进入RESTART状态
			else if (sm->eapolLogoff || !sm->portValid)     //      2.1 如果收到eapol-logoff报文，或者
				SM_ENTER(AUTH_PAE, DISCONNECTED);           //      2.2 如果 !portValid，则进入DISCONNECTED状态
			break;                                                                                                                                  
		case AUTH_PAE_AUTHENTICATING:                       // 当前处于AUTHENTICATING状态的话，需要分为3种情况进行处理：
			if (sm->authSuccess && sm->portValid)           //      1. 如果BE_AUTH SM设置了authSuccess标志，并且portValid，则进入AUTHENTICATED状态
				SM_ENTER(AUTH_PAE, AUTHENTICATED);                                                                                                  
			else if (sm->authFail ||                                                                                                                
				 (sm->keyDone && !sm->portValid))           //      2.1 如果BE_AUTH SM设置了authFail标志，或者
				SM_ENTER(AUTH_PAE, HELD);                   //      2.2 如果AUTH_KEY_TX SM设置了keyDone标志，并且portValid，则进入HELD状态
			else if (sm->eapolStart || sm->eapolLogoff ||                                                                                           
				 sm->authTimeout)                           //      3.1 如果收到eapol-start或eapol-logoff报文，或者
				SM_ENTER(AUTH_PAE, ABORTING);               //      3.2 如果BE_AUTH SM设置了authTimeout标志，则进入ABORTING状态
			break;                                                                                                                                  
		case AUTH_PAE_ABORTING:                             // 当前处于ABORTING状态的话，需要分为2种情况进行处理：
			if (sm->eapolLogoff && !sm->authAbort)          //      1. 如果收到eapol-logoff报文，并且authAbort标志被清除，则进入DISCONNECTED状态
				SM_ENTER(AUTH_PAE, DISCONNECTED);                                                                                                   
			else if (!sm->eapolLogoff && !sm->authAbort)    //      2. 如果没有收到eapol-logoff报文，并且authAbort标志被清除，则进入RESTART状态
				SM_ENTER(AUTH_PAE, RESTART);
			break;
		case AUTH_PAE_FORCE_AUTH:
			if (sm->eapolStart)
				SM_ENTER(AUTH_PAE, FORCE_AUTH);
			break;
		case AUTH_PAE_FORCE_UNAUTH:
			if (sm->eapolStart)
				SM_ENTER(AUTH_PAE, FORCE_UNAUTH);
			break;
		}
	}
}



/* Backend Authentication state machine */

/* BE_AUTH SM进入INITIALIZE状态的EA:
 *      1. 清除之前可能认证过产生的痕迹
 *      2. 清除EAP->EAPOL交互标志eapNoReq
 *      3. 清除authAbort标志
 */
SM_STATE(BE_AUTH, INITIALIZE)
{
	SM_ENTRY_MA(BE_AUTH, INITIALIZE, be_auth);

	abortAuth();
	sm->eap_if->eapNoReq = FALSE;
	sm->authAbort = FALSE;
}


/* BE_AUTH SM进入REQUEST状态的EA:
 *      1. 发送eap->eapol交互缓存eapReqData中的request消息给请求者
 *      2. 清除EAP->EAPOL交互标志eapReq
 *      3. 相应计数器累加
 *      4. 清除eapolEap标志
 */
SM_STATE(BE_AUTH, REQUEST)
{
	SM_ENTRY_MA(BE_AUTH, REQUEST, be_auth);

	txReq();
	sm->eap_if->eapReq = FALSE;
	sm->backendOtherRequestsToSupplicant++;

	/*
	 * Clearing eapolEap here is not specified in IEEE Std 802.1X-2004, but
	 * it looks like this would be logical thing to do there since the old
	 * EAP response would not be valid anymore after the new EAP request
	 * was sent out.
     * 此处清除eapolEap标志不在标准范围中
	 *
	 * A race condition has been reported, in which hostapd ended up
	 * sending out EAP-Response/Identity as a response to the first
	 * EAP-Request from the main EAP method. This can be avoided by
	 * clearing eapolEap here.
	 */
	sm->eapolEap = FALSE;
}


/* BE_AUTH SM进入RESPONSE状态的EA:
 *      1. 清除authTimeout标志
 *      2. 清除eapolEap标志
 *      3. 清除EAP->EAPOL交互标志eapNoReq
 *      4. 开启aWhile定时器，用于计时eapol层将eapoleap报文递交给eap层，然后等待
 *      5. EAPOL->EAP交互标志eapResp设置TRUE，用于通知EAP层接收一个eap-reap报文
 *      6. 累加计数器
 *
 * 备注：标准要求的sendRespToServer函数并未做实现，因为数据已经承载在EAPOL->EAP交互缓存eapRespData
 */
SM_STATE(BE_AUTH, RESPONSE)
{
	SM_ENTRY_MA(BE_AUTH, RESPONSE, be_auth);

	sm->authTimeout = FALSE;
	sm->eapolEap = FALSE;
	sm->eap_if->eapNoReq = FALSE;
	sm->aWhile = sm->serverTimeout;
	sm->eap_if->eapResp = TRUE;
	/* sendRespToServer(); */
	sm->backendResponses++;
}


/* BE_AUTH SM进入SUCCESS状态的EA:
 *      1. 发送eap->eapol交互缓存eapReqData中的success消息给请求者
 *      2. authSuccess标志设置为TRUE，用于通知AUTH_PAE SM
 */
SM_STATE(BE_AUTH, SUCCESS)
{
	SM_ENTRY_MA(BE_AUTH, SUCCESS, be_auth);

	txReq();
	sm->authSuccess = TRUE;
	sm->keyRun = TRUE;
}


/* BE_AUTH SM进入FAIL状态的EA:
 *      1. 发送eap->eapol交互缓存eapReqData中的fail消息给请求者
 *      2. authFail标志设置为TRUE，用于通知AUTH_PAE SM
 */
SM_STATE(BE_AUTH, FAIL)
{
	SM_ENTRY_MA(BE_AUTH, FAIL, be_auth);

	txReq();
	sm->authFail = TRUE;
}


/* BE_AUTH SM进入TIMEOUT状态的EA:
 *      authTimeout标志设置为TRUE，用于通知AUTH_PAE SM
 * 备注：进入此状态的可能原因有2个，一个是EAPOL层的aWhile定时器超时引起，另一个是EAP-EAPOL交互变量eapTimeout(其实就是retransWhile定时器超时)被置位引起
 */
SM_STATE(BE_AUTH, TIMEOUT)
{
	SM_ENTRY_MA(BE_AUTH, TIMEOUT, be_auth);

	sm->authTimeout = TRUE;
}


/* BE_AUTH SM进入IDLE状态的EA:
 *      清除authStart标志
 */
SM_STATE(BE_AUTH, IDLE)
{
	SM_ENTRY_MA(BE_AUTH, IDLE, be_auth);

	sm->authStart = FALSE;
}


/* BE_AUTH SM进入IGNORE状态的EA:
 *      清除EAP->EAPOL交互标志eapNoReq
 */
SM_STATE(BE_AUTH, IGNORE)
{
	SM_ENTRY_MA(BE_AUTH, IGNORE, be_auth);

	sm->eap_if->eapNoReq = FALSE;
}


// 检查BE_AUTH SM的条件变量，然后根据情况进行状态切换
SM_STEP(BE_AUTH)
{
    // 进入INITIALIZE条件(满足以下任何一条即可)：
    //  1. 受控端口的全局控制模式非Auto
    //  2. 全局的强制初始化标志被置位
    //  3. authAbort标志被置位
	if (sm->portControl != Auto || sm->initialize || sm->authAbort) {
		SM_ENTER_GLOBAL(BE_AUTH, INITIALIZE);
		return;
	}

	switch (sm->be_auth_state) {
	case BE_AUTH_INITIALIZE:        // 当前处于INITIALIZE状态的话，无条件进入IDLE状态
		SM_ENTER(BE_AUTH, IDLE);
		break;
	case BE_AUTH_REQUEST:                       // 当前处于REQUEST状态的话，需要分为3种情况进行处理：
		if (sm->eapolEap)                       //  1. 如果eapolEap标志被置位，则进入RESPONSE状态(这种应该是标准流程)
			SM_ENTER(BE_AUTH, RESPONSE);                                                                                                   
		else if (sm->eap_if->eapReq)            //  2. 如果EAP层设置了eapReq标志，则进入REQUEST状态(这种自循环应该是EAP层重传机制导致的)
			SM_ENTER(BE_AUTH, REQUEST);                                                                                                    
		else if (sm->eap_if->eapTimeout)        //  3. 如果EAP层设置了eapTimeout标志，则进入TIMEOUT状态(这种情况应该是EAP层重传超时导致的)
			SM_ENTER(BE_AUTH, TIMEOUT);                                                                                                    
		break;                                                                                                                             
	case BE_AUTH_RESPONSE:                      // 当前处于RESPONSE状态的话，需要分为5种情况进行处理：
		if (sm->eap_if->eapNoReq)               //  1. 如果EAP层设置了eapNoReq标志，则进入IGNORE状态
			SM_ENTER(BE_AUTH, IGNORE);                                                                                                     
		if (sm->eap_if->eapReq) {               //  2. 如果EAP层设置了eapReq标志，则进入REQUEST状态
			sm->backendAccessChallenges++;                                                                                                 
			SM_ENTER(BE_AUTH, REQUEST);                                                                                                    
		} else if (sm->aWhile == 0)             //  3. 如果aWhile定时器超时，则进入TIMEOUT状态
			SM_ENTER(BE_AUTH, TIMEOUT);                                                                                                    
		else if (sm->eap_if->eapFail) {         //  4. 如果EAP层设置了eapFail标志，则进入FAIL状态，并累加相应计数器
			sm->backendAuthFails++;                                                                                                        
			SM_ENTER(BE_AUTH, FAIL);                                                                                                       
		} else if (sm->eap_if->eapSuccess) {    //  5. 如果EAP层设置了eapSuccess标志，则进入SUCCESS状态，并累加相应计数器
			sm->backendAuthSuccesses++;                                                                                                    
			SM_ENTER(BE_AUTH, SUCCESS);                                                                                                    
		}                                                                                                                                  
		break;                                                                                                                             
	case BE_AUTH_SUCCESS:                       // 当前处于SUCCESS状态的话，无条件进入IDLE状态
		SM_ENTER(BE_AUTH, IDLE);                                                                                                           
		break;                                                                                                                             
	case BE_AUTH_FAIL:                          // 当前处于FAIL状态的话，无条件进入IDLE状态
		SM_ENTER(BE_AUTH, IDLE);                                                                                                           
		break;                                                                                                                             
	case BE_AUTH_TIMEOUT:                       // 当前处于TIMEOUT状态的话，无条件进入IDLE状态
		SM_ENTER(BE_AUTH, IDLE);
		break;
	case BE_AUTH_IDLE:                                     // 当前处于IDLE状态的话，需要分为3种情况进行处理： 
		if (sm->eap_if->eapFail && sm->authStart)          //  1. 如果EAP层设置了eapFail标志，并且AUTH_PAE SM设置了authStart标志，则进入FAIL状态
			SM_ENTER(BE_AUTH, FAIL);                                                                                                                   
		else if (sm->eap_if->eapReq && sm->authStart)      //  2. 如果EAP层设置了eapReq标志，并且AUTH_PAE SM设置了authStart标志，则进入REQUEST状态
			SM_ENTER(BE_AUTH, REQUEST);                                                                                                                
		else if (sm->eap_if->eapSuccess && sm->authStart)  //  3. 如果EAP层设置了eapSuccess标志，并且AUTH_PAE SM设置了authStart标志，则进入SUCCESS状态
			SM_ENTER(BE_AUTH, SUCCESS);                    
		break;
	case BE_AUTH_IGNORE:                    // 当前处于IGNORE状态的话，需要分为3种情况进行处理：
		if (sm->eapolEap)                   //  1. 如果eapolEap标志被置位，则进入RESPONSE状态
			SM_ENTER(BE_AUTH, RESPONSE);                                                            
		else if (sm->eap_if->eapReq)        //  2. 如果EAP层设置了eapReq标志，则进入REQUEST状态
			SM_ENTER(BE_AUTH, REQUEST);                                                             
		else if (sm->eap_if->eapTimeout)    //  3. 如果EAP层设置了eapTimeout标志，则进入TIMEOUT状态
			SM_ENTER(BE_AUTH, TIMEOUT);
		break;
	}
}



/* Reauthentication Timer state machine */

/* REAUTH_TIMER SM进入INITIALIZE状态的EA:
 *      开启/刷新 重认证定时器
 */
SM_STATE(REAUTH_TIMER, INITIALIZE)
{
	SM_ENTRY_MA(REAUTH_TIMER, INITIALIZE, reauth_timer);

	sm->reAuthWhen = sm->reAuthPeriod;
}


/* REAUTH_TIMER SM进入REAUTHENTICATE状态的EA:
 *      1. reAuthenticate标志设置为TRUE，用于通知AUTH_PAE SM
 *      2. 调用了一个eapol层事件回调函数，实际应该没有做任何事
 */
SM_STATE(REAUTH_TIMER, REAUTHENTICATE)
{
	SM_ENTRY_MA(REAUTH_TIMER, REAUTHENTICATE, reauth_timer);

	sm->reAuthenticate = TRUE;
	sm->eapol->cb.eapol_event(sm->eapol->conf.ctx, sm->sta,
				  EAPOL_AUTH_REAUTHENTICATE);
}


// 检查REAUTH_TIMER SM 的条件变量，然后根据情况进行状态切换
SM_STEP(REAUTH_TIMER)
{
    // 进入INITIALIZE条件(满足以下任何一条即可)：
    //  1. 受控端口的全局控制模式非Auto
    //  2. 全局的强制初始化标志被置位
    //  3. 全局的端口当前状态为未授权
    //  4. 重认证功能关闭
	if (sm->portControl != Auto || sm->initialize ||
	    sm->authPortStatus == Unauthorized || !sm->reAuthEnabled) {
		SM_ENTER_GLOBAL(REAUTH_TIMER, INITIALIZE);
		return;
	}

	switch (sm->reauth_timer_state) {
	case REAUTH_TIMER_INITIALIZE:   // 处于INITIALIZE状态的REAUTH_TIMER SM将一直等待，直到重认证定时器超时，进入REAUTHENTICATE状态
		if (sm->reAuthWhen == 0)
			SM_ENTER(REAUTH_TIMER, REAUTHENTICATE);
		break;
	case REAUTH_TIMER_REAUTHENTICATE:   // 当前处于REAUTHENTICATE状态的话，无条件进入INITIALIZE状态
		SM_ENTER(REAUTH_TIMER, INITIALIZE);
		break;
	}
}



/* Authenticator Key Transmit state machine */

/* NO_KEY_TRANSMIT SM进入NO_KEY_TRANSMIT状态的EA:
 *      只进行了状态切换，没做任何事
 */
SM_STATE(AUTH_KEY_TX, NO_KEY_TRANSMIT)
{
	SM_ENTRY_MA(AUTH_KEY_TX, NO_KEY_TRANSMIT, auth_key_tx);
}


SM_STATE(AUTH_KEY_TX, KEY_TRANSMIT)
{
	SM_ENTRY_MA(AUTH_KEY_TX, KEY_TRANSMIT, auth_key_tx);

	txKey();
	sm->eap_if->eapKeyAvailable = FALSE;
	sm->keyDone = TRUE;
}


// 检查AUTH_KEY_TX SM 的条件变量，然后根据情况进行状态切换
SM_STEP(AUTH_KEY_TX)
{
    // 进入NO_KEY_TRANSMIT条件(满足以下任何一条即可)：
    //  1. 全局的强制初始化标志被置位
    //  2. 受控端口的全局控制模式非Auto
	if (sm->initialize || sm->portControl != Auto) {
		SM_ENTER_GLOBAL(AUTH_KEY_TX, NO_KEY_TRANSMIT);
		return;
	}

	switch (sm->auth_key_tx_state) {
	case AUTH_KEY_TX_NO_KEY_TRANSMIT:
		if (sm->keyTxEnabled && sm->eap_if->eapKeyAvailable &&
		    sm->keyRun && !(sm->flags & EAPOL_SM_USES_WPA))
			SM_ENTER(AUTH_KEY_TX, KEY_TRANSMIT);
		break;
	case AUTH_KEY_TX_KEY_TRANSMIT:
		if (!sm->keyTxEnabled || !sm->keyRun)
			SM_ENTER(AUTH_KEY_TX, NO_KEY_TRANSMIT);
		else if (sm->eap_if->eapKeyAvailable)
			SM_ENTER(AUTH_KEY_TX, KEY_TRANSMIT);
		break;
	}
}



/* Key Receive state machine */

// 定义了key接收状态机
SM_STATE(KEY_RX, NO_KEY_RECEIVE)
{
	SM_ENTRY_MA(KEY_RX, NO_KEY_RECEIVE, key_rx);
}


SM_STATE(KEY_RX, KEY_RECEIVE)
{
	SM_ENTRY_MA(KEY_RX, KEY_RECEIVE, key_rx);

	processKey();
	sm->rxKey = FALSE;
}


SM_STEP(KEY_RX)
{
	if (sm->initialize || !sm->eap_if->portEnabled) {
		SM_ENTER_GLOBAL(KEY_RX, NO_KEY_RECEIVE);
		return;
	}

	switch (sm->key_rx_state) {
	case KEY_RX_NO_KEY_RECEIVE:
		if (sm->rxKey)
			SM_ENTER(KEY_RX, KEY_RECEIVE);
		break;
	case KEY_RX_KEY_RECEIVE:
		if (sm->rxKey)
			SM_ENTER(KEY_RX, KEY_RECEIVE);
		break;
	}
}



/* Controlled Directions state machine */

/* CTRL_DIR SM进入FORCE_BOTH状态的EA:
 *
 */
SM_STATE(CTRL_DIR, FORCE_BOTH)
{
	SM_ENTRY_MA(CTRL_DIR, FORCE_BOTH, ctrl_dir);
	sm->operControlledDirections = Both;
}


/* CTRL_DIR SM进入IN_OR_BOTH状态的EA:
 *      设置受控方向为配置值
 */
SM_STATE(CTRL_DIR, IN_OR_BOTH)
{
	SM_ENTRY_MA(CTRL_DIR, IN_OR_BOTH, ctrl_dir);
	sm->operControlledDirections = sm->adminControlledDirections;
}


// 检查CTRL_DIR SM 的条件变量，然后根据情况进行状态切换
SM_STEP(CTRL_DIR)
{
    // 当全局的强制初始化标志被置位，则进入IN_OR_BOTH状态
	if (sm->initialize) {
		SM_ENTER_GLOBAL(CTRL_DIR, IN_OR_BOTH);
		return;
	}

	switch (sm->ctrl_dir_state) {
	case CTRL_DIR_FORCE_BOTH:                           // 处于IN_OR_BOTH状态的CTRL_DIR SM，需要分为以下2种情况进系处理： 
		if (sm->eap_if->portEnabled && sm->operEdge)    //  1. 如果检测到adminControlledDirections状态被改变，则进入IN_OR_BOTH状态进行刷新
			SM_ENTER(CTRL_DIR, IN_OR_BOTH);                                                                                                 
		break;                                                                                                                              
	case CTRL_DIR_IN_OR_BOTH:                           //  2.1 如果检测到EAP->EAPOL交互标志portEnabled被清零(通常意味着一次认证失败)，或者
		if (sm->operControlledDirections !=             //  2.1 如果operEdge标志被清0
		    sm->adminControlledDirections)
			SM_ENTER(CTRL_DIR, IN_OR_BOTH);
		if (!sm->eap_if->portEnabled || !sm->operEdge)
			SM_ENTER(CTRL_DIR, FORCE_BOTH);
		break;
	}
}

/* 创建认证系统eapol层 + eap层 状态机统一管理块
 * 备注：不同于eapol_auth_init，本函数是针对bss上具体一个sta的802.1x-eapol模块执行初始化
 */
struct eapol_state_machine *
eapol_auth_alloc(struct eapol_authenticator *eapol, const u8 *addr,
		 int flags, const struct wpabuf *assoc_wps_ie,
		 const struct wpabuf *assoc_p2p_ie, void *sta_ctx)
{
	struct eapol_state_machine *sm;
	struct eap_config eap_conf;

	if (eapol == NULL)
		return NULL;

	sm = os_zalloc(sizeof(*sm));
	if (sm == NULL) {
		wpa_printf(MSG_DEBUG, "IEEE 802.1X state machine allocation "
			   "failed");
		return NULL;
	}
	sm->radius_identifier = -1;
	os_memcpy(sm->addr, addr, ETH_ALEN);
	sm->flags = flags;

	sm->eapol = eapol;
	sm->sta = sta_ctx;

	/* Set default values for state machine constants */
	sm->auth_pae_state = AUTH_PAE_INITIALIZE;
	sm->quietPeriod = AUTH_PAE_DEFAULT_quietPeriod;
	sm->reAuthMax = AUTH_PAE_DEFAULT_reAuthMax;

	sm->be_auth_state = BE_AUTH_INITIALIZE;
	sm->serverTimeout = BE_AUTH_DEFAULT_serverTimeout;

	sm->reauth_timer_state = REAUTH_TIMER_INITIALIZE;
	sm->reAuthPeriod = eapol->conf.eap_reauth_period;
	sm->reAuthEnabled = eapol->conf.eap_reauth_period > 0 ? TRUE : FALSE;

	sm->auth_key_tx_state = AUTH_KEY_TX_NO_KEY_TRANSMIT;

	sm->key_rx_state = KEY_RX_NO_KEY_RECEIVE;

	sm->ctrl_dir_state = CTRL_DIR_IN_OR_BOTH;

	sm->portControl = Auto;

	if (!eapol->conf.wpa &&
	    (eapol->default_wep_key || eapol->conf.individual_wep_key_len > 0))
		sm->keyTxEnabled = TRUE;
	else
		sm->keyTxEnabled = FALSE;
	if (eapol->conf.wpa)
		sm->portValid = FALSE;
	else
		sm->portValid = TRUE;

	os_memset(&eap_conf, 0, sizeof(eap_conf));
	eap_conf.eap_server = eapol->conf.eap_server;
	eap_conf.ssl_ctx = eapol->conf.ssl_ctx;
	eap_conf.msg_ctx = eapol->conf.msg_ctx;
	eap_conf.eap_sim_db_priv = eapol->conf.eap_sim_db_priv;
	eap_conf.pac_opaque_encr_key = eapol->conf.pac_opaque_encr_key;
	eap_conf.eap_fast_a_id = eapol->conf.eap_fast_a_id;
	eap_conf.eap_fast_a_id_len = eapol->conf.eap_fast_a_id_len;
	eap_conf.eap_fast_a_id_info = eapol->conf.eap_fast_a_id_info;
	eap_conf.eap_fast_prov = eapol->conf.eap_fast_prov;
	eap_conf.pac_key_lifetime = eapol->conf.pac_key_lifetime;
	eap_conf.pac_key_refresh_time = eapol->conf.pac_key_refresh_time;
	eap_conf.eap_sim_aka_result_ind = eapol->conf.eap_sim_aka_result_ind;
	eap_conf.tnc = eapol->conf.tnc;
	eap_conf.wps = eapol->conf.wps;
	eap_conf.assoc_wps_ie = assoc_wps_ie;
	eap_conf.assoc_p2p_ie = assoc_p2p_ie;
	eap_conf.peer_addr = addr;
	eap_conf.fragment_size = eapol->conf.fragment_size;
	eap_conf.pwd_group = eapol->conf.pwd_group;
	eap_conf.pbc_in_m1 = eapol->conf.pbc_in_m1;
    // 创建一个认证系统eap层状态机
	sm->eap = eap_server_sm_init(sm, &eapol_cb, &eap_conf);
	if (sm->eap == NULL) {
		eapol_auth_free(sm);
		return NULL;
	}
    // 设置eap<-->eapol交互接口
	sm->eap_if = eap_get_interface(sm->eap);

    // 创建一个认证系统EAPOL层状态机组，并运行EAPOL层 + EAP层所有状态机
	eapol_auth_initialize(sm);

	return sm;
}


/* 释放该状态机统一管理块
 * 备注：不同于eapol_auth_deinit，本函数是针对bss上具体一个sta的802.1x-eapol模块执行注销
 */
void eapol_auth_free(struct eapol_state_machine *sm)
{
	if (sm == NULL)
		return;

	eloop_cancel_timeout(eapol_port_timers_tick, NULL, sm);
	eloop_cancel_timeout(eapol_sm_step_cb, sm, NULL);
	if (sm->eap)
		eap_server_sm_deinit(sm->eap);
	os_free(sm);
}


// 检测当前的站表元素还是否有效
static int eapol_sm_sta_entry_alive(struct eapol_authenticator *eapol,
				    const u8 *addr)
{
	return eapol->cb.sta_entry_alive(eapol->conf.ctx, addr);
}


// eapol层状态机组 + eap层状态机 自平衡流程
static void eapol_sm_step_run(struct eapol_state_machine *sm)
{
	struct eapol_authenticator *eapol = sm->eapol;
	u8 addr[ETH_ALEN];
	unsigned int prev_auth_pae, prev_be_auth, prev_reauth_timer,
		prev_auth_key_tx, prev_key_rx, prev_ctrl_dir;
	int max_steps = 100;

	os_memcpy(addr, sm->addr, ETH_ALEN);

	/*
	 * Allow EAPOL state machines to run as long as there are state
	 * changes, but exit and return here through event loop if more than
	 * 100 steps is needed as a precaution against infinite loops inside
	 * eloop callback.
	 */
restart:
    // 首先是记录了6个状态机的旧状态
	prev_auth_pae = sm->auth_pae_state;
	prev_be_auth = sm->be_auth_state;
	prev_reauth_timer = sm->reauth_timer_state;
	prev_auth_key_tx = sm->auth_key_tx_state;
	prev_key_rx = sm->key_rx_state;
	prev_ctrl_dir = sm->ctrl_dir_state;

    // 检查AUTH_PAE SM的条件变量，然后根据情况进行状态切换
	SM_STEP_RUN(AUTH_PAE);
    // 执行其余5个SM有了附加的条件（满足以下任何一个即可）
    //  1. 全局标志initializing被置位
    //  2. 当前状态机所属的站表元素仍旧有效
	if (sm->initializing || eapol_sm_sta_entry_alive(eapol, addr))
		SM_STEP_RUN(BE_AUTH);
	if (sm->initializing || eapol_sm_sta_entry_alive(eapol, addr))
		SM_STEP_RUN(REAUTH_TIMER);
	if (sm->initializing || eapol_sm_sta_entry_alive(eapol, addr))
		SM_STEP_RUN(AUTH_KEY_TX);
	if (sm->initializing || eapol_sm_sta_entry_alive(eapol, addr))
		SM_STEP_RUN(KEY_RX);
	if (sm->initializing || eapol_sm_sta_entry_alive(eapol, addr))
		SM_STEP_RUN(CTRL_DIR);

    // 只要6个状态机中有任何一个状态进行了切换，就会再次迭代，迭代次数上限为max_steps次
	if (prev_auth_pae != sm->auth_pae_state ||
	    prev_be_auth != sm->be_auth_state ||
	    prev_reauth_timer != sm->reauth_timer_state ||
	    prev_auth_key_tx != sm->auth_key_tx_state ||
	    prev_key_rx != sm->key_rx_state ||
	    prev_ctrl_dir != sm->ctrl_dir_state) {
		if (--max_steps > 0)
			goto restart;
		/* Re-run from eloop timeout */
        // 当迭代次数超限后，这里选择的处理方式是退出到最外层的总循环，然后再进来执行一遍
		eapol_auth_step(sm);
		return;
	}

    // 到这里意味着eapol层状态机组在最近一次迭代中已经稳定下来
    // 开始进行eap层状态机自平衡
	if (eapol_sm_sta_entry_alive(eapol, addr) && sm->eap) {
		if (eap_server_sm_step(sm->eap)) {
            // 只要eap层状态机中进行了切换，就会再次从头迭代，迭代次数上限为max_steps次
			if (--max_steps > 0)
				goto restart;
			/* Re-run from eloop timeout */
            // 当迭代次数超限后，这里选择的处理方式是退出到最外层的总循环，然后再进来执行一遍
			eapol_auth_step(sm);
			return;
		}

        // 到这里意味着eapol层 + eap层 状态机在最近一次迭代中全部已经稳定下来
		/* TODO: find a better location for this */
        // 如果EAP->EAPOL_AAA交互标志aaaEapResp被置位，意味着有aaa数据需要被发送
		if (sm->eap_if->aaaEapResp) {
			sm->eap_if->aaaEapResp = FALSE;
			if (sm->eap_if->aaaEapRespData == NULL) {
				wpa_printf(MSG_DEBUG, "EAPOL: aaaEapResp set, "
					   "but no aaaEapRespData available");
				return;
			}
            // 执行基于802.1X协议的eapol层的aaa数据发送回调函数
			sm->eapol->cb.aaa_send(
				sm->eapol->conf.ctx, sm->sta,
				wpabuf_head(sm->eap_if->aaaEapRespData),
				wpabuf_len(sm->eap_if->aaaEapRespData));
		}
	}

    // 如果当前站表元素处于有效状态，则执行基于802.1X协议的eapol层状态机事件处理回调函数(实际应该没做任何处理)
	if (eapol_sm_sta_entry_alive(eapol, addr))
		sm->eapol->cb.eapol_event(sm->eapol->conf.ctx, sm->sta,
					  EAPOL_AUTH_SM_CHANGE);
}


// 一个即时触发定时器的回调函数,目的是运行状态机组自平衡流程
static void eapol_sm_step_cb(void *eloop_ctx, void *timeout_ctx)
{
	struct eapol_state_machine *sm = eloop_ctx;
	eapol_sm_step_run(sm);
}


/**
 * eapol_auth_step - Advance EAPOL state machines
 * @sm: EAPOL state machine
 * 注册一个即时触发的定时器
 *
 * This function is called to advance EAPOL state machines after any change
 * that could affect their state.
 * 这个函数的目的是当所有可能影响的因素都已经执行完毕后，再运行一遍状态机自平衡流程
 */
void eapol_auth_step(struct eapol_state_machine *sm)
{
	/*
	 * Run eapol_sm_step_run from a registered timeout to make sure that
	 * other possible timeouts/events are processed and to avoid long
	 * function call chains.
	 */

	eloop_register_timeout(0, 0, eapol_sm_step_cb, sm, NULL);
}


// 执行认证者EAPOL层 + EAP层状态机组初始化
static void eapol_auth_initialize(struct eapol_state_machine *sm)
{
	sm->initializing = TRUE;
	/* Initialize the state machines by asserting initialize and then
	 * deasserting it after one step */
	sm->initialize = TRUE;
	eapol_sm_step_run(sm);
	sm->initialize = FALSE;
	eapol_sm_step_run(sm);
	sm->initializing = FALSE;

	/* Start one second tick for port timers state machine */
    // 注册一个1s定时器，用于实现PORT_TIMER SM
	eloop_cancel_timeout(eapol_port_timers_tick, NULL, sm);
	eloop_register_timeout(1, 0, eapol_port_timers_tick, NULL, sm);
}


// eapol层提供给eap层的用户信息获取接口
static int eapol_sm_get_eap_user(void *ctx, const u8 *identity,
				 size_t identity_len, int phase2,
				 struct eap_user *user)
{
	struct eapol_state_machine *sm = ctx;
	return sm->eapol->cb.get_eap_user(sm->eapol->conf.ctx, identity,
					  identity_len, phase2, user);
}


// eapol层提供给eap层的接口，返回一段字符串消息，用于eap层组建eap-req-id包时附带信息，这是非必须的
static const char * eapol_sm_get_eap_req_id_text(void *ctx, size_t *len)
{
	struct eapol_state_machine *sm = ctx;
	*len = sm->eapol->conf.eap_req_id_text_len;
	return sm->eapol->conf.eap_req_id_text;
}


// 定义了eap层需要用到的eapol层回调函数
static struct eapol_callbacks eapol_cb =
{
	eapol_sm_get_eap_user,
	eapol_sm_get_eap_req_id_text
};


int eapol_auth_eap_pending_cb(struct eapol_state_machine *sm, void *ctx)
{
	if (sm == NULL || ctx == NULL || ctx != sm->eap)
		return -1;

	eap_sm_pending_cb(sm->eap);
	eapol_auth_step(sm);

	return 0;
}


// 将一份新的eapol认证配置导入到eapol认证控制块中
static int eapol_auth_conf_clone(struct eapol_auth_config *dst,
				 struct eapol_auth_config *src)
{
	dst->ctx = src->ctx;
	dst->eap_reauth_period = src->eap_reauth_period;
	dst->wpa = src->wpa;
	dst->individual_wep_key_len = src->individual_wep_key_len;
	dst->eap_server = src->eap_server;
	dst->ssl_ctx = src->ssl_ctx;
	dst->msg_ctx = src->msg_ctx;
	dst->eap_sim_db_priv = src->eap_sim_db_priv;
	os_free(dst->eap_req_id_text);
	dst->pwd_group = src->pwd_group;
	dst->pbc_in_m1 = src->pbc_in_m1;
	if (src->eap_req_id_text) {
		dst->eap_req_id_text = os_malloc(src->eap_req_id_text_len);
		if (dst->eap_req_id_text == NULL)
			return -1;
		os_memcpy(dst->eap_req_id_text, src->eap_req_id_text,
			  src->eap_req_id_text_len);
		dst->eap_req_id_text_len = src->eap_req_id_text_len;
	} else {
		dst->eap_req_id_text = NULL;
		dst->eap_req_id_text_len = 0;
	}
	if (src->pac_opaque_encr_key) {
		dst->pac_opaque_encr_key = os_malloc(16);
		os_memcpy(dst->pac_opaque_encr_key, src->pac_opaque_encr_key,
			  16);
	} else
		dst->pac_opaque_encr_key = NULL;
	if (src->eap_fast_a_id) {
		dst->eap_fast_a_id = os_malloc(src->eap_fast_a_id_len);
		if (dst->eap_fast_a_id == NULL) {
			os_free(dst->eap_req_id_text);
			return -1;
		}
		os_memcpy(dst->eap_fast_a_id, src->eap_fast_a_id,
			  src->eap_fast_a_id_len);
		dst->eap_fast_a_id_len = src->eap_fast_a_id_len;
	} else
		dst->eap_fast_a_id = NULL;
	if (src->eap_fast_a_id_info) {
		dst->eap_fast_a_id_info = os_strdup(src->eap_fast_a_id_info);
		if (dst->eap_fast_a_id_info == NULL) {
			os_free(dst->eap_req_id_text);
			os_free(dst->eap_fast_a_id);
			return -1;
		}
	} else
		dst->eap_fast_a_id_info = NULL;
	dst->eap_fast_prov = src->eap_fast_prov;
	dst->pac_key_lifetime = src->pac_key_lifetime;
	dst->pac_key_refresh_time = src->pac_key_refresh_time;
	dst->eap_sim_aka_result_ind = src->eap_sim_aka_result_ind;
	dst->tnc = src->tnc;
	dst->wps = src->wps;
	dst->fragment_size = src->fragment_size;
	return 0;
}

// 释放eapol层配置信息管理块
static void eapol_auth_conf_free(struct eapol_auth_config *conf)
{
	os_free(conf->eap_req_id_text);
	conf->eap_req_id_text = NULL;
	os_free(conf->pac_opaque_encr_key);
	conf->pac_opaque_encr_key = NULL;
	os_free(conf->eap_fast_a_id);
	conf->eap_fast_a_id = NULL;
	os_free(conf->eap_fast_a_id_info);
	conf->eap_fast_a_id_info = NULL;
}


/* 执行eapol认证器初始化,实际就是申请一个eapol认证控制块，并导入配置信息和回调函数
 * 备注：不同于eapol_auth_alloc，本函数是对整个bss上的802.x-eapol模块执行初始化
 */
struct eapol_authenticator * eapol_auth_init(struct eapol_auth_config *conf,
					     struct eapol_auth_cb *cb)
{
	struct eapol_authenticator *eapol;

	eapol = os_zalloc(sizeof(*eapol));
	if (eapol == NULL)
		return NULL;

	if (eapol_auth_conf_clone(&eapol->conf, conf) < 0) {
		os_free(eapol);
		return NULL;
	}

	if (conf->individual_wep_key_len > 0) {
		/* use key0 in individual key and key1 in broadcast key */
		eapol->default_wep_key_idx = 1;
	}

	eapol->cb.eapol_send = cb->eapol_send;
	eapol->cb.aaa_send = cb->aaa_send;
	eapol->cb.finished = cb->finished;
	eapol->cb.get_eap_user = cb->get_eap_user;
	eapol->cb.sta_entry_alive = cb->sta_entry_alive;
	eapol->cb.logger = cb->logger;
	eapol->cb.set_port_authorized = cb->set_port_authorized;
	eapol->cb.abort_auth = cb->abort_auth;
	eapol->cb.tx_key = cb->tx_key;
	eapol->cb.eapol_event = cb->eapol_event;

	return eapol;
}

/* 注销整个eapol认证器(意味着所在bss即将关闭802.1x认证功能)
 * 备注：不同于eapol_auth_free，本函数是对整个bss上的802.x-eapol模块执行注销
 */
void eapol_auth_deinit(struct eapol_authenticator *eapol)
{
	if (eapol == NULL)
		return;

	eapol_auth_conf_free(&eapol->conf);
	os_free(eapol->default_wep_key);
	os_free(eapol);
}
