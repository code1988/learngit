/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_UART.c
**创    建    人: wangyao
**创  建  日  期: 131211
**最  新  版  本: V0.1
**描          述: UART驱动
**				  20140718 修改每次发送至发送FIFO中的数据长度，消除UART中断触发过于频繁的问题
                  20140801 修正0xff无法接收问题
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人:
**日          期:
**修 改 前 版 本:
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "include.h"
#include "bsp.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
#include "gpio.h"
#include "gpio_v2.h"
#include "bsp_int.h"
#include "am335x_irq.h"
#include "uart_irda_cir.h"
#include "def_config.h"
#include "arm_com.h"


/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
// 状态位
typedef union
{
	INT8U byte;
	struct
	{
		unsigned bEn:1;							// 设备使能
		unsigned bTxBusy:1;						// 发送中标志
		unsigned bRxBusy:1;						// 接收中标志		跟接收完成标志类似，有点重复
		unsigned bRxOver:1;						// 接收完成标志
		unsigned bTxErr:2;						// 发送出错
		unsigned bRxErr:2;						// 接收出错
	};
}_BSPUART_STATE;
/* Private macro------------------------------------------------------------------------------*/
const INT32U UART_REGS_Table[] = {
    SOC_UART_0_REGS,
    SOC_UART_1_REGS,
    SOC_UART_2_REGS,
    SOC_UART_3_REGS,
    SOC_UART_4_REGS,
    SOC_UART_5_REGS
};
//UART的中断号
const INT8U UART_IRQS_Table[]={
    SYS_INT_UART0INT,
    SYS_INT_UART1INT,
    SYS_INT_UART2INT,
    SYS_INT_UART3INT,
    SYS_INT_UART4INT,
    SYS_INT_UART5INT
};
// 配置值对应关系,这样操作的目的是为了让用户看不到具体的配置值,
const INT8U UART_PARITY_Table[] = {//校验位
    UART_PARITY_NONE,
    UART_ODD_PARITY,
    UART_EVEN_PARITY
};
const INT8U UART_STOPBITS_Table[] = {//停止位
    UART_FRAME_NUM_STB_1,
    UART_FRAME_NUM_STB_1_5,
    UART_FRAME_NUM_STB_2,
};
const INT8U UART_WORDLENGTH_Table[] = {//数据位
    UART_FRAME_WORD_LENGTH_5,
    UART_FRAME_WORD_LENGTH_6,
    UART_FRAME_WORD_LENGTH_7,
    UART_FRAME_WORD_LENGTH_8
};
//const _HW_DIV_NAME UART_ENABLE_Table[] = {HW_DIV_UART0,HW_DIV_UART1,HW_DIV_UART2,HW_DIV_UART3,HW_DIV_UART4};
/* Private variables--------------------------------------------------------------------------*/
typedef struct
{
	volatile _BSPUART_STATE State;				// 串口状态

	INT16U TxAddr;								// 当前发送地址(发送过程中递增,等于SendLen说明数据发送完成)
	INT16U TxLen;								// 发送数据总长度(发送过程中不变,结束后清零)

	INT16U RxStart;								// 数据起始位置
	INT16U RxLen;								// 数据长度

	volatile INT16U TxSpacetime;				// 间隔时间,发送帧最小间隔
	volatile INT16U RxOvertime;					// 保存最后一次接收到数据时的时间,如果超过设置的超时时间,则认为超时
	_BSP_MESSAGE TxMessage;						// 保存发送结束消息内容
	_BSP_MESSAGE RxMessage;						// 保存接收结束消息内容
}_BSPUART_CONTROL;
static _BSPUART_CONFIG UART_Config[BSPUART_MAX_NUMBER];	// UART配置参数
static _BSPUART_CONTROL UART_Control[BSPUART_MAX_NUMBER];	// UART控制参数
/* Private functions--------------------------------------------------------------------------*/
// 发送接收引脚切换
//#define	UARTTxEnable(num)
//#define	UARTRxEnable(num)
/***********************************************************************************************
* Function Name	: BSP_UARTTxClear,BSP_UARTRxClear
* Description	: UART清除
* Input			: num:端口号
* Return		:
* Note(s)		: 清除UART数据,准备开始重新收发数据
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_UARTTxClear(INT8U num)
{
	//UARTRxEnable(num);							   	// 切换到接送状态,对485有效
    UARTIntDisable(UART_REGS_Table[num], UART_INT_THR);	// 关发送中断
	UART_Control[num].TxAddr = 0;
	UART_Control[num].TxLen = 0;
	UART_Control[num].TxSpacetime = 0;					// 当前发送间隔清零
	UART_Control[num].State.bTxBusy = 0;				// 退出发送状态
}
void BSP_UARTRxClear(INT8U num)
{
	UART_Control[num].RxStart = 0;
	UART_Control[num].RxLen = 0;
	UART_Control[num].RxOvertime = 0;					// 当前超时时间清零
	UART_Control[num].State.bRxBusy = 0;				// 退出接收状态
	UART_Control[num].State.bRxOver = 0;
}
//static void UARTTxErr(INT8U num)
//{
//	UART_Control[num].TxMessage.MsgID = BSP_MSGID_UART_TXERR;		// 串口发送消息
//	UART_Control[num].TxMessage.DivNum = num;	// 串口号
//	UART_Control[num].TxMessage.pData = UART_Config[num].pTxBuffer;	// 保存接收数据指针
//	UART_Control[num].TxMessage.DataLen = UART_Control[num].TxLen;	// 保存发送数据长度
//	SYSPost(UART_Config[num].pEvent,&UART_Control[num].TxMessage);	// 发送事件消息
//	// 调用发送完成函数,对于485就是切换端口方向
//	if(UART_HD_Table[num].Port_Pin_WD)
//
//	if(UART_Control[num].State.bTxErr < 3)
//		UART_Control[num].State.bTxErr++;		// 发送出错
//	BSP_UARTTxClear(num);
//}
static void UARTRxErr(INT8U num)
{
	UART_Control[num].RxMessage.MsgID = BSP_MSGID_UART_RXERR;	// 串口接收消息
	UART_Control[num].RxMessage.DivNum = num;	// 串口号
	UART_Control[num].RxMessage.pData = UART_Config[num].pRxBuffer;	// 保存接收数据指针
	UART_Control[num].RxMessage.DataLen = UART_Control[num].RxLen;	// 保存接收数据长度
	SYSPost(UART_Config[num].pEvent,&UART_Control[num].RxMessage);	// 发送事件消息
	if(UART_Control[num].State.bRxErr < 3)
		UART_Control[num].State.bRxErr++;		// 接收出错
	BSP_UARTRxClear(num);
}

/***********************************************************************************************
* Function		: UARTTxOver
* Description	: Post发送完成消息,并清空UART_Control中相关数据，为重新发送做好准备
* Input			: num UART0~5
* Output		:
* Note(s)		:
* Contributor	: 07/07/2014	wangyao
***********************************************************************************************/
static void UARTTxOver(INT8U num)
{
	UART_Control[num].TxMessage.MsgID = BSP_MSGID_UART_TXOVER;		// 串口发送消息
	UART_Control[num].TxMessage.DivNum = num;						// 串口号
	UART_Control[num].TxMessage.pData = UART_Config[num].pTxBuffer;	// 保存发送数据指针
	UART_Control[num].TxMessage.DataLen = UART_Control[num].TxLen;	// 保存发送数据长度
	SYSPost(UART_Config[num].pEvent,&UART_Control[num].TxMessage);	// 发送事件消息到UARTx的ECB
	//UART_Control[num].FuncTxOver();	// 调用发送完成函数,对于485就是切换端口方向
	UART_Control[num].State.bTxErr = 0;								// 发送出错标志清零
	BSP_UARTTxClear(num);											// 清空UART_Control中TX相关数据
}

/***********************************************************************************************
* Function		: UARTRxOver
* Description	: Post接收完成消息,并清空UART_Control中相关数据，为重新接收做好准备
* Input			: num UART0~5
* Output		:
* Note(s)		:
* Contributor	: 07/07/2014	wangyao
***********************************************************************************************/
static void UARTRxOver(INT8U num)
{
	UART_Control[num].RxMessage.MsgID = BSP_MSGID_UART_RXOVER;		// 串口接收消息
	UART_Control[num].RxMessage.DivNum = num;	                	// 串口号
	UART_Control[num].RxMessage.pData = UART_Config[num].pRxBuffer;	// 保存接收数据指针
	UART_Control[num].RxMessage.DataLen = UART_Control[num].RxLen;	// 保存接收数据长度
	SYSPost(UART_Config[num].pEvent,&UART_Control[num].RxMessage);	// 发送事件消息到UARTx的ECB
	UART_Control[num].State.bRxErr = 0;								// 接收错误标志清零
	UART_Control[num].State.bRxOver = 1;							// 接收完成标志置1
	UART_Control[num].State.bRxBusy = 0;							// 退出接收状态
}
//static void UART0TxOver(void)	{UARTTxOver(0);}
//static void UART1TxOver(void)	{UARTTxOver(1);}
//static void UART2TxOver(void)	{UARTTxOver(2);}
//static void UART3TxOver(void)	{UARTTxOver(3);}
//static void UART0TxErr(void)	{UARTTxErr(0);}
//static void UART1TxErr(void)	{UARTTxErr(1);}
//static void UART2TxErr(void)	{UARTTxErr(2);}
//static void UART3TxErr(void)	{UARTTxErr(3);}
/***********************************************************************************************
* Function Name	: BSP_UARTOverTime
* Description	: 超时判断,系统定时器调用,用户不关心
* Input			:
* Return		:
* Note(s)		:
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_UARTOverTime(void)
{
	INT8U num;          //串口号0～5

	for(num=0;num<BSPUART_MAX_NUMBER;num++)
	{
		// 如果处于发送状态,发送间隔使能
		if(UART_Config[num].TxSpacetime)
		{
			if(UART_Control[num].TxSpacetime < UART_Config[num].TxSpacetime)
				UART_Control[num].TxSpacetime++;
		}
	}
	for(num=0;num<BSPUART_MAX_NUMBER;num++)
	{
		// 处于接收状态,并且接收超时使能
		if((UART_Control[num].State.bRxBusy) && UART_Config[num].RxOvertime)
		{
			UART_Control[num].RxOvertime++;
			if(UART_Control[num].RxOvertime > UART_Config[num].RxOvertime)
			{
				UART_Control[num].RxOvertime = 0;
				UARTRxOver(num);			//发送接收超时消息
			}
		}
		else
			UART_Control[num].RxOvertime = 0;
	}
}
/***********************************************************************************************
* Function Name	: BSP_UARTWrite
* Description	: UART数据写
* Input			: num:端口号
				  *pData:待发送数据指针
				  len:数据长度
* Return		: TRUE/FALSE
* Note(s)		: 调用后,复制数据到发送缓冲
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_UARTWrite(INT8U num,const INT8U *pData,INT16U len)
{
    ///OS_ENTER_CRITICAL();

	// 空闲判断
	if(UART_Control[num].State.bTxBusy)
	{
        ///OS_EXIT_CRITICAL();
		return FALSE;
	}

	if(len > UART_Config[num].MaxTxBuffer)
	{
        ///OS_EXIT_CRITICAL();
		return FALSE;
	}

	UART_Control[num].State.bTxBusy = 1;	// 发送中
	// 发送间隔
	while(UART_Control[num].TxSpacetime < UART_Config[num].TxSpacetime)
	{
		SYSTimeDly(1);
	}
	//UARTTxEnable(num);							// 切换到发送状态,对485有效
	// 如果是查询模式(未实现)
	if(UART_Config[num].TxRespond == BSPUART_RESPOND_NORMAL)
	{
		while(len--)
		{
		}
		//UARTRxEnable(num);					// 切换到接送状态,对485有效
		UART_Control[num].State.bTxBusy = 0;	// 闲
	}
	else										// 中断或者DMA模式
	{
		if(len)
		{
			//memset(UART_Config[num].pTxBuffer,0,XXX_RX_NUM);
			if(pData != UART_Config[num].pTxBuffer)
				memcpy(UART_Config[num].pTxBuffer,pData,len);
			UART_Control[num].TxAddr = 0;
			UART_Control[num].TxLen = len;
			if(UART_Config[num].TxRespond == BSPUART_RESPOND_INT)	// 中断模式
                /* Enabling the specified UART interrupts. */
                UARTIntEnable(UART_REGS_Table[num],UART_INT_THR);	// 发送中断
			//else	                                                // DMA模式
				//BSP_DMAStart(UART_HD_Table[num].TxDMA,len);
		}
		else
		{
			UART_Control[num].State.bTxBusy = 0;	// 退出发送状态
		}
	}
	return TRUE;
}



/****************************************************************************************************
**名称:INT8U BSP_UARTWrite_two(INT8U num,const INT8U *pData,INT16U len)
**功能:UART数据写
* 入口:无
* 出口:无
**auth:hxj, date: 2014-12-26 16:46
*****************************************************************************************************/
INT8U BSP_UARTWrite_two(INT8U num,const INT8U *pData,INT16U len)
{
    ///OS_ENTER_CRITICAL();

	// 空闲判断
	if(UART_Control[num].State.bTxBusy)
	{
        ///OS_EXIT_CRITICAL();
		return FALSE;
	}

	if(len > UART_Config[num].MaxTxBuffer)
	{
        ///OS_EXIT_CRITICAL();
		return FALSE;
	}

	UART_Control[num].State.bTxBusy = 1;	// 发送中
	// 发送间隔
	while(UART_Control[num].TxSpacetime < UART_Config[num].TxSpacetime)
	{
		SYSTimeDly(1);
	}
	//UARTTxEnable(num);							// 切换到发送状态,对485有效
	// 如果是查询模式(未实现)
	if(UART_Config[num].TxRespond == BSPUART_RESPOND_NORMAL)
	{
		while(len--)
		{
		}
		//UARTRxEnable(num);					// 切换到接送状态,对485有效
		UART_Control[num].State.bTxBusy = 0;	// 闲
	}
	else										// 中断或者DMA模式
	{
		if(len)
		{
			//memset(UART_Config[num].pTxBuffer,0,XXX_RX_NUM);
			if(pData != UART_Config[num].pTxBuffer)
				memcpy(UART_Config[num].pTxBuffer,pData,len);
			UART_Control[num].TxAddr = 0;
			UART_Control[num].TxLen = len;
			if(UART_Config[num].TxRespond == BSPUART_RESPOND_INT)	// 中断模式
                /* Enabling the specified UART interrupts. */
                UARTIntEnable(UART_REGS_Table[num],UART_INT_THR);	// 发送中断
			//else	                                                // DMA模式
				//BSP_DMAStart(UART_HD_Table[num].TxDMA,len);
		}
		else
		{
			UART_Control[num].State.bTxBusy = 0;	// 退出发送状态
		}
	}
	return TRUE;
}










/***********************************************************************************************
* Function Name	: BSP_UARTRead
* Description	: UART数据读
* Input			: num:端口号
				  *pData:数据读取缓冲指针
				  len:数据长度
* Return		: 实际读取的数据长度
* Note(s)		: 复制串口接收缓冲的数据到pData,然后准备接收下一帧数据,也就是说如果用户使用了这个函数
                  读取串口数据,那么就不需要调用BSP_UARTRxClear()函数了.
	              如果用户指定的数据长度为0,则使用接收消息(RxMessage)中保存的数据长度
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT16U BSP_UARTRead(INT8U num,INT8U *pData,INT16U len)
{
	// 是否有新数据
	if(UART_Control[num].State.bRxOver ==0)
		return 0;
	// 如果是查询模式(未实现)
	if(UART_Config[num].TxRespond == BSPUART_RESPOND_NORMAL)
	{
		while(len--)
		{
		}
	}
	else  // 中断或者DMA模式
	{
		if(len==0)
			len = UART_Control[num].RxMessage.DataLen;
		if(len >= UART_Config[num].MaxRxBuffer)
			len = UART_Config[num].MaxRxBuffer;
		if(pData != UART_Config[num].pRxBuffer)
			memcpy(pData,UART_Config[num].pRxBuffer,len);
	}
	BSP_UARTRxClear(num);						// 接收数据处理完成,准备下一帧
	return len;
}
/***********************************************************************************************
* Function Name	: BSP_UARTIRQHandler
* Description	: UART中断服务程序
* Input			: num:端口号
* Return		:
* Note(s)		:
* Contributor	: 131211	wangyao
***********************************************************************************************/
#define NUM_TX_BYTES_PER_TRANS    (56)
#define NUM_RX_BYTES_PER_RECEV	  (56)



void BSP_UARTIRQHandler(INT8U num)
{
    INT32U  intId = 0;													// UART中断类型
    INT8U	RemainBytes;												// 剩余发送字节数
    int i;


    /* Checking ths source of UART interrupt. */
    intId = UARTIntIdentityGet(UART_REGS_Table[num]);

    // 如果是中断模式
	if(UART_Config[num].TxRespond != BSPUART_RESPOND_NORMAL)
    {
        switch(intId)
        {
            case UART_INTID_TX_THRES_REACH:								// 中断发送
                if(( UART_Control[num].TxLen - UART_Control[num].TxAddr) >= NUM_TX_BYTES_PER_TRANS)	// 是否还有数据需要发送
                {
                    UARTFIFOWrite(UART_REGS_Table[num],					// 发送寄存器
                                  &UART_Config[num].pTxBuffer[UART_Control[num].TxAddr],
                                  NUM_TX_BYTES_PER_TRANS);
                    UART_Control[num].TxAddr += NUM_TX_BYTES_PER_TRANS;
                }
                else													// 无数据,结束发送
                {
					RemainBytes = UART_Control[num].TxLen % NUM_TX_BYTES_PER_TRANS;
					UARTFIFOWrite(UART_REGS_Table[num],					// 发送寄存器
                                  &UART_Config[num].pTxBuffer[UART_Control[num].TxAddr],
                                  RemainBytes);
                    UARTIntDisable(UART_REGS_Table[num], UART_INT_THR);	// 关发送中断,不影响已经放入TX FIFO的数据的发出
                    // 发送发送完数据通知
                    UARTTxOver(num);
                }
                break;
            case UART_INTID_RX_THRES_REACH:								// 中断接收
                {
                        if(UART_Control[num].State.bRxOver ==0)				// 如果接收还没有完成
                    {
                        UART_Control[num].RxOvertime = 0;				// 超时判断时间清零
                        UART_Control[num].State.bRxBusy = 1;			// 接收中标志
                        // 如果接收缓冲已满,丢弃接收的数据,从头开始接收,相关参数复位
                        if(UART_Control[num].RxLen >= UART_Config[num].MaxRxBuffer)
                            UARTRxErr(num);								// 发送接收错消息
                        // 保存接收数据
                        for(i=0;i<NUM_RX_BYTES_PER_RECEV;i++)
	                    {
							UART_Config[num].pRxBuffer[UART_Control[num].RxLen] = UARTCharGetNonBlocking(UART_REGS_Table[num]);
                        	UART_Control[num].RxLen++;
	                    }
                    }
                }
                break;
            case UART_INTID_RX_LINE_STAT_ERROR:							// 中断接收线上错误(接收线中断不开启，此处不用)
            {
                INT32U rxErrorType;
                rxErrorType = UARTRxErrorGet(SOC_UART_0_REGS);
                /* Check if Overrun Error has occured. */
                if(rxErrorType & UART_LSR_RX_OE)
                    UARTRxErr(num);
                /* Check if Break Condition has occured. */
                else if(rxErrorType & UART_LSR_RX_BI)
                    UARTRxErr(num);
                /* Check if Framing Error has occured. */
                else if(rxErrorType & UART_LSR_RX_FE)
                    UARTRxErr(num);
                /* Check if Parity Error has occured. */
                else if(rxErrorType & UART_LSR_RX_PE)
                    UARTRxErr(num);
                break;
            }
            case UART_INTID_CHAR_TIMEOUT:									// 中断接收超时
			{
            	while(TRUE == UARTCharsAvail(UART_REGS_Table[num]))	// 读出接收FIFO中剩余字节
					UART_Config[num].pRxBuffer[UART_Control[num].RxLen++] = UARTFIFOCharGet(UART_REGS_Table[num]);
				UARTRxOver(num);
                break;
            }
            default:
                break;
        }
    }
	else
	{
		// 错误,关中断
		UARTIntDisable(UART_REGS_Table[num], (UART_INT_LINE_STAT | UART_INT_THR | UART_INT_RHR_CTI));// 关发送中断
	}




}



unsigned int UARTFIFOWrite_two( unsigned char *pBuffer, unsigned int numTxBytes)
{
    unsigned int lIndex = 0;
    unsigned int baseAdd;

    baseAdd=UART_REGS_Table[2];

    for(lIndex = 0; lIndex < numTxBytes; lIndex++)
    {
        /* Writing data to the TX FIFO. */
        HWREG(baseAdd + UART_THR) = *pBuffer++;
    }

    return lIndex;
}


void uart_tx_int_enable(uint8 num)
{
    unsigned int baseAdd=UART_REGS_Table[num];
    UARTIntEnable(baseAdd,UART_INT_THR);
}



void uart_tx_int_disable(uint8 num)
{
    unsigned int baseAdd=UART_REGS_Table[num];
    UARTIntDisable(baseAdd,UART_INT_THR);
}


void BSP_UARTIRQHandler_two(INT8U num)
{
    INT32U  intId = 0;													// UART中断类型
    INT8U	RemainBytes;												// 剩余发送字节数
    static uint8 read_buf2[NUM_RX_BYTES_PER_RECEV];
    int i;
    int r_cnt=0;


    /* Checking ths source of UART interrupt. */
    intId = UARTIntIdentityGet(UART_REGS_Table[num]);

    if(2==num)
    {
        //如果是串口2,则特殊处理
        // 如果是中断模式
    	if(UART_Config[num].TxRespond != BSPUART_RESPOND_NORMAL)
        {
            switch(intId)
            {
                case UART_INTID_TX_THRES_REACH:								// 中断发送
                {
                    if(NULL==g_p_arm_uart_send_cbuf)
                    {
                        UARTIntDisable(UART_REGS_Table[num], UART_INT_THR);	// 关发送中断,不影响已经放入TX FIFO的数据的发出
                        return;
                    }

                    strRecvCycBuf_N *recCycBuf=g_p_arm_uart_send_cbuf;
                    uint32  temp_counter = 0;
                    unsigned int baseAdd=UART_REGS_Table[num];

                    recCycBuf->pre_read_point=recCycBuf->read_point;
                    while ( ( recCycBuf->pre_read_point != recCycBuf->write_point ) || ( 1 == recCycBuf->full_flag ) )
                    {
                        HWREG(baseAdd + UART_THR) = recCycBuf->rec_buf[recCycBuf->pre_read_point];
                        temp_counter++;
                        recCycBuf->pre_read_point++;
                        if ( recCycBuf->pre_read_point >= recCycBuf->rec_buf_size ) recCycBuf->pre_read_point = 0;  //读指针到尾时,则指向头
                        if ( ( recCycBuf->pre_read_point == recCycBuf->write_point ) || ( temp_counter >= NUM_RX_BYTES_PER_RECEV ) )    break;  //读完一串数据或者读完接收环形缓冲区,则退出
                    }

                    if ( recCycBuf->pre_read_point == recCycBuf->write_point )
                    {
                        if ( recCycBuf->read_point == recCycBuf->write_point )
                        {
                            UARTIntDisable(UART_REGS_Table[num], UART_INT_THR); // 关发送中断,不影响已经放入TX FIFO的数据的发出
                            recCycBuf->frameData.frame_flag=1;
                        }
                    }

                    recCycBuf->read_point=recCycBuf->pre_read_point;
            		if ( 1 == recCycBuf->full_flag )
            		{
            			recCycBuf->write_point = 0;
            			recCycBuf->read_point = 0;
                        recCycBuf->pre_read_point = 0;
            			recCycBuf->full_flag = 0;
            		}

                }
                break;
                case UART_INTID_RX_THRES_REACH:								// 中断接收
                    {
                        if(UART_Control[num].State.bRxOver ==0)				// 如果接收还没有完成
                        {
                            UART_Control[num].RxOvertime = 0;				// 超时判断时间清零
                            UART_Control[num].State.bRxBusy = 1;			// 接收中标志

                            //读数据
                            r_cnt=0;
                            for(i=0;i<NUM_RX_BYTES_PER_RECEV;i++)
    	                    {
                                read_buf2[r_cnt++]=UARTCharGetNonBlocking(UART_REGS_Table[num]);
    	                    }
                            //放入缓冲中
                            arm_uart_recv_data_to_cyc_buf_n (read_buf2,r_cnt);

                        }
                    }
                    break;
                case UART_INTID_RX_LINE_STAT_ERROR:							// 中断接收线上错误(接收线中断不开启，此处不用)
                {
                    INT32U rxErrorType;
                    rxErrorType = UARTRxErrorGet(SOC_UART_0_REGS);
                    /* Check if Overrun Error has occured. */
                    if(rxErrorType & UART_LSR_RX_OE)
                        UARTRxErr(num);
                    /* Check if Break Condition has occured. */
                    else if(rxErrorType & UART_LSR_RX_BI)
                        UARTRxErr(num);
                    /* Check if Framing Error has occured. */
                    else if(rxErrorType & UART_LSR_RX_FE)
                        UARTRxErr(num);
                    /* Check if Parity Error has occured. */
                    else if(rxErrorType & UART_LSR_RX_PE)
                        UARTRxErr(num);
                    break;
                }
                case UART_INTID_CHAR_TIMEOUT:									// 中断接收超时
    			{
                    r_cnt=0;
                	while(TRUE == UARTCharsAvail(UART_REGS_Table[num]))	// 读出接收FIFO中剩余字节
                	{
                        read_buf2[r_cnt++]=UARTFIFOCharGet(UART_REGS_Table[num]);
                        if(r_cnt>=sizeof(read_buf2)) break;
                	}
                    //放入缓冲中
                    arm_uart_recv_data_to_cyc_buf_n (read_buf2,r_cnt);

                    break;
                }
                default:
                break;
            }
        }
    	else
    	{
    		// 错误,关中断
    		UARTIntDisable(UART_REGS_Table[num], (UART_INT_LINE_STAT | UART_INT_THR | UART_INT_RHR_CTI));// 关发送中断
    	}

    }
    else
    {
        // 如果是中断模式
    	if(UART_Config[num].TxRespond != BSPUART_RESPOND_NORMAL)
        {
            switch(intId)
            {
                case UART_INTID_TX_THRES_REACH:								// 中断发送
                    if(( UART_Control[num].TxLen - UART_Control[num].TxAddr) >= NUM_TX_BYTES_PER_TRANS)	// 是否还有数据需要发送
                    {
                        UARTFIFOWrite(UART_REGS_Table[num],					// 发送寄存器
                                      &UART_Config[num].pTxBuffer[UART_Control[num].TxAddr],
                                      NUM_TX_BYTES_PER_TRANS);
                        UART_Control[num].TxAddr += NUM_TX_BYTES_PER_TRANS;
                    }
                    else													// 无数据,结束发送
                    {
    					RemainBytes = UART_Control[num].TxLen % NUM_TX_BYTES_PER_TRANS;
    					UARTFIFOWrite(UART_REGS_Table[num],					// 发送寄存器
                                      &UART_Config[num].pTxBuffer[UART_Control[num].TxAddr],
                                      RemainBytes);
                        UARTIntDisable(UART_REGS_Table[num], UART_INT_THR);	// 关发送中断,不影响已经放入TX FIFO的数据的发出
                        // 发送发送完数据通知
                        UARTTxOver(num);
                    }
                    break;
                case UART_INTID_RX_THRES_REACH:								// 中断接收
                    {
                            if(UART_Control[num].State.bRxOver ==0)				// 如果接收还没有完成
                        {
                            UART_Control[num].RxOvertime = 0;				// 超时判断时间清零
                            UART_Control[num].State.bRxBusy = 1;			// 接收中标志
                            // 如果接收缓冲已满,丢弃接收的数据,从头开始接收,相关参数复位
                            if(UART_Control[num].RxLen >= UART_Config[num].MaxRxBuffer)
                                UARTRxErr(num);								// 发送接收错消息
                            // 保存接收数据
                            for(i=0;i<NUM_RX_BYTES_PER_RECEV;i++)
    	                    {
    							UART_Config[num].pRxBuffer[UART_Control[num].RxLen] = UARTCharGetNonBlocking(UART_REGS_Table[num]);
                            	UART_Control[num].RxLen++;
    	                    }
                        }
                    }
                    break;
                case UART_INTID_RX_LINE_STAT_ERROR:							// 中断接收线上错误(接收线中断不开启，此处不用)
                {
                    INT32U rxErrorType;
                    rxErrorType = UARTRxErrorGet(SOC_UART_0_REGS);
                    /* Check if Overrun Error has occured. */
                    if(rxErrorType & UART_LSR_RX_OE)
                        UARTRxErr(num);
                    /* Check if Break Condition has occured. */
                    else if(rxErrorType & UART_LSR_RX_BI)
                        UARTRxErr(num);
                    /* Check if Framing Error has occured. */
                    else if(rxErrorType & UART_LSR_RX_FE)
                        UARTRxErr(num);
                    /* Check if Parity Error has occured. */
                    else if(rxErrorType & UART_LSR_RX_PE)
                        UARTRxErr(num);
                    break;
                }
                case UART_INTID_CHAR_TIMEOUT:									// 中断接收超时
    			{
                	while(TRUE == UARTCharsAvail(UART_REGS_Table[num]))	// 读出接收FIFO中剩余字节
    					UART_Config[num].pRxBuffer[UART_Control[num].RxLen++] = UARTFIFOCharGet(UART_REGS_Table[num]);
    				UARTRxOver(num);
                    break;
                }
                default:
                    break;
            }
        }
	else
	{
		// 错误,关中断
		UARTIntDisable(UART_REGS_Table[num], (UART_INT_LINE_STAT | UART_INT_THR | UART_INT_RHR_CTI));// 关发送中断
	}


    }



}





// 中断向量映射
//void USART0_IRQHandler(void)	{BSP_UARTIRQHandler(0);}
//void USART1_IRQHandler(void)	{BSP_UARTIRQHandler(1);}
//void USART2_IRQHandler(void)	{BSP_UARTIRQHandler(2);}
//void UART3_IRQHandler(void)		{BSP_UARTIRQHandler(3);}
//void UART4_IRQHandler(void)		{BSP_UARTIRQHandler(4);}

void (*UART_IRQ_PFUNC[6])(void)=
{
    UART0_IRQHandler,
    UART1_IRQHandler,
    UART2_IRQHandler,
    UART3_IRQHandler,
    UART4_IRQHandler,
    UART5_IRQHandler,
};

//FIFO配置
static void UartFIFOConfigure(INT8U num)
{
    unsigned int fifoConfig = 0;

    /*
    ** - Transmit Trigger Level Granularity is 4					发送触发间隔4
    ** - Receiver Trigger Level Granularity is 1					接收触发间隔1
    ** - Transmit FIFO Space Setting is 56. Hence TX Trigger level	发送FIFO空间尺寸56
    **   is 8 (64 - 56). The TX FIFO size is 64 bytes.				发送触发深度8
    ** - The Receiver Trigger Level is 56.							接收触发深度56
    ** - Clear the Transmit FIFO.									清接收FIFO
    ** - Clear the Receiver FIFO.									清发送FIFO
    ** - DMA Mode enabling shall happen through SCR register.
    ** - DMA Mode 0 is enabled. DMA Mode 0 corresponds to No		NO DMA
    **   DMA Mode. Effectively DMA Mode is disabled.
    */
    fifoConfig = UART_FIFO_CONFIG(UART_TRIG_LVL_GRANULARITY_4,
                                  UART_TRIG_LVL_GRANULARITY_1,
                                  UART_FCR_TX_TRIG_LVL_56,
                                  56,
                                  1,
                                  1,
                                  UART_DMA_EN_PATH_SCR,
                                  UART_DMA_MODE_0_ENABLE);

    /* Configuring the FIFO settings. */
    UARTFIFOConfig(UART_REGS_Table[num], fifoConfig);
}
/***********************************************************************************************
* Function Name	: BSPUARTInit
* Description	: 串口初始化程序
* Input			: num:串口号0 ~ BSPUART_MAX_NUMBER-1
				  *pConfig:配置参数(_BSPUART_CONFIG)
					pConfig->pEvent:事件指针,用来传递消息事件
					pConfig->Baudrate:波特率,(最大1MHz)
					pConfig->Parity:校验位
						BSPUART_PARITY_NO:无校验(默认)
						BSPUART_PARITY_ODD:奇校验
						BSPUART_PARITY_EVEN:偶检验
					pConfig->StopBits:停止位
						BSPUART_STOPBITS_1:1位(默认)
						BSPUART_STOPBITS_0_5:0.5位(STR9不支持)
						BSPUART_STOPBITS_1_5:1.5位(STR9不支持)
						BSPUART_STOPBITS_2:2位
					pConfig->WordLength:数据位
						BSPUART_WORDLENGTH_8D:8位数据(默认)
						BSPUART_WORDLENGTH_7DP:7位数据+校验位
						BSPUART_WORDLENGTH_8DP:8位数据+校验位
					pConfig->Work:工作模式
						BSPUART_WORK_FULLDUPLEX:全双工(默认)
						BSPUART_WORK_HALFDUPLEX:半双工(未实现)
						BSPUART_WORK_IRDA:红外(未实现)
						BSPUART_WORK_IRDA_LOWPOWER:红外低功耗(未实现)
					pConfig->TxRespond:发送响应方式
						BSPUART_RESPOND_NORMAL:查询方式(默认)(未实现)
						BSPUART_RESPOND_INT:中断方式
						BSPUART_RESPOND_DMA:DMA方式(只有发送是DMA,接收还是中断方式)
					pConfig->MaxTxBuffer,最大发送缓冲
					pConfig->MaxRxBuffer,最大接收缓冲
					pConfig->TxBuffer,发送缓冲头指针
					pConfig->RxBuffer,接收缓冲头指针
					pConfig->RxOverLen,通知接收个数,接收满多少个数据,发送通知.为0时,只在收到一帧数据后通知
					pConfig->TxSpacetime,间隔时间,发送帧最小间隔
					pConfig->RxOvertime,超时时间,在这个时间里收不到新数据,说明一帧完成
* Return		: TRUE/FALSE
* Note(s)		:
* Contributor	: 131211	wangyao
***********************************************************************************************/
#define UART_MODULE_CLOCK     48000000
INT8U BSP_UARTConfig(INT8U num,_BSPUART_CONFIG *pConfig)
{
	INT32U divisorvalue = 0;

	//_BSPUART_CONFIG uartConfig;
#ifdef UART_EDMA_ENABLE
    /* Configuring the system clocks for EDMA. */
    EDMAModuleClkConfig();
#endif

    /* Configuring the system clocks for UART0 instance. */
    UARTModuleClkConfig(num);
    /* Performing the Pin Multiplexing for UART0 instance. */
    UARTPinMuxSetup(num);
#ifndef UART_EDMA_ENABLE
    /* Performing a module reset. */
    UARTModuleReset(UART_REGS_Table[num]);
    /* Performing FIFO configurations. */
    UartFIFOConfigure(num);													// 配置FIFO，内部实际上没有用DMA
#endif

    //计算目标波特率对应的因子系数
    divisorvalue = UARTDivisorValCompute(UART_MODULE_CLOCK,
                                         pConfig->Baudrate,					// 波特率值
                                         UART16x_OPER_MODE,
                                         UART_MIR_OVERSAMPLING_RATE_42);
    /* Programming the Divisor Latches. */
	//计算得到的因子系数写入相应寄存器，从而完成波特率配置
    UARTDivisorLatchWrite(UART_REGS_Table[num], divisorvalue);
    /* Switching to Configuration Mode B. */
	//串口控制寄存器配置成B(0xbf)模式:波特率因子锁存器开锁、强制TX线拉低等
    UARTRegConfigModeEnable(UART_REGS_Table[num], UART_REG_CONFIG_MODE_B);
    /* Programming the Line Characteristics. */
	//串口控制寄存器配置数据位、停止位、校验位
    UARTLineCharacConfig(UART_REGS_Table[num],
                         (UART_WORDLENGTH_Table[pConfig->WordLength]     	// 数据位
                          | UART_STOPBITS_Table[pConfig->StopBits]),     	// 停止位
                          UART_PARITY_Table[pConfig->Parity]);           	// 校验位

    /* Disabling write access to Divisor Latches. */
	//波特率因子锁存器上锁，恢复正常操作模式
    UARTDivisorLatchDisable(UART_REGS_Table[num]);
    /* Disabling Break Control. */
	//取消TX强制拉低，恢复正常操作模式
    UARTBreakCtl(UART_REGS_Table[num], UART_BREAK_COND_DISABLE);
    /* Switching to UART16x operating mode. */
    UARTOperatingModeSelect(UART_REGS_Table[num], UART16x_OPER_MODE);

    // 控制参数初始化,
	BSP_UARTRxClear(num);
	BSP_UARTTxClear(num);
    memcpy(&UART_Config[num],pConfig,sizeof(_BSPUART_CONFIG));				// 保存配置
	// 传输模式选择
	switch(pConfig->TxRespond)
	{
		case BSPUART_RESPOND_DMA:				// DMA
			// 初始化DMA通道,发送
//			if(UART_HD_Table[num].TxDMA!=BSPDMA_CHANNEL_NONE)
//			{
//                /* Initialization of EDMA3 */
//                EDMA3Init(SOC_EDMA30CC_0_REGS, EVT_QUEUE_NUM);
//                /* Registering EDMA3 Channel Controller 0 transfer completion interrupt.  */
//                BSP_IntVectReg(SYS_INT_EDMACOMPINT, Edma3CompletionIsr);
//                /* Setting the priority for EDMA3CC0 completion interrupt in AINTC. */
//                IntPrioritySet(SYS_INT_EDMACOMPINT, 0, AINTC_HOSTINT_ROUTE_IRQ);
//                /* Registering EDMA3 Channel Controller 0 Error Interrupt. */
//                BSP_IntVectReg(SYS_INT_EDMAERRINT, Edma3CCErrorIsr);
//                /* Setting the priority for EDMA3CC0 Error interrupt in AINTC. */
//                IntPrioritySet(SYS_INT_EDMAERRINT, 0, AINTC_HOSTINT_ROUTE_IRQ);
//                /* Enabling the EDMA3CC0 completion interrupt in AINTC. */
//                IntSystemEnable(SYS_INT_EDMACOMPINT);
//                /* Enabling the EDMA3CC0 Error interrupt in AINTC. */
//                IntSystemEnable(SYS_INT_EDMAERRINT);
//			}
//			else
				return FALSE;					// 无可用的DMA通道,出错
			// nobreak
		case BSPUART_RESPOND_INT:				// 中断
            /* Register the ISR in the Interrupt Vector Table.*/
            BSP_IntVectReg(UART_IRQS_Table[num], UART_IRQ_PFUNC[num]);
            IntPrioritySet(UART_IRQS_Table[num], 0, AINTC_HOSTINT_ROUTE_IRQ );
            /* Enable the System Interrupts for AINTC.*/
            IntSystemEnable(UART_IRQS_Table[num]);
            UARTIntEnable(UART_REGS_Table[num],UART_INT_RHR_CTI);
			break;
		default:								// 默认,BSPUART_RESPOND_NORMAL
			break;
	}
	return TRUE;
}

// config baudrate  by wwg
void BSP_UARTCfgBaud(INT8U num, INT32U baud)
{
	INT16U divisorvalue;
	divisorvalue = UARTDivisorValCompute(UART_MODULE_CLOCK,baud,UART16x_OPER_MODE,UART_MIR_OVERSAMPLING_RATE_42);
	/* Programming the Divisor Latches. */
	UARTDivisorLatchWrite(UART_REGS_Table[num], divisorvalue);

}


/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
