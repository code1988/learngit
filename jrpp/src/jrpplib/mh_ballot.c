#include "base.h"
#include "ring.h"
#include "rpp_in.h"
#include "rpp_to.h"
#include "machine.h"

#define STATES {\
                CHOOSE(BALLOT_ITTV),\
                CHOOSE(BALLOT_PSSV),\
                CHOOSE(BALLOT_FAIL),\
                CHOOSE(BALLOT_FIN)\
}

#define GET_STATE_NAME RPP_ballot_get_state_name 
#include "choose.h"

static Bool force = False;

Bool ballot_if_fin(rppPort_t *port)
{
    return (port->flag.f_ballot == BALLOT_FIN);
}

int ballot_packet_parse(rppPort_t *this, RPP_PDU_T *pdu)
{
    tRMsgBallot *ballot = &pdu->body.msg.ballot;

    if(!this->neighber_vaild)
    {
        LOG_ERROR("port %d get ballot,but is neighber invalid",this->port_index);
        return 0;
    }
    
    if(!memcmp(pdu->hdr.src_mac,RPP_OUT_get_mac(),MAC_LEN))
    {
        log_info("port %d recieve a packet for self ballot loopback",this->port_index);
        rppPort_t *owner = port_get_owner(this->owner,ballot->port);

        memcpy(&owner->ballot_id,&ballot->id,sizeof(NODE_ID_T));
        RPP_OUT_close_timer(&owner->ballot_timer);
        owner->flag.f_ballot = BALLOT_FIN;
    }
    else
    {
        NODE_ID_T *minId;
        NODE_ID_T *nodeId = RPP_OUT_get_nodeid(this->owner);
        switch(ballot->type)
        {
            case MSG_BALLOT_INITIATIVE:
                log_info("port %d recieve a packet for ballot initiative",this->port_index);
                minId = port_min_id(nodeId,&ballot->id);
                port_forwarding_bydir(this,pdu,minId);

                /**<    if ballot is't from peer node,passive ballot may be trigger */
                if(memcmp(pdu->hdr.src_mac,this->neighber_mac,MAC_LEN) || pdu->body.res3[0] == True)
                    this->flag.f_ballot = BALLOT_PSSV;
                break;
            case MSG_BALLOT_PASSIVE:
                log_info("port %d recieve a packet for ballot passive",this->port_index);
                minId = port_min_id(nodeId,&ballot->id);
                port_forwarding_bydir(this,pdu,minId);
                break;
            case MSG_BALLOT_LOOPBACK:
                log_info("port %d recieve a packet for ballot loopback",this->port_index);
                port_ballot_fwd(this,pdu);
                break;
            default:
                return -5;
        }
    }

    return 0;
}

void ballot_timer_handler(struct uloop_timeout *timer)
{
    rppPort_t *port = rpp_get_port_addr(timer,ballot_timer);

    tx_ballot_ittv(port,force);
    RPP_OUT_set_timer(timer,RPP_OUT_get_time(port->owner,BALLOT_TIME));

	RPP_ring_update (port->owner);
}

static void ballot_role_compute(rppPort_t *port)
{
    NODE_ID_T   *nodeId   = RPP_OUT_get_nodeid(port->owner);
    rppPort_t   *peer     = port_get_peer(port);
    BALLOT_ID_T *master_id;
    eNodeType   role;

    if(peer->neighber_vaild)
    {
        if(!ballot_if_fin(peer))
            return;
             
        role = (memcmp(nodeId,&port->ballot_id,sizeof(NODE_ID_T)) || memcmp(nodeId,&peer->ballot_id,sizeof(NODE_ID_T)));
        master_id = port_min_id(port_min_id(nodeId,&port->ballot_id),&peer->ballot_id);
    }
    else
    {
        role = memcmp(nodeId,&port->ballot_id,sizeof(NODE_ID_T));
        master_id = port_min_id(nodeId,&port->ballot_id);
    }
    
    RPP_ring_role_handler(port->owner,role,master_id);
    //log_info("this is a %s node",(role == NODE_TYPE_MASTER)?"master":"transit");
}

void RPP_ballot_enter_state(STATE_MACH_T* this)
{
    register rppPort_t *port = this->owner.port;

    switch(this->state)
    {
        case BEGIN:
            RPP_OUT_close_timer(&port->ballot_timer);
            port->flag.f_ballot = 0;
            break;
        case BALLOT_ITTV:
            RPP_OUT_set_timer(&port->ballot_timer,RPP_OUT_get_time(port->owner,BALLOT_TIME));
            log_info("  \033[1;33m<ballot machine> port %d in initiative status!\033[0m",port->port_index);
            break;
        case BALLOT_PSSV:
            tx_ballot_pssv(port);
            log_info("  \033[1;33m<ballot machine> port %d in passive status!\033[0m",port->port_index);
            break;
        case BALLOT_FAIL:
            force = False;
            log_info("  \033[1;33m<ballot machine> port %d in fail status!\033[0m",port->port_index);
            break;
        case BALLOT_FIN:
            force = False;
            log_info("  \033[1;33m<ballot machine> port %d in finish status!\033[0m",port->port_index);
            ballot_role_compute(port);
            break;
    }
}

Bool RPP_ballot_check_conditions(STATE_MACH_T* this)
{
    register rppPort_t *port    = this->owner.port;

    switch(this->state)
    {
        case BEGIN:
            if(port->neighber_vaild) 
                return RPP_hop_2_state(this,BALLOT_ITTV);
            break;
        case BALLOT_ITTV:
            if(port->flag.f_ballot == BALLOT_FIN)
                return RPP_hop_2_state(this,BALLOT_FIN);
            if(port->flag.f_ballot == BALLOT_FAIL)
                return RPP_hop_2_state(this,BALLOT_FAIL);
            if(!port->neighber_vaild)
                return RPP_hop_2_state(this,BEGIN);
            break;
        case BALLOT_PSSV:
            if(port->flag.f_ballot == BALLOT_FIN)
                return RPP_hop_2_state(this,BALLOT_FIN);
            if(port->flag.f_ballot == BALLOT_FAIL)
                return RPP_hop_2_state(this,BALLOT_FAIL);
            if(!port->neighber_vaild)
                return RPP_hop_2_state(this,BEGIN);
            break;
        case BALLOT_FAIL:
            return RPP_hop_2_state(this,BEGIN);
        case BALLOT_FIN:
            if(port->flag.f_ballot == BALLOT_PSSV)
                return RPP_hop_2_state(this,BALLOT_PSSV);
            if(!port->neighber_vaild)
                return RPP_hop_2_state(this,BEGIN);
            break;
        default:
            LOG_ERROR("ballot machine in error status!\n");
            break;
    }

    return False;
}

// start ballot force
Bool RPP_ballot_force(STATE_MACH_T* this)
{
    force = True;
    this->owner.port->flag.f_ballot = 0;
    return RPP_hop_2_state(this,BALLOT_ITTV);
}
