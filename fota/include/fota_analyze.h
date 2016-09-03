#ifndef _FOTA_ANALYZE_H_
#define _FOTA_ANALYZE_H_

#include "utypes.h"

#define DYNAMIC_MEMERY_SUPPORT		


#ifndef DYNAMIC_MEMERY_SUPPORT
#define memset(ptr, value, size)	do {								\
										s32_t	i;						\
										s8_t	*p = (s8_t *)ptr;		\
										for (i = 0; i < size; i++) {	\
											p[i] = value;				\
										}								\
									}while(0)

#define memcpy(dst, src, size)	do {									\
										s32_t	i;						\
										s8_t	*pd = (s8_t *)dst;		\
										s8_t	*ps = (s8_t *)src;		\
										for (i = 0; i < size; i++) {	\
											pd[i] = ps[i];				\
										}								\
									}while(0)
#endif

typedef status_t (*MSG_POST)(void_t *h);

typedef struct tagFrameHeaderS {
	u8_t				head[2];
	u8_t				FC;
	u8_t				Ecode;
	u16_t				frame_index;
	u8_t				protocoal_ver;
	u8_t				ACK;
	u32_t				data_len;
	u8_t				reserv[4];
}FRAME_HEADER_S;

typedef status_t (*DATA_ANALYZE_F)(void_t *h);

typedef struct tagFOTA_ANALYZE_S {
	u32_t				step;
	FRAME_HEADER_S		frm_head;
	u32_t				rd_len;
	u32_t				head_size;
	u32_t				data_size;
	u8_t				*frm_data;
	u8_t				head_sum;
	u8_t				packet_sum;
	u8_t				eof;
	DATA_ANALYZE_F		data_analyze;
	MSG_POST			post;
}FOTA_ANALYZE_S;

void_t* fota_frame_create(MSG_POST *post);
void fota_frame_destroy(FOTA_ANALYZE_S *h);
status_t fota_analyze(FOTA_ANALYZE_S *h, u8_t *buf, s32_t size);

status_t v1_data_analyze(void_t *d_s);
status_t v1_bootcheck(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_commsucess(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_commfail(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_dspapprun(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_moneyinfo(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_moneycheck(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_moneyfull(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_dspupgrade(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_upresult(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_readversion(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_versionresult(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_blackset(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_blackadd(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_blackremove(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_blackresult(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_ipset(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_ipresult(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_ipread(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_ipcurrent(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_display(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_dispresult(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_moneytime(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_codeback(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_bigpicstart(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_bigpicdata(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_bigpicend(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_versionget(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_upgraderesp(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_upgradeconf(FOTA_ANALYZE_S *h, u8_t *ptr);
status_t v1_moneysave(FOTA_ANALYZE_S *h, u8_t *ptr);

#endif







