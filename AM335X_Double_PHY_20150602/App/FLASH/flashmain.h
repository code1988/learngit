/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: flashmain.h
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
/* Private typedef----------------------------------------------------------------------------*/
struct FramePROStruct
{		 
    INT16U DataID;
    INT16U FrameLength;		 
    INT8U  *pdata;	        
};
extern struct FramePROStruct FrameInformation;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: GetFrameInformation
* Description	: 获取接收到帧的信息
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 15/1/2015	songchao
***********************************************************************************************/
void GetFrameInformation(INT8U *pdata);

//把SD卡上boot下载到NAND中
void DealWithUIDATA(INT8U Port,INT8U *p,INT16U Len);
void Send_UI_DATA_From_Nand(void);
INT8U SearchParaManagementTable(INT16U DataId,INT8U flag);
INT16U ProtocolATOperation(InterfaceStruct *terminal_para);
/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/


