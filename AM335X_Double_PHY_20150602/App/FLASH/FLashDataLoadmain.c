/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName		: FLashDataLoadmain.c
* Author		: 
* Date First Issued	: 150113   
* Version		: V
* Description		: 数据和Flash交互部分的任务
*----------------------------------------历史版本信息-------------------------------------------
* History		    :
* //2013		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "flashmain.h"
#include "save_para.h"
#include "flashinterface.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
struct FramePROStruct     FrameInformation;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
OS_EVENT *FlashEvent; 
void *Flash_DATALoadOSQ[4];					                    // 消息队列
INT8U FlashRxBuff[512] = {0};
INT8U FlashTxBuff[512] = {0};
INT8U FlashDatalen = 0;

/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function	    : TaskFlash
* Description	: 对FLASH存取操作的任务
* Input		    : 
* Return	    : 
* Note(s)	    : 
* Contributor	:150114   songchao
***********************************************************************************************/
_BSP_MESSAGE Flashsend_message; 

void TaskFlash(void *pdata)
{
    INT8U err;    
    INT16U datalength = 0;
    static _BSP_MESSAGE *pMsg;	
    
    FlashEvent = OSQCreate(Flash_DATALoadOSQ,4);

    OSTimeDlyHMSM(0,0,0,500);
    
    while(1)
    {
        // 消息指针
        pMsg = OSQPend(FlashEvent,SYS_DELAY_500ms,&err);
        if(err == OS_NO_ERR)				    // 收到消息
        {
            switch(pMsg->MsgID)					// 判断消息
            {
                case APP_COMFROM_UI:
                    if(pMsg->DataLen > 512)
                    {
                        pMsg->DataLen=0;
                        break;
                    }
                    datalength = pMsg->DataLen; 
                    memcpy(FlashRxBuff,pMsg->pData,pMsg->DataLen);
                    //解析从UI收到的数据
                    DealWithUIDATA(pMsg->DivNum,FlashRxBuff,datalength);
                    break;
                default:
                    break;
            }
        }
    }
}

/***********************************************************************************************
* Function		: GetFrameInformation
* Description	: 获取接收到帧的信息
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 15/1/2015	songchao
***********************************************************************************************/
void GetFrameInformation(INT8U *pdata)
{    
    FrameInformation.DataID = (*(pdata+1)|((*pdata)<<8));//ID号
    FrameInformation.pdata = pdata+2;//具体数据
}
 
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/