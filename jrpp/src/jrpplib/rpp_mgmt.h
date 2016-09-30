
#ifndef _RPP_MGMT_H_
#define _RPP_MGMT_H_

#include "bitmap.h"

#ifndef MAC_LEN
#define MAC_LEN     6
#endif

#define M_NAME_LEN  32

typedef enum {
	RPP_DISABLED = 0,
	RPP_ENABLED,
} RPP_MODE_T;

typedef struct {
    unsigned char prio;
    unsigned char addr[MAC_LEN];
}BALLOT_ID_T,NODE_ID_T;

/*****************************************************************
        Ring configuration and state management
 *****************************************************************/
#define DEFAULT_RING_PRIO    0
#define DEFAULT_RING_HELLO   1
#define DEFAULT_RING_FAIL    3

typedef enum {
    RING_DISABLED = 0,
    RING_ENABLED
} RING_MODE_T;

typedef enum {
	RPP_FAULT = 0,
	RPP_HEALTH,
} RING_STATUS_T;

typedef enum {
	NODE_TYPE_MASTER			= 0,
	NODE_TYPE_TRANSIT			= 1,
	NODE_TYPE_NNKNOWN           = 2
} eNodeType;

// 环配置参数标记位
#define RING_CFG_STATE       (1L << 0)
#define RING_CFG_PRIO        (1L << 1)
#define RING_CFG_HELLO       (1L << 2)
#define RING_CFG_FAIL        (1L << 3)
#define RING_CFG_BALLOT      (1L << 4)
#define RING_CFG_AUTH        (1L << 5)
#define RING_CFG_ALL        RING_CFG_STATE     | \
                            RING_CFG_PRIO      | \
                            RING_CFG_HELLO     | \
                            RING_CFG_BALLOT    | \
                            RING_CFG_FAIL      | \
                            RING_CFG_AUTH

/*****************************************************************
        Port configuration and state management
 *****************************************************************/
typedef enum {
	PRIMARY_PORT = 0,
	SECONDARY_PORT,
} PORT_ROLE_T;

#if 0
typedef enum {
	RPP_PORT_DISABLED = 0,
	RPP_PORT_BLOCKING,
	RPP_PORT_FORWARDING,
} PORT_STATE_T;
#endif

typedef enum {
    DISABLED					= 0,
    BLOCKING					= 1,
    LEARNING					= 2,
    FORWARDING					= 3,
    STP_UNKOWN					= 4
} ePortStpState;

typedef enum {
    LINK_DOWN					= 2,
    LINK_UP						= 1
} ePortLinkState;

#define PORT_CFG_ROLE        (1L << 0)
#define PORT_CFG_ALL        PORT_CFG_ROLE

typedef struct {
	unsigned long   field_mask;
	BITMAP_T        port_bmp;   

	/* protocol data */
	PORT_ROLE_T     port_role;
} RPP_PORT_CFG_T;

typedef struct {
	int             port_no;

	/* protocol data */
    PORT_ROLE_T     role;
	ePortStpState   dot1d_state;
    ePortLinkState  link_state;

    char            m_auth_st[M_NAME_LEN];
    char            m_ballot_st[M_NAME_LEN];
    char            m_tx_st[M_NAME_LEN];
	
	unsigned long   rx_hello;
	unsigned long   rx_linkdown;
	unsigned long   rx_complete;

	int             neigbor_port_no;
	unsigned char   neigbor_mac[6];

} RPP_PORT_STATE_T;


/*****************************************************************
        Ring configuration and state management
 *****************************************************************/
typedef struct {
	unsigned long   field_mask;     // 环配置参数标记字段
	RPP_MODE_T      rpp_enabled;    // 环使能/禁止,暂时不用

	/* protocol data */
	unsigned char   node_priority;  // 节点优先级
	int             hello_time;        
	int             fail_time;         
	int             ballot_time;
    int             auth_time;
} RPP_RING_CFG_T;

typedef struct {
	RING_MODE_T     state;        // 环使能/禁止
	NODE_ID_T       node_id;      
	BALLOT_ID_T     master_id;    
	RING_STATUS_T   ring_status;  // 环状态
    eNodeType       node_role;

    char            m_node_st[M_NAME_LEN];

    RPP_PORT_STATE_T primary;
    RPP_PORT_STATE_T secondary;
} RPP_RING_STATE_T;



#endif

