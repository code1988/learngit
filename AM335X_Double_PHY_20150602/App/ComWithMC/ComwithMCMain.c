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
#include "comwithMCMain.h"
#include "CanDataDealProtocol.h"
#include "CanSendUIData.h"
/* Private define-----------------------------------------------------------------------------*/
#define COMMCBUFF_LENGTH    1024*(1024)	                    //2K����ӦSD���������ĵ�һ���С

#define FRAMEFIXEDLEN       9u                              // �̶�����=�̶�ͷ+���ݳ���
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
void *ComWithMCOSQ[16];					                    // ��Ϣ����
OS_EVENT *canEvent;			                        	    // ʹ�õ��¼�

INT32U canTxBuff[2]={0};
INT32U canRxBuff[2]={0};//8�ֽڵ�buffer

INT8U ComWithMCTxBuff[COMMCBUFF_LENGTH] = {0};
INT8U ComWithMCRxBuff[COMMCBUFF_LENGTH] = {0};

INT8U MainMenuPage = 0;

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: CanTaskInit
* Description	: CANͨѶ�����ʼ��
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 11/7/2014	songchao
***********************************************************************************************/
void CanTaskInit(void)
{
    _BSPDCAN_CONFIG CAN_Config;
    
    canEvent = OSQCreate(ComWithMCOSQ,4);
    CAN_Config.pEvent = canEvent;	// ��Ϣ�¼�
    CAN_Config.num = 1;		        // �˿ں�
    CAN_Config.pTxBuffer = canTxBuff;	// ���ͻ���ָ��
    CAN_Config.pRxBuffer = canRxBuff;	// ���ջ���ָ��
    CAN_Config.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_RX);// ����֡����Ϊ����
    BSP_DCANInit(1,&CAN_Config);    
}
/***********************************************************************************************
* Function		: TaskComWithMC
* Description	: ������ͨѶ����
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
    //��ʼ��can����
    _BSPDCAN_CONFIG CANWriteConfig;
    
    canEvent = OSQCreate(ComWithMCOSQ,4);
    CANWriteConfig.pEvent = canEvent;	// ��Ϣ�¼�
    CANWriteConfig.num = 1;		        // �˿ں�
    CANWriteConfig.pTxBuffer = canTxBuff;	// ���ͻ���ָ��
    CANWriteConfig.pRxBuffer = canRxBuff;	// ���ջ���ָ��
    CANWriteConfig.entry.flag = (BSP_CANDATA_STDFRAME | BSP_CANMSG_DIR_RX);// ����֡����Ϊ����
    BSP_DCANInit(1,&CANWriteConfig);  
   
    OSTimeDlyHMSM(0,0,0,500);
        
    while(1)
    {
       					// ��Ϣָ��
        pMsg = OSQPend(canEvent,SYS_DELAY_100ms,&err);
        if(err == OS_NO_ERR)				    // �յ���Ϣ
        {
            switch(pMsg->MsgID)					// �ж���Ϣ
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
                    //�յ�UIA Display���񴫹�������Ϣ
                    memcpy(ComWithMCRxBuff,pMsg->pData,pMsg->DataLen);
                    DealWitchUIData(ComWithMCRxBuff,pMsg->DivNum,CANWriteConfig);
                    break;
                case BSP_MSGID_DCAN_EXTRXOVER:		
                    memcpy(canTxBuff,pMsg->pData,pMsg->DataLen);
                    CANWriteConfig.entry.flag = (BSP_CANDATA_EXIFRAME | BSP_CANMSG_DIR_TX);// ��������֡����Ϊ����
                    CANWriteConfig.entry.id = BSP_CANTX_MSG_EXTD_ID;//����֡
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


/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/


