/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: main.c
* Author			:
* Date First Issued	: 130722
* Version			: V
* Description		:
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2013	        : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "net_server.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
#define APP_LOAD_ADDR	0x80000			// nand��app�洢λ��
#define BOOT_LOAD_ADDR	0x00000			// nand��boot�洢λ��

#define APP_SAVE_DAT    0x01
#define APP_SD_VOLUM    0x02

#define RECV_DAT_LENS   sizeof(TypeStruct_RecvFormat)       // ����һ��ֽ�ҵ����ݳ���,198�ֽ�
#define DAT_HEAD_LENS   sizeof(TypeStruct_DatHeadFromat)    // Dat�ļ�ͷ���ȣ�65�ֽ�
// #define sizeof(TypeStruct_UnitAttribute)

#define MAX_LIST_NUM	1024*10		    // list���֧�ֵ��ļ���

extern OS_EVENT *MMCSDEvent;
extern void *MMCSDOSQ[4];
extern DAT_FILE_BUF *p_dat_file_buf_data;

/* Private function prototypes----------------------------------------------------------------*/

#if FZ1500
void DSPBOOT_LOAD_FROM_SD(void);
void FPGABOOT_LOAD_FROM_SD(void);
void CNYBOOT_LOAD_FROM_SD(void);
void MTARM_LOAD_FROM_SD(void);
#endif


/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/


