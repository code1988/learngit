/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: main.c
* Author			:
* Date First Issued	: 130722
* Version			: V
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
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
#define APP_LOAD_ADDR	0x80000			// nand中app存储位置
#define BOOT_LOAD_ADDR	0x00000			// nand中boot存储位置

#define APP_SAVE_DAT    0x01
#define APP_SD_VOLUM    0x02

#define RECV_DAT_LENS   sizeof(TypeStruct_RecvFormat)       // 接收一张纸币的数据长度,198字节
#define DAT_HEAD_LENS   sizeof(TypeStruct_DatHeadFromat)    // Dat文件头长度，65字节
// #define sizeof(TypeStruct_UnitAttribute)

#define MAX_LIST_NUM	1024*10		    // list最多支持的文件数

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


/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/


