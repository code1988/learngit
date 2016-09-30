
#ifndef _RPP_PORT_H_
#define _RPP_PORT_H_

#include <libubox/uloop.h>

#include "statmch.h"

#define F_MASK      0xFF

#define F_TX_AUTH_REQ       0x01    
#define F_TX_AUTH_TACK      0x02    
#define F_TX_BALLOT_ITTV    0x04    
#define F_TX_BALLOT_PSSV    0x08    
#define F_TX_FORWARDING     0x10     
#define F_TX_HELLO          0x20 
#define F_TX_COMPLETE       0x40 
#define F_TX_REPORT         0x80 

typedef struct {
    unsigned char f_auth;       // flag for auth machine
    unsigned char f_ballot;     // flag for ballot machine
    unsigned char f_tx;         // flag for transmit machine
    void *opt;                  // optional parameter used for deliver data
}mach_flag_t;

typedef struct rppPort{
    unsigned char   port_index;
    PORT_ROLE_T     port_role;
    BALLOT_ID_T     ballot_id; 
    ePortLinkState  link_st;    
    ePortStpState   stp_st;
    unsigned char   auth_count;  
    mach_flag_t     flag;       
    RPP_PDU_T       fwding;     // buffer used for forwarding packet

    struct uloop_timeout auth_timer;
    struct uloop_timeout ballot_timer;

    unsigned char neighber_vaild;
    unsigned char neighber_mac[MAC_LEN];
    unsigned char neighber_port;

    STATE_MACH_T *auth;         
    STATE_MACH_T *ballot;       
    STATE_MACH_T *transmit;     
    STATE_MACH_T *machines;
    
    struct rppRing *owner;     
}rppPort_t;

int RPP_port_get_state(rppPort_t *,RPP_PORT_STATE_T *);
int RPP_port_rx_pdu (rppPort_t *, RPP_PDU_T *);
void RPP_port_link_change(rppPort_t *,ePortLinkState);
int RPP_port_start(rppPort_t *);
int RPP_port_stop(rppPort_t *);
rppPort_t *port_get_peer(rppPort_t *);
rppPort_t *port_get_owner(struct rppRing *,unsigned char);
NODE_ID_T *port_min_id(NODE_ID_T *,NODE_ID_T *);
void port_forwarding_bydir(rppPort_t *, RPP_PDU_T *,NODE_ID_T *);
void port_ballot_fwd(rppPort_t *, RPP_PDU_T *);
int port_set_stp(rppPort_t *,ePortStpState);

#ifdef RPP_DBG
int RPP_port_trace_state_machine (PORT_T* this, char* mach_name, int enadis, int ring_id);
#endif

#endif

