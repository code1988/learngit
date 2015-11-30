#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <sys/types.h>
#include <sys/stat.h>
#include "utility.h"
#include "xbuffer.h"

#define MONITOR_MAX_LISTEN			5
#define	SEND_BUFFER_MAX_SIZE		100*1024
#define RECV_BUFFER_MAX_SIZE		100*1024

typedef struct tagAnalyzeS {
	void_t* (*create)(void);
	status_t (*destroy)(void_t* h);
	status_t (*analyze)(void_t* h, u8_t* ptr, s32_t size);
}ANALYZE_S;

typedef struct tagSocketS {
	struct tagSocketS	*prev;
	struct tagSocketS	*next;
	s32_t				sock;
	XBUFFER_S			*sbuf;
	void_t				*hd;
}SOCKET_S;

typedef struct tagMonitorS {
	s32_t				listen_sock;
	u16_t				listen_port;
	u16_t				reserve;
	void_t				*hThread;
	bool_t				b_loop;
	void_t				*hMutex;
	SOCKET_S			*header;
	ANALYZE_S			*hAnalyze;
}MONITOR_S;

typedef struct tagMonitorHeaderS {
	u8_t			sync[2];
	u16_t			cmd;
	u32_t			id;
	u16_t			datalen;
	u8_t			index;
	u8_t			headsum;
}MONITOR_HEADER_S;

typedef struct tagMonitorAnalyzeS {
	u32_t				step;
	MONITOR_HEADER_S	frm_head;
	u32_t				rd_len;
	u8_t				*frm_data;
	u8_t				packet_sum;
	status_t			(*data_analyze)(void_t *h, void_t *args);
	bool_t				moneyinfo_b;
	bool_t				addata_b;
	bool_t				staticinfo_b;
	bool_t				dynamicinfo_b;
	bool_t				image_b;
}MONITOR_ANALYZE_S;

#define MONEYINFO_SEND_F		0x10
#define ADDATA_SEND_F			0x20
#define STATIC_SEND_F			0x21
#define DYNAMIC_SEND_F			0x22


status_t monitor_initialize(void);
status_t monitor_release(void);
int monitor_init_f(u16_t port, ANALYZE_S *h);
int monitor_release_f(void);
status_t mv1_data_analyze(void_t *d_s, void_t *hsock);
#endif	//_MONITOR_H_


