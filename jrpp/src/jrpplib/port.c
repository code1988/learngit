
#include "base.h"
#include "ring.h"
#include "port.h"
#include "rpp_in.h"
#include "rpp_to.h"
#include "machine.h"

int port_set_stp(rppPort_t *port,ePortStpState state)
{
    if(port->stp_st == state)
        return 0;

    log_info("port %d set %s",port->port_index,(state == FORWARDING)?"forwarding":"blocking");
    port->stp_st = state;
    return RPP_OUT_set_port_stp(port->port_index,state,port->link_st);
}

// 获取当前节点的另一个端口控制块
rppPort_t *port_get_peer(rppPort_t *this)
{
    rppRing_t *ring = this->owner;
    
    return (this != ring->primary)?ring->primary:ring->secondary;
}

// 根据端口号获取所属的端口控制块
rppPort_t *port_get_owner(rppRing_t *ring,unsigned char lport)
{
    return (lport== ring->primary->port_index)?ring->primary:ring->secondary;
}

// compute min ballot id from two id
NODE_ID_T *port_min_id(NODE_ID_T *id1,NODE_ID_T *id2)
{
    return (memcmp(id1,id2,sizeof(NODE_ID_T)) < 0)?id1:id2;
}

// 转发/回传选举报文
void port_forwarding_bydir(rppPort_t *this, RPP_PDU_T *pdu,NODE_ID_T *minid)
{
    // 通过判断平行端口邻居有效/无效，决定选举报文转发方向
    rppPort_t *peer = port_get_peer(this);
    if(peer->neighber_vaild)
    {
        memcpy(&peer->fwding,pdu,sizeof(RPP_PDU_T));                        
        memcpy(&peer->fwding.body.msg.ballot.id,minid,sizeof(NODE_ID_T));
        tx_forwarding(peer);
    }
    else
    {
        memcpy(&this->fwding,pdu,sizeof(RPP_PDU_T));                        
        this->fwding.body.msg.ballot.type = MSG_BALLOT_LOOPBACK;
        memcpy(&this->fwding.body.msg.ballot.id,minid,sizeof(NODE_ID_T));
        tx_forwarding(this);
    }
}

// 转发选举回传报文
void port_ballot_fwd(rppPort_t *this, RPP_PDU_T *pdu)
{
    rppPort_t *peer = port_get_peer(this);
    if(peer->neighber_vaild)
    {
        memcpy(&peer->fwding,pdu,sizeof(RPP_PDU_T));                        
        tx_forwarding(peer);
    }
}
#define port_complete_fwd   port_ballot_fwd
#define port_report_fwd     port_ballot_fwd

// 转发hello报文
static void port_hello_fwd(rppPort_t *this, RPP_PDU_T *pdu,unsigned short bline)
{
    rppPort_t *peer = port_get_peer(this);
    if(ballot_if_fin(peer))
    {
        int blocks = (this->stp_st == BLOCKING) + (peer->stp_st == BLOCKING);
        memcpy(&peer->fwding,pdu,sizeof(RPP_PDU_T));                        
        peer->fwding.body.res3[0]                                       += blocks;
        peer->fwding.body.msg.hello.txport_state                        = peer->stp_st;
        *(unsigned short *)peer->fwding.body.msg.hello.block_line_num   = htons(bline);        
        tx_forwarding(peer);
    }
}

static int port_parse_hello_packet(rppPort_t *this, RPP_PDU_T *pdu)
{
    if(!ballot_if_fin(this))
    {
        LOG_ERROR("port %d is not ready to recv hello packet",this->port_index);
        return 0;
    }

    tRMsgHello *hello           = &pdu->body.msg.hello;
    rppRing_t *ring             = this->owner;
    unsigned short blockline    = ntohs(*(unsigned short *)hello->block_line_num);

    // compute blockline
    if(hello->txport_state == BLOCKING || this->stp_st == BLOCKING)
        blockline++;

    log_info("port %d recieve a packet for hello(%d,%d)",this->port_index,ntohs(*(unsigned short *)pdu->body.hello_seq),pdu->body.res3[0]);
    if(ring->node_role == NODE_TYPE_MASTER)
    {
#if 1
        pdu->body.res3[0] += (this->stp_st == BLOCKING);
        if(pdu->body.res3[0] != 2)
        {
            LOG_ERROR("this port stp state = %d,block points num = %d",this->stp_st,pdu->body.res3[0]);
        }
#endif
        //log_info("port %d recieve a packet for hello(%d,%d)",this->port_index,ntohs(*(unsigned short *)pdu->body.hello_seq),pdu->body.res3[0]);
        switch(blockline)
        {
            case 1:
                if(ring->status == RPP_FAULT)
                {
                    // 只刷节点状态
                    if(ring->primary->flag.opt != NULL)
                    {
                        LOG_ERROR("port %d tmp buffer is not empty!",ring->primary->port_index);
                        free(ring->primary->flag.opt);
                    }
                    tRMsgComplete *complete = (tRMsgComplete *)malloc(sizeof(tRMsgComplete));
                    complete->type  = MSG_COMPLETE_UPDATE_NODE;
                    ring->primary->flag.opt = complete;
                }
                break;
            case 0:
                LOG_ERROR("no block port in ring!");
            default:
                // 主端口forwarding,副端口blocking
                port_set_stp(ring->primary,FORWARDING);
                port_set_stp(ring->secondary,BLOCKING);
                // 发complete报文(刷fdb表和节点状态)
                if(ring->primary->flag.opt != NULL)
                {
                    LOG_ERROR("port %d tmp buffer is not empty!",ring->primary->port_index);
                    free(ring->primary->flag.opt);
                }
                tRMsgComplete *complete = (tRMsgComplete *)malloc(sizeof(tRMsgComplete));
                complete->type  = MSG_COMPLETE_WITH_FLUSH_FDB;
                ring->primary->flag.opt = complete;
                // 刷表
                RPP_OUT_flush_ports(ring->primary->port_index,ring->secondary->port_index);
                break;
        }

        // 主节点切换到健康状态
        if(ring->status == RPP_FAULT)
        {
            ring->status = RPP_HEALTH;
            RPP_OUT_led_set(0);
        }

        // 复位fail定时器
        RPP_OUT_set_timer(&ring->fail_timer,RPP_OUT_get_time(ring,FAIL_TIME));
    }
    else
    {
        // 传输节点转发hello报文
        port_hello_fwd(this,pdu,blockline);
        if(ring->status == RPP_HEALTH)
        {
            // 复位fail定时器
            RPP_OUT_set_timer(&ring->fail_timer,RPP_OUT_get_time(ring,FAIL_TIME));
        }
    }

    return 0;
}

static int port_parse_complete_packet(rppPort_t *this, RPP_PDU_T *pdu)
{
    tRMsgComplete *complete = &pdu->body.msg.complete;
    rppRing_t *ring         = this->owner;

    // master abandon complete packet
    if(ring->node_role == NODE_TYPE_TRANSIT)
    {
        /* transit forwarding complete packet 
         * change to health ring
         */
        port_complete_fwd(this,pdu);
        ring->status = RPP_HEALTH;
        RPP_OUT_led_set(0);

        switch(complete->type)
        {
            case MSG_COMPLETE_UPDATE_NODE:
                log_info("port %d recieve a packet for complete(only update node)",this->port_index);
                break;
            case MSG_COMPLETE_WITH_FLUSH_FDB:
                log_info("port %d recieve a packet for complete(update node and fdb)",this->port_index);
                port_set_stp(this,FORWARDING);
                rppPort_t *peer = port_get_peer(this);
                
                /**<    if peer node for peer port is master,blocking peer port,otherwise,forwarding peer port */
                if(!memcmp(peer->neighber_mac,pdu->hdr.src_mac,MAC_LEN))
                    port_set_stp(peer,BLOCKING);
                else
                    port_set_stp(peer,FORWARDING);

                // flush fdb
                RPP_OUT_flush_ports(ring->primary->port_index,ring->secondary->port_index);
                break;
            default:
                return -6;
        }
    }
    else
        log_info("port %d recieve a packet for self complete",this->port_index);
    return 0;
}

static int port_parse_report_packet(rppPort_t *this, RPP_PDU_T *pdu)
{
    tRMsgLinkDown *report   = &pdu->body.msg.linkdown;
    rppRing_t *ring         = this->owner;

    log_info("port %d recieve a packet for report linkdown",this->port_index);
    if(ring->status == RPP_HEALTH)
    {
        /* transit forwarding linkdown packet
         * master do nothing if linkdown is just occured on peer port of master     
         */
        if(ring->node_role == NODE_TYPE_TRANSIT)
            port_report_fwd(this,pdu);            
        else
        {
            if(!memcmp(report->ext_neighbor_mac,RPP_OUT_get_mac(),MAC_LEN))
                return 0;
            else
                RPP_OUT_close_timer(&ring->hello_timer);
        }

        // recieve linkdown packet means ring fault
        ring->status = RPP_FAULT;
        RPP_OUT_close_timer(&ring->fail_timer);
        RPP_OUT_led_set(1);
        port_set_stp(ring->primary,FORWARDING);
        port_set_stp(ring->secondary,FORWARDING);

        if(report->type == MSG_LINKDOWN_REQ_FLUSH_FDB)
        {
            ring->switch_cnts++;
            RPP_OUT_flush_ports(ring->primary->port_index,ring->secondary->port_index);
        }
    }

    return 0;
}

int RPP_port_rx_pdu (rppPort_t *this, RPP_PDU_T *pdu)
{
    switch(pdu->body.type)
    {
        case PACKET_AUTHENTICATION:
            return auth_packet_parse(this,pdu);
        case PACKET_BALLOT:
            return ballot_packet_parse(this,pdu);   
        case PACKET_HELLO:
            return port_parse_hello_packet(this,pdu);   
        case PACKET_COMPLETE:
            return port_parse_complete_packet(this,pdu);
        case PACKET_LINKDOWN:
            return port_parse_report_packet(this,pdu);
        default:
            return -3;
    }

  	return 0;
}

void RPP_port_link_change(rppPort_t *this,ePortLinkState link_status)
{
    // clear auth count、ballot id and invalid neighber info in case of link change
    this->auth_count        = 0;
    this->neighber_vaild    = 0;
    memset(&this->ballot_id,0,sizeof(BALLOT_ID_T));

    // update link state 
    this->link_st           = link_status;

    if(link_status == LINK_DOWN)
    {
        rppRing_t *ring = this->owner;
        rppPort_t *peer = port_get_peer(this);
        if(ring->status == RPP_HEALTH)
        {
            ring->status = RPP_FAULT;
            RPP_OUT_close_timer(&ring->fail_timer);
            RPP_OUT_led_set(1);
            /* master:  if link-down port is in forwarding status,
             *          block it,forwarding peer port and flush all
             * transit: if link-down port is in forwarding status,
             *          block it,forwarding peer port,fluch all and report linkdown packet with flushing fdb.
             *          otherwise,just report linkdown packet with updating node
             **/
            if(ring->node_role == NODE_TYPE_MASTER)
            {
                RPP_OUT_close_timer(&ring->hello_timer);
                if(this->stp_st == FORWARDING)
                {
                    ring->switch_cnts++;
                    port_set_stp(peer,FORWARDING);
                    RPP_OUT_flush_ports(ring->primary->port_index,ring->secondary->port_index);
                }
            }
            else
            {
                tRMsgLinkDown *msg = (tRMsgLinkDown *)malloc(sizeof(tRMsgLinkDown));
                msg->ext_neighbor_port = this->neighber_port;
                memcpy(msg->ext_neighbor_mac,this->neighber_mac,MAC_LEN);

                if(this->stp_st == FORWARDING)
                {
                    port_set_stp(peer,FORWARDING);
                    msg->type = MSG_LINKDOWN_REQ_FLUSH_FDB;
                    
                    RPP_OUT_flush_ports(ring->primary->port_index,ring->secondary->port_index);
                } 
                else
                {
                    msg->type = MSG_LINKDOWN_REQ_UPDATE_NODE;
                }

                if(peer->flag.opt != NULL)
                {
                    LOG_ERROR("port %d tmp buffer is not empty!",peer->port_index);
                    free(peer->flag.opt);
                }
                peer->flag.opt      = msg;
                tx_report(peer);
            }
        }
        else
        {
            // in rpp fault state,ballot will force start if peer port has a valid neighber
            if(peer->neighber_vaild)
                RPP_ballot_force(peer->ballot);  
        }
    }

    port_set_stp(this,BLOCKING);
}

int RPP_port_get_state(rppPort_t *port,RPP_PORT_STATE_T *state)
{
    state->port_no      = port->port_index;
    state->role         = port->port_role;
    state->dot1d_state  = port->stp_st;
    state->link_state   = port->link_st;
    strcpy(state->m_auth_st,RPP_machine_get_state(auth,port->auth));
    strcpy(state->m_ballot_st,RPP_machine_get_state(ballot,port->ballot));
    strcpy(state->m_tx_st,RPP_machine_get_state(transmit,port->transmit));
    
    return 0;
}

int RPP_port_start(rppPort_t *port)
{
    return port_set_stp(port,BLOCKING);
}

int RPP_port_stop(rppPort_t *port)
{
    return port_set_stp(port,FORWARDING);
}

#ifdef RPP_DBG

int RPP_port_trace_state_machine (PORT_T* this, char* mach_name, int enadis, int ring_id)
{
	register struct state_mach_t* stater;

	for (stater = this->machines; stater; stater = stater->next) {
		if (! strcmp (mach_name, "all") || ! strcmp (mach_name, stater->name)) {
			/* if (stater->debug != enadis) */
			{
				rpp_trace ("port %d on ring#%d trace %-8s (was %s) now %s",
					this->port_index, this->owner->ring_id,
					stater->name,
					stater->debug ? " enabled" :"disabled",
					enadis        ? " enabled" :"disabled");
			}
			stater->debug = enadis;
		}
	}

	return 0;
}

#endif


