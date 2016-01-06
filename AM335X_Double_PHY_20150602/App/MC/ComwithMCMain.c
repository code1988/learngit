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
/* Private define-----------------------------------------------------------------------------*/
#define COMMCBUFF_LENGTH    100u                        
#define FRAMESTARTFIXEDLEN 16
#define FRAMEFIXEDLEN 22                                    // �̶�����=�̶�ͷ+���ݳ���+����λ+�汾+У��λ
/* Private variables--------------------------------------------------------------------------*/
void *ComWithMCOSQ[16];					                    // ��Ϣ����
OS_EVENT *ComWithMCTaskEvent;				                // ʹ�õ��¼�

//INT8U Rxbuff1[5100]; 
//    INT8U *ptr1 = Rxbuff1;

/* Private functions--------------------------------------------------------------------------*/
INT8U Analyse_Protocol_Frame_Start(_BSP_MESSAGE *FrameMsg);
INT8U Analyse_Protocol_Frame(_BSP_MESSAGE *FrameMsg);
/***********************************************************************************************
* Function		: TaskComWithMC
* Description	: ������ͨѶ����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 11/7/2014	songchao
***********************************************************************************************/  
void TaskComWithMC(void *pdata)
{   
	INT8U ComWithMCTxBuff[COMMCBUFF_LENGTH] = {0};
	INT8U ComWithMCRxBuff[COMMCBUFF_LENGTH] = {0};
    _BSP_MESSAGE *pMsg;
    INT8U err;
    _BSPUART_CONFIG UART_InitStructure;    
    
    
    
    ComWithMCTaskEvent = OSQCreate(ComWithMCOSQ,16);
    UART_InitStructure.Baudrate = 115200;		            // ������
    UART_InitStructure.Parity = BSPUART_PARITY_NO;			// У��λ
    UART_InitStructure.StopBits = BSPUART_STOPBITS_1;		// ֹͣλ
    UART_InitStructure.WordLength = BSPUART_WORDLENGTH_8D;	// ����λ��
    UART_InitStructure.Work = BSPUART_WORK_FULLDUPLEX;		// ����ģʽ
    UART_InitStructure.TxRespond = BSPUART_RESPOND_INT;	    // �ж�ģʽ
    UART_InitStructure.pEvent = ComWithMCTaskEvent;	        // ��Ϣ�¼�
    UART_InitStructure.MaxTxBuffer = COMMCBUFF_LENGTH;		// ���ͻ�������
    UART_InitStructure.MaxRxBuffer = COMMCBUFF_LENGTH;		// ���ջ�������
    UART_InitStructure.pTxBuffer = ComWithMCTxBuff;			// ���ͻ���ָ��
    UART_InitStructure.pRxBuffer = ComWithMCRxBuff;			// ���ջ���ָ��
    UART_InitStructure.TxSpacetime = 0;						// ����֡���
    UART_InitStructure.RxOvertime = 10;						// ����֡���
       
    BSP_UARTConfig(UART0,&UART_InitStructure);
   
    while(1)
    {					
        pMsg = OSQPend(ComWithMCTaskEvent,0,(INT8U *)&err);
        if(err == OS_NO_ERR)							               	// �յ���Ϣ
        {
            switch(pMsg->MsgID)							           		// �ж���Ϣ��Դ
            {
                case BSP_MSGID_UART_TXOVER:					           	// ���巢�����
					NOP();
                    break;
				case BSP_MSGID_UART_RXOVER:					           	// ����������
                    if(pMsg->DataLen > 600)
                    {
                        pMsg->DataLen=0;						    
                        break;
                    } 
                    if(TRUE == Analyse_Protocol_Frame(pMsg))       		// ����Э��֡ ��ȷ
                        OSQPost(DispTaskEvent,pMsg);                    // ��Һ����ʾ
                    else                                                // Э��֡����
                        BSP_UARTWrite(UART0,pMsg->pData,pMsg->DataLen);
                    BSP_UARTRxClear(UART0);                  
                    break;
                case APP_COMFROM_UI:					             	// ����disp����Ϣ
                    memcpy(ComWithMCRxBuff,pMsg->pData,pMsg->DataLen);
                    BSP_UARTWrite(UART0,ComWithMCRxBuff,pMsg->DataLen);
                    BSP_UARTTxClear(UART0);
                    break;
                default:
                    break;
            }
        }
    }
}

/***********************************************************************************************
* Function		: Analyse_Protocol_Frame_Start
* Description	: У��֡ͷ������CS1
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 14/10/2014	songchao
***********************************************************************************************/				
INT8U Analyse_Protocol_Frame_Start(_BSP_MESSAGE *FrameMsg)
{
    INT16U offset = 0;
    INT8U  cs = 0;
    INT16U count;
    
    if(FrameMsg->DataLen < FRAMESTARTFIXEDLEN)
        return FALSE;
    while(offset < FrameMsg->DataLen)
    {
        if((*(FrameMsg->pData + offset) == 0xa0) && (*(FrameMsg->pData + offset + 1) == 0x02))     // �ж�֡��ʼ��
        {
            //��У��
            for(count=offset; count<(FRAMESTARTFIXEDLEN); count++)
                cs += *(FrameMsg->pData + count);  
            cs = cs & 0xff;
            if(cs == *(FrameMsg->pData +  FRAMESTARTFIXEDLEN - 1 ))   				// У��ͨ��
            {                
                return TRUE;
            }
        }
        offset++;
    }
    
    return FALSE;
}

/***********************************************************************************************
* Function		: Analyse_Protocol_Frame
* Description	: ����У���CS2
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 17/06/2010	wangyao
***********************************************************************************************/				
INT8U Analyse_Protocol_Frame(_BSP_MESSAGE *FrameMsg)
{
    INT16U offset = 0;
    INT16U datalen = 0;
    INT8U  cs;
    INT16U count;
    
    if(FrameMsg->DataLen < FRAMEFIXEDLEN)
        return FALSE;
    while(offset < FrameMsg->DataLen)
    {
        if((*(FrameMsg->pData + offset) == 0xa0) && (*(FrameMsg->pData + offset + 1) == 0x02))     // �ж�֡��ʼ��
        {
            datalen = *(FrameMsg->pData + offset + 8) | (*(FrameMsg->pData + offset + 9) << 8);      // ȡ��֡������Ϣ
            cs = 0;
            if(*(FrameMsg->pData + offset + datalen + FRAMEFIXEDLEN + 4) == 0x16)         				// �ж�֡������
            {
                for(count=offset; count<(offset + datalen + FRAMEFIXEDLEN - 1 + 4); count++)
                    cs += *(FrameMsg->pData + count);  
                cs = cs & 0xff;
                if(cs == *(FrameMsg->pData + offset + datalen + FRAMEFIXEDLEN - 1 + 4))   				// У��ͨ��
                {
                    FrameMsg->pData = FrameMsg->pData + offset;
                    FrameMsg->DataLen = datalen + FRAMEFIXEDLEN + 1 + 4;
                    FrameMsg->MsgID = APP_DISP_COMFROM_MC;
                    
                    return TRUE;
                }
            }
        }
        offset++;
    }
    
    return FALSE;
}

/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/