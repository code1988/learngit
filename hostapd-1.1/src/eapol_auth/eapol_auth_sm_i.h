/*
 * IEEE 802.1X-2004 Authenticator - EAPOL state machine (internal definitions)
 * Copyright (c) 2002-2009, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * 本文件内容只包含了认证系统，不包含请求者部分
 * See README and COPYING for more details.
 */

#ifndef EAPOL_AUTH_SM_I_H
#define EAPOL_AUTH_SM_I_H

#include "common/defs.h"
#include "radius/radius.h"

/* IEEE Std 802.1X-2004, Ch. 8.2 */

typedef enum { ForceUnauthorized = 1, ForceAuthorized = 3, Auto = 2 }
	PortTypes;  // 受控端口类型：强制端口保持在未授权状态、强制端口保持在啊授权状态、自动（根据认证结果而定）
typedef enum { Unauthorized = 2, Authorized = 1 } PortState;    // 受控端口当前状态：未授权状态、授权状态
typedef enum { Both = 0, In = 1 } ControlledDirection;          // 受控方向：双向受控、输入受控
typedef unsigned int Counter;


/**
 * struct eapol_authenticator - Global EAPOL authenticator data
 * 供认证者使用的eapol层数据块
 * 备注：不同于下面的struct eapol_state_machine，该数据结构的服务对象是整个bss，所以在bss初始化时同时完成了初始化
 *       
 */
struct eapol_authenticator {
	struct eapol_auth_config conf;  // 配置信息
	struct eapol_auth_cb cb;        // 回调函数集

	u8 *default_wep_key;
	u8 default_wep_key_idx;
};


/**
 * struct eapol_state_machine - Per-Supplicant Authenticator state machines
 * 定义了每个请求者对应的认证系统eapol层 + eap层 所有状态机统一管理块
 * 备注：不同于上面的struct eapol_authenticator，该数据结构的服务对象是bss上接入的每个sta，所以是在有客户端接入并生成对应的sta时初始化
 */
struct eapol_state_machine {
	/* timers */
	int aWhile;     // 用于BE_AUTH SM ，定义了eapol层递交eapolEap报文给eap层，然后等待回复的计时,
                    // 也就是BE_AUTH进入BE_AUTH_RESPONSE时开启，并且只在BE_AUTH_RESPONSE期间有效
                    // 超时值为serverTimeout(也就是BE_AUTH_DEFAULT_serverTimeout)
                    
	int quietWhile; // 用于AUTH_PAE SM，此时间内认证者不会接受任何请求者，超时值为quietPeriod(也就是AUTH_PAE_DEFAULT_quietPeriod)
	int reAuthWhen; // 用于REAUTH_TIMER SM ，定义了认证成功后nas发起重认证的间隔,超时值为reAuthPeriod
	/* global variables */
	Boolean authAbort;  // AUTH_PAE SM 进入ABORTING状态时设置TRUE；BE_AUTH SM 进入INITIALIZE状态时设置FALSE
	Boolean authFail;   // BE_AUTH SM 进入FAIL状态时设置TRUE；AUTH_PAE SM 进入AUTHENTICATING状态时设置FALSE
	PortState authPortStatus;   // 端口当前状态,由AUTH_PAE SM 完全控制
	Boolean authStart;      // AUTH_PAE SM 进入AUTHENTICATING状态时设置TRUE；BE_AUTH SM 进入IDLE状态时设置FALSE
	Boolean authTimeout;    // BE_AUTH SM 进入TIMEOUT状态时设置TRUE；AUTH_PAE SM 进入AUTHENTICATING状态时设置FALSE
	Boolean authSuccess;    // BE_AUTH SM 进入SUCCESS状态时设置TRUE；AUTH_PAE SM 进入AUTHENTICATING状态时设置FALSE
	Boolean eapolEap;       // 作为认证者，接收到承载了EAP-PACKET(通常是一个resp)的EAPOL报文时设置TRUE；
                            // BE_AUTH SM 进入REQUEST(非标准)/RESPONSE状态时设置FALSE。
                            // 只在BE_AUTH_REQUEST/BE_AUTH_IGNORE期间有效
                            
	Boolean initialize;     // 当该标志强制初始化eapol层所有状态机
	Boolean keyDone;                                                                                                                                           
	Boolean keyRun;         // BE_AUTH SM 进入SUCCESS状态时设置TRUE；AUTH_PAE SM 进入AUTHENTICATING/ABORTING状态时设置FALSE
	Boolean keyTxEnabled;
	PortTypes portControl;  // 受控端口的全局控制模式,作为802.1x认证者，在eapol_auth_alloc时设置了固定值AUTO
	Boolean portValid;      // 只在无线网络中(wpa功能开启时)被用到
	Boolean reAuthenticate; // Reauthentication Timer状态机进入REAUTHENTICATE状态时设置TRUE；Authenticator PAE状态机进入CONNECTING状态时设置FALSE

	/* Port Timers state machine */
	/* 'Boolean tick' implicitly handled as registered timeout */

	/* Authenticator PAE state machine */
	enum { AUTH_PAE_INITIALIZE, AUTH_PAE_DISCONNECTED, AUTH_PAE_CONNECTING,
	       AUTH_PAE_AUTHENTICATING, AUTH_PAE_AUTHENTICATED,
	       AUTH_PAE_ABORTING, AUTH_PAE_HELD, AUTH_PAE_FORCE_AUTH,
	       AUTH_PAE_FORCE_UNAUTH, AUTH_PAE_RESTART } auth_pae_state;    // Authenticator PAE状态机
	/* variables */
	Boolean eapolLogoff;    // eapol层接收到eapol-logoff报文时设置TRUE；状态机进入DISCONNECTED/HELD状态时设置FALSE
	Boolean eapolStart;     // eapol层接收到eapol-start报文时设置TRUE；状态机进入AUTHENTICATING/FORCE_AUTH/FORCE_UNAUTH状态时设置FALSE
	PortTypes portMode;     // AUTH_PEA SM私有的端口模式,初始化时设置Auto，进入FORCE_AUTH状态时设置ForceAuthorized，进入FORCE_UNAUTH状态时设置ForceUnauthorized
	unsigned int reAuthCount;   // 状态机进入CONNECTING状态时累加，一旦超过reAuthMax则进入DISCONNECTED状态，进入DISCONNECTED/AUTHENTICATED时清0

	/* constants */
	unsigned int quietPeriod; /* default 60; 0..65535 静默时间,应该是认证失败后到重新认证的间隔，用于设置定时器quietWhile*/
#define AUTH_PAE_DEFAULT_quietPeriod 60
	unsigned int reAuthMax; /* default 2 重认证次数*/
#define AUTH_PAE_DEFAULT_reAuthMax 2
	/* counters */
	Counter authEntersConnecting;                   // 记录了进入CONNECTING状态的次数
	Counter authEapLogoffsWhileConnecting;          // 记录了在CONNECTING状态下收到eapol-logoff报文而进入DISCONNECTED状态的次数
	Counter authEntersAuthenticating;               // 记录了在CONNECTING状态下收到eap-resp/identify报文而进入AUTHENTICATING状态的次数
	Counter authAuthSuccessesWhileAuthenticating;   // 记录了在AUTHENTICATING状态下，因为authSuccess标志被置为TRUE,而进入AUTHENTICATED状态的次数
	Counter authAuthTimeoutsWhileAuthenticating;    // 记录了在AUTHENTICATING状态下，因为authTimeout标志被置为TRUE,而进入ABORTING状态的次数
	Counter authAuthFailWhileAuthenticating;        // 记录了在AUTHENTICATING状态下，因为authFail标志被置为TRUE,而进入HELD状态的次数
	Counter authAuthEapStartsWhileAuthenticating;   // 记录了在AUTHENTICATING状态下收到eapol-start报文而进入ABORTING状态的次数
	Counter authAuthEapLogoffWhileAuthenticating;   // 记录了在AUTHENTICATING状态下收到eapol-logoff报文而进入ABORTING状态的次数
	Counter authAuthReauthsWhileAuthenticated;      // 记录了在AUTHENTICATED状态下，因为reAuthenticate标志被置为TRUE,而进入RESTART状态的次数
	Counter authAuthEapStartsWhileAuthenticated;    // 记录了在AUTHENTICATED状态下收到eapol-start报文而进入CONNECTING状态的次数
	Counter authAuthEapLogoffWhileAuthenticated;    // 记录了在AUTHENTICATED状态下收到eapol-start报文而进入DISCONNECTED状态的次数

	/* Backend Authentication state machine */
	enum { BE_AUTH_REQUEST, BE_AUTH_RESPONSE, BE_AUTH_SUCCESS,
	       BE_AUTH_FAIL, BE_AUTH_TIMEOUT, BE_AUTH_IDLE, BE_AUTH_INITIALIZE,
	       BE_AUTH_IGNORE
	} be_auth_state;    // 后台认证状态机
	/* constants */
	unsigned int serverTimeout; /* default 30; 1..X 后台服务器认证超时时间，用于设置定时器aWhile*/
#define BE_AUTH_DEFAULT_serverTimeout 30
	/* counters */
	Counter backendResponses;                   // 记录了BE_AUTH SM 发送给EAP层response的数量
	Counter backendAccessChallenges;                                                          
	Counter backendOtherRequestsToSupplicant;   // 记录了BE_AUTH SM 发送给请求者request的数量
	Counter backendAuthSuccesses;                                                             
	Counter backendAuthFails;                   // 记录了BE_AUTH SM 从RESPONSE进入FAIL的数量

	/* Reauthentication Timer state machine */
	enum { REAUTH_TIMER_INITIALIZE, REAUTH_TIMER_REAUTHENTICATE
	} reauth_timer_state;   // 重认证定时器状态机
	/* constants */
	unsigned int reAuthPeriod; /* default 3600 s 重认证周期(可以由本地配置文件设置，也可以被radius服务器设置),用于设置重认证定时器reAuthWhen*/
	Boolean reAuthEnabled;  // 重认证功能使能位(只用于认证系统)

	/* Authenticator Key Transmit state machine */
	enum { AUTH_KEY_TX_NO_KEY_TRANSMIT, AUTH_KEY_TX_KEY_TRANSMIT
	} auth_key_tx_state;    // 认证者key发送状态机

	/* Key Receive state machine */
	enum { KEY_RX_NO_KEY_RECEIVE, KEY_RX_KEY_RECEIVE } key_rx_state;    // key接收状态机
	/* variables */
	Boolean rxKey;

	/* Controlled Directions state machine */
	enum { CTRL_DIR_FORCE_BOTH, CTRL_DIR_IN_OR_BOTH } ctrl_dir_state;   // 受控方向状态机
	/* variables */
	ControlledDirection adminControlledDirections;  // 受控方向，本值不会被CTRL_DIR SM修改
	ControlledDirection operControlledDirections;   // 受控方向，本值会被CTRL_DIR SM写入adminControlledDirections
	Boolean operEdge;                               // 端口没有开启VLAN时，此标志位设置为TRUE

	/* Authenticator Statistics Table */
	Counter dot1xAuthEapolFramesRx;            // 记录了接收eapol帧数量 
	Counter dot1xAuthEapolFramesTx;            // 记录了发送eapol帧数量
	Counter dot1xAuthEapolStartFramesRx;                                                      
	Counter dot1xAuthEapolLogoffFramesRx;                                                     
	Counter dot1xAuthEapolRespIdFramesRx;      // 记录了接收eap-resp-id帧数量
	Counter dot1xAuthEapolRespFramesRx;        // 记录了接收eap-resp帧数量
	Counter dot1xAuthEapolReqIdFramesTx;       // 记录了发送code = req,type=id的eapol帧数量
	Counter dot1xAuthEapolReqFramesTx;         // 记录了发送code = req的eapol帧数量
	Counter dot1xAuthInvalidEapolFramesRx;                                                    
	Counter dot1xAuthEapLengthErrorFramesRx;   
	Counter dot1xAuthLastEapolFrameVersion;    // 记录了最近收到的eapol帧的版本2001/2004/2010

	/* Other variables - not defined in IEEE 802.1X */
	u8 addr[ETH_ALEN]; /* Supplicant address 请求者mac地址*/
	int flags; /* EAPOL_SM_* eapol层附加功能位的集合*/

	/* EAPOL/AAA <-> EAP full authenticator interface */
	struct eap_eapol_interface *eap_if; // 指向EAP<-->EAPOL交互接口地址
	int radius_identifier;              // 记录了radius客户端最近发送的radius-request报文的id字段(实际是0～255),用于匹配收到的radius报文
	/* TODO: check when the last messages can be released */
	struct radius_msg *last_recv_radius;    // 指向最近一个收到的radius报文消息管理块
	u8 last_eap_id; /* last used EAP Identifier 记录了最后一个使用的EAP报文的Identify字段*/
	u8 *identity;           // AAA接口在封装eap-resp-identify数据到radius报文时会记录下用户名信息;在收到radius-accept报文时会再次更新
	size_t identity_len;    // 记录的用户名长度
	u8 eap_type_authsrv; /* EAP type of the last EAP packet from
			      * Authentication server 记录了从radius认证服务器发来的最后一个EAP报文的type字段*/
	u8 eap_type_supp; /* EAP type of the last EAP packet from Supplicant 记录了从请求者发来的最后一个EAP报文的type字段*/
	struct radius_class_data radius_class;

	/* Keys for encrypting and signing EAPOL-Key frames */
	u8 *eapol_key_sign;
	size_t eapol_key_sign_len;
	u8 *eapol_key_crypt;
	size_t eapol_key_crypt_len;

	struct eap_sm *eap; // 指向EAP层状态机

	Boolean initializing; /* in process of initializing state machines 标志位，表示正在进行状态机初始化*/
	Boolean changed;    // 记录eapol层状态机组的状态是否发生变化

	struct eapol_authenticator *eapol;  // 指向eapol认证控制块

	void *sta; /* station context pointer to use in callbacks 指向对应站表元素控制块*/
};

#endif /* EAPOL_AUTH_SM_I_H */
