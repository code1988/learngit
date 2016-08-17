
#ifndef _RSTPD_H_
#define _RSTPD_H_

#include <net/if.h>
#include "base.h"
#include "stpm.h"

/****************************************************
 *                  Micro define
 ****************************************************/ 
#define STP_MAX_PORT_NUM		32
#define STP_DEFAULT_VLAN_ID     0
#define STP_DEFAULT_VLAN_NAME   "vlan-none"

/* Stp admin mode */
#define STP_MODE_DISABLED		0
#define STP_MODE_ENABLED		1

/* Stp running status */
#define STP_DOWN				0
#define STP_RUNNING				1

/* Port disable/enable */
#define STP_PORT_DISABLED   	0
#define STP_PORT_ENABLED		1

/* Port link status */
#define STP_PORT_LINKDOWN   	0
#define STP_PORT_LINKUP     	1

/* Port duplex status */
#define STP_PORT_DUPLEX_HALF	0
#define STP_PORT_DUPLEX_FULL	1

/****************************************************
 *                  Bridge Parameters
 ****************************************************/ 
typedef struct {
	STPM_T*         stpm;               // 整棵生成树内的网桥链表
	int             brIndex;            // 桥的接口序号
	char            brName[IFNAMSIZ];   // 桥的接口名
	int             portCount;          // 桥的端口数量
	int            *portList;
	
	/* bridge config */
	UID_STP_MODE_T  stp_enabled;        // 生成树使能/禁止，针对该桥
	int             bridge_priority;    // 桥优先级
	int             max_age;
	int             hello_time;
	int             forward_delay;
	int             force_version;
	int             hold_time;	
} stpBridge_t;

/****************************************************
 *                  Port Parameters
 ****************************************************/
typedef struct {
	PORT_T*         port;
    int             ifIndex;            // 端口序号
	char            ifName[IFNAMSIZ];   // 端口名
	unsigned char   portEnable;         // 端口使能/禁止
	int             linkStatus;         // 端口link状态
	int             speed;              // 端口速率
	int             duplex;             // 端口双工模式
	
	/* port config */
	int             port_priority;          // 端口优先级
	int             admin_port_path_cost;   // 端口路径成本
	ADMIN_P2P_T     admin_point2point;
	unsigned char   admin_edge;
	unsigned char   admin_non_stp;	        // 端口rstp功能使能/禁止
} stpPort_t;

// rstp总控制块
typedef struct {
	int             running;    // 运行状态
	stpBridge_t     bridge;     // 桥控制块
	stpPort_t      *ports;      // stp端口控制块
} rstp_t;

#endif

