/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName		: FLashDataLoadmain.c
* Author		: 
* Date First Issued	: 150113   
* Version		: V
* Description		: ���ݺ�Flash�������ֵ�����
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
void *Flash_DATALoadOSQ[4];					                    // ��Ϣ����
INT8U FlashRxBuff[512] = {0};
INT8U FlashTxBuff[512] = {0};
INT8U FlashDatalen = 0;

/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function	    : TaskFlash
* Description	: ��FLASH��ȡ����������
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
        // ��Ϣָ��
        pMsg = OSQPend(FlashEvent,SYS_DELAY_500ms,&err);
        if(err == OS_NO_ERR)				    // �յ���Ϣ
        {
            switch(pMsg->MsgID)					// �ж���Ϣ
            {
                case APP_COMFROM_UI:
                    if(pMsg->DataLen > 512)
                    {
                        pMsg->DataLen=0;
                        break;
                    }
                    datalength = pMsg->DataLen; 
                    memcpy(FlashRxBuff,pMsg->pData,pMsg->DataLen);
                    //������UI�յ�������
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
* Description	: ��ȡ���յ�֡����Ϣ
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 15/1/2015	songchao
***********************************************************************************************/
void GetFrameInformation(INT8U *pdata)
{    
    FrameInformation.DataID = (*(pdata+1)|((*pdata)<<8));//ID��
    FrameInformation.pdata = pdata+2;//��������
}
 
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/