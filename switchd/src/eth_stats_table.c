/* *< 中威文件编码格式为gb2312,cp936 */
/* * $Id$ $DateTime$
 * * @file eth_stats_table.c
 * * @brief This is a table of the switch's port ethernet statistics.
 * * @version 0.0.1
 * * @since 0.0.1
 * * @author code<songyaofei@obtelecom.com>
 * * @date 2016-04-25
 * Created it
 * */
/* *****************************************************************
 * *@note
 * Copyright 2012, OB Telecom Electronics Co., Ltd.
 * ALL RIGHTS RESERVED
 * Permission is hereby granted to licensees of OB Telecom Electronics
 * Co., Ltd. products to use or abstract this computer program for
 * the sole purpose of implementing a product based on OB Telecom
 * Electronics Co., Ltd. products. No other rights to reproduce, use,
 * or disseminate this computer program,whether in part or in whole,
 * are granted. OB Telecom Electronics Co., Ltd. makes no
 * representation or warranties with respect to the performance of
 * this computer program, and specifically disclaims any
 * responsibility for any damages, special or consequential,
 * connected with the use of this program.
 * For details, see http://obtelecom.com
 * *******************************************************************/
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <libubox/uloop.h>

#include "switchd.h"

static struct blob_buf b;
static struct ubus_context *_ctx;

static int JW_GetRxBytes(struct blob_buf *buf, int port_idx);
static int JW_GetRxUcastPkts(struct blob_buf *buf, int port_idx);
static int JW_GetRxBcastPkts(struct blob_buf *buf, int port_idx);
static int JW_GetRxMcastPkts(struct blob_buf *buf, int port_idx);
static int JW_GetRxPausePkts(struct blob_buf *buf, int port_idx);
static int JW_GetTxBytes(struct blob_buf *buf, int port_idx);
static int JW_GetTxUcastPkts(struct blob_buf *buf, int port_idx);
static int JW_GetTxBcastPkts(struct blob_buf *buf, int port_idx);
static int JW_GetTxMcastPkts(struct blob_buf *buf, int port_idx);
static int JW_GetTxPausePkts(struct blob_buf *buf, int port_idx);
static int JW_EthStatsHandle(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg);

enum {
    SWITCH_ETH_STATS_GET_ARRAY,
    __SWITCH_ETH_STATS_GET_MAX
};

static const struct blobmsg_policy eth_stats_get_policy[] = {
    [SWITCH_ETH_STATS_GET_ARRAY] = {.name = "get_args", .type = BLOBMSG_TYPE_ARRAY},
};

enum {
    SWITCH_RX_BYTES,
    SWITCH_RX_UCAST_PKTS,
    SWITCH_RX_BCAST_PKTS,
    SWITCH_RX_MCAST_PKTS,
    SWITCH_RX_PAUSE_PKTS,
    SWITCH_TX_BYTES,
    SWITCH_TX_UCAST_PKTS,
    SWITCH_TX_BCAST_PKTS,
    SWITCH_TX_MCAST_PKTS,
    SWITCH_TX_PAUSE_PKTS,
    __SWITCH_ETH_STATS_MAX,
};

static const struct ubus_method eth_stats_methods[] = {
	UBUS_METHOD("eth_stats_get", JW_EthStatsHandle,eth_stats_get_policy),
};


static struct ubus_object_type eth_stats_object_type =
	UBUS_OBJECT_TYPE("eth_stats", eth_stats_methods);

static struct ubus_object eth_stats_object = {
	.name = "eth_stats",
	.type = &eth_stats_object_type,
	.methods = eth_stats_methods,
	.n_methods = ARRAY_SIZE(eth_stats_methods),
};

static const struct jw_switch_policy eth_stats_tbl[] = {
    [SWITCH_RX_BYTES]       = {.name = "rx_bytes", .type = BLOBMSG_TYPE_INT32, .get_handler = JW_GetRxBytes}, 
    [SWITCH_RX_UCAST_PKTS]  = {.name = "rx_unicast_packets", .type = BLOBMSG_TYPE_INT32, .get_handler = JW_GetRxUcastPkts},
    [SWITCH_RX_BCAST_PKTS]  = {.name = "rx_broadcast_packets", .type = BLOBMSG_TYPE_INT32, .get_handler = JW_GetRxBcastPkts},
    [SWITCH_RX_MCAST_PKTS]  = {.name = "rx_multicast_packets", .type = BLOBMSG_TYPE_INT32, .get_handler = JW_GetRxMcastPkts},
    [SWITCH_RX_PAUSE_PKTS]  = {.name = "rx_pause_packets", .type = BLOBMSG_TYPE_INT32, .get_handler = JW_GetRxPausePkts},
    [SWITCH_TX_BYTES]       = {.name = "tx_bytes", .type = BLOBMSG_TYPE_INT32, .get_handler = JW_GetTxBytes},
    [SWITCH_TX_UCAST_PKTS]  = {.name = "tx_unicast_packets", .type = BLOBMSG_TYPE_INT32, .get_handler = JW_GetTxUcastPkts},
    [SWITCH_TX_BCAST_PKTS]  = {.name = "tx_broadcast_packets",.type = BLOBMSG_TYPE_INT32,.get_handler = JW_GetTxBcastPkts},
    [SWITCH_TX_MCAST_PKTS]  = {.name = "tx_multicast_packets",.type = BLOBMSG_TYPE_INT32,.get_handler = JW_GetTxMcastPkts},
    [SWITCH_TX_PAUSE_PKTS]  = {.name = "tx_pause_packets",.type = BLOBMSG_TYPE_INT32,.get_handler = JW_GetTxPausePkts},
}; 

/* *
 * * @fn JW_GetRxBytes
 * * @brief get rx bytes
 * * @param[port_idx] port index.
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_GetRxBytes(struct blob_buf *buf, int port_idx)
{
    int rx_bytes = 1;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: This is port %d\n", __func__, __LINE__,port_idx);
    blobmsg_add_u32(buf, "rx_bytes", rx_bytes);
    blobmsg_close_table(buf, table);

    return 0;
}

/* *
 * * @fn JW_GetRxUcastPkts
 * * @brief get rx unicast packets
 * * @param[port_idx] port index.
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_GetRxUcastPkts(struct blob_buf *buf, int port_idx)
{
    int rx_ucast_pkts = 2;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: This is port %d\n", __func__, __LINE__,port_idx);
    blobmsg_add_u32(buf, "rx_unicast_packets", rx_ucast_pkts);
    blobmsg_close_table(buf, table);

    return 0;
}

/* *
 * * @fn JW_GetRxBcastPkts
 * * @brief get rx broadcast packets
 * * @param[port_idx] port index.
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_GetRxBcastPkts(struct blob_buf *buf, int port_idx)
{
    int rx_bytes = 3;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: This is port %d\n", __func__, __LINE__,port_idx);
    blobmsg_add_u32(buf, "rx_broadcast_packets", rx_bytes);
    blobmsg_close_table(buf, table);

    return 0;
}

/* *
 * * @fn JW_GetRxMcastPkts
 * * @brief get rx multicast packets
 * * @param[port_idx] port index.
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_GetRxMcastPkts(struct blob_buf *buf, int port_idx)
{
    int rx_bytes = 4;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: This is port %d\n", __func__, __LINE__,port_idx);
    blobmsg_add_u32(buf, "rx_multicast_packets", rx_bytes);
    blobmsg_close_table(buf, table);

    return 0;
}

/* *
 * * @fn JW_GetRxPausePkts
 * * @brief get rx pause packets
 * * @param[port_idx] port index.
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_GetRxPausePkts(struct blob_buf *buf, int port_idx)
{
    int rx_bytes = 5;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: This is port %d\n", __func__, __LINE__,port_idx);
    blobmsg_add_u32(buf, "rx_pause_packets", rx_bytes);
    blobmsg_close_table(buf, table);

    return 0;
}

/* *
 * * @fn JW_GetTxBytes
 * * @brief get tx bytes
 * * @param[port_idx] port index.
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_GetTxBytes(struct blob_buf *buf, int port_idx)
{
    int rx_bytes = 6;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: This is port %d\n", __func__, __LINE__,port_idx);
    blobmsg_add_u32(buf, "tx_bytes", rx_bytes);
    blobmsg_close_table(buf, table);

    return 0;
}

/* *
 * * @fn JW_GetTxUcastPkts
 * * @brief get tx unicast packets
 * * @param[port_idx] port index.
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_GetTxUcastPkts(struct blob_buf *buf, int port_idx)
{
    int rx_bytes = 7;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: This is port %d\n", __func__, __LINE__,port_idx);
    blobmsg_add_u32(buf, "tx_unicast_packets", rx_bytes);
    blobmsg_close_table(buf, table);

    return 0;
}

/* *
 * * @fn JW_GetTxBcastPkts
 * * @brief get tx broadcast packets
 * * @param[port_idx] port index.
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_GetTxBcastPkts(struct blob_buf *buf, int port_idx)
{
    int rx_bytes = 8;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: This is port %d\n", __func__, __LINE__,port_idx);
    blobmsg_add_u32(buf, "tx_bordcast_packets", rx_bytes);
    blobmsg_close_table(buf, table);

    return 0;
}

/* *
 * * @fn JW_GetTxMcastPkts
 * * @brief get tx multicast packets
 * * @param[port_idx] port index.
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_GetTxMcastPkts(struct blob_buf *buf, int port_idx)
{
    int rx_bytes = 9;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: This is port %d\n", __func__, __LINE__,port_idx);
    blobmsg_add_u32(buf, "tx_multicast_packets", rx_bytes);
    blobmsg_close_table(buf, table);

    return 0;
}

/* *
 * * @fn JW_GetTxPausePkts
 * * @brief get tx pause packets
 * * @param[port_idx] port index.
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_GetTxPausePkts(struct blob_buf *buf, int port_idx)
{
    int rx_bytes = 10;
    void *table = NULL;

    table = blobmsg_open_table(buf, "jw_switch");
    printf("[%s][%d]: This is port %d\n", __func__, __LINE__,port_idx);
    blobmsg_add_u32(buf, "tx_pause_packets", rx_bytes);
    blobmsg_close_table(buf, table);

    return 0;
}

/* *
 * * @fn JW_ParsePortEthStatsGet
 * * @brief parse port ethernet statistic to get config list
 * * @param[lvl3] point to3 level message structure.
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_ParsePortEthStatsGet(struct blob_attr *lvl3, struct blob_buf *buf)
{	
    int port_idx = -1;
    struct blobmsg_hdr *hdr = NULL;
    char *pidx = NULL;
    
    hdr = blob_data(lvl3);

    pidx = strstr((char *)hdr->name, "idx-");
    if (pidx) 
    {
        printf("[%s][%d], pidx = [%s]\n", __func__, __LINE__, pidx);
        port_idx = atoi(pidx + 4);
        if (port_idx < 0 || port_idx > 32) 
        {
            printf("[%s][%d], invalid port index %d\n", __func__, __LINE__, port_idx);
            return -1;
        }
    } 
    else 
    {
        printf("[%s][%d], invalid port index arg [%s]\n", __func__, __LINE__, hdr->name);
        return -1;
    }

    void *msg_array = NULL;
    void *msg_table = NULL;
    struct blob_attr *lvl4 = NULL;  
    int rem = 0;
    char *pmsg = NULL;
    struct jw_switch_policy *eth_stats_p = NULL;

    msg_table = blobmsg_open_table(buf, (char *)hdr->name); 
    msg_array = blobmsg_open_array(buf, (char *)hdr->name);

    blobmsg_for_each_attr(lvl4, lvl3, rem) 
    {
        if (blobmsg_type(lvl4) == BLOBMSG_TYPE_STRING)  
        {
            pmsg = blobmsg_get_string(lvl4);
            printf("[%s][%d]Level 4 structure is string,name: %s\n",__func__,__LINE__,pmsg);
            eth_stats_p = (struct jw_switch_policy *)jw_switchd_get_context(pmsg,eth_stats_tbl, __SWITCH_ETH_STATS_MAX);
            if (eth_stats_p && eth_stats_p->get_handler) 
            {
                printf("[%s][%d]:call policy handle in %p\n",__func__,__LINE__,eth_stats_p);
                eth_stats_p->get_handler(buf, port_idx);
            }
            else
                printf("[%s][%d]:call policy handle fail!, eth_stats_p addr: %p, handler fun: %p\n",__func__,__LINE__,eth_stats_p,eth_stats_p->get_handler);
        }
    }

    blobmsg_close_array(buf, msg_array);
    blobmsg_close_table(buf, msg_table);

    return 0;
}

/* *
 * * @fn JW_EthStatsHandle
 * * @brief handle function for the method of "eth_stats_get"
 * * @retval 0: OK
 * * @retval -1: ERROR
 * */
static int JW_EthStatsHandle(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
    struct blob_attr *tb[__SWITCH_ETH_STATS_GET_MAX];    
	struct blob_attr *lvl1 = NULL;  

    blobmsg_parse(eth_stats_get_policy, __SWITCH_ETH_STATS_GET_MAX, tb, blob_data(msg), blob_len(msg));
	lvl1 = tb[SWITCH_ETH_STATS_GET_ARRAY];     
    if (!lvl1) 
	{
        ERROR("invalid params in %s\n", __func__);
        return -1;
    }

	if (blobmsg_type(lvl1) != BLOBMSG_TYPE_ARRAY) 
	{
        ERROR("blobmsg type is not array\n");
        return -1;
    }

    struct blob_attr *lvl2 = NULL; 
    int rem = 0;
    void *ret_table = NULL;
		
	blob_buf_init(&b, 0);

    ret_table = blobmsg_open_array(&b, "ret"); 

    blobmsg_for_each_attr(lvl2,lvl1, rem) 
    {
        if (blobmsg_type(lvl2) == BLOBMSG_TYPE_TABLE) 
        {
            struct blob_attr *lvl3 = blobmsg_data(lvl2);
            
			if (blobmsg_type(lvl3) ==  BLOBMSG_TYPE_ARRAY) 
			{
                printf("Level 3 structor is array\n");
				JW_ParsePortEthStatsGet(lvl3, &b);
            } 
        } 
        else 
		{
            ERROR("%s[%d]: invalid attr type\n", __func__, __LINE__);
        }
    }

    blobmsg_close_array(&b, ret_table);

	ubus_send_reply(ctx, req, b.head);

	return 0;
}

/* *
 * * @fn ubus_init_eth_stats
 * * @brief add object "eth_stats" to ubus
 * */
void ubus_init_eth_stats(struct ubus_context *ctx)
{
    int ret;

	_ctx = ctx;
	ret = ubus_add_object(ctx, &eth_stats_object);
	if (ret)
		ERROR("Failed to add object port config: %s\n", ubus_strerror(ret));
}
