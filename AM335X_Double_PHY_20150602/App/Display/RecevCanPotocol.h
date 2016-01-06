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
//UIA��ʾ��
#define UIADISPLAY 0x10

//UIA��ʾ�ľ���Э��Ķ�λ
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

//Э�鱣��֡
#define DEFAULTBYTE 0xFF

//UIA���͸�CAN�����ݸ���Э���ID��
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

//UIA���͸�CAN�����ݸ���Э����Ҫ��ʾ����ҳ��
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

//UIA����CAN���ݵĻظ�
#define UIACANRECEVOK   0x00
#define UIACANRECEVFAIL   0x01
#define UIACANMUSTREPLY 0x55

//����������
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
//UIA��������
//���˵��ܳ�ʵʱ��Ϣ
void UIA_Recev_Main_Menu(INT8U *CanData);
//�����ϸ��ѯ
void UIA_Recev_Mixed(INT8U *CanData);
//�����ڹ��ֺŲ�ѯ
void UIA_Recev_OUTLET(INT8U *CanData);
//�ӳ��ڹ��ֺŲ�ѯ
void UIA_Recev_INLET(INT8U *CanData);
//�汾��Ϣ
void UIA_Recev_Edition(INT8U *CanData);
//����ٶȵ���
void UIA_Recev_JCSPEED(INT8U *CanData);
//�ظ�֡
void UIA_Reply_CAN(INT8U *CanData);
//�����������Ϣ
void DealWithMC(INT8U *CanData);
//ˢ�����˵�
void RefreshMainMenu(void);
//����AD�鿴
void UIA_Recev_Inlet_AD(INT8U *CanData);
//����AD����
void UIA_Recev_IR_AD(INT8U *CanData);
//��С��ͷ����
void UIA_Recev_MT_AD(INT8U *CanData);
//ӫ��AD����
void UIA_Recev_UV_AD(INT8U *CanData);
//�߳���Ϣ
void UIA_Recev_Info_AD(INT8U *CanData);
//���������Ϣ
void UIA_Recev_Level_Up(INT8U *CanData);
//MT������ʾ
void UIA_Recev_Waveform(INT8U *CanData);
/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/
