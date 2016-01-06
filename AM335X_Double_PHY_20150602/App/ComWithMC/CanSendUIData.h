/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: comwithMCMain.h
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
#include "appstruct.h"
/* Private define-----------------------------------------------------------------------------*/


//UIA��ʾ�ľ���Э��Ķ�λ
#define UIACANHEAD 0x00
#define UIACANINFO 0x02
#define UIACANLIST01 0x00
#define UIACANLIST02 0x01
#define UIACANLIST03 0x02
#define UIACANLIST04 0x03
#define UIACANLIST05 0x04
#define UIACANLIST06 0x05
#define UIACANLIST07 0x06
#define UIACANLIST08 0x07

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
#define UIACANLEVELUPSTART 0xF0
#define UIACANLEVELUPREADY 0xF1
#define UIACANLEVELUPDATA 0xF2

//UIA���͸�CAN�����ݸ���Э����Ҫ��ʾ����ҳ��
#define UIACANPAGE0 0x00
#define UIACANPAGE1 0x01
#define UIACANPAGE2 0x02
#define UIACANPAGE8 0x08
#define UIACANAUTHENTICATIONTOTALPAGES 0x1C
#define UIACANAUTHENTICLEVELTOTALPAGES 0x01
#define UIACANAUTHENTICSWITCHTOTALPAGES 0x02
#define UIACANCLEARPAGES 0x01

//Э�鱣��֡
#define DEFAULTBYTE 0xFF

/* Private macro------------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
//��������
void DealWitchUIData(INT8U *UIData,INT16U DivNum,_BSPDCAN_CONFIG CANWriteConfig);
//���arm �����������
void DealwithMCLevelUp(INT8U *MTLevelUP,INT32U *MTLevelUPSize,_BSPDCAN_CONFIG CANWriteConfig);

void UIA_Send_MC_Authentication_Shark(INT8U *CanData);
void UIA_Send_MC_Authentication_Param(INT8U *UIData,INT8U Page,INT8U *CanData);
void UIA_Send_MC_Motor_Flag_Set(INT8U *CanData,INT8U *UIData);
//���arm������ʼ
void UIA_Send_MC_LevelUp_Start(INT8U *CanData);
//����ֵ����
void UIA_Send_MC_batch(INT8U *CanData,INT8U *UIData);
//�㳮ģʽ
void UIA_Send_MC_Mode(INT8U *CanData,INT8U *UIData);
//�����ϸ��ѯ֡
void UIA_Send_MC_Mixed_Shark(INT8U *CanData);
//�˳��ڹ��ֺ����ѯ
void UIA_Send_OutLet_Search(INT8U *CanData);
//�����ڹ��ֺ����ѯ
void UIA_Send_InLets_Search(INT8U *CanData);
//��α�ȼ�1
void UIA_Send_MC_AuthenticSwitch_Param1(INT8U *CanData,INT8U *UIData);
//��α�ȼ�2
void UIA_Send_MC_AuthenticSwitch_Param2(INT8U *CanData,INT8U *UIData);
//��ֵȼ�
void UIA_Send_MC_ClearLevel_Param(INT8U *CanData,INT8U *UIData);
//���arm����
void UIA_Send_MC_LevelUp(INT8U *CanData,INT32U *UIData);

/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/


