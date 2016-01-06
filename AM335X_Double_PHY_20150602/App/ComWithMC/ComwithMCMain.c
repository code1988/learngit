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
#include "comwithMCMain.h"
#include "CanDataDealProtocol.h"
#include "CanSendUIData.h"
/* Private define-----------------------------------------------------------------------------*/
#define COMMCBUFF_LENGTH    1024*(1024)	                    //2K，对应SD卡读出来的的一桢大小

#define FRAMEFIXEDLEN       9u                              // 固定长度=固定头+数据长度
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
void *ComWithMCOSQ[16];					                    // 消息队列
OS_EVENT *canEvent;			                        	    // 使用的事件

INT32U canTxBuff[2]={0};
INT32U canRxBuff[2]={0};//8字节的buffer

INT8U ComWithMCTxBuff[COMMCBUFF_LENGTH] = {0};
INT8U ComWithMCRxBuff[COMMCBUFF_LENGTH] = {0};

INT8U MainMenuPage = 0;

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: CanTaskInit
* Description	: CAN通讯任务初始化
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 11/7/2014	songchao
***********************************************************************************************/
void CanTaskInit(void)
{
    _BSPDCAN_CONFIG CAN_Config;
    
    canEvent = OSQCreate(ComWithMCOSQ,4);
    CAN_Config.pEvent = canEvent;	// 消息事件
    CAN_Config.num = 1;		        // 端口号
    CAN_Config.pTxBuffer = canTxBuff;	// 发送缓冲指针
    CAN_Config.pRxBuffer = canRxBuff;	// 接收缓冲指针
    CAN_Config.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_RX);// 数据帧方向为接收
    BSP_DCANInit(1,&CAN_Config);    
}
/***********************************************************************************************
* Function		: TaskComWithMC
* Description	: 和马达板通讯任务
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 11/7/2014	songchao
***********************************************************************************************/
_BSP_MESSAGE send_message;  
void TaskComWithMC(void *pdata)
{   

    INT8U err;
    static _BSP_MESSAGE *pMsg;	
    //初始化can任务
    _BSPDCAN_CONFIG CANWriteConfig;
    
    canEvent = OSQCreate(ComWithMCOSQ,4);
    CANWriteConfig.pEvent = canEvent;	// 消息事件
    CANWriteConfig.num = 1;		        // 端口号
    CANWriteConfig.pTxBuffer = canTxBuff;	// 发送缓冲指针
    CANWriteConfig.pRxBuffer = canRxBuff;	// 接收缓冲指针
    CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_RX);// 数据帧方向为接收
    BSP_DCANInit(1,&CANWriteConfig);  
   
    OSTimeDlyHMSM(0,0,0,500);
        
    while(1)
    {
       					// 消息指针
        pMsg = OSQPend(canEvent,SYS_DELAY_100ms,&err);
        if(err == OS_NO_ERR)				    // 收到消息
        {
            switch(pMsg->MsgID)					// 判断消息
            {
                case BSP_MSGID_DCAN_STDTXOVER:		
                    NOP();
                    break;
                case BSP_MSGID_DCAN_STDRXOVER:		
                    if(pMsg->DataLen > 8)
                    {
                        pMsg->DataLen=0;						    
                        break;
                    }
                    memcpy(canRxBuff,pMsg->pData,pMsg->DataLen);
                    McDataControl((INT8U *)canRxBuff);
                    break;
                case APP_COMFROM_UI:					
                    //收到UIA Display任务传过来的消息
                    memcpy(ComWithMCRxBuff,pMsg->pData,pMsg->DataLen);
                    DealWitchUIData(ComWithMCRxBuff,pMsg->DivNum,CANWriteConfig);
                    break;
                case BSP_MSGID_DCAN_EXTRXOVER:		
                    memcpy(canTxBuff,pMsg->pData,pMsg->DataLen);
                    CANWriteConfig.entry.flag = (BSP_CANDATA_EXIFRAME | BSP_CANMSG_DIR_TX);// 扩张数据帧方向为发送
                    CANWriteConfig.entry.id = BSP_CANTX_MSG_EXTD_ID;//扩张帧
                    CANWriteConfig.entry.dlc = pMsg->DataLen;
                    CANWriteConfig.entry.data = canTxBuff;                           
                    BSP_DCANWrite(CANWriteConfig);                                      							
                    break;
                case APP_MT_LEVEL_UP:		
                    memcpy(g_cTmpBuf,pMsg->pData,pMsg->DataLen);                    
                    DealwithMCLevelUp(g_cTmpBuf,&(pMsg->DataLen),CANWriteConfig); 
                    break;
                default:
                    break;
            }
        }
    }
}


/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/


