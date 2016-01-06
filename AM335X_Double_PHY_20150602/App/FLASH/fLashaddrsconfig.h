/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: fLashaddrsconfig.h
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
/* Private define-----------------------------------------------------------------------------*/
//Flash存储器每页字节数量
#define FLASH_PAGE_BYTES			(MAXPAGELEN/2)
//分配flash地址配置
#define    CSCHECK_LEN                 1		  //预留校验位1个字节长度



//1、FLASH的起始地址（UI部分，从2K开始，一共1M，暂定）
#define FLASH_UI_PARA_ADDR         	(0x800)
#define FLASH_UI_PARA_LEN       	0

/*---------------AT45DB中各类数据存储线性地址管理配置--------------------*/
#define    PARA_1001_FLASHADDR              (FLASH_UI_PARA_ADDR+FLASH_UI_PARA_LEN)
#define    PARA_1001_FLASHLEN               3
#define    PARA_1001_ID_SAVE_BAT_VAL        0X1001

#define    PARA_1002_FLASHADDR              (PARA_1001_FLASHADDR+PARA_1001_FLASHLEN+CSCHECK_LEN)
#define    PARA_1002_FLASHLEN               6
#define    PARA_1002_ID_FAKE_LEVEL_DATA     0X1002

#define    PARA_1003_FLASHADDR              (PARA_1002_FLASHADDR+PARA_1002_FLASHLEN+CSCHECK_LEN)
#define    PARA_1003_FLASHLEN               7
#define    PARA_1003_ID_FZ_LEVEL_DATA       0X1003

#define    PARA_1004_FLASHADDR              (PARA_1003_FLASHADDR+PARA_1003_FLASHLEN+CSCHECK_LEN)
#define    PARA_1004_FLASHLEN               4
#define    PARA_1004_ID_LOCAL_IP            0X1004

#define    PARA_1005_FLASHADDR              (PARA_1004_FLASHADDR+PARA_1004_FLASHLEN+CSCHECK_LEN)
#define    PARA_1005_FLASHLEN               4
#define    PARA_1005_ID_LOCAL_MK            0X1005

#define    PARA_1006_FLASHADDR              (PARA_1005_FLASHADDR+PARA_1005_FLASHLEN+CSCHECK_LEN)
#define    PARA_1006_FLASHLEN               4
#define    PARA_1006_ID_LOCAL_GW            0X1006

#define    PARA_1007_FLASHADDR              (PARA_1006_FLASHADDR+PARA_1006_FLASHLEN+CSCHECK_LEN)
#define    PARA_1007_FLASHLEN               6
#define    PARA_1007_ID_LOCAL_MAC           0X1007

#define    PARA_1008_FLASHADDR              (PARA_1007_FLASHADDR+PARA_1007_FLASHLEN+CSCHECK_LEN)
#define    PARA_1008_FLASHLEN               2
#define    PARA_1008_ID_LOCAL_PORT          0X1008

#define    PARA_1009_FLASHADDR              (PARA_1008_FLASHADDR+PARA_1008_FLASHLEN+CSCHECK_LEN)
#define    PARA_1009_FLASHLEN               4
#define    PARA_1009_ID_SERVER_IP           0X1009

#define    PARA_100A_FLASHADDR              (PARA_1009_FLASHADDR+PARA_1009_FLASHLEN+CSCHECK_LEN)
#define    PARA_100A_FLASHLEN               2
#define    PARA_100A_ID_SERVER_PORT         0X100A

#define    PARA_100B_FLASHADDR              (PARA_100A_FLASHADDR+PARA_100A_FLASHLEN+CSCHECK_LEN)
#define    PARA_100B_FLASHLEN               1
#define    PARA_100B_ID_NET_SWITCH          0X100B

#define    PARA_100C_FLASHADDR              (PARA_100B_FLASHADDR+PARA_100B_FLASHLEN+CSCHECK_LEN)
#define    PARA_100C_FLASHLEN               1
#define    PARA_100C_ID_NET_PROTOCOL_TYPE   0X100C

#define    PARA_100D_FLASHADDR              (PARA_100C_FLASHADDR+PARA_100C_FLASHLEN+CSCHECK_LEN)
#define    PARA_100D_FLASHLEN               1
#define    PARA_100D_ID_NET_PROTOCOL_MODE   0X100D

#define    PARA_100E_FLASHADDR              (PARA_100D_FLASHADDR+PARA_100D_FLASHLEN+CSCHECK_LEN)
#define    PARA_100E_FLASHLEN               40
#define    PARA_100E_ID_NET_STOP_TIME       0X100E

#define    PARA_100F_FLASHADDR              (PARA_100E_FLASHADDR+PARA_100E_FLASHLEN+CSCHECK_LEN)
#define    PARA_100F_FLASHLEN               2
#define    PARA_100F_ID_SD_REMAIN_SIZE_THR  0X100F

#define    PARA_1010_FLASHADDR              (PARA_100F_FLASHADDR+PARA_100F_FLASHLEN+CSCHECK_LEN)
#define    PARA_1010_FLASHLEN               1
#define    PARA_1010_ID_SD_ALARM_EN         0X1010

#define    PARA_1011_FLASHADDR              (PARA_1010_FLASHADDR+PARA_1010_FLASHLEN+CSCHECK_LEN)
#define    PARA_1011_FLASHLEN               1
#define    PARA_1011_ID_SD_ALARM_TIME       0X1011

#define    PARA_1012_FLASHADDR              (PARA_1011_FLASHADDR+PARA_1011_FLASHLEN+CSCHECK_LEN)
#define    PARA_1012_FLASHLEN               4
#define    PARA_1012_ID_SD_SEND_OK_DAY      0X1012

#define    PARA_1013_FLASHADDR              (PARA_1012_FLASHADDR+PARA_1012_FLASHLEN+CSCHECK_LEN)
#define    PARA_1013_FLASHLEN               4
#define    PARA_1013_ID_SD_SEND_ERR_DAY     0X1013

#define    PARA_1014_FLASHADDR              (PARA_1013_FLASHADDR+PARA_1013_FLASHLEN+CSCHECK_LEN)
#define    PARA_1014_FLASHLEN               270
#define    PARA_1014_ID_AUTHENTICATIO_PARA  0X1014

#define    PARA_1015_FLASHADDR              (PARA_1014_FLASHADDR+PARA_1014_FLASHLEN+CSCHECK_LEN)
#define    PARA_1015_FLASHLEN               200
#define    PARA_1015_ID_BLACK_LIST          0X1015

/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/


