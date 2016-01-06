/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_spi.c
**Ӳ          ��: am335x
**��    ��    ��: wangyao
**��  ��  ��  ��: 131211
**��  ��  ��  ��: V0.1
**��          ��: SPI������������ʱֻ��Ĭ�ϵĲ�ѯģʽ���ж�ģʽ��DMAģʽ����������
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: 
**��          ��: 
**��          ��: 
**��          ��:
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
//// ����ֵ��Ӧ��ϵ,����������Ŀ����Ϊ�����û����������������ֵ,�����Ͳ���Ҫ���ǰ���"spi.h"
//����SPI0��Ҫ��W550��AT45DB�����ã���SPI0��һ·��չ������ͬ��ʽʹ�ܽ�CS��ͬ
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
static _BSPSPI_CONFIG SPI_Config[3];	// SPI���ò���
#endif
#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment=32
static _BSPSPI_CONTROL SPI_Control[3];	// SPI���Ʋ���
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
	SPI_Control[num].State.bRxBusy = 0;			// �˳�����״̬
	SPI_Control[num].State.bRxErr = 1;			// ���ճ���
}
//static void SPITxErr(INT8U num)
//{
//	SPI_Control[num].State.bTxBusy = 0;			// �˳�����״̬
//}
static void SPIRxOver(INT8U num)
{
	SPI_Control[num].RxMessage.MsgID = BSP_MSGID_SPI_RXOVER;
	SPI_Control[num].RxMessage.DivNum = num;
	SPI_Control[num].RxMessage.DataLen = SPI_Control[num].RxLen;
	SPI_Control[num].RxMessage.pData = SPI_Config[num].pRxBuffer;
	SYSPost(SPI_Config[num].pEvent,&SPI_Control[num].RxMessage);
	SPI_Control[num].State.bRxBusy = 0;			// �˳�����״̬
	SPI_Control[num].State.bRxOver = 1;			// �������
}
static void SPITxOver(INT8U num)
{
	// ���ﲻ��Ҫ�����¼�,�ڽ�������д���
	SPI_Control[num].State.bTxBusy = 0;			// �˳�����״̬
}

/***********************************************************************************************
* Function Name	: BSP_SPIOverTime
* Description	: ��ʱ�ж�,ϵͳ��ʱ������,�û�������
* Input			: 
* Return		: 
* Note(s)		: ���պͷ��ͳ�ʱ����һ����,������ֳ�ʱ����Ϊ����,���״̬
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_SPIOverTime(void)
{
	INT8U num;
	
	for(num=0;num<BSPSPI_MAX_NUMBER;num++)
	{
		// ���ڽ��ջ���״̬,���ҽ��ճ�ʱʹ��
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
    // ʹ��Ƭѡ
    McSPICSAssert(SPI_REGS_Table[num], SPI_Config[num].channel); 
    // ʹ��SPIͨ��
    McSPIChannelEnable(SPI_REGS_Table[num], SPI_Config[num].channel); 
}
void BSP_SPICSDisEnable(INT8U num)
{
    McSPICSDeAssert(SPI_REGS_Table[num], SPI_Config[num].channel);                      // �ر�Ƭѡ
	McSPIChannelDisable(SPI_REGS_Table[num], SPI_Config[num].channel);	                // ��ͨ��
    OSMutexPost(SPI0Mutex);
}
/***********************************************************************************************
* Function Name	: BSP_SPITransOneByte
* Description	: SPI���ݷ��ͺ���,����һ���ֽ�
* Input			: num:�˿ں�
				  *pData:��������ָ��,���ص�����Ҳ��������
* Return		: TRUE/FALSE
* Note(s)		: ��ѯģʽʹ��,����Ҳ�Ͳ�̫׷���ٶ�,һ��һ���Ĳ���
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
	// ���ͽ���
    McSPITransmitData(SPI_REGS_Table[num],(INT32U)(*pData),SPI_Config[num].channel);//���� 
	// �ȴ�����
	count = 0;
	while((McSPIChannelStatusGet(SPI_REGS_Table[num],SPI_Config[num].channel) & MCSPI_CH_STAT_RXS_FULL)==RESET)
	{
		if(count++ >= 0x100)
			return FALSE;
	}
	*pData = (INT8U)McSPIReceiveData(SPI_REGS_Table[num],SPI_Config[num].channel);	// ����

    
	return TRUE;
}
//spi ����  1��дģʽ 0Ϊ��ģʽ
static INT8U BSP_SPITransBytes(INT8U num,INT8U *pData,INT16U len,INT8U state)
{
    if(len == 0)
		return TRUE;

	switch(SPI_Config[num].TxRespond)
	{
		case BSPSPI_RESPOND_INT:
            if(len > SPI_Config[num].MaxTxBuffer)
				return FALSE;
			// �������ݵ����ͻ���
			if(pData != SPI_Config[num].pTxBuffer)
				memcpy(SPI_Config[num].pTxBuffer,pData,len);

			// ���÷��ͳ���
			McSPIWordCountSet(SPI_REGS_Table[num],len);
			SPI_Config[num].MaxRxBuffer = len;

		    SPI_Control[num].TxAddr = 0;		// �����׵�ַ
			SPI_Control[num].RxLen = 0;			// ������ʼ 
			
		    // ʹ��Ƭѡ
		    McSPICSAssert(SPI_REGS_Table[num], SPI_Config[num].channel);                                                
		    // ���շ��ж�
		    McSPIIntEnable(SPI_REGS_Table[num],MCSPI_INT_TX_EMPTY(SPI_Config[num].channel) | MCSPI_INT_RX_FULL(SPI_Config[num].channel));  
		    // ʹ��SPI
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
				if(state)						// 1:д0����
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
* Description	: SPI���
* Input			: num:�˿ں�
* Return		: 
* Note(s)		: ���SPI����,׼�����¿�ʼ,�жϻ���DMAģʽ��,�û������ڴ�����һ֡���ݺ���ô˺���
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_SPIClear(INT8U num)
{
	memset((INT8U *)&SPI_Control[num],0x00,sizeof(_BSPSPI_CONTROL));
}
/***********************************************************************************************
* Function Name	: BSP_SPIWrite
* Description	: SPI����д
* Input			: num:�˿ں�
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
* Description	: SPI���ݶ�
* Input			: num:�˿ں�
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
* Description	: SPI�жϷ������
* Input			: num:�˿ں�
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
		if(MCSPI_INT_EOWKE == (intCode & MCSPI_INT_EOWKE))		                                    // ����Ƿ�������ж�Դ
		{
			McSPIIntDisable(SPI_REGS_Table[num], MCSPI_INT_TX_EMPTY(SPI_Config[num].channel));	    // �ط����ж�
			McSPIIntStatusClear(SPI_REGS_Table[num], MCSPI_INT_EOWKE);	
			SPITxOver(num);
		}
		else if(MCSPI_INT_TX_EMPTY(SPI_Config[num].channel) ==(intCode & MCSPI_INT_TX_EMPTY(SPI_Config[num].channel)))  // ����Ƿ����ж�Դ
		{
			McSPIIntStatusClear(SPI_REGS_Table[num], MCSPI_INT_TX_EMPTY(SPI_Config[num].channel));				
			McSPITransmitData(SPI_REGS_Table[num],
					(INT32U)SPI_Config[num].pTxBuffer[SPI_Control[num].TxAddr++], 
					SPI_Config[num].channel);	
		}
		
	    if(MCSPI_INT_RX_FULL(SPI_Config[num].channel) == (intCode & MCSPI_INT_RX_FULL(SPI_Config[num].channel)))        // ����ǽ����ж�Դ
	    {
	    	INT8U data;
	    	
			McSPIIntStatusClear(SPI_REGS_Table[num], MCSPI_INT_RX_FULL(SPI_Config[num].channel));
			data = (INT8U) McSPIReceiveData(SPI_REGS_Table[num], SPI_Config[num].channel);
			SPI_Control[num].RxOvertime = 0;

			SPI_Config[num].pRxBuffer[SPI_Control[num].RxLen++]= data;

			if(SPI_Control[num].RxLen == SPI_Config[num].MaxRxBuffer)			                    // ����������
			{
				McSPIIntDisable(SPI_REGS_Table[num], MCSPI_INT_RX_FULL(SPI_Config[num].channel));   // �ؽ����ж�
				/* Force SPIEN line to the inactive state.*/    
				McSPICSDeAssert(SPI_REGS_Table[num], SPI_Config[num].channel);                      // �ر�Ƭѡ
				McSPIChannelDisable(SPI_REGS_Table[num], SPI_Config[num].channel);	                // ��ͨ��
				SPIRxOver(num);													                    // �������ͽ��յ�������Ϣ
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
* Description	: SPI����
* Input			: num:�˿ں�(0 ~ BSPSPI_MAX_NUMBER-1)
				  *pConfig:���ò���(_BSPSPI_CONFIG)
					pConfig->pEvent,�¼�ָ��
					pConfig->Baudrate,������,ʵ�ʲ����ʿ��ܺ�����ֵ��ͬ(���18MHz)
					pConfig->FirstBit,������λ(_BSPSPI_FIRSTBIT)
						BSPSPI_FIRSTBIT_LOW,��λ��ǰ
						BSPSPI_FIRSTBIT_HIGH,��λ��ǰ
					pConfig->Phase,ʱ����λ(_BSPSPI_CLK_PHASE)
						BSPSPI_CLK_PHASE_1EDGE,�����ӵ�һ��ʱ�ӱ��ؿ�ʼ
						BSPSPI_CLK_PHASE_2EDGE,�����ӵڶ���ʱ�ӱ��ؿ�ʼ
					pConfig->Polarity,ʱ�Ӽ���(_BSPSPI_CLK_POLARITY)
						BSPSPI_CLK_POLARITY_LOW,����ʱ��
						BSPSPI_CLK_POLARITY_HIGH,����ʱ��
					pConfig->LineMode,��ģʽ(_BSPSPI_LINEMODE)
						BSPSPI_LINEMODE_FULLDUPLEX,˫��ȫ˫��
						BSPSPI_LINEMODE_2LINERX,˫��ֻ����
						BSPSPI_LINEMODE_1LINERXTX,���߽��շ���,������Լ��ӵ�,��ʼ����,�Զ��л�����
						BSPSPI_LINEMODE_1LINERX,���߽���
						BSPSPI_LINEMODE_1LINETX,���߷���
					pConfig->Mode,����ģʽ(_BSPSPI_WORKMODE)
						BSPSPI_WORKMODE_MASTER,��ģʽ
						BSPSPI_WORKMODE_SLAVE,��ģʽ
					pConfig->TxRespond,������Ӧģʽ(��ѯ,INT,DMA)(_BSPSPI_RESPOND)
						BSPSPI_RESPOND_NORMAL,��ѯ��ʽ(Ĭ��)
						BSPSPI_RESPOND_INT,�жϷ�ʽ(���ͽ��ն����жϷ�ʽ)
						BSPSPI_RESPOND_DMA,DMA��ʽ(���ͽ��ն���DMA�жϷ�ʽ)
					pConfig->MaxTxBuffer,����ͻ���
					pConfig->MaxRxBuffer,�����ջ���
					pConfig->TxBuffer,���ͻ���ָ��
					pConfig->RxBuffer,���ջ���ָ��
					pConfig->RxOvertime,���ճ�ʱ
* Return		: TRUE/FALSE
* Note(s)		: 
* Contributor	: 090505	wangyao
***********************************************************************************************/
#define MCSPI_OUT_FREQ                   (48000000u)	// 48M ���
#define MCSPI_IN_CLK                     (48000000u)    // 48M CLK
INT8U BSP_SPIConfig(_BSPSPI_CONFIG *pConfig)
{	
	
    // ʹ��Ƭѡ���ܣ�ֻ��MCSPI_MODULCTRL_PIN34���㣬����ִ�к����Ƭѡ����
    McSPICSEnable(SPI_REGS_Table[pConfig->num]);
	
    switch(pConfig->Mode)
    {        
        case BSPSPI_WORKMODE_SLAVE:								// ��ģʽ
            //��ģʽʹ��
            McSPISlaveModeEnable(SPI_REGS_Table[pConfig->num]);	
			// ���ͽ���˫��ģʽ��D1���ա�D0�߷�
            McSPISlaveModeConfig(SPI_REGS_Table[pConfig->num],MCSPI_TX_RX_MODE, MCSPI_DATA_LINE_COMM_MODE_6,pConfig->channel);   	
			// SPIЭ��0ģʽ����IDLEʱCLK�͵�ƽ����������������
    		HWREG(SPI_REGS_Table[pConfig->num] + MCSPI_CHCONF(pConfig->channel)) |= (MCSPI_CLK_MODE_0 & (MCSPI_CH0CONF_PHA | MCSPI_CH0CONF_POL));
			// SPIЭ��3ģʽ����Ӧ��ǰDSP����
//    		HWREG(SPI_REGS_Table[pConfig->num] + MCSPI_CHCONF(pConfig->channel)) |= (MCSPI_CLK_MODE_3 & (MCSPI_CH0CONF_PHA | MCSPI_CH0CONF_POL));
            break;
        case BSPSPI_WORKMODE_MASTER:							// ��ģʽ
            // ��ģʽʹ��
            McSPIMasterModeEnable(SPI_REGS_Table[pConfig->num]);			
            // ���ͽ���˫��ģʽ����ͨ����D1�߷���D0����
            McSPIMasterModeConfig(SPI_REGS_Table[pConfig->num],MCSPI_SINGLE_CH,MCSPI_TX_RX_MODE,MCSPI_DATA_LINE_COMM_MODE_1,pConfig->channel);  						
            // SPIЭ��0ģʽ��ʱ��Ƶ��48MHz
            McSPIClkConfig(SPI_REGS_Table[pConfig->num],MCSPI_IN_CLK,MCSPI_OUT_FREQ,pConfig->channel,MCSPI_CLK_MODE_0);            
            break;
        default:
        	break;
    } 
	
    // 8bit����
    McSPIWordLengthSet(SPI_REGS_Table[pConfig->num], MCSPI_WORD_LENGTH(8), pConfig->channel);	
    // CS�߼���:�͵�ƽ��Ծ
    McSPICSPolarityConfig(SPI_REGS_Table[pConfig->num], MCSPI_CS_POL_LOW, pConfig->channel);   
	// ʹ��TX	FIFO
    McSPITxFIFOConfig(SPI_REGS_Table[pConfig->num], MCSPI_TX_FIFO_ENABLE, pConfig->channel);		
    // ʹ��RX	FIFO
    McSPIRxFIFOConfig(SPI_REGS_Table[pConfig->num], MCSPI_RX_FIFO_ENABLE, pConfig->channel); 

//	McSPIFIFOTrigLvlSet(SPI_REGS_Table[pConfig->num],2,2,MCSPI_TX_RX_MODE);
	
	switch(pConfig->TxRespond)
	{
		case BSPSPI_RESPOND_INT:				// �ж�
		{
            // ע��SPI�жϴ�����
            BSP_IntVectReg(SPI_INT_Table[pConfig->num], PFUNC[pConfig->num]);
			// ����SPI�ж����ȼ�
            IntPrioritySet(SPI_INT_Table[pConfig->num], 0, AINTC_HOSTINT_ROUTE_IRQ );
            // �жϿ��ƿ���ʹ��SPI�ж�
            IntSystemEnable(SPI_INT_Table[pConfig->num]);     
			// �������ֽ������ã�Ϊ�˲���EOW�ж�
		    McSPIWordCountSet(SPI_REGS_Table[pConfig->num],pConfig->MaxTxBuffer);
			// ʹ��EOW�ж�
			McSPIIntEnable(SPI_REGS_Table[pConfig->num],MCSPI_INT_EOWKE);
            //McSPICSDisable(SPI_REGS_Table[num]);// ������,��Ȼ��һ���ֽڻ��
			break;
		}
		case BSPSPI_RESPOND_DMA:				// DMA
		{			
			// no break;
		}
		default:								// Ĭ��,BSPSPI_RESPOND_NORMAL
			// ��ֹ�����ж�			
			break;
	}
	
	// ���úͿ��Ʋ�����ʼ��
	BSP_SPIClear(pConfig->num);											
    memcpy(&SPI_Config[pConfig->num],pConfig,sizeof(_BSPSPI_CONFIG));	

	return TRUE;
}
/***********************************************************************************************
* Function Name	: BSP_SPIInit
* Description	: SPI��ʼ��,ʹ��Ĭ������
* Input			: num:�˿ں�(0 ~ BSPSPI_MAX_NUMBER-1)
* Return		: TRUE/FALSE
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_SPIInit(INT8U num)
{
	INT8U perr;

    // SPIʱ������
    McSPIModuleClkConfig(num);

    // SPI�ܽ�����
    McSPIPinMuxSetup(num);
	McSPICSPinMuxSetup(num);
	
    // ��λSPI
    McSPIReset(SPI_REGS_Table[num]);
    
    if(!num)
        SPI0Mutex = OSMutexCreate(PRI_SPI_MUTEX,&perr);

    return TRUE;
}
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/