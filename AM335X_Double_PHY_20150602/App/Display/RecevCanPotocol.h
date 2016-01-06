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
#include "BaseDisp.h"
/* Private define-----------------------------------------------------------------------------*/
//UIA显示用
#define UIADISPLAY 0x10

//UIA显示的具体协议的定位
//#define UIARECVCANHEAD 0x01
//#define UIARECVCANINFO 0x03
#define UIACANLIST01 0x00
#define UIACANLIST02 0x01
#define UIACANLIST03 0x02
#define UIACANLIST04 0x03
#define UIACANLIST05 0x04
#define UIACANLIST06 0x05
#define UIACANLIST07 0x06
#define UIACANLIST08 0x07

//协议保留帧
#define DEFAULTBYTE 0xFF

//UIA发送给CAN的数据根据协议得ID号
#define UIACANID 0x10
#define UIACANMODE 0x01
#define UIACANBATCH 0x02
#define UIACANMOTORSET 0x03
#define UIACANAUTHENTICATION 0x04
#define UIACANAUTHENTICLEVEL1 0x05
#define UIACANAUTHENTICLEVEL2 0x06
#define UIACANCLEARLEVEL 0x07
#define UIACANMIXEDSEARCH 0x08
#define UIACANOUTLETSEARCH 0x09
#define UIACANINLERSSEARCH 0x0A
#define UIACANMAINMENU 0x0B
#define UIACANERRORCODE 0x0C
#define UIACANEDITION 0x0D
#define UIACANJCSPEED 0x0E
#define UIACANINLETAD 0x0F
#define UIACANIRAD    0x10
#define UIACANMGAD    0x11
#define UIACANUVAD    0x12
#define UIACANINFOAD  0x13
#define UIACANWAVEFORM  0x14
#define UIACANLEVELUPSTART  0xF0

//UIA发送给CAN的数据根据协议需要显示的总页码
#define UIACANPAGE0 0x00
#define UIACANPAGE1 0x01
#define UIACANPAGE2 0x02
#define UIACANPAGE3 0x03
#define UIACANPAGE7 0x07
#define UIACANPAGE8 0x08
#define UIACANAUTHENTICATIONTOTALPAGES 0x24
#define UIACANAUTHENTICLEVELTOTALPAGES 0x01
#define UIACANAUTHENTICSWITCHTOTALPAGES 0x02
#define UIACANCLEARPAGES 0x01

//UIA接收CAN数据的回复
#define UIACANRECEVOK   0x00
#define UIACANRECEVFAIL   0x01
#define UIACANMUSTREPLY 0x55

//马达调试命令
#define MCDEBUGOUTMODE 0x01
#define MCDEBUGJCSPEED 0x02
#define MCDEBUGOLDTEST 0x03
#define MCDEBUGINLETAD 0x04
#define MCDEBUGIRAD 0x05
#define MCDEBUGMGAD 0x06
#define MCDEBUGUVAD 0x07
#define MCDEBUGINFOAD 0x08
#define MCDEBUGCHECKCS 0x09
#define MCDEBUGWAVEFORM 0x0A

/* Private macro------------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
//UIA接收数据
//主菜单跑钞实时信息
void UIA_Recev_Main_Menu(INT8U *CanData);
//混点明细查询
void UIA_Recev_Mixed(INT8U *CanData);
//出钞口冠字号查询
void UIA_Recev_OUTLET(INT8U *CanData);
//接钞口冠字号查询
void UIA_Recev_INLET(INT8U *CanData);
//版本信息
void UIA_Recev_Edition(INT8U *CanData);
//马达速度调试
void UIA_Recev_JCSPEED(INT8U *CanData);
//回复帧
void UIA_Reply_CAN(INT8U *CanData);
//处理马达板的信息
void DealWithMC(INT8U *CanData);
//刷新主菜单
void RefreshMainMenu(void);
//进钞AD查看
void UIA_Recev_Inlet_AD(INT8U *CanData);
//红外AD采样
void UIA_Recev_IR_AD(INT8U *CanData);
//大小磁头采样
void UIA_Recev_MT_AD(INT8U *CanData);
//荧光AD采样
void UIA_Recev_UV_AD(INT8U *CanData);
//走钞信息
void UIA_Recev_Info_AD(INT8U *CanData);
//马达升级信息
void UIA_Recev_Level_Up(INT8U *CanData);
//MT数据显示
void UIA_Recev_Waveform(INT8U *CanData);
/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/
