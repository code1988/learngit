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
#define CANSendHead 0xF9B3

/* Private macro------------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
//�˵��еķ�̩���������˵���غ���
INT8U CanSearchParaManagementTable(INT16U DataId,INT8U flag);
INT16U CanProtocolATOperation(InterfaceStruct *terminal_para);
void CanDealWithUIDATA(INT8U Port,INT8U *pData,INT16U Len);
INT16U CanParaAppfunction(InterfaceStruct *terminal_para);
INT16U CanProtocolMianMenuATOperation(InterfaceStruct *terminal_para);
void CanGetCanFrameInformation(INT8U *pdata);
INT8U CanSearchParaManagementTable(INT16U DataId,INT8U flag);
void McDataControl(INT8U *CanData);
void Reply_CAN_Frame(INT8U *CanData);
void CombinedCanFrame();

/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/


