/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_UART.c
**��    ��    ��: wangyao
**��  ��  ��  ��: 131211
**��  ��  ��  ��: V0.1
**��          ��: UART����
**				  20140718 �޸�ÿ�η���������FIFO�е����ݳ��ȣ�����UART�жϴ�������Ƶ��������
                  20140801 ����0xff�޷���������
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��:
**��          ��:
**�� �� ǰ �� ��:
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
#include "am335x_irq.h"
#include "uart_irda_cir.h"
#include "def_config.h"
#include "arm_com.h"


/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
// ״̬λ
typedef union
{
	INT8U byte;
	struct
	{
		unsigned bEn:1;							// �豸ʹ��
		unsigned bTxBusy:1;						// �����б�־
		unsigned bRxBusy:1;						// �����б�־		��������ɱ�־���ƣ��е��ظ�
		unsigned bRxOver:1;						// ������ɱ�־
		unsigned bTxErr:2;						// ���ͳ���
		unsigned bRxErr:2;						// ���ճ���
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
//UART���жϺ�
const INT8U UART_IRQS_Table[]={
    SYS_INT_UART0INT,
    SYS_INT_UART1INT,
    SYS_INT_UART2INT,
    SYS_INT_UART3INT,
    SYS_INT_UART4INT,
    SYS_INT_UART5INT
};
// ����ֵ��Ӧ��ϵ,����������Ŀ����Ϊ�����û����������������ֵ,
const INT8U UART_PARITY_Table[] = {//У��λ
    UART_PARITY_NONE,
    UART_ODD_PARITY,
    UART_EVEN_PARITY
};
const INT8U UART_STOPBITS_Table[] = {//ֹͣλ
    UART_FRAME_NUM_STB_1,
    UART_FRAME_NUM_STB_1_5,
    UART_FRAME_NUM_STB_2,
};
const INT8U UART_WORDLENGTH_Table[] = {//����λ
    UART_FRAME_WORD_LENGTH_5,
    UART_FRAME_WORD_LENGTH_6,
    UART_FRAME_WORD_LENGTH_7,
    UART_FRAME_WORD_LENGTH_8
};
//const _HW_DIV_NAME UART_ENABLE_Table[] = {HW_DIV_UART0,HW_DIV_UART1,HW_DIV_UART2,HW_DIV_UART3,HW_DIV_UART4};
/* Private variables--------------------------------------------------------------------------*/
typedef struct
{
	volatile _BSPUART_STATE State;				// ����״̬

	INT16U TxAddr;								// ��ǰ���͵�ַ(���͹����е���,����SendLen˵�����ݷ������)
	INT16U TxLen;								// ���������ܳ���(���͹����в���,����������)

	INT16U RxStart;								// ������ʼλ��
	INT16U RxLen;								// ���ݳ���

	volatile INT16U TxSpacetime;				// ���ʱ��,����֡��С���
	volatile INT16U RxOvertime;					// �������һ�ν��յ�����ʱ��ʱ��,����������õĳ�ʱʱ��,����Ϊ��ʱ
	_BSP_MESSAGE TxMessage;						// ���淢�ͽ�����Ϣ����
	_BSP_MESSAGE RxMessage;						// ������ս�����Ϣ����
}_BSPUART_CONTROL;
static _BSPUART_CONFIG UART_Config[BSPUART_MAX_NUMBER];	// UART���ò���
static _BSPUART_CONTROL UART_Control[BSPUART_MAX_NUMBER];	// UART���Ʋ���
/* Private functions--------------------------------------------------------------------------*/
// ���ͽ��������л�
//#define	UARTTxEnable(num)
//#define	UARTRxEnable(num)
/***********************************************************************************************
* Function Name	: BSP_UARTTxClear,BSP_UARTRxClear
* Description	: UART���
* Input			: num:�˿ں�
* Return		:
* Note(s)		: ���UART����,׼����ʼ�����շ�����
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_UARTTxClear(INT8U num)
{
	//UARTRxEnable(num);							   	// �л�������״̬,��485��Ч
    UARTIntDisable(UART_REGS_Table[num], UART_INT_THR);	// �ط����ж�
	UART_Control[num].TxAddr = 0;
	UART_Control[num].TxLen = 0;
	UART_Control[num].TxSpacetime = 0;					// ��ǰ���ͼ������
	UART_Control[num].State.bTxBusy = 0;				// �˳�����״̬
}
void BSP_UARTRxClear(INT8U num)
{
	UART_Control[num].RxStart = 0;
	UART_Control[num].RxLen = 0;
	UART_Control[num].RxOvertime = 0;					// ��ǰ��ʱʱ������
	UART_Control[num].State.bRxBusy = 0;				// �˳�����״̬
	UART_Control[num].State.bRxOver = 0;
}
//static void UARTTxErr(INT8U num)
//{
//	UART_Control[num].TxMessage.MsgID = BSP_MSGID_UART_TXERR;		// ���ڷ�����Ϣ
//	UART_Control[num].TxMessage.DivNum = num;	// ���ں�
//	UART_Control[num].TxMessage.pData = UART_Config[num].pTxBuffer;	// �����������ָ��
//	UART_Control[num].TxMessage.DataLen = UART_Control[num].TxLen;	// ���淢�����ݳ���
//	SYSPost(UART_Config[num].pEvent,&UART_Control[num].TxMessage);	// �����¼���Ϣ
//	// ���÷�����ɺ���,����485�����л��˿ڷ���
//	if(UART_HD_Table[num].Port_Pin_WD)
//
//	if(UART_Control[num].State.bTxErr < 3)
//		UART_Control[num].State.bTxErr++;		// ���ͳ���
//	BSP_UARTTxClear(num);
//}
static void UARTRxErr(INT8U num)
{
	UART_Control[num].RxMessage.MsgID = BSP_MSGID_UART_RXERR;	// ���ڽ�����Ϣ
	UART_Control[num].RxMessage.DivNum = num;	// ���ں�
	UART_Control[num].RxMessage.pData = UART_Config[num].pRxBuffer;	// �����������ָ��
	UART_Control[num].RxMessage.DataLen = UART_Control[num].RxLen;	// ����������ݳ���
	SYSPost(UART_Config[num].pEvent,&UART_Control[num].RxMessage);	// �����¼���Ϣ
	if(UART_Control[num].State.bRxErr < 3)
		UART_Control[num].State.bRxErr++;		// ���ճ���
	BSP_UARTRxClear(num);
}

/***********************************************************************************************
* Function		: UARTTxOver
* Description	: Post���������Ϣ,�����UART_Control��������ݣ�Ϊ���·�������׼��
* Input			: num UART0~5
* Output		:
* Note(s)		:
* Contributor	: 07/07/2014	wangyao
***********************************************************************************************/
static void UARTTxOver(INT8U num)
{
	UART_Control[num].TxMessage.MsgID = BSP_MSGID_UART_TXOVER;		// ���ڷ�����Ϣ
	UART_Control[num].TxMessage.DivNum = num;						// ���ں�
	UART_Control[num].TxMessage.pData = UART_Config[num].pTxBuffer;	// ���淢������ָ��
	UART_Control[num].TxMessage.DataLen = UART_Control[num].TxLen;	// ���淢�����ݳ���
	SYSPost(UART_Config[num].pEvent,&UART_Control[num].TxMessage);	// �����¼���Ϣ��UARTx��ECB
	//UART_Control[num].FuncTxOver();	// ���÷�����ɺ���,����485�����л��˿ڷ���
	UART_Control[num].State.bTxErr = 0;								// ���ͳ����־����
	BSP_UARTTxClear(num);											// ���UART_Control��TX�������
}

/***********************************************************************************************
* Function		: UARTRxOver
* Description	: Post���������Ϣ,�����UART_Control��������ݣ�Ϊ���½�������׼��
* Input			: num UART0~5
* Output		:
* Note(s)		:
* Contributor	: 07/07/2014	wangyao
***********************************************************************************************/
static void UARTRxOver(INT8U num)
{
	UART_Control[num].RxMessage.MsgID = BSP_MSGID_UART_RXOVER;		// ���ڽ�����Ϣ
	UART_Control[num].RxMessage.DivNum = num;	                	// ���ں�
	UART_Control[num].RxMessage.pData = UART_Config[num].pRxBuffer;	// �����������ָ��
	UART_Control[num].RxMessage.DataLen = UART_Control[num].RxLen;	// ����������ݳ���
	SYSPost(UART_Config[num].pEvent,&UART_Control[num].RxMessage);	// �����¼���Ϣ��UARTx��ECB
	UART_Control[num].State.bRxErr = 0;								// ���մ����־����
	UART_Control[num].State.bRxOver = 1;							// ������ɱ�־��1
	UART_Control[num].State.bRxBusy = 0;							// �˳�����״̬
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
* Description	: ��ʱ�ж�,ϵͳ��ʱ������,�û�������
* Input			:
* Return		:
* Note(s)		:
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_UARTOverTime(void)
{
	INT8U num;          //���ں�0��5

	for(num=0;num<BSPUART_MAX_NUMBER;num++)
	{
		// ������ڷ���״̬,���ͼ��ʹ��
		if(UART_Config[num].TxSpacetime)
		{
			if(UART_Control[num].TxSpacetime < UART_Config[num].TxSpacetime)
				UART_Control[num].TxSpacetime++;
		}
	}
	for(num=0;num<BSPUART_MAX_NUMBER;num++)
	{
		// ���ڽ���״̬,���ҽ��ճ�ʱʹ��
		if((UART_Control[num].State.bRxBusy) && UART_Config[num].RxOvertime)
		{
			UART_Control[num].RxOvertime++;
			if(UART_Control[num].RxOvertime > UART_Config[num].RxOvertime)
			{
				UART_Control[num].RxOvertime = 0;
				UARTRxOver(num);			//���ͽ��ճ�ʱ��Ϣ
			}
		}
		else
			UART_Control[num].RxOvertime = 0;
	}
}
/***********************************************************************************************
* Function Name	: BSP_UARTWrite
* Description	: UART����д
* Input			: num:�˿ں�
				  *pData:����������ָ��
				  len:���ݳ���
* Return		: TRUE/FALSE
* Note(s)		: ���ú�,�������ݵ����ͻ���
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_UARTWrite(INT8U num,const INT8U *pData,INT16U len)
{
    ///OS_ENTER_CRITICAL();

	// �����ж�
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

	UART_Control[num].State.bTxBusy = 1;	// ������
	// ���ͼ��
	while(UART_Control[num].TxSpacetime < UART_Config[num].TxSpacetime)
	{
		SYSTimeDly(1);
	}
	//UARTTxEnable(num);							// �л�������״̬,��485��Ч
	// ����ǲ�ѯģʽ(δʵ��)
	if(UART_Config[num].TxRespond == BSPUART_RESPOND_NORMAL)
	{
		while(len--)
		{
		}
		//UARTRxEnable(num);					// �л�������״̬,��485��Ч
		UART_Control[num].State.bTxBusy = 0;	// ��
	}
	else										// �жϻ���DMAģʽ
	{
		if(len)
		{
			//memset(UART_Config[num].pTxBuffer,0,XXX_RX_NUM);
			if(pData != UART_Config[num].pTxBuffer)
				memcpy(UART_Config[num].pTxBuffer,pData,len);
			UART_Control[num].TxAddr = 0;
			UART_Control[num].TxLen = len;
			if(UART_Config[num].TxRespond == BSPUART_RESPOND_INT)	// �ж�ģʽ
                /* Enabling the specified UART interrupts. */
                UARTIntEnable(UART_REGS_Table[num],UART_INT_THR);	// �����ж�
			//else	                                                // DMAģʽ
				//BSP_DMAStart(UART_HD_Table[num].TxDMA,len);
		}
		else
		{
			UART_Control[num].State.bTxBusy = 0;	// �˳�����״̬
		}
	}
	return TRUE;
}



/****************************************************************************************************
**����:INT8U BSP_UARTWrite_two(INT8U num,const INT8U *pData,INT16U len)
**����:UART����д
* ���:��
* ����:��
**auth:hxj, date: 2014-12-26 16:46
*****************************************************************************************************/
INT8U BSP_UARTWrite_two(INT8U num,const INT8U *pData,INT16U len)
{
    ///OS_ENTER_CRITICAL();

	// �����ж�
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

	UART_Control[num].State.bTxBusy = 1;	// ������
	// ���ͼ��
	while(UART_Control[num].TxSpacetime < UART_Config[num].TxSpacetime)
	{
		SYSTimeDly(1);
	}
	//UARTTxEnable(num);							// �л�������״̬,��485��Ч
	// ����ǲ�ѯģʽ(δʵ��)
	if(UART_Config[num].TxRespond == BSPUART_RESPOND_NORMAL)
	{
		while(len--)
		{
		}
		//UARTRxEnable(num);					// �л�������״̬,��485��Ч
		UART_Control[num].State.bTxBusy = 0;	// ��
	}
	else										// �жϻ���DMAģʽ
	{
		if(len)
		{
			//memset(UART_Config[num].pTxBuffer,0,XXX_RX_NUM);
			if(pData != UART_Config[num].pTxBuffer)
				memcpy(UART_Config[num].pTxBuffer,pData,len);
			UART_Control[num].TxAddr = 0;
			UART_Control[num].TxLen = len;
			if(UART_Config[num].TxRespond == BSPUART_RESPOND_INT)	// �ж�ģʽ
                /* Enabling the specified UART interrupts. */
                UARTIntEnable(UART_REGS_Table[num],UART_INT_THR);	// �����ж�
			//else	                                                // DMAģʽ
				//BSP_DMAStart(UART_HD_Table[num].TxDMA,len);
		}
		else
		{
			UART_Control[num].State.bTxBusy = 0;	// �˳�����״̬
		}
	}
	return TRUE;
}










/***********************************************************************************************
* Function Name	: BSP_UARTRead
* Description	: UART���ݶ�
* Input			: num:�˿ں�
				  *pData:���ݶ�ȡ����ָ��
				  len:���ݳ���
* Return		: ʵ�ʶ�ȡ�����ݳ���
* Note(s)		: ���ƴ��ڽ��ջ�������ݵ�pData,Ȼ��׼��������һ֡����,Ҳ����˵����û�ʹ�����������
                  ��ȡ��������,��ô�Ͳ���Ҫ����BSP_UARTRxClear()������.
	              ����û�ָ�������ݳ���Ϊ0,��ʹ�ý�����Ϣ(RxMessage)�б�������ݳ���
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT16U BSP_UARTRead(INT8U num,INT8U *pData,INT16U len)
{
	// �Ƿ���������
	if(UART_Control[num].State.bRxOver ==0)
		return 0;
	// ����ǲ�ѯģʽ(δʵ��)
	if(UART_Config[num].TxRespond == BSPUART_RESPOND_NORMAL)
	{
		while(len--)
		{
		}
	}
	else  // �жϻ���DMAģʽ
	{
		if(len==0)
			len = UART_Control[num].RxMessage.DataLen;
		if(len >= UART_Config[num].MaxRxBuffer)
			len = UART_Config[num].MaxRxBuffer;
		if(pData != UART_Config[num].pRxBuffer)
			memcpy(pData,UART_Config[num].pRxBuffer,len);
	}
	BSP_UARTRxClear(num);						// �������ݴ������,׼����һ֡
	return len;
}
/***********************************************************************************************
* Function Name	: BSP_UARTIRQHandler
* Description	: UART�жϷ������
* Input			: num:�˿ں�
* Return		:
* Note(s)		:
* Contributor	: 131211	wangyao
***********************************************************************************************/
#define NUM_TX_BYTES_PER_TRANS    (56)
#define NUM_RX_BYTES_PER_RECEV	  (56)



void BSP_UARTIRQHandler(INT8U num)
{
    INT32U  intId = 0;													// UART�ж�����
    INT8U	RemainBytes;												// ʣ�෢���ֽ���
    int i;


    /* Checking ths source of UART interrupt. */
    intId = UARTIntIdentityGet(UART_REGS_Table[num]);

    // ������ж�ģʽ
	if(UART_Config[num].TxRespond != BSPUART_RESPOND_NORMAL)
    {
        switch(intId)
        {
            case UART_INTID_TX_THRES_REACH:								// �жϷ���
                if(( UART_Control[num].TxLen - UART_Control[num].TxAddr) >= NUM_TX_BYTES_PER_TRANS)	// �Ƿ���������Ҫ����
                {
                    UARTFIFOWrite(UART_REGS_Table[num],					// ���ͼĴ���
                                  &UART_Config[num].pTxBuffer[UART_Control[num].TxAddr],
                                  NUM_TX_BYTES_PER_TRANS);
                    UART_Control[num].TxAddr += NUM_TX_BYTES_PER_TRANS;
                }
                else													// ������,��������
                {
					RemainBytes = UART_Control[num].TxLen % NUM_TX_BYTES_PER_TRANS;
					UARTFIFOWrite(UART_REGS_Table[num],					// ���ͼĴ���
                                  &UART_Config[num].pTxBuffer[UART_Control[num].TxAddr],
                                  RemainBytes);
                    UARTIntDisable(UART_REGS_Table[num], UART_INT_THR);	// �ط����ж�,��Ӱ���Ѿ�����TX FIFO�����ݵķ���
                    // ���ͷ���������֪ͨ
                    UARTTxOver(num);
                }
                break;
            case UART_INTID_RX_THRES_REACH:								// �жϽ���
                {
                        if(UART_Control[num].State.bRxOver ==0)				// ������ջ�û�����
                    {
                        UART_Control[num].RxOvertime = 0;				// ��ʱ�ж�ʱ������
                        UART_Control[num].State.bRxBusy = 1;			// �����б�־
                        // ������ջ�������,�������յ�����,��ͷ��ʼ����,��ز�����λ
                        if(UART_Control[num].RxLen >= UART_Config[num].MaxRxBuffer)
                            UARTRxErr(num);								// ���ͽ��մ���Ϣ
                        // �����������
                        for(i=0;i<NUM_RX_BYTES_PER_RECEV;i++)
	                    {
							UART_Config[num].pRxBuffer[UART_Control[num].RxLen] = UARTCharGetNonBlocking(UART_REGS_Table[num]);
                        	UART_Control[num].RxLen++;
	                    }
                    }
                }
                break;
            case UART_INTID_RX_LINE_STAT_ERROR:							// �жϽ������ϴ���(�������жϲ��������˴�����)
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
            case UART_INTID_CHAR_TIMEOUT:									// �жϽ��ճ�ʱ
			{
            	while(TRUE == UARTCharsAvail(UART_REGS_Table[num]))	// ��������FIFO��ʣ���ֽ�
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
		// ����,���ж�
		UARTIntDisable(UART_REGS_Table[num], (UART_INT_LINE_STAT | UART_INT_THR | UART_INT_RHR_CTI));// �ط����ж�
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
    INT32U  intId = 0;													// UART�ж�����
    INT8U	RemainBytes;												// ʣ�෢���ֽ���
    static uint8 read_buf2[NUM_RX_BYTES_PER_RECEV];
    int i;
    int r_cnt=0;


    /* Checking ths source of UART interrupt. */
    intId = UARTIntIdentityGet(UART_REGS_Table[num]);

    if(2==num)
    {
        //����Ǵ���2,�����⴦��
        // ������ж�ģʽ
    	if(UART_Config[num].TxRespond != BSPUART_RESPOND_NORMAL)
        {
            switch(intId)
            {
                case UART_INTID_TX_THRES_REACH:								// �жϷ���
                {
                    if(NULL==g_p_arm_uart_send_cbuf)
                    {
                        UARTIntDisable(UART_REGS_Table[num], UART_INT_THR);	// �ط����ж�,��Ӱ���Ѿ�����TX FIFO�����ݵķ���
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
                        if ( recCycBuf->pre_read_point >= recCycBuf->rec_buf_size ) recCycBuf->pre_read_point = 0;  //��ָ�뵽βʱ,��ָ��ͷ
                        if ( ( recCycBuf->pre_read_point == recCycBuf->write_point ) || ( temp_counter >= NUM_RX_BYTES_PER_RECEV ) )    break;  //����һ�����ݻ��߶�����ջ��λ�����,���˳�
                    }

                    if ( recCycBuf->pre_read_point == recCycBuf->write_point )
                    {
                        if ( recCycBuf->read_point == recCycBuf->write_point )
                        {
                            UARTIntDisable(UART_REGS_Table[num], UART_INT_THR); // �ط����ж�,��Ӱ���Ѿ�����TX FIFO�����ݵķ���
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
                case UART_INTID_RX_THRES_REACH:								// �жϽ���
                    {
                        if(UART_Control[num].State.bRxOver ==0)				// ������ջ�û�����
                        {
                            UART_Control[num].RxOvertime = 0;				// ��ʱ�ж�ʱ������
                            UART_Control[num].State.bRxBusy = 1;			// �����б�־

                            //������
                            r_cnt=0;
                            for(i=0;i<NUM_RX_BYTES_PER_RECEV;i++)
    	                    {
                                read_buf2[r_cnt++]=UARTCharGetNonBlocking(UART_REGS_Table[num]);
    	                    }
                            //���뻺����
                            arm_uart_recv_data_to_cyc_buf_n (read_buf2,r_cnt);

                        }
                    }
                    break;
                case UART_INTID_RX_LINE_STAT_ERROR:							// �жϽ������ϴ���(�������жϲ��������˴�����)
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
                case UART_INTID_CHAR_TIMEOUT:									// �жϽ��ճ�ʱ
    			{
                    r_cnt=0;
                	while(TRUE == UARTCharsAvail(UART_REGS_Table[num]))	// ��������FIFO��ʣ���ֽ�
                	{
                        read_buf2[r_cnt++]=UARTFIFOCharGet(UART_REGS_Table[num]);
                        if(r_cnt>=sizeof(read_buf2)) break;
                	}
                    //���뻺����
                    arm_uart_recv_data_to_cyc_buf_n (read_buf2,r_cnt);

                    break;
                }
                default:
                break;
            }
        }
    	else
    	{
    		// ����,���ж�
    		UARTIntDisable(UART_REGS_Table[num], (UART_INT_LINE_STAT | UART_INT_THR | UART_INT_RHR_CTI));// �ط����ж�
    	}

    }
    else
    {
        // ������ж�ģʽ
    	if(UART_Config[num].TxRespond != BSPUART_RESPOND_NORMAL)
        {
            switch(intId)
            {
                case UART_INTID_TX_THRES_REACH:								// �жϷ���
                    if(( UART_Control[num].TxLen - UART_Control[num].TxAddr) >= NUM_TX_BYTES_PER_TRANS)	// �Ƿ���������Ҫ����
                    {
                        UARTFIFOWrite(UART_REGS_Table[num],					// ���ͼĴ���
                                      &UART_Config[num].pTxBuffer[UART_Control[num].TxAddr],
                                      NUM_TX_BYTES_PER_TRANS);
                        UART_Control[num].TxAddr += NUM_TX_BYTES_PER_TRANS;
                    }
                    else													// ������,��������
                    {
    					RemainBytes = UART_Control[num].TxLen % NUM_TX_BYTES_PER_TRANS;
    					UARTFIFOWrite(UART_REGS_Table[num],					// ���ͼĴ���
                                      &UART_Config[num].pTxBuffer[UART_Control[num].TxAddr],
                                      RemainBytes);
                        UARTIntDisable(UART_REGS_Table[num], UART_INT_THR);	// �ط����ж�,��Ӱ���Ѿ�����TX FIFO�����ݵķ���
                        // ���ͷ���������֪ͨ
                        UARTTxOver(num);
                    }
                    break;
                case UART_INTID_RX_THRES_REACH:								// �жϽ���
                    {
                            if(UART_Control[num].State.bRxOver ==0)				// ������ջ�û�����
                        {
                            UART_Control[num].RxOvertime = 0;				// ��ʱ�ж�ʱ������
                            UART_Control[num].State.bRxBusy = 1;			// �����б�־
                            // ������ջ�������,�������յ�����,��ͷ��ʼ����,��ز�����λ
                            if(UART_Control[num].RxLen >= UART_Config[num].MaxRxBuffer)
                                UARTRxErr(num);								// ���ͽ��մ���Ϣ
                            // �����������
                            for(i=0;i<NUM_RX_BYTES_PER_RECEV;i++)
    	                    {
    							UART_Config[num].pRxBuffer[UART_Control[num].RxLen] = UARTCharGetNonBlocking(UART_REGS_Table[num]);
                            	UART_Control[num].RxLen++;
    	                    }
                        }
                    }
                    break;
                case UART_INTID_RX_LINE_STAT_ERROR:							// �жϽ������ϴ���(�������жϲ��������˴�����)
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
                case UART_INTID_CHAR_TIMEOUT:									// �жϽ��ճ�ʱ
    			{
                	while(TRUE == UARTCharsAvail(UART_REGS_Table[num]))	// ��������FIFO��ʣ���ֽ�
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
		// ����,���ж�
		UARTIntDisable(UART_REGS_Table[num], (UART_INT_LINE_STAT | UART_INT_THR | UART_INT_RHR_CTI));// �ط����ж�
	}


    }



}





// �ж�����ӳ��
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

//FIFO����
static void UartFIFOConfigure(INT8U num)
{
    unsigned int fifoConfig = 0;

    /*
    ** - Transmit Trigger Level Granularity is 4					���ʹ������4
    ** - Receiver Trigger Level Granularity is 1					���մ������1
    ** - Transmit FIFO Space Setting is 56. Hence TX Trigger level	����FIFO�ռ�ߴ�56
    **   is 8 (64 - 56). The TX FIFO size is 64 bytes.				���ʹ������8
    ** - The Receiver Trigger Level is 56.							���մ������56
    ** - Clear the Transmit FIFO.									�����FIFO
    ** - Clear the Receiver FIFO.									�巢��FIFO
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
* Description	: ���ڳ�ʼ������
* Input			: num:���ں�0 ~ BSPUART_MAX_NUMBER-1
				  *pConfig:���ò���(_BSPUART_CONFIG)
					pConfig->pEvent:�¼�ָ��,����������Ϣ�¼�
					pConfig->Baudrate:������,(���1MHz)
					pConfig->Parity:У��λ
						BSPUART_PARITY_NO:��У��(Ĭ��)
						BSPUART_PARITY_ODD:��У��
						BSPUART_PARITY_EVEN:ż����
					pConfig->StopBits:ֹͣλ
						BSPUART_STOPBITS_1:1λ(Ĭ��)
						BSPUART_STOPBITS_0_5:0.5λ(STR9��֧��)
						BSPUART_STOPBITS_1_5:1.5λ(STR9��֧��)
						BSPUART_STOPBITS_2:2λ
					pConfig->WordLength:����λ
						BSPUART_WORDLENGTH_8D:8λ����(Ĭ��)
						BSPUART_WORDLENGTH_7DP:7λ����+У��λ
						BSPUART_WORDLENGTH_8DP:8λ����+У��λ
					pConfig->Work:����ģʽ
						BSPUART_WORK_FULLDUPLEX:ȫ˫��(Ĭ��)
						BSPUART_WORK_HALFDUPLEX:��˫��(δʵ��)
						BSPUART_WORK_IRDA:����(δʵ��)
						BSPUART_WORK_IRDA_LOWPOWER:����͹���(δʵ��)
					pConfig->TxRespond:������Ӧ��ʽ
						BSPUART_RESPOND_NORMAL:��ѯ��ʽ(Ĭ��)(δʵ��)
						BSPUART_RESPOND_INT:�жϷ�ʽ
						BSPUART_RESPOND_DMA:DMA��ʽ(ֻ�з�����DMA,���ջ����жϷ�ʽ)
					pConfig->MaxTxBuffer,����ͻ���
					pConfig->MaxRxBuffer,�����ջ���
					pConfig->TxBuffer,���ͻ���ͷָ��
					pConfig->RxBuffer,���ջ���ͷָ��
					pConfig->RxOverLen,֪ͨ���ո���,���������ٸ�����,����֪ͨ.Ϊ0ʱ,ֻ���յ�һ֡���ݺ�֪ͨ
					pConfig->TxSpacetime,���ʱ��,����֡��С���
					pConfig->RxOvertime,��ʱʱ��,�����ʱ�����ղ���������,˵��һ֡���
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
    UartFIFOConfigure(num);													// ����FIFO���ڲ�ʵ����û����DMA
#endif

    //����Ŀ�겨���ʶ�Ӧ������ϵ��
    divisorvalue = UARTDivisorValCompute(UART_MODULE_CLOCK,
                                         pConfig->Baudrate,					// ������ֵ
                                         UART16x_OPER_MODE,
                                         UART_MIR_OVERSAMPLING_RATE_42);
    /* Programming the Divisor Latches. */
	//����õ�������ϵ��д����Ӧ�Ĵ������Ӷ���ɲ���������
    UARTDivisorLatchWrite(UART_REGS_Table[num], divisorvalue);
    /* Switching to Configuration Mode B. */
	//���ڿ��ƼĴ������ó�B(0xbf)ģʽ:����������������������ǿ��TX�����͵�
    UARTRegConfigModeEnable(UART_REGS_Table[num], UART_REG_CONFIG_MODE_B);
    /* Programming the Line Characteristics. */
	//���ڿ��ƼĴ�����������λ��ֹͣλ��У��λ
    UARTLineCharacConfig(UART_REGS_Table[num],
                         (UART_WORDLENGTH_Table[pConfig->WordLength]     	// ����λ
                          | UART_STOPBITS_Table[pConfig->StopBits]),     	// ֹͣλ
                          UART_PARITY_Table[pConfig->Parity]);           	// У��λ

    /* Disabling write access to Divisor Latches. */
	//�����������������������ָ���������ģʽ
    UARTDivisorLatchDisable(UART_REGS_Table[num]);
    /* Disabling Break Control. */
	//ȡ��TXǿ�����ͣ��ָ���������ģʽ
    UARTBreakCtl(UART_REGS_Table[num], UART_BREAK_COND_DISABLE);
    /* Switching to UART16x operating mode. */
    UARTOperatingModeSelect(UART_REGS_Table[num], UART16x_OPER_MODE);

    // ���Ʋ�����ʼ��,
	BSP_UARTRxClear(num);
	BSP_UARTTxClear(num);
    memcpy(&UART_Config[num],pConfig,sizeof(_BSPUART_CONFIG));				// ��������
	// ����ģʽѡ��
	switch(pConfig->TxRespond)
	{
		case BSPUART_RESPOND_DMA:				// DMA
			// ��ʼ��DMAͨ��,����
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
				return FALSE;					// �޿��õ�DMAͨ��,����
			// nobreak
		case BSPUART_RESPOND_INT:				// �ж�
            /* Register the ISR in the Interrupt Vector Table.*/
            BSP_IntVectReg(UART_IRQS_Table[num], UART_IRQ_PFUNC[num]);
            IntPrioritySet(UART_IRQS_Table[num], 0, AINTC_HOSTINT_ROUTE_IRQ );
            /* Enable the System Interrupts for AINTC.*/
            IntSystemEnable(UART_IRQS_Table[num]);
            UARTIntEnable(UART_REGS_Table[num],UART_INT_RHR_CTI);
			break;
		default:								// Ĭ��,BSPUART_RESPOND_NORMAL
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


/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/
