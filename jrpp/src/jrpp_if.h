
#ifndef _JRPP_IF_H_
#define _JRPP_IF_H_

int br_init_ops(void);
int rpp_init(void);
int br_get_config(void);
int br_set_state(int ifindex, int brstate);
void br_pdu_rcv(unsigned char *data, int len);
void br_notify(int br_index, int if_index, int newlink, unsigned flags);
void sig_kill_handler(int signo);
#endif


