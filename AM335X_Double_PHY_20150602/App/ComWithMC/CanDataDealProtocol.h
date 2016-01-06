/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: comwithMCMain.h
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
#include "appstruct.h"
/* Private define-----------------------------------------------------------------------------*/
#define CANSendHead 0xF9B3

/* Private macro------------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
//菜单中的方泰参数调整菜单相关函数
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

/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/


