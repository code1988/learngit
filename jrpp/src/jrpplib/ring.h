
#ifndef _RPP_RING_MACHINE_H_
#define _RPP_RING_MACHINE_H_

#include "port.h"

typedef struct rppRing {
    RING_STATUS_T   status;
	NODE_ID_T       master_id;
    eNodeType       node_role;
	unsigned short  switch_cnts; 
    unsigned short  hello_syn;

    struct uloop_timeout fail_timer;
    struct uloop_timeout hello_timer;

    rppPort_t       *primary;
    rppPort_t       *secondary;

    STATE_MACH_T *node;      // 节点状态状态机
    STATE_MACH_T *machines;
}rppRing_t;

/*********************************************************************
                 Functions prototypes 
 *********************************************************************/
int RPP_ring_start(rppRing_t *);
int RPP_ring_stop(rppRing_t *);
int RPP_ring_update(rppRing_t *);
void RPP_ring_role_handler(rppRing_t *,eNodeType,NODE_ID_T *);

#define RPP_ring_reset_timer(r,name) RPP_OUT_set_timer(&r->name ## _timer,)
#define RPP_ring_close_timer(r,name) (r->name ## _timer.pending = false)


#endif

