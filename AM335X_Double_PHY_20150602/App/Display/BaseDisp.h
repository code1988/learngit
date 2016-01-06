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
/* Private define-----------------------------------------------------------------------------*/
#define DISPBUFF_LENGTH     500u

#define FRAMESTARTFIXEDLEN 16
#define FRAMEFIXEDLEN 22

//数据高位地位
#define BIGNUM      6
#define LITTLENUM   7
//运行过程中出错信息的起始长度
#define ERRORCODEMESSAGELEN 15
//运行过程中出错提示的起始长度
#define ERRORMESSAGELEN 6
//开机过程总出错信息的起始长度
#define STARTERRORMESSAGELEN 6
//查看的数据位的起始长度
#define ERRDETIALMESSAGELEN 6

//查看菜单每一页报警代码的首地址
#define DETAILERRORSTARTADDRESSPAGE1 0x0020
#define DETAILERRORSTARTADDRESSPAGE2 0x003E
#define DETAILERRORSTARTADDRESSPAGE3 0x0A85
#define DETAILERRORSTARTADDRESSPAGE4 0x0AA3
#define DETAILERRORSTARTADDRESSPAGE5 0x0AC1
#define DETAILERRORSTARTADDRESSPAGE6 0x0ADF
#define DETAILERRORSTARTADDRESSPAGE7 0x0AFD
#define DETAILERRORSTARTADDRESSPAGE8 0x0B1B
#define DETAILERRORSTARTADDRESSPAGE9 0x0B39
#define DETAILERRORSTARTADDRESSPAGE10 0x0B57
#define DETAILERRORSTARTADDRESSPAGE11 0x0B75
#define DETAILERRORSTARTADDRESSPAGE12 0x0B93
#define DETAILERRORSTARTADDRESSPAGE13 0x0BB1
#define DETAILERRORSTARTADDRESSPAGE14 0x0BCF
#define DETAILERRORSTARTADDRESSPAGE15 0x0BED
#define DETAILERRORSTARTADDRESSPAGE16 0x0C0B
#define DETAILERRORSTARTADDRESSPAGE17 0x0C29
#define DETAILERRORSTARTADDRESSPAGE18 0x0C47
#define DETAILERRORSTARTADDRESSPAGE19 0x0C65
#define DETAILERRORSTARTADDRESSPAGE20 0x0C83
#define DETAILERRORSTARTADDRESSPAGE21 0x0CA1
#define DETAILERRORSTARTADDRESSPAGE22 0x0CBF

//查看菜单报警代码的数据长度
#define DETAILERRORCODELEN 0x03

//查看菜单报警代码的数据长度
#define DETAILERRORMESSAGELEN 0x10

#define COMMONHEAD1 0x5A
#define COMMONHEAD2 0xA5

#define MCCOMMONHEAD1 0xA0
#define MCCOMMONHEAD2 0x02
#define MCCOMMONHEAD3 0x55
#define MCCOMMONHEAD4 0xDD
#define MCCOMMONHEAD5 0x02

#define MCFRAMENUM 0x00
#define MCFUNCTIONCODE 0x00
#define MCVERSION1 0x00
#define MCNECESSARYACK 0xFF
#define MCUNNECESSARYACK 0xBB
#define MCREMARK 0x00

#define POS_ARMDISP_CMD_HEAD 0
#define POS_ARMDISP_CMD_LEN 2
#define POS_ARMDISP_CMD_ORDER 3
#define POS_ARMDISP_CMD_ADDRESS 4
#define POS_ARMDISP_CMD_VARIABLE 6

#define POS_ARMTOMC_CMD_HEAD 0
#define POS_ARMTOMC_CMD_LEN 7
#define POS_ARMTOMC_CMD_FUNCTIONNUM 11
#define POS_ARMTOMC_CMD_VERSION 14
#define POS_ARMTOMC_CMD_REMARK 16
#define POS_ARMTOMC_CMD_DATA 16

#define COUNTMODE 0x00
#define INTELLIGENTMODE 0x01
#define MIXEDMODE 0x02
#define EDITIONMODE 0x03
#define ATMMODE 0x04
#define COMPLEXMODE 0x05

#define INMAINMENU 0x00
#define OUTMAINMENU 0x01

#define MTENABLE 0x00
#define MTDISABLE 0x01

#define KEYENABLE 0x00
#define KEYDISABLE 0x01
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/

/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/


