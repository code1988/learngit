#ifndef _MACHINE_H_
#define _MACHINE_H_

void RPP_node_enter_state(STATE_MACH_T* this);
Bool RPP_node_check_conditions(STATE_MACH_T* this);
char *RPP_node_get_state_name(int);
void fail_timer_handler(struct uloop_timeout *timer);
void hello_timer_handler(struct uloop_timeout *timer);

void RPP_auth_enter_state(STATE_MACH_T* this);
Bool RPP_auth_check_conditions(STATE_MACH_T* this);
char *RPP_auth_get_state_name(int);
void auth_timer_handler(struct uloop_timeout *timer);
int auth_packet_parse(rppPort_t *this, RPP_PDU_T *pdu);

void RPP_ballot_enter_state(STATE_MACH_T* this);
Bool RPP_ballot_check_conditions(STATE_MACH_T* this);
char *RPP_ballot_get_state_name(int);
Bool RPP_ballot_force(STATE_MACH_T* this);
void ballot_timer_handler(struct uloop_timeout *timer);
int ballot_packet_parse(rppPort_t *this, RPP_PDU_T *pdu);
Bool ballot_if_fin(rppPort_t *port);

void RPP_transmit_enter_state (STATE_MACH_T* this);  
Bool RPP_transmit_check_conditions (STATE_MACH_T* this);
char* RPP_transmit_get_state_name (int state);
void RPP_transmit_balllot_patch(Bool force);

#define tx_auth_req(p)      (p->flag.f_tx |= F_TX_AUTH_REQ)
#define tx_auth_ack(p)      (p->flag.f_tx |= F_TX_AUTH_TACK)
#define tx_ballot_ittv(p,v) {p->flag.f_tx |= F_TX_BALLOT_ITTV;RPP_transmit_balllot_patch(v);}
#define tx_ballot_pssv(p)   (p->flag.f_tx |= F_TX_BALLOT_PSSV)
#define tx_forwarding(p)    (p->flag.f_tx |= F_TX_FORWARDING)
#define tx_report(p)        (p->flag.f_tx |= F_TX_REPORT)
#define tx_hello(p)         (p->flag.f_tx |= F_TX_HELLO)
#define tx_complete(p)      (p->flag.f_tx |= F_TX_COMPLETE)

#define RPP_machine_get_state(name,addr) RPP_ ## name ## _get_state_name(addr->state)
#endif
