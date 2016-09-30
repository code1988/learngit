#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/un.h>
#include <linux/types.h>
#include <linux/filter.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_bridge.h>
#include <linux/sockios.h>
#include <net/if.h>

#include "jssdk_mode.h"
#include "fal/fal.h"
#include "jws_convert.h"

#include "ring.h"
#include "rpp_in.h"
#include "rpp_to.h"

#include "log.h"
#include "utils.h"
#include "packet.h"
#include "jrpp_if.h"
#include "jrppd.h"
#include "epoll_loop.h"

rpp_t *pRpp = NULL;
	
RPP_RING_CFG_T default_ring_cfg = {
	.node_priority = 0,
    .auth_time = 2,
	.hello_time = 1,
	.fail_time =  3,
	.ballot_time = 2,
};

// 根据key索引对应的环(flag: 1- 按环号索引；0- 按端口号索引)
static ring_t *rpp_find_ring(int key,int flag)
{
    ring_t *pRing,*pRing_bak;  
    list_for_each_entry_safe(pRing, pRing_bak, &pRpp->ring, node) 
    {
        if(flag && pRing->ring_id == key)   
            return pRing;
        else if(!flag && RPP_IN_port_valid(&pRing->rpp_ring,key))
            return pRing;
    }

    return NULL;
}

int rpp_init(void)
{
    pRpp = (rpp_t *)calloc(1,sizeof(rpp_t));        
    if(pRpp == NULL)
    {
        LOG_ERROR("calloc rpp fail!\n");
        return -1;
    }

    pRpp->running   = RPP_DISABLED;
    pRpp->ring_num  = 0;
    INIT_LIST_HEAD(&pRpp->ring);

    init_ring_led();

    return 0;
}

void br_pdu_rcv(const unsigned char *data, int len)
{
    RPP_PDU_T *rpp_pdu = (RPP_PDU_T *)data;
    uint8 lport;

    if(pRpp == NULL || pRpp->running != RPP_ENABLED)
        return;

    if(RPP_OUT_h2lport(rpp_pdu->hdr.dsa_tag[1] >> 3,&lport))
    {
        LOG_ERROR("get logical port fail!\n");
        return;
    }
    
    ring_t *pRing = rpp_find_ring(lport,0);
    if(pRing == NULL)
    {
		LOG_ERROR("port %d is not in any ring\n",lport);
		return;
    }

    if(pRing->enable != RING_ENABLED)
    {
        LOG_ERROR("rpp ring#%d is not enabled!\n",pRing->ring_id);
        return;
    }

    if(RPP_IN_rx_pdu(&pRing->rpp_ring,lport,rpp_pdu))
    {
        LOG_ERROR("deal with rx packet fail\n");
    }
}

void br_notify(int br_index, int if_index, int newlink, unsigned flags)
{
    if(pRpp == NULL || pRpp->running != RPP_ENABLED)
        return;

    ring_t *pRing = rpp_find_ring(if_index,0);
    if(pRing == NULL)
		return;

    if(pRing->enable != RING_ENABLED)
        return;

    RPP_IN_port_link_check(&pRing->rpp_ring,if_index,((flags & IFF_UP) && (flags & IFF_RUNNING))?LINK_UP:LINK_DOWN);
}

int RPP_OUT_flush_ports(uint8 p1,uint8 p2)
{
    if(fal_mac_flush_port(0,p1) != SW_OK || fal_mac_flush_port(0,p2) != SW_OK)
    {
        LOG_ERROR("flush port %d and port %d error!",p1,p2);
        return -1;
    }

	return 0;
}

static const uint8 h2lport[PORT_NUM] = {[0] = 9,[1] = 10,[2] = 7,[3] = 8,[4] = 5,[5] = 6,[6] = 3,[7] = 4,[8] = 2,[10] = 1};
static const uint8 l2hport[PORT_NUM] = {[1] = 10,[2] = 8,[3] = 6,[4] = 7,[5] = 4,[6] = 5,[7] = 2,[8] = 3,[9] = 0,[10] = 1};
int RPP_OUT_h2lport(uint8 hport, uint8 *lport)
{
    if(hport == PORT_CPU_H || hport >= PORT_NUM)
        return -1;

    *lport = h2lport[hport];

    return 0;
}

int RPP_OUT_l2hport(uint8 lport, uint8 *hport)
{
    if(lport >= PORT_NUM)
        return -1;

    *hport = l2hport[lport];
    return 0;
}

// 设置端口stp
int RPP_OUT_set_port_stp(int port_index, ePortStpState state,ePortLinkState link)
{
    fal_port_dot1d_state_t dot1d_state;
    uint8 br_state;

	switch(state) 
    {
		case FORWARDING:
			dot1d_state = FAL_PORT_DOT1D_STATE_FORWARDING;
            br_state    = BR_STATE_FORWARDING;
			break;
		case BLOCKING:
			dot1d_state = FAL_PORT_DOT1D_STATE_BLOCKING;
            br_state    = BR_STATE_BLOCKING;
			break;
		default:
			dot1d_state = FAL_PORT_DOT1D_STATE_DISABLED;
            br_state    = BR_STATE_DISABLED;
			break;
	}

	if(link == LINK_UP && br_set_state(port_index, br_state)) 
    {
		LOG_ERROR("set kernel bridge state fail");	
		return -1;
	}

    if(fal_port_dot1d_state_set(0, port_index, dot1d_state) != SW_OK)
    {
		LOG_ERROR("set jssdk dot1d state fail");	
		return -1;
    }

	return 0;
}

// 获取MAC
const uint8 *RPP_OUT_get_mac (void)
{
    return (uint8 *)pRpp->mac;
}

// 发送报文
int RPP_OUT_tx_pdu (int port_index, unsigned char* pdu, size_t pdu_len)
{
    pdu_pkt_send(port_index,pdu,pdu_len);
	return 0;
}

// 添加定时器(秒->毫秒)
int RPP_OUT_set_timer(struct uloop_timeout *timer, uint8 sec)
{
    return timer_add(timer,(int)sec * 1000);   
}

int RPP_OUT_close_timer(struct uloop_timeout *timer)
{
    return timer_del(timer);
}

// 获取定时器值
uint8 RPP_OUT_get_time(rppRing_t *rppRing,TIMER_ID id)
{
    ring_t *ring = container_of(rppRing,ring_t,rpp_ring);

    if(id == AUTH_TIME)
        return ring->times.auth;
    if(id == BALLOT_TIME)
        return ring->times.ballot;
    if(id == FAIL_TIME)
        return ring->times.fail;
    if(id == HELLO_TIME)
        return ring->times.hello;

    return 0;
}

// 获取环id
uint16 RPP_OUT_get_ringid(rppRing_t *rppRing)
{
    ring_t *ring = container_of(rppRing,ring_t,rpp_ring);

    return ring->ring_id;
}

NODE_ID_T *RPP_OUT_get_nodeid(rppRing_t *rppRing)
{
    ring_t *ring = container_of(rppRing,ring_t,rpp_ring);
    return &ring->node_id;
}

int RPP_OUT_link_req(void)
{
    return br_get_config();
}

int RPP_OUT_led_set(int value)
{
    return ring_led_set(value);
}

/****************************************************************************************************
 *                                  API for CLI
 * *************************************************************************************************/
int CTL_enable_jrpp(int enable)
{
    pRpp->running = enable;

    if(enable == RPP_ENABLED)
    {
        if(get_bridge_name(pRpp->if_name))
        {
            LOG_ERROR("get bridge name fail!\n");
            return -1;
        }
        if(get_if_mac(pRpp->if_name,pRpp->mac))
        {
            LOG_ERROR("get mac address fail!\n");
            return -1;
        }
    }

	log_info("rpp is %s\n",(enable == RPP_ENABLED)?"enable":"disable");

	return 0;
}

int CTL_enable_ring(int ring_id, int enable) 
{
    if(pRpp->running != RPP_ENABLED)
    {
        log_info("rpp is disabled\n");
        return -1;
    }

    int r = 0;
    
    
    ring_t *pRing = rpp_find_ring(ring_id,1);
    if(pRing == NULL)
    {
		LOG_ERROR("can't find ring #%d\n", ring_id);
		return -1;
    }

    if(enable == RING_ENABLED)
    {
        if(pRing->rpp_ring.primary == NULL || pRing->rpp_ring.secondary == NULL)
        {
            LOG_ERROR("arguements for ring#%d is not completely\n", ring_id);
            return -1;
        }
	    r = RPP_IN_ring_enable(&pRing->rpp_ring);
    }
    else	
    { 
        r = RPP_IN_ring_disable(&pRing->rpp_ring);
    }

    if(r) 
    {
		LOG_ERROR("rpp ring#%d %s failed\n",ring_id,(enable == RING_ENABLED)?"enable":"disable");
		return -1;
	}
	
    pRing->enable = enable;

	log_info("ring#%d %s success\n",ring_id,(enable == RING_ENABLED)?"enable":"disable");

	return 0;
}

int CTL_add_ring(int ring_id) 
{
    if(rpp_find_ring(ring_id,1) != NULL)
    {
        LOG_ERROR("warning: ring #%d already added", ring_id);
        return -1;
    }

    ring_t *pRing = (ring_t *)calloc(1,sizeof(ring_t));
    list_add_tail(&pRing->node,&pRpp->ring);
    pRing->enable   = RING_DISABLED;
    pRing->ring_id  = ring_id;
    pRing->times.auth   = default_ring_cfg.auth_time;
    pRing->times.ballot = default_ring_cfg.ballot_time;
    pRing->times.fail   = default_ring_cfg.fail_time;
    pRing->times.hello  = default_ring_cfg.hello_time;
    pRing->node_id.prio = default_ring_cfg.node_priority;
    memcpy(pRing->node_id.addr,pRpp->mac,MAC_LEN);
    
    if(RPP_IN_ring_create(&pRing->rpp_ring))
    {
        LOG_ERROR("create rpp ring fail!\n");
        return -1;
    }
    
	log_info("add ring#%d OK\n",ring_id);
	return 0;
}

int CTL_del_ring(int ring_id) 
{
    ring_t *pRing = rpp_find_ring(ring_id,1);
    if(pRing == NULL)
    {
        LOG_ERROR("ring #%d is not exist", ring_id);
        return -1;
    }

    if(pRing->enable == RING_ENABLED)
    {
        LOG_ERROR("ring #%d must be closed before delete", ring_id);
        return -1;
    }

	return 0;
}

int CTL_add_ringport(int ring_id, int p_port, int s_port) 
{
    if(rpp_find_ring(p_port,0) || rpp_find_ring(s_port,0))
    {
		LOG_ERROR("port %d or %d has been used for rpp!\n", p_port,s_port);
		return -1;
    }

    ring_t *pRing = rpp_find_ring(ring_id,1);
    if(pRing == NULL)
    {
        LOG_ERROR("ring #%d is not exist", ring_id);
        return -1;
    }

    if(RPP_IN_port_create(&pRing->rpp_ring,p_port,s_port))
    {
        LOG_ERROR("rpp ring#%d add primary port %d and secondary port %d fail!\n",ring_id,p_port,s_port);
        return -1;
    }

	log_info("ring#%d add port success\n",ring_id);
	return 0;
}

int CTL_del_ringport(int ring_id) 
{
    ring_t *pRing = rpp_find_ring(ring_id,1);
    if(pRing == NULL)
    {
        LOG_ERROR("ring #%d is not exist", ring_id);
        return -1;
    }

    if(RPP_IN_port_delete(&pRing->rpp_ring))
    {
        LOG_ERROR("delete rpp ring#%d ports fail!\n",ring_id);
        return -1;
    }

	return 0;
}

int CTL_set_ring_config(int ring_id, RPP_RING_CFG_T *ring_cfg) 
{
    ring_t *pRing = rpp_find_ring(ring_id,1);
    if(pRing == NULL)
    {
		LOG_ERROR("can't find ring #%d", ring_id);
		return -1;
    }
    if(pRing->enable == RING_ENABLED)
    {
		LOG_ERROR("please close ring#%d before set parameter", ring_id);
		return -1;
    }

    if(ring_cfg->field_mask & RING_CFG_PRIO)
    {
        pRing->node_id.prio = ring_cfg->node_priority;
    }
    if(ring_cfg->field_mask & RING_CFG_FAIL)
    {
        pRing->times.fail   = ring_cfg->fail_time;
    }
    if(ring_cfg->field_mask & RING_CFG_BALLOT)
    {
        pRing->times.ballot = ring_cfg->ballot_time;
    }
    if(ring_cfg->field_mask & RING_CFG_HELLO)
    {
        pRing->times.hello  = ring_cfg->hello_time;
    }
    if(ring_cfg->field_mask & RING_CFG_AUTH)
    {
        pRing->times.auth   = ring_cfg->auth_time;
    }

	log_info("ring#%d set config success\n",ring_id);
	return 0;
}

int CTL_get_ring_config(int ring_id, RPP_RING_CFG_T *ring_cfg) 
{
	return 0;
}

int CTL_get_ring_state(int ring_id, RPP_RING_STATE_T *ring_state, RPP_PORT_STATE_T *p_port_state, RPP_PORT_STATE_T *s_port_state) 
{
    ring_t *pRing = rpp_find_ring(ring_id,1);
    if(pRing == NULL)
    {
		LOG_ERROR("can't find ring #%d", ring_id);
		return -1;
    }

    ring_state->state = pRing->enable;
    memcpy(&ring_state->node_id,&pRing->node_id,sizeof(NODE_ID_T));
	if(RPP_IN_ring_get_state(&pRing->rpp_ring, ring_state)) 
    {
		LOG_ERROR(" get ring state failed");
		return -1;
	}

	return 0;
}

int CTL_set_debug_level(int level)
{
	log_info("CTL_set_debug_level, level=%d\n", level);

	return 0;
}

