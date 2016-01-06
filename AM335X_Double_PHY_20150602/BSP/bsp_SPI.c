/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_spi.c
**硬          件: am335x
**创    建    人: wangyao
**创  建  日  期: 131211
**最  新  版  本: V0.1
**描          述: SPI驱动，这里暂时只做默认的查询模式和中断模式，DMA模式后期再增加
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
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
#include "mcspi.h"
#include "am335x_irq.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
//// 配置值对应关系,这样操作的目的是为了让用户看不到具体的配置值,这样就不需要总是包含"spi.h"
//由于SPI0被要被W550和AT45DB都调用，将SPI0做一路扩展管理，不同制式使能脚CS不同
const INT32U SPI_REGS_Table[] = {
   
    SOC_SPI_0_REGS,  
    SOC_SPI_0_REGS,
    SOC_SPI_1_REGS,
};
const INT8U SPI_INT_Table[] = {
    
    SYS_INT_SPI0INT,
    SYS_INT_SPI0INT,
    SYS_INT_SPI1INT,
};
//const INT16U SPI_FIRSTBIT_Table[] = {SPI_FirstBit_MSB,SPI_FirstBit_LSB};
//const INT16U SPI_CLK_PHASE_Table[] = {SPI_CPHA_1Edge,SPI_CPHA_2Edge};
//const INT16U SPI_CLK_POLARITY_Table[] = {SPI_CPOL_Low,SPI_CPOL_High};
//const INT16U SPI_WORKMODE_Table[] = {SPI_Mode_Master,SPI_Mode_Slave};
//const _BSPDMA_CHANNELS SPI_DMATX_Table[] = {BSPDMA_CHANNEL_SPI1_TX,BSPDMA_CHANNEL_SPI2_TX,BSPDMA_CHANNEL_SPI3_TX};
//const _BSPDMA_CHANNELS SPI_DMARX_Table[] = {BSPDMA_CHANNEL_SPI1_RX,BSPDMA_CHANNEL_SPI2_RX,BSPDMA_CHANNEL_SPI3_RX};
//const _HW_DIV_NAME SPI_ENABLE_Table[] = {HW_DIV_SPI0,HW_DIV_SPI1,HW_DIV_SPI2};
/* Private variables--------------------------------------------------------------------------*/  

#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment=32
static _BSPSPI_CONFIG SPI_Config[3];	// SPI配置参数
#endif
#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment=32
static _BSPSPI_CONTROL SPI_Control[3];	// SPI控制参数
#endif

static OS_EVENT *SPI0Mutex;

/* Private functions--------------------------------------------------------------------------*/
static void SPIRxErr(INT8U num)
{
	SPI_Control[num].RxMessage.MsgID = BSP_MSGID_SPI_RXERR;
	SPI_Control[num].RxMessage.DivNum = num;
	SPI_Control[num].RxMessage.DataLen = SPI_Control[num].RxLen;
	SPI_Control[num].RxMessage.pData = SPI_Config[num].pRxBuffer;
	SYSPost(SPI_Config[num].pEvent,&SPI_Control[num].RxMessage);
	SPI_Control[num].State.bRxBusy = 0;			// 退出接收状态
	SPI_Control[num].State.bRxErr = 1;			// 接收出错
}
//static void SPITxErr(INT8U num)
//{
//	SPI_Control[num].State.bTxBusy = 0;			// 退出发送状态
//}
static void SPIRxOver(INT8U num)
{
	SPI_Control[num].RxMessage.MsgID = BSP_MSGID_SPI_RXOVER;
	SPI_Control[num].RxMessage.DivNum = num;
	SPI_Control[num].RxMessage.DataLen = SPI_Control[num].RxLen;
	SPI_Control[num].RxMessage.pData = SPI_Config[num].pRxBuffer;
	SYSPost(SPI_Config[num].pEvent,&SPI_Control[num].RxMessage);
	SPI_Control[num].State.bRxBusy = 0;			// 退出接收状态
	SPI_Control[num].State.bRxOver = 1;			// 接收完成
}
static void SPITxOver(INT8U num)
{
	// 这里不需要发送事件,在接收完成中处理
	SPI_Control[num].State.bTxBusy = 0;			// 退出发送状态
}

/***********************************************************************************************
* Function Name	: BSP_SPIOverTime
* Description	: 超时判断,系统定时器调用,用户不关心
* Input			: 
* Return		: 
* Note(s)		: 接收和发送超时做在一起了,如果出现超时则认为出错,清除状态
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_SPIOverTime(void)
{
	INT8U num;
	
	for(num=0;num<BSPSPI_MAX_NUMBER;num++)
	{
		// 处于接收或发送状态,并且接收超时使能
		if((SPI_Control[num].State.bTxBusy || SPI_Control[num].State.bRxBusy) && SPI_Config[num].RxOvertime)
		{
			SPI_Control[num].RxOvertime++;
			if(SPI_Control[num].RxOvertime > SPI_Config[num].RxOvertime)
			{
				//BSP_SPINSSDisable(num);
                //McSPICSDisable(SPI_REGS_Table[num]);
                McSPIChannelDisable(SPI_REGS_Table[num], num);
				SPIRxErr(num);
			}
		}
		else
			SPI_Control[num].RxOvertime = 0;
	}
}
void BSP_SPICSEnable(INT8U num)
{
    INT8U perr;
    
    OSMutexPend(SPI0Mutex,0,&perr);
    // 使能片选
    McSPICSAssert(SPI_REGS_Table[num], SPI_Config[num].channel); 
    // 使能SPI通道
    McSPIChannelEnable(SPI_REGS_Table[num], SPI_Config[num].channel); 
}
void BSP_SPICSDisEnable(INT8U num)
{
    McSPICSDeAssert(SPI_REGS_Table[num], SPI_Config[num].channel);                      // 关闭片选
	McSPIChannelDisable(SPI_REGS_Table[num], SPI_Config[num].channel);	                // 关通道
    OSMutexPost(SPI0Mutex);
}
/***********************************************************************************************
* Function Name	: BSP_SPITransOneByte
* Description	: SPI数据发送函数,发送一个字节
* Input			: num:端口号
				  *pData:发送数据指针,返回的数据也放在这里
* Return		: TRUE/FALSE
* Note(s)		: 查询模式使用,所以也就不太追求速度,一个一个的操作
* Contributor	: 131211	wangyao
***********************************************************************************************/
static INT8U BSP_SPITransOneByte(INT8U num,INT8U *pData)
{
	INT16U count=0;
    

	while((McSPIChannelStatusGet(SPI_REGS_Table[num],SPI_Config[num].channel) & MCSPI_CH_STAT_TXS_EMPTY) == RESET)
	{
		if(count++ >= 0x800)
			return FALSE;
	}
	// 发送接收
    McSPITransmitData(SPI_REGS_Table[num],(INT32U)(*pData),SPI_Config[num].channel);//发送 
	// 等待接收
	count = 0;
	while((McSPIChannelStatusGet(SPI_REGS_Table[num],SPI_Config[num].channel) & MCSPI_CH_STAT_RXS_FULL)==RESET)
	{
		if(count++ >= 0x100)
			return FALSE;
	}
	*pData = (INT8U)McSPIReceiveData(SPI_REGS_Table[num],SPI_Config[num].channel);	// 接收

    
	return TRUE;
}
//spi 传输  1：写模式 0为读模式
static INT8U BSP_SPITransBytes(INT8U num,INT8U *pData,INT16U len,INT8U state)
{
    if(len == 0)
		return TRUE;

	switch(SPI_Config[num].TxRespond)
	{
		case BSPSPI_RESPOND_INT:
            if(len > SPI_Config[num].MaxTxBuffer)
				return FALSE;
			// 复制数据到发送缓冲
			if(pData != SPI_Config[num].pTxBuffer)
				memcpy(SPI_Config[num].pTxBuffer,pData,len);

			// 配置发送长度
			McSPIWordCountSet(SPI_REGS_Table[num],len);
			SPI_Config[num].MaxRxBuffer = len;

		    SPI_Control[num].TxAddr = 0;		// 发送首地址
			SPI_Control[num].RxLen = 0;			// 接收起始 
			
		    // 使能片选
		    McSPICSAssert(SPI_REGS_Table[num], SPI_Config[num].channel);                                                
		    // 开收发中断
		    McSPIIntEnable(SPI_REGS_Table[num],MCSPI_INT_TX_EMPTY(SPI_Config[num].channel) | MCSPI_INT_RX_FULL(SPI_Config[num].channel));  
		    // 使能SPI
		    McSPIChannelEnable(SPI_REGS_Table[num], SPI_Config[num].channel);                                           
		    break;
		case BSPSPI_RESPOND_DMA:
			break;
		default:
        {
			INT8U data,*p;
			p = &data;
			while(len--)
			{
				if(state)						// 1:写0：读
					data = *pData;
				else
					p = pData;
				if(BSP_SPITransOneByte(num,p) == FALSE)
				{
					return FALSE;
				}
				pData++;
			}
            break;
		}
	}
    return TRUE;
}
/***********************************************************************************************
* Function Name	: BSP_SPIClear
* Description	: SPI清除
* Input			: num:端口号
* Return		: 
* Note(s)		: 清除SPI数据,准备重新开始,中断或者DMA模式下,用户必须在处理完一帧数据后调用此函数
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_SPIClear(INT8U num)
{
	memset((INT8U *)&SPI_Control[num],0x00,sizeof(_BSPSPI_CONTROL));
}
/***********************************************************************************************
* Function Name	: BSP_SPIWrite
* Description	: SPI数据写
* Input			: num:端口号
* Return		: 
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_SPIWrite(INT8U num,INT8U *pData,INT16U len)
{
	return BSP_SPITransBytes(num,pData,len,1);
}
/***********************************************************************************************
* Function Name	: BSP_SPIRead
* Description	: SPI数据读
* Input			: num:端口号
* Return		: 
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_SPIRead(INT8U num,INT8U *pData,INT16U len)
{
	return BSP_SPITransBytes(num,pData,len,0);
}
/***********************************************************************************************
* Function Name	: BSP_SPIIRQHandler
* Description	: SPI中断服务程序
* Input			: num:端口号
* Return		: 
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_SPIIRQHandler(INT8U num)
{

    INT32U intCode = 0;
    
    intCode =  0x2ffff & McSPIIntStatusGet(SPI_REGS_Table[num]);
	while(intCode)
	{
		if(MCSPI_INT_EOWKE == (intCode & MCSPI_INT_EOWKE))		                                    // 如果是发送完毕中断源
		{
			McSPIIntDisable(SPI_REGS_Table[num], MCSPI_INT_TX_EMPTY(SPI_Config[num].channel));	    // 关发送中断
			McSPIIntStatusClear(SPI_REGS_Table[num], MCSPI_INT_EOWKE);	
			SPITxOver(num);
		}
		else if(MCSPI_INT_TX_EMPTY(SPI_Config[num].channel) ==(intCode & MCSPI_INT_TX_EMPTY(SPI_Config[num].channel)))  // 如果是发送中断源
		{
			McSPIIntStatusClear(SPI_REGS_Table[num], MCSPI_INT_TX_EMPTY(SPI_Config[num].channel));				
			McSPITransmitData(SPI_REGS_Table[num],
					(INT32U)SPI_Config[num].pTxBuffer[SPI_Control[num].TxAddr++], 
					SPI_Config[num].channel);	
		}
		
	    if(MCSPI_INT_RX_FULL(SPI_Config[num].channel) == (intCode & MCSPI_INT_RX_FULL(SPI_Config[num].channel)))        // 如果是接收中断源
	    {
	    	INT8U data;
	    	
			McSPIIntStatusClear(SPI_REGS_Table[num], MCSPI_INT_RX_FULL(SPI_Config[num].channel));
			data = (INT8U) McSPIReceiveData(SPI_REGS_Table[num], SPI_Config[num].channel);
			SPI_Control[num].RxOvertime = 0;

			SPI_Config[num].pRxBuffer[SPI_Control[num].RxLen++]= data;

			if(SPI_Control[num].RxLen == SPI_Config[num].MaxRxBuffer)			                    // 如果接收完毕
			{
				McSPIIntDisable(SPI_REGS_Table[num], MCSPI_INT_RX_FULL(SPI_Config[num].channel));   // 关接收中断
				/* Force SPIEN line to the inactive state.*/    
				McSPICSDeAssert(SPI_REGS_Table[num], SPI_Config[num].channel);                      // 关闭片选
				McSPIChannelDisable(SPI_REGS_Table[num], SPI_Config[num].channel);	                // 关通道
				SPIRxOver(num);													                    // 给任务发送接收到数据消息
			}
	    }
	    intCode =  0x2ffff & McSPIIntStatusGet(SPI_REGS_Table[num]);
	}
}

void (*PFUNC[2])(void)=
{
    MCSPI0_IRQHandler,
    MCSPI1_IRQHandler
};
/***********************************************************************************************
* Function Name	: BSP_SPIConfig
* Description	: SPI配置
* Input			: num:端口号(0 ~ BSPSPI_MAX_NUMBER-1)
				  *pConfig:配置参数(_BSPSPI_CONFIG)
					pConfig->pEvent,事件指针
					pConfig->Baudrate,波特率,实际波特率可能和设置值不同(最大18MHz)
					pConfig->FirstBit,数据首位(_BSPSPI_FIRSTBIT)
						BSPSPI_FIRSTBIT_LOW,低位在前
						BSPSPI_FIRSTBIT_HIGH,高位在前
					pConfig->Phase,时钟相位(_BSPSPI_CLK_PHASE)
						BSPSPI_CLK_PHASE_1EDGE,采样从第一个时钟边沿开始
						BSPSPI_CLK_PHASE_2EDGE,采样从第二个时钟边沿开始
					pConfig->Polarity,时钟极性(_BSPSPI_CLK_POLARITY)
						BSPSPI_CLK_POLARITY_LOW,空闲时低
						BSPSPI_CLK_POLARITY_HIGH,空闲时高
					pConfig->LineMode,线模式(_BSPSPI_LINEMODE)
						BSPSPI_LINEMODE_FULLDUPLEX,双线全双工
						BSPSPI_LINEMODE_2LINERX,双线只接收
						BSPSPI_LINEMODE_1LINERXTX,单线接收发送,这个是自己加的,初始接收,自动切换方向
						BSPSPI_LINEMODE_1LINERX,单线接收
						BSPSPI_LINEMODE_1LINETX,单线发送
					pConfig->Mode,工作模式(_BSPSPI_WORKMODE)
						BSPSPI_WORKMODE_MASTER,主模式
						BSPSPI_WORKMODE_SLAVE,从模式
					pConfig->TxRespond,发送响应模式(查询,INT,DMA)(_BSPSPI_RESPOND)
						BSPSPI_RESPOND_NORMAL,查询方式(默认)
						BSPSPI_RESPOND_INT,中断方式(发送接收都是中断方式)
						BSPSPI_RESPOND_DMA,DMA方式(发送接收都是DMA中断方式)
					pConfig->MaxTxBuffer,最大发送缓冲
					pConfig->MaxRxBuffer,最大接收缓冲
					pConfig->TxBuffer,发送缓冲指针
					pConfig->RxBuffer,接收缓冲指针
					pConfig->RxOvertime,接收超时
* Return		: TRUE/FALSE
* Note(s)		: 
* Contributor	: 090505	wangyao
***********************************************************************************************/
#define MCSPI_OUT_FREQ                   (48000000u)	// 48M 输出
#define MCSPI_IN_CLK                     (48000000u)    // 48M CLK
INT8U BSP_SPIConfig(_BSPSPI_CONFIG *pConfig)
{	
	
    // 使能片选功能，只有MCSPI_MODULCTRL_PIN34清零，才能执行后面的片选设置
    McSPICSEnable(SPI_REGS_Table[pConfig->num]);
	
    switch(pConfig->Mode)
    {        
        case BSPSPI_WORKMODE_SLAVE:								// 从模式
            //从模式使能
            McSPISlaveModeEnable(SPI_REGS_Table[pConfig->num]);	
			// 发送接收双工模式，D1线收、D0线发
            McSPISlaveModeConfig(SPI_REGS_Table[pConfig->num],MCSPI_TX_RX_MODE, MCSPI_DATA_LINE_COMM_MODE_6,pConfig->channel);   	
			// SPI协议0模式，即IDLE时CLK低电平、数据上升沿锁存
    		HWREG(SPI_REGS_Table[pConfig->num] + MCSPI_CHCONF(pConfig->channel)) |= (MCSPI_CLK_MODE_0 & (MCSPI_CH0CONF_PHA | MCSPI_CH0CONF_POL));
			// SPI协议3模式，对应当前DSP传输
//    		HWREG(SPI_REGS_Table[pConfig->num] + MCSPI_CHCONF(pConfig->channel)) |= (MCSPI_CLK_MODE_3 & (MCSPI_CH0CONF_PHA | MCSPI_CH0CONF_POL));
            break;
        case BSPSPI_WORKMODE_MASTER:							// 主模式
            // 主模式使能
            McSPIMasterModeEnable(SPI_REGS_Table[pConfig->num]);			
            // 发送接收双工模式，单通道，D1线发、D0线收
            McSPIMasterModeConfig(SPI_REGS_Table[pConfig->num],MCSPI_SINGLE_CH,MCSPI_TX_RX_MODE,MCSPI_DATA_LINE_COMM_MODE_1,pConfig->channel);  						
            // SPI协议0模式，时钟频率48MHz
            McSPIClkConfig(SPI_REGS_Table[pConfig->num],MCSPI_IN_CLK,MCSPI_OUT_FREQ,pConfig->channel,MCSPI_CLK_MODE_0);            
            break;
        default:
        	break;
    } 
	
    // 8bit长度
    McSPIWordLengthSet(SPI_REGS_Table[pConfig->num], MCSPI_WORD_LENGTH(8), pConfig->channel);	
    // CS线极性:低电平活跃
    McSPICSPolarityConfig(SPI_REGS_Table[pConfig->num], MCSPI_CS_POL_LOW, pConfig->channel);   
	// 使能TX	FIFO
    McSPITxFIFOConfig(SPI_REGS_Table[pConfig->num], MCSPI_TX_FIFO_ENABLE, pConfig->channel);		
    // 使能RX	FIFO
    McSPIRxFIFOConfig(SPI_REGS_Table[pConfig->num], MCSPI_RX_FIFO_ENABLE, pConfig->channel); 

//	McSPIFIFOTrigLvlSet(SPI_REGS_Table[pConfig->num],2,2,MCSPI_TX_RX_MODE);
	
	switch(pConfig->TxRespond)
	{
		case BSPSPI_RESPOND_INT:				// 中断
		{
            // 注册SPI中断处理函数
            BSP_IntVectReg(SPI_INT_Table[pConfig->num], PFUNC[pConfig->num]);
			// 配置SPI中断优先级
            IntPrioritySet(SPI_INT_Table[pConfig->num], 0, AINTC_HOSTINT_ROUTE_IRQ );
            // 中断控制块中使能SPI中断
            IntSystemEnable(SPI_INT_Table[pConfig->num]);     
			// 发送总字节数设置，为了产生EOW中断
		    McSPIWordCountSet(SPI_REGS_Table[pConfig->num],pConfig->MaxTxBuffer);
			// 使能EOW中断
			McSPIIntEnable(SPI_REGS_Table[pConfig->num],MCSPI_INT_EOWKE);
            //McSPICSDisable(SPI_REGS_Table[num]);// 不能少,不然第一个字节会错
			break;
		}
		case BSPSPI_RESPOND_DMA:				// DMA
		{			
			// no break;
		}
		default:								// 默认,BSPSPI_RESPOND_NORMAL
			// 禁止所有中断			
			break;
	}
	
	// 配置和控制参数初始化
	BSP_SPIClear(pConfig->num);											
    memcpy(&SPI_Config[pConfig->num],pConfig,sizeof(_BSPSPI_CONFIG));	

	return TRUE;
}
/***********************************************************************************************
* Function Name	: BSP_SPIInit
* Description	: SPI初始化,使用默认配置
* Input			: num:端口号(0 ~ BSPSPI_MAX_NUMBER-1)
* Return		: TRUE/FALSE
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_SPIInit(INT8U num)
{
	INT8U perr;

    // SPI时钟配置
    McSPIModuleClkConfig(num);

    // SPI管脚配置
    McSPIPinMuxSetup(num);
	McSPICSPinMuxSetup(num);
	
    // 复位SPI
    McSPIReset(SPI_REGS_Table[num]);
    
    if(!num)
        SPI0Mutex = OSMutexCreate(PRI_SPI_MUTEX,&perr);

    return TRUE;
}
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/