
#include "fota_analyze.h"


#ifndef DYNAMIC_MEMERY_SUPPORT
#define	FRM_DATA_SIZE		2048
static	u8_t				frm_data[FRM_DATA_SIZE];
static	FOTA_ANALYZE_S		fota_a;
#endif

static status_t frame_head_check(FOTA_ANALYZE_S *h);
static status_t frame_packet_check(FOTA_ANALYZE_S *h);
static status_t protocol_callback_set(FOTA_ANALYZE_S *h, u8_t ver);

void_t* fota_frame_create(MSG_POST *post)
{
	FOTA_ANALYZE_S		*h;
#ifdef DYNAMIC_MEMERY_SUPPORT
	h = (FOTA_ANALYZE_S *)malloc(sizeof(FOTA_ANALYZE_S));
	if (h == NULL)
		return NULL;
#else
	h = &fota_a;
#endif
	memset((void*)h, 0x00, sizeof(FOTA_ANALYZE_S));
	h->post = post;
#ifndef DYNAMIC_MEMERY_SUPPORT
	fota_a.frm_data = frm_data;
#endif
	return h;
}

void fota_frame_destroy(FOTA_ANALYZE_S *h)
{
	if (h == NULL)
		return;
#ifdef DYNAMIC_MEMERY_SUPPORT
	if (h->frm_data)
		free(h->frm_data);
	free(h);
#endif
	return;
}

status_t fota_analyze(FOTA_ANALYZE_S *h, u8_t *buf, s32_t size)
{
	s32_t		ret;
	s32_t		n;

	if (h == NULL)
		return ERROR_T;

	n = 0;
	
	while(n < size) {
		switch(h->step) {
			case 0:			//Frame header first bit
				if (buf[n] == 0xA0) {
					h->frm_head.head[0] = 0xA0;
					h->step++;
				}
				n++;
				break;
			case 1:			//Frame header second bit
				if (buf[n] == 0x02) {
					h->frm_head.head[1] = 0x02;
					h->head_size = sizeof(FRAME_HEADER_S);
					h->rd_len = 2;
					h->step++;
				}
				else if(buf[n] != 0xA0)
					h->step = 0;
				n++;
				break;
			case 2:			//Frame header packet read
				if ((size - n) < (h->head_size - h->rd_len)) {
					u8_t	*addr;
					addr = (u8_t *)(&(h->frm_head)) + h->rd_len;
					memcpy(addr, buf+n, size - n);
					h->rd_len += size - n;
					n = size;
					break;
				} else {
					u8_t	*addr;
					addr = (u8_t *)(&(h->frm_head)) + h->rd_len;
					memcpy(addr, buf+n, h->head_size - h->rd_len);
					n += h->head_size - h->rd_len;
					h->step++;
					break;
				}
				break;
			case 3:				//frame header packet sum 
				h->head_sum = buf[n];
				ret = frame_head_check(h);
				if (ret == ERROR_T)
					h->step = 0;
				else {
					h->step++;
					h->data_size = h->frm_head.data_len;
					h->rd_len = 0;
#ifdef DYNAMIC_MEMERY_SUPPORT
					if (h->frm_data)
						free(h->frm_data);
					h->frm_data = malloc(h->data_size);
					if (h->frm_data == NULL) {
						printf("it have not enough space\n");
						h->step = 0;
					}
#endif
				}
				n++;
				break;
			case 4:				//Frame data packet read=
				if ((size - n) < (h->data_size - h->rd_len)) {
					u8_t	*addr;
					addr = (u8_t *)(h->frm_data) + h->rd_len;
					memcpy(addr, buf+n, size - n);
					h->rd_len += size - n;
					n = size;
					break;
				} else {
					u8_t	*addr;
					addr = (u8_t *)(h->frm_data) + h->rd_len;
					memcpy(addr, buf+n, h->data_size - h->rd_len);
					h->step++;
					n += h->data_size - h->rd_len;
					break;
				}
				break;
			case 5:				//packet sum read			
				h->packet_sum = buf[n];
				n++;
				h->step++;
				break;
			case 6:				//packet end flag read
				h->eof = buf[n];
				frame_packet_check(h);
				h->step = 0;
				n++;
				break;
			default:
				h->step = 0;
				break;
		}
	}
	return OK_T;
}

static status_t frame_head_check(FOTA_ANALYZE_S *h)
{
	status_t	ret;
	u8_t		*addr;
	u32_t		i;
	u8_t		sum;
#ifdef ENDIAN_CHANGE
	h->frm_head.frame_index = ENDIAN_SHORT(h->frm_head.frame_index);
	h->frm_head.data_len = ENDIAN_LONG(h->frm_head.data_len);
#endif
	// check frame header sum 
	addr = (u8_t *)(&(h->frm_head));
	sum = 0;
	for (i = 0; i < sizeof(FRAME_HEADER_S); i++)
		sum += addr[i];
	if (sum != h->head_sum) {
		printf("Invalid Frame header check_sum\n");
		return ERROR_T;
	}
	// protocol analyze callback register	
	ret = protocol_callback_set(h, h->frm_head.protocoal_ver);
	if (ret != OK_T) {
		printf("Invalid Frame protocol version = %d\n", h->frm_head.protocoal_ver);
		return ERROR_T;
	}

	return OK_T;
}

static status_t frame_packet_check(FOTA_ANALYZE_S *h)
{
	u8_t	*addr;
	u32_t	i;
	u8_t	sum;
	// check frame data sum
	addr = (u8_t *)h->frm_data;
	sum = 0;
	for (i = 0; i < h->data_size; i++) 
		sum += addr[i];
	sum += h->head_sum*2;
	if (sum != h->packet_sum) {
		printf("Invalid Frame Data Check_Sum\n");
		return ERROR_T;
	}
	if (h->data_analyze == NULL)
		return ERROR_T;
	return h->data_analyze((void_t *)h);
}

static status_t protocol_callback_set(FOTA_ANALYZE_S *h, u8_t ver)
{
	switch(ver) {
		case 0x01:
			h->data_analyze = v1_data_analyze;
			return OK_T;
		default:
			return ERROR_T;
	}
	return ERROR_T;
}











