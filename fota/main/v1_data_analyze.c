
#include "fota_analyze.h"

typedef status_t (*FOTA_CALLBACK_F)(FOTA_ANALYZE_S *h, u8_t *ptr);

typedef struct tagFOTA_DATA_S {
		s16_t				id;
		FOTA_CALLBACK_F		func;
}FOTA_DATA_S;

#define COMMAND_ID_NUM			26

static FOTA_DATA_S	fota_data[] = {
									{0x10BB, v1_commsucess},
									{0x10B0, v1_commfail},
									{0x2001, v1_moneyinfo},
									{0x2013, v1_moneycheck},
									{0x2014, v1_moneyfull},
									{0x1006, v1_upresult},
									{0x2001, v1_readversion},
									{0x2002, v1_versionresult},
									{0x2010, v1_blackset},
									{0x2011, v1_blackadd},
									{0x2012, v1_blackremove},
									{0x2020, v1_ipset},
									{0x2021, v1_ipresult},
									{0x2022, v1_ipread},
									{0x2023, v1_ipcurrent},
									{0x2030, v1_display},
									{0x2031, v1_dispresult},
									{0x2040, v1_moneytime},
									{0x2041, v1_codeback},
									{0x3201, v1_versionget},
									{0x3101, v1_upgraderesp},
									{0x3102, v1_upgradeconf},
									{0x3503, v1_moneysave},
									{0x3506, v1_bigpicstart},
									{0x3507, v1_bigpicdata},
									{0x3508, v1_bigpicend},
};

static int		dsp_boot = 0;		

status_t v1_data_analyze(void_t *d_s)
{
	s16_t			ID;
	s32_t			i;
	FOTA_ANALYZE_S	*h;
	h = (FOTA_ANALYZE_S *)d_s;
	memcpy(&ID, h->frm_data, 2);
#ifdef ENDIAN_CHANGE
	ID = ENDIAN_SHORT(ID);
#endif
	for (i = 0; i < COMMAND_ID_NUM; i++) {
		if (fota_data[i].id != ID)
			continue;
		return fota_data[i].func(h, h->frm_data + 2);
	}
	return ERROR_T;
}

status_t v1_commsucess(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	dsp_boot = 1;
	return OK_T;
}

status_t v1_commfail(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	dsp_boot = 0;	
	return OK_T;
}

status_t v1_moneyinfo(FOTA_ANALYZE_S *h, u8_t *ptr)
{	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_moneycheck(FOTA_ANALYZE_S *h, u8_t *ptr)
{	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_moneyfull(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_dspupgrade(FOTA_ANALYZE_S *h, u8_t *ptr)
{	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_upresult(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_readversion(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_versionresult(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_blackset(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_blackadd(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_blackremove(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_blackresult(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_ipset(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_ipresult(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_ipread(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_ipcurrent(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_display(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_dispresult(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_moneytime(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_codeback(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	printf("%s\n", __func__);
	return OK_T;
}

status_t v1_moneysave(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	printf("%s\n", __func__);
	return OK_T;
}

static FILE		*ifd = NULL;

status_t v1_bigpicstart(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	char				buf[256];
	char				buff[2000];
	unsigned int		datalen, i;

	ifd = fopen("./big.bmp", "wb");
	buf[0] = h->frm_head.head[0];
	buf[1] = h->frm_head.head[1];
	buf[2] = h->frm_head.FC & 0x1F | 0x80;
	buf[3] = h->frm_head.Ecode;
	memcpy(buf + 4, &(h->frm_head.frame_index), 2);
	buf[6] = h->frm_head.protocoal_ver;
	buf[7] = 0x00;
	datalen = 2;
	memcpy(buf+8, &datalen, 4);
	memcpy(buf + 12, h->frm_head.reserv, 4);
	buf[16] = 0;
	for (i = 0; i < 16; i++)
		buf[16] += buf[i];
	buf[17] = 0x06;
	buf[18] = 0x35;
	buf[19] = buf[16] * 2 + buf[17] + buf[18];
	buf[20] = 0x16;
	comm_send_data(NULL, (s8_t *)buf, 21);
	return OK_T;
}

status_t v1_bigpicdata(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	char				buf[256];
	unsigned int		datalen, i;
	printf("%s\n", __func__);
	buf[0] = h->frm_head.head[0];
	buf[1] = h->frm_head.head[1];
	buf[2] = h->frm_head.FC & 0x1F | 0x80;
	buf[3] = h->frm_head.Ecode;
	memcpy(buf + 4, &(h->frm_head.frame_index), 2);
	buf[6] = h->frm_head.protocoal_ver;
	buf[7] = 0x00;
	datalen = 2;
	memcpy(buf+8, &datalen, 4);
	memcpy(buf + 12, h->frm_head.reserv, 4);
	buf[16] = 0;
	for (i = 0; i < 16; i++)
		buf[16] += buf[i];
	buf[17] = 0x07;
	buf[18] = 0x35;
	buf[19] = buf[16] * 2 + buf[17] + buf[18];
	buf[20] = 0x16;
	comm_send_data(NULL, (s8_t *)buf, 21);
	return OK_T;
}

status_t v1_bigpicend(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	char				buf[256];
	unsigned int		datalen, i;
	printf("%s\n", __func__);
	buf[0] = h->frm_head.head[0];
	buf[1] = h->frm_head.head[1];
	buf[2] = h->frm_head.FC & 0x1F | 0x80;
	buf[3] = h->frm_head.Ecode;
	memcpy(buf + 4, &(h->frm_head.frame_index), 2);
	buf[6] = h->frm_head.protocoal_ver;
	buf[7] = 0x00;
	datalen = 2;
	memcpy(buf+8, &datalen, 4);
	memcpy(buf + 12, h->frm_head.reserv, 4);
	buf[16] = 0;
	for (i = 0; i < 16; i++)
		buf[16] += buf[i];
	buf[17] = 0x08;
	buf[18] = 0x35;
	buf[19] = buf[16] * 2 + buf[17] + buf[18];
	buf[20] = 0x16;
	comm_send_data(NULL, (s8_t *)buf, 21);
	return OK_T;
}

status_t v1_versionget(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	unsigned int		i;
	printf("%s\n", __func__);
	for (i = 0; i < h->frm_head.data_len - 2; i++) {
		printf("0x%02x, ", ptr[i]);
		if ((i%8) == 7)
			printf("\r\n");
	}
	printf("\r\n");
	return OK_T;
}

#define			UP_SIZE			2048

static FILE		*ufd = NULL;
static int		up_index = 0;
static int		up_total = 0;
static int		up_section = 0;

int SendUpgradeFile(void)
{
	unsigned char	buf[4096];
	unsigned char		header_sum;
	unsigned char		total_sum;
	unsigned short		cmd;
	int				datalen;		
	int			len, size, n, i;
	if (ufd == NULL) {
		ufd = fopen("cny.bmp", "rb");
		if (ufd == NULL)
			return -1;
		fseek(ufd, 0, SEEK_END);
		up_total = ftell(ufd);
		printf("up_total = %d\n", up_total);
		up_index = 0;
		up_section = (up_total + UP_SIZE - 1)/UP_SIZE;
		fseek(ufd, 0, SEEK_SET);
	}
	if ((up_index * UP_SIZE) >= up_total) {
		fclose(ufd);
		return 0;
	}
	buf[0] = 0xA0;				//FRAME HEADER
	buf[1] = 0x02;				//FRAME HEADER
	buf[2] = 0x80 | 0x02;		//FC
	buf[3] = 0x03;				//ECODE
	buf[4] = 0x01;				//FRAME INDEX
	buf[5] = 0x80;				//FRAME INDEX
	buf[6] = 0x01;				//PROTOCOL
	buf[7] = 0xFF;				//ACK
	if ((up_total - up_index*UP_SIZE) > UP_SIZE)
		size = UP_SIZE;
	else
		size = up_total - up_index*UP_SIZE;
	datalen = 19 + size;
	memcpy(buf+8, &datalen, 4);
	memset(buf+12, 0x00, 4);
	header_sum = 0;
	for (i = 0; i < 16; i++)
		header_sum += buf[i];
	buf[16] = header_sum;
	cmd = 0x3101;
	memcpy(buf+17, &cmd, 2);
	buf[19] = 3;
	memcpy(buf + 20, &up_section, 4);
	memcpy(buf + 24, &up_total, 4);
	len = up_index + 1;
	memcpy(buf + 28, &len, 4);
	memcpy(buf + 32, &size, 4);
	fseek(ufd, up_index * UP_SIZE, SEEK_SET);
	n = size;
	while(n) {
		int		ret;
		ret = fread(buf + 36 + size - n, 1, n, ufd);
		printf("ret = %d\n", ret);
		if (ret <= 0)
			break;				//?????
		n -= ret;
	}
	if (n != 0)
		printf("send data error\n");
	total_sum = 0;
	for (i = 0; i < size + 19; i++)
		total_sum += buf[17+i];
	total_sum += header_sum*2;
	buf[size + 36] = total_sum;
	buf[size + 37] = 0x16;
	printf("0x%02x, ", buf[36]);
	comm_send_data(NULL, (s8_t *)buf, size + 17 + 19 + 2);
	return 0;
}

status_t v1_upgraderesp(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	unsigned int		i;
	
//	printf("%s\n", __func__);
	up_index++;
//	printf("up_index = %d\n", up_index);
	SendUpgradeFile();
	return 0;
}


status_t v1_upgradeconf(FOTA_ANALYZE_S *h, u8_t *ptr)
{
	printf("%s\n", __func__);
	return 0;
}











