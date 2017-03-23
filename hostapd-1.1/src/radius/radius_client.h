/*
 * RADIUS client
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

#ifndef RADIUS_CLIENT_H
#define RADIUS_CLIENT_H

#include "ip_addr.h"

struct radius_msg;

/**
 * struct hostapd_radius_server - RADIUS server information for RADIUS client
 *
 * This structure contains information about a RADIUS server. The values are
 * mainly for MIB information. The MIB variable prefix (radiusAuth or
 * radiusAcc) depends on whether this is an authentication or accounting
 * server.
 * radius客户端上记录的某个radius服务器信息（认证/计费服务器）
 * 这些信息往往用于MIB库
 *
 * radiusAuthClientPendingRequests (or radiusAccClientPendingRequests) is the
 * number struct radius_client_data::msgs for matching msg_type.
 */
struct hostapd_radius_server {
	/**
	 * addr - radiusAuthServerAddress or radiusAccServerAddress
	 */
	struct hostapd_ip_addr addr;

	/**
	 * port - radiusAuthClientServerPortNumber or radiusAccClientServerPortNumber
	 */
	int port;

	/**
	 * shared_secret - Shared secret for authenticating RADIUS messages
	 */
	u8 *shared_secret;

	/**
	 * shared_secret_len - Length of shared_secret in octets
	 */
	size_t shared_secret_len;

	/* Dynamic (not from configuration file) MIB data */

	/**
	 * index - radiusAuthServerIndex or radiusAccServerIndex
     * 当前bss中该radius服务器的序号（认证/计费服务器分开排序）
	 */
	int index;

	/**
	 * round_trip_time - radiusAuthClientRoundTripTime or radiusAccClientRoundTripTime
	 * Round-trip time in hundredths of a second.
     * radius-req报文被响应的往返时间
	 */
	int round_trip_time;

	/**
	 * requests - radiusAuthClientAccessRequests or radiusAccClientRequests
	 */
	u32 requests;

	/**
	 * retransmissions - radiusAuthClientAccessRetransmissions or radiusAccClientRetransmissions
	 */
	u32 retransmissions;

	/**
	 * access_accepts - radiusAuthClientAccessAccepts
	 */
	u32 access_accepts;

	/**
	 * access_rejects - radiusAuthClientAccessRejects
	 */
	u32 access_rejects;

	/**
	 * access_challenges - radiusAuthClientAccessChallenges
	 */
	u32 access_challenges;

	/**
	 * responses - radiusAccClientResponses
	 */
	u32 responses;

	/**
	 * malformed_responses - radiusAuthClientMalformedAccessResponses or radiusAccClientMalformedResponses
	 */
	u32 malformed_responses;

	/**
	 * bad_authenticators - radiusAuthClientBadAuthenticators or radiusAccClientBadAuthenticators
	 */
	u32 bad_authenticators;

	/**
	 * timeouts - radiusAuthClientTimeouts or radiusAccClientTimeouts
	 */
	u32 timeouts;

	/**
	 * unknown_types - radiusAuthClientUnknownTypes or radiusAccClientUnknownTypes
	 */
	u32 unknown_types;

	/**
	 * packets_dropped - radiusAuthClientPacketsDropped or radiusAccClientPacketsDropped
	 */
	u32 packets_dropped;
};

/**
 * struct hostapd_radius_servers - RADIUS servers for RADIUS client
 * radius客户端配置的所有radius服务器信息管理块
 */
struct hostapd_radius_servers {
	/**
	 * auth_servers - RADIUS Authentication servers in priority order
	 */
	struct hostapd_radius_server *auth_servers; // radius认证服务器信息管理块数组

	/**
	 * num_auth_servers - Number of auth_servers entries
     * radius认证服务器数量
	 */
	int num_auth_servers;

	/**
	 * auth_server - The current Authentication server
     * 指向当前使用的radius认证服务器
	 */
	struct hostapd_radius_server *auth_server;

	/**
	 * acct_servers - RADIUS Accounting servers in priority order
	 */
	struct hostapd_radius_server *acct_servers; // radius计费服务器信息管理块

	/**
	 * num_acct_servers - Number of acct_servers entries
	 */
	int num_acct_servers;

	/**
	 * acct_server - The current Accounting server
	 */
	struct hostapd_radius_server *acct_server;

	/**
	 * retry_primary_interval - Retry interval for trying primary server
	 *
	 * This specifies a retry interval in sexconds for trying to return to
	 * the primary RADIUS server. RADIUS client code will automatically try
	 * to use the next server when the current server is not replying to
	 * requests. If this interval is set (non-zero), the primary server
	 * will be retried after the specified number of seconds has passed
	 * even if the current used secondary server is still working.
     * 主服务器的重试间隔(单位s)
     * 以radius认证服务器为例，如果radius客户端配置了多个radius认证服务器，意味着有一个主服务器和若干个备份服务器，
     * 当主服务器没有响应客户端发出的radius请求报文时，客户端会自动依次尝试使用备份服务器。
     * 针对这种机制，由此产生了primfary-interval变量，表示客户端对主服务器的重试间隔，
     * 即便备份服务器工作得很好，客户端都会在重试间隔之后尝试切换回主服务器
	 */
	int retry_primary_interval;

	/**
	 * msg_dumps - Whether RADIUS message details are shown in stdout
	 */
	int msg_dumps;

	/**
	 * client_addr - Client (local) address to use if force_client_addr
	 */
	struct hostapd_ip_addr client_addr;

	/**
	 * force_client_addr - Whether to force client (local) address
	 */
	int force_client_addr;
};


/**
 * RadiusType - RADIUS server type for RADIUS client
 * radius客户端使用到的radius服务类型
 */
typedef enum {
	/**
	 * RADIUS authentication
	 */
	RADIUS_AUTH,

	/**
	 * RADIUS_ACCT - RADIUS accounting
	 */
	RADIUS_ACCT,

	/**
	 * RADIUS_ACCT_INTERIM - RADIUS interim accounting message
	 *
	 * Used only with radius_client_send(). This behaves just like
	 * RADIUS_ACCT, but removes any pending interim RADIUS Accounting
	 * messages for the same STA before sending the new interim update.
	 */
	RADIUS_ACCT_INTERIM
} RadiusType;

/**
 * RadiusRxResult - RADIUS client RX handler result
 */
typedef enum {
	/**
	 * RADIUS_RX_PROCESSED - Message processed
	 *
	 * This stops handler calls and frees the message.
	 */
	RADIUS_RX_PROCESSED,

	/**
	 * RADIUS_RX_QUEUED - Message has been queued
	 *
	 * This stops handler calls, but does not free the message; the handler
	 * that returned this is responsible for eventually freeing the
	 * message.
	 */
	RADIUS_RX_QUEUED,

	/**
	 * RADIUS_RX_UNKNOWN - Message is not for this handler
	 */
	RADIUS_RX_UNKNOWN,

	/**
	 * RADIUS_RX_INVALID_AUTHENTICATOR - Message has invalid Authenticator
	 */
	RADIUS_RX_INVALID_AUTHENTICATOR
} RadiusRxResult;

struct radius_client_data;

int radius_client_register(struct radius_client_data *radius,
			   RadiusType msg_type,
			   RadiusRxResult (*handler)
			   (struct radius_msg *msg, struct radius_msg *req,
			    const u8 *shared_secret, size_t shared_secret_len,
			    void *data),
			   void *data);
int radius_client_send(struct radius_client_data *radius,
		       struct radius_msg *msg,
		       RadiusType msg_type, const u8 *addr);
u8 radius_client_get_id(struct radius_client_data *radius);
void radius_client_flush(struct radius_client_data *radius, int only_auth);
struct radius_client_data *
radius_client_init(void *ctx, struct hostapd_radius_servers *conf);
void radius_client_deinit(struct radius_client_data *radius);
void radius_client_flush_auth(struct radius_client_data *radius,
			      const u8 *addr);
int radius_client_get_mib(struct radius_client_data *radius, char *buf,
			  size_t buflen);
void radius_client_reconfig(struct radius_client_data *radius,
			    struct hostapd_radius_servers *conf);

#endif /* RADIUS_CLIENT_H */
