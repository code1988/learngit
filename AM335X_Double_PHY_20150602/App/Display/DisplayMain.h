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
#include "BaseDisp.h"
/* Private define-----------------------------------------------------------------------------*/
#define VALUE1 1
#define VALUE2 2

#define PAGE1 0x0A
#define PAGE2 0x0B
#define PAGE3 0x0C

#define TOTALNUM1 0x08
#define TOTALNUM2 0x07

#define MONEYNUM1 0x03 
#define MONEYNUM2 0x05

#define XSINLE1 1
#define XSINLE2 2

#define DEPOSITMODE 0x00
#define WITHDRAWMODE 0x01

#define SUMMATIONON 0x01
#define SUMMATIONOFF 0x00

#define NETABL 0x01
#define NETUNABL 0x00

//���˵�֡�Ľ���
#define DENOMHIGHBYTE 0x00
#define DENOMLOWBYTE 0x01
#define OUTLETHIGHBYTE 0x02
#define OUTLETLOWBYTE 0x03
#define INLETHIGHBYTE 0x04
#define INLETLOWBYTE 0x05
#define TOTALNUM1BYTE 0x06
#define TOTALNUM2BYTE 0x07
#define TOTALNUM3BYTE 0x08
#define TOTALNUM4BYTE 0x09
#define AMOUNTHIGHBYTE 0x0A
#define AMOUNTMIDDLEBYTE 0x0B
#define AMOUNTLOWBYTE 0x0C
//���˵�ͼ�����ʾ
#define IMAGEICON1 0x64
#define IMAGEICON2 0x65
#define KEYBOARDIMAGE 0x0E
#define KEYBOARDACRTIVE 0x67
#define BLACKLISTMARK 0x18
#define BLACKLISTBACK 0x17

/* Private macro------------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
//�˵��еķ�̩���������˵���غ���
void  ParperMainMenu1(void);   

/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/


