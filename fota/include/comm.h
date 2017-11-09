#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_
#include "utility.h"
#include "fota_analyze.h"
#include "xbuffer.h"

#define	SEND_BUFFER_MAX_SIZE		20*1024
#define RECV_BUFFER_MAX_SIZE		20*1024

typedef struct tagSocketS {
	struct tagSocketS		*prev;
	struct tagSocketS		*next;
	s32_t				sock;
	XBUFFER_S			*sbuf;
	bool_t				b_start;
	FOTA_ANALYZE_S			*fota;
	s32_t				pay_pos;
	s32_t				pay_end;
	s32_t				log_pos;
	s32_t				log_end;
}SOCKET_S;

typedef struct tagServerS {
	s32_t				listen_sock;
	u16_t				listen_port;
	u16_t				reserve;
	void_t				*hThread;
	bool_t				b_loop;
	void_t				*hCtrl;
	bool_t				b_ctrl;
	void_t				*hMutex;
	SOCKET_S			*header;
}SERVER_S;

status_t comm_server_init(u16_t port);
status_t comm_server_release(void);
status_t comm_send_data(void_t* h, s8_t *ptr, s32_t size);





#endif //_COMMUNICATION_H_

