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
#define PORT_NUM    11  // port number(contain cpu port)
#define PORT_CPU_H  9   // physical cpu-port
#define PORT_CPU_L  11  // logical cpu-port


typedef struct {
    RING_STATE_T    enable; 
	uint16          ring_id;
	NODE_ID_T       node_id;
	TIME_VALUES_T   times;

    rppRing_t       rpp_ring;

    struct list_head node;
}ring_t;

typedef struct {
    RPP_STATE_T running;
    char        if_name[IFNAMSIZ];
    char        mac[MAC_LEN];
    uint16      ring_num;
	struct {
		uint8	in_rpp;
		int		link_st;
	}mports[PORT_NUM - 1];

    struct list_head ring;
}rpp_t;

#endif


