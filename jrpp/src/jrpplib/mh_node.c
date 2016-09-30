#include "base.h"
#include "ring.h"
#include "rpp_in.h"
#include "rpp_to.h"
#include "machine.h"

#define STATES {\
                CHOOSE(NODE_COMPLETE),\
                CHOOSE(NODE_FAILED),\
                CHOOSE(NODE_LINK_UP),\
                CHOOSE(NODE_LINK_DOWN),\
                CHOOSE(NODE_PRE_FORWARDING)\
}

#define GET_STATE_NAME RPP_node_get_state_name 
#include "choose.h"

/* fail timer timeout means ring fault 
 * forwarding port authing ok if previous status is fault already
 * master node should close hello timer
 **/
void fail_timer_handler(struct uloop_timeout *timer)
{
    rppRing_t *ring = rpp_get_ring_addr(timer,fail_timer);

    log_info("this is in fail timer");

    if(ring->status == RPP_FAULT)
    {
        // forwarding认证成功的端口
        if(ballot_if_fin(ring->primary))
            port_set_stp(ring->primary,FORWARDING);
        if(ballot_if_fin(ring->secondary))
            port_set_stp(ring->secondary,FORWARDING);
    }
    else
    {
        ring->status = RPP_FAULT;
        RPP_OUT_led_set(1);
    }

    if(ring->node_role == NODE_TYPE_MASTER)
        RPP_OUT_close_timer(&ring->hello_timer);

	RPP_ring_update (ring);
}

void hello_timer_handler(struct uloop_timeout *timer)
{
    rppRing_t *ring = rpp_get_ring_addr(timer,hello_timer);

    tx_hello(ring->primary);
    RPP_OUT_set_timer(timer,RPP_OUT_get_time(ring,HELLO_TIME));

	RPP_ring_update (ring);
}


void RPP_node_enter_state(STATE_MACH_T* this)
{
    register rppRing_t *ring = this->owner.ring;

    switch(this->state)
    {
        case BEGIN:
            break;
        case NODE_COMPLETE:
            tx_complete(ring->primary);
            log_info("  \033[1;33m<node machine> in complete status!\033[0m");
            break;
        case NODE_FAILED:
            log_info("  \033[1;33m<node machine> in failed status!\033[0m");
            break;
        case NODE_LINK_UP:
            log_info("  \033[1;33m<node machine> in link-up status!\033[0m");
            break;
        case NODE_LINK_DOWN:
            log_info("  \033[1;33m<node machine> in link-down status!\033[0m");
            break;
        case NODE_PRE_FORWARDING:
            log_info("  \033[1;33m<node machine> in pre-forwarding status!\033[0m");
            break;
    }
}

#define node_if_linkup(r) (r->node_role == NODE_TYPE_TRANSIT && \
                           (r->primary->link_st & r->secondary->link_st) == LINK_UP && \
                           (r->primary->stp_st & r->secondary->stp_st) == FORWARDING) 
#define node_if_linkdown(r) (r->node_role == NODE_TYPE_TRANSIT && \
                           (r->primary->link_st & r->secondary->link_st) != LINK_UP)
#define node_if_prefwd(r) (r->node_role == NODE_TYPE_TRANSIT && \
                           (r->primary->link_st & r->secondary->link_st) == LINK_UP && \
                           (r->primary->stp_st & r->secondary->stp_st) != FORWARDING) 

Bool RPP_node_check_conditions(STATE_MACH_T* this)
{
    register rppRing_t *ring = this->owner.ring;

    switch(this->state)
    {
        case BEGIN:
            if(ring->node_role == NODE_TYPE_MASTER && ring->status == RPP_FAULT)
                return RPP_hop_2_state(this,NODE_FAILED);
            if(node_if_prefwd(ring))
                return RPP_hop_2_state(this,NODE_PRE_FORWARDING);
            if(node_if_linkdown(ring))
                return RPP_hop_2_state(this,NODE_LINK_DOWN);
            break;
        case NODE_COMPLETE:
            if(ring->status == RPP_FAULT)
                return RPP_hop_2_state(this,NODE_FAILED);
            break;
        case NODE_FAILED:
            if(ring->node_role == NODE_TYPE_MASTER && ring->status == RPP_HEALTH)
                return RPP_hop_2_state(this,NODE_COMPLETE);
            if(node_if_linkup(ring))
                return RPP_hop_2_state(this,NODE_LINK_UP);
            if(node_if_linkdown(ring))
                return RPP_hop_2_state(this,NODE_LINK_DOWN);
            break;
        case NODE_LINK_UP:
            if(ring->node_role == NODE_TYPE_MASTER && ring->status == RPP_FAULT)
                return RPP_hop_2_state(this,NODE_FAILED);
            if(node_if_linkdown(ring))
                return RPP_hop_2_state(this,NODE_LINK_DOWN);
            if(node_if_prefwd(ring))
                return RPP_hop_2_state(this,NODE_PRE_FORWARDING);
            break;
        case NODE_LINK_DOWN:
            if(ring->node_role == NODE_TYPE_MASTER && ring->status == RPP_FAULT)
                return RPP_hop_2_state(this,NODE_FAILED);
            if(node_if_linkup(ring))
                return RPP_hop_2_state(this,NODE_LINK_UP);
            if(node_if_prefwd(ring))
                return RPP_hop_2_state(this,NODE_PRE_FORWARDING);
            break;
        case NODE_PRE_FORWARDING:
            if(ring->node_role == NODE_TYPE_MASTER && ring->status == RPP_FAULT)
                return RPP_hop_2_state(this,NODE_FAILED);
            if(node_if_linkdown(ring))
                return RPP_hop_2_state(this,NODE_LINK_DOWN);
            if(node_if_linkup(ring))
                return RPP_hop_2_state(this,NODE_LINK_UP);
            break;
        default:
            LOG_ERROR("node machine in error status!\n");
            break;
    }
    return False;
} 
