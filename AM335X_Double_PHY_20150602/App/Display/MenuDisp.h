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
#include "display.h"
/* Private define-----------------------------------------------------------------------------*/
#define MENU_X_START  10    //�˵���ʾx�ᶨλ���ĸ��ַ�λ��ʼ
#define MENU_Y_START  10    //�˵���ʾy�ᶨλ�����п�ʼ
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
//������2�������ݲ���ʱ1ms����
void Uart2Post_delay10ms(INT8U num,const INT8U *pData,INT16U len);
//������2�������ݲ���ʱ10ms����
void Uart2Post_delay20ms(INT8U num,const INT8U *pData,INT16U len);
//��MC��������
void SendMCHandshake(INT8U *DisplayTxBuff,INT8U len);
//��Key��������
void SendKeyHandshake(INT8U *DisplayTxBuff);
/* Private function prototypes -----------------------------------------------*/
ST_Menu gsMenu0;
ST_Menu gsMenu0_1;
ST_Menu gsMenu0_2;
ST_Menu gsMenu0_3;
ST_Menu gsMenu0_4;
ST_Menu gsMenu0_5;
ST_Menu gsMenu0_6;
ST_Menu gsMenu0_7;
ST_Menu gsMenu0_8;
ST_Menu gsMenu0_9;
ST_Menu gsMenu0_10;
ST_Menu gsMenu0_11;
ST_Menu gsMenu0_12;
ST_Menu gsMenu_14;
ST_Menu gsMenu_15;
ST_Menu gsMenu_16;
ST_Menu gsMenu_17;
ST_Menu gsMenu_18;
ST_Menu gsMenu_19;
ST_Menu gsMenu_20;
ST_Menu gsMenu_21;
ST_Menu gsMenu_22;
ST_Menu gsMenu_23;
ST_Menu gsMenu_24;
ST_Menu gsMenu_25;
ST_Menu gsMenu_26;
ST_Menu gsMenu_27;
ST_Menu gsMenu_28;
ST_Menu gsMenu_30;
ST_Menu gsMenu_40;
ST_Menu gsMenu_41;
ST_Menu gsMenu_50;
ST_Menu gsMenu_51;
ST_Menu gsMenu_52;
ST_Menu gsMenu_53;
ST_Menu gsMenu_54;
ST_Menu gsMenu_55;
ST_Menu gsMenu_56;
ST_Menu gsMenu_79;
ST_Menu gsMenu_80;
ST_Menu gsMenu_81;
ST_Menu gsMenu_82;
ST_Menu gsMenu_83;
ST_Menu gsMenu_84;

/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/


