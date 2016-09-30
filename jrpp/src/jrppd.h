#ifndef _JRPPD_H_
#define _JRPPD_H_

#include <net/if.h>

#include <libubox/uloop.h>
#include <libubox/utils.h>

#include "rpp_mgmt.h"
#include "rpp_pdu.h"
#include "statmch.h"
#include "port.h"
#include "ring.h"


typedef unsigned char       uint8;
typedef char                int8;
typedef unsigned short      uint16;
typedef short               int16;
typedef unsigned int        uint32;
typedef int                 int32;
typedef unsigned long long  uint64;

#define MAC_LEN     6
#define PORT_NUM    11  // 包含cpu口
#define PORT_CPU_H  9

typedef struct {
    RING_MODE_T     enable; 
	uint16          ring_id;
	NODE_ID_T       node_id;
	TIME_VALUES_T   times;

    rppRing_t       rpp_ring;

    struct list_head node;
}ring_t;

typedef struct {
    RPP_MODE_T  running;
    char        if_name[IFNAMSIZ];
    char        mac[MAC_LEN];
    uint16      ring_num;

    struct list_head ring;
}rpp_t;

#endif


