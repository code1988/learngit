#include "comm.h"


static int BigImageRequest(void);
static int EtherIndication(void);
static int BlackListIndication(void);
static int UpgradeIndication(void);
static int VersionRequest(void);

int main(int argc, char* argv[])
{
	comm_server_init(7566);
	while(1) {
		char				value;
		value = getchar();
		switch(value) {
			case 'i':
				BigImageRequest();
				break;
			case 'u':
				UpgradeIndication();
				break;
			case 'b':
				BlackListIndication();
				break;
			case 's':
				EtherIndication();
				break;
			case 'e':
				comm_server_release();
				return 0;
			case 'v':
				VersionRequest();
				break;
			default:
				break;
		}
	}
	
	return 0;
}

static int BigImageRequest(void)
{
	unsigned int		i, datalen;
	unsigned char		header_sum;
	unsigned char		total_sum;
	unsigned short		cmd;		
	unsigned char		ptr[256];

	ptr[0] = 0xA0;				//FRAME HEADER
	ptr[1] = 0x02;				//FRAME HEADER
	ptr[2] = 0x80 | 0x01;		//FC
	ptr[3] = 0x03;				//ECODE
	ptr[4] = 0x01;				//FRAME INDEX
	ptr[5] = 0x80;				//FRAME INDEX
	ptr[6] = 0x01;				//PROTOCOL
	ptr[7] = 0x00;				//ACK
	datalen = 0x02;
	memcpy(ptr+8, &datalen, 4);
	memset(ptr+12, 0x00, 4);
	header_sum = 0;
	for (i = 0; i < 16; i++)
		header_sum += ptr[i];
	ptr[16] = header_sum;
	cmd = 0x350a;
	memcpy(ptr+17, &cmd, 2);
	total_sum = header_sum*2 + ptr[17] + ptr[18];
	ptr[19] = total_sum;
	ptr[20] = 0x16;
		
	comm_send_data(NULL, (s8_t *)ptr, 21);
	return 0;
}

static int EtherIndication(void)
{
	unsigned int		i, datalen;
	unsigned char		header_sum;
	unsigned char		total_sum;
	unsigned short		cmd;		
	unsigned char		ptr[256];

	ptr[0] = 0xA0;				//FRAME HEADER
	ptr[1] = 0x02;				//FRAME HEADER
	ptr[2] = 0x80 | 0x02;		//FC
	ptr[3] = 0x03;				//ECODE
	ptr[4] = 0x01;				//FRAME INDEX
	ptr[5] = 0x80;				//FRAME INDEX
	ptr[6] = 0x01;				//PROTOCOL
	ptr[7] = 0x00;				//ACK
	datalen = 0x02;
	memcpy(ptr+8, &datalen, 4);
	memset(ptr+12, 0x00, 4);
	header_sum = 0;
	for (i = 0; i < 16; i++)
		header_sum += ptr[i];
	ptr[16] = header_sum;
	cmd = 0x3301;
	memcpy(ptr+17, &cmd, 2);
	
	total_sum = header_sum*2 + ptr[17] + ptr[18];
	ptr[19] = total_sum;
	ptr[20] = 0x16;
		
	comm_send_data(NULL, (s8_t *)ptr, 21);
	return 0;
} 

static int BlackListIndication(void)
{
	return 0;
}

static int UpgradeIndication(void)
{
	SendUpgradeFile();
	return 0;
}

static int VersionRequest(void)
{
	unsigned int		i, datalen;
	unsigned char		header_sum;
	unsigned char		total_sum;
	unsigned short		cmd;		
	unsigned char	ptr[256];

	ptr[0] = 0xA0;				//FRAME HEADER
	ptr[1] = 0x02;				//FRAME HEADER
	ptr[2] = 0x80 | 0x01;		//FC
	ptr[3] = 0x03;				//ECODE
	ptr[4] = 0x01;				//FRAME INDEX
	ptr[5] = 0x80;				//FRAME INDEX
	ptr[6] = 0x01;				//PROTOCOL
	ptr[7] = 0x00;				//ACK
	datalen = 0x02;
	memcpy(ptr+8, &datalen, 4);
	memset(ptr+12, 0x00, 4);
	header_sum = 0;
	for (i = 0; i < 16; i++)
		header_sum += ptr[i];
	ptr[16] = header_sum;
	cmd = 0x3201;
	memcpy(ptr+17, &cmd, 2);
	total_sum = header_sum*2 + ptr[17] + ptr[18];
	ptr[19] = total_sum;
	ptr[20] = 0x16;
		
	comm_send_data(NULL, (s8_t *)ptr, 21);
	return 0;
}



