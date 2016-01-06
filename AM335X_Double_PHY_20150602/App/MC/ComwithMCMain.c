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
/* Private define-----------------------------------------------------------------------------*/
#define COMMCBUFF_LENGTH    100u                        
#define FRAMESTARTFIXEDLEN 16
#define FRAMEFIXEDLEN 22                                    // 固定长度=固定头+数据长度+功能位+版本+校验位
/* Private variables--------------------------------------------------------------------------*/
void *ComWithMCOSQ[16];					                    // 消息队列
OS_EVENT *ComWithMCTaskEvent;				                // 使用的事件

//INT8U Rxbuff1[5100]; 
//    INT8U *ptr1 = Rxbuff1;

/* Private functions--------------------------------------------------------------------------*/
INT8U Analyse_Protocol_Frame_Start(_BSP_MESSAGE *FrameMsg);
INT8U Analyse_Protocol_Frame(_BSP_MESSAGE *FrameMsg);
/***********************************************************************************************
* Function		: TaskComWithMC
* Description	: 和马达板通讯任务
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
    UART_InitStructure.Baudrate = 115200;		            // 波特率
    UART_InitStructure.Parity = BSPUART_PARITY_NO;			// 校验位
    UART_InitStructure.StopBits = BSPUART_STOPBITS_1;		// 停止位
    UART_InitStructure.WordLength = BSPUART_WORDLENGTH_8D;	// 数据位数
    UART_InitStructure.Work = BSPUART_WORK_FULLDUPLEX;		// 工作模式
    UART_InitStructure.TxRespond = BSPUART_RESPOND_INT;	    // 中断模式
    UART_InitStructure.pEvent = ComWithMCTaskEvent;	        // 消息事件
    UART_InitStructure.MaxTxBuffer = COMMCBUFF_LENGTH;		// 发送缓冲容量
    UART_InitStructure.MaxRxBuffer = COMMCBUFF_LENGTH;		// 接收缓冲容量
    UART_InitStructure.pTxBuffer = ComWithMCTxBuff;			// 发送缓冲指针
    UART_InitStructure.pRxBuffer = ComWithMCRxBuff;			// 接收缓冲指针
    UART_InitStructure.TxSpacetime = 0;						// 发送帧间隔
    UART_InitStructure.RxOvertime = 10;						// 接收帧间隔
       
    BSP_UARTConfig(UART0,&UART_InitStructure);
   
    while(1)
    {					
        pMsg = OSQPend(ComWithMCTaskEvent,0,(INT8U *)&err);
        if(err == OS_NO_ERR)							               	// 收到消息
        {
            switch(pMsg->MsgID)							           		// 判断消息来源
            {
                case BSP_MSGID_UART_TXOVER:					           	// 马达板发送完成
					NOP();
                    break;
				case BSP_MSGID_UART_RXOVER:					           	// 马达板接收完成
                    if(pMsg->DataLen > 600)
                    {
                        pMsg->DataLen=0;						    
                        break;
                    } 
                    if(TRUE == Analyse_Protocol_Frame(pMsg))       		// 解析协议帧 正确
                        OSQPost(DispTaskEvent,pMsg);                    // 给液晶显示
                    else                                                // 协议帧错误
                        BSP_UARTWrite(UART0,pMsg->pData,pMsg->DataLen);
                    BSP_UARTRxClear(UART0);                  
                    break;
                case APP_COMFROM_UI:					             	// 来自disp的消息
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
* Description	: 校验帧头，计算CS1
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
        if((*(FrameMsg->pData + offset) == 0xa0) && (*(FrameMsg->pData + offset + 1) == 0x02))     // 判断帧起始符
        {
            //和校验
            for(count=offset; count<(FRAMESTARTFIXEDLEN); count++)
                cs += *(FrameMsg->pData + count);  
            cs = cs & 0xff;
            if(cs == *(FrameMsg->pData +  FRAMESTARTFIXEDLEN - 1 ))   				// 校验通过
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
* Description	: 计算校验和CS2
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
        if((*(FrameMsg->pData + offset) == 0xa0) && (*(FrameMsg->pData + offset + 1) == 0x02))     // 判断帧起始符
        {
            datalen = *(FrameMsg->pData + offset + 8) | (*(FrameMsg->pData + offset + 9) << 8);      // 取得帧长度信息
            cs = 0;
            if(*(FrameMsg->pData + offset + datalen + FRAMEFIXEDLEN + 4) == 0x16)         				// 判断帧结束符
            {
                for(count=offset; count<(offset + datalen + FRAMEFIXEDLEN - 1 + 4); count++)
                    cs += *(FrameMsg->pData + count);  
                cs = cs & 0xff;
                if(cs == *(FrameMsg->pData + offset + datalen + FRAMEFIXEDLEN - 1 + 4))   				// 校验通过
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

/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/