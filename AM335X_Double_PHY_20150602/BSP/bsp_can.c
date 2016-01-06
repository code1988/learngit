/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_can.c
**Ӳ          ��: am335x
**��    ��    ��: wangyao
**��  ��  ��  ��: 2013-09-25
**��  ��  ��  ��: V0.1
**��          ��: DCAN��������
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
#include "bsp_int.h"
#include "dcan_frame.h"
#include "dcan.h"
#include "interrupt.h"
#include "am335x_irq.h"
/* Private define-----------------------------------------------------------------------------*/
#define DCAN_IN_CLK                       (24000000u)
#define DCAN_BIT_RATE                     (20000u)//(1000000u) //λ����20k
/* Private typedef----------------------------------------------------------------------------*/
typedef struct
{
    INT32U TxLen; //���ͳ���   
    INT32U RxLen; //���ճ���
    _BSP_MESSAGE TxMessage;						// ���淢�ͽ�����Ϣ����
	_BSP_MESSAGE RxMessage;						// ������ս�����Ϣ����    
}_BSPDCAN_CONTROL;
static _BSPDCAN_CONTROL DCAN_Control[BSPCAN_MAX_NUMBER];	// CAN���Ʋ���
static _BSPDCAN_CONFIG  DCAN_Config[BSPCAN_MAX_NUMBER];     // CAN���ò���
/* Private macro------------------------------------------------------------------------------*/
//CAN�Ĵ���
const INT32U DCAN_REGS_Table[] = {SOC_DCAN_0_REGS, SOC_DCAN_1_REGS};
//CAN���жϺ�
const INT8U DCAN_IRQS_Table[] = {SYS_INT_DCAN0_INT0, SYS_INT_DCAN1_INT0};//����������ֻ��LINE0��
//CAN����żУ���жϺ�
const INT8U DCAN_ParityIRQS_Table[] = {SYS_INT_DCAN0_PARITY, SYS_INT_DCAN1_PARITY};
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
static void DCANSTDRxOver(INT8U num)//��׼֡�������
{
	DCAN_Control[num].RxMessage.MsgID = BSP_MSGID_DCAN_STDRXOVER;	// ��׼֡������Ϣ
	DCAN_Control[num].RxMessage.DivNum = num;	                    // CAN�˿ں�
	DCAN_Control[num].RxMessage.pData = (INT8U *)DCAN_Config[num].pRxBuffer;	// �����������ָ��
	DCAN_Control[num].RxMessage.DataLen = DCAN_Control[num].RxLen;	// ����������ݳ���
	SYSPost(DCAN_Config[num].pEvent,&DCAN_Control[num].RxMessage);	// �����¼���Ϣ
}
static void DCANEXTRxOver(INT8U num)//����֡�������
{
	DCAN_Control[num].RxMessage.MsgID = BSP_MSGID_DCAN_EXTRXOVER;	// ����֡������Ϣ
	DCAN_Control[num].RxMessage.DivNum = num;	                    // CAN�˿ں�
	DCAN_Control[num].RxMessage.pData = (INT8U *)DCAN_Config[num].pRxBuffer;	// �����������ָ��
	DCAN_Control[num].RxMessage.DataLen = DCAN_Control[num].RxLen;	// ����������ݳ���
	SYSPost(DCAN_Config[num].pEvent,&DCAN_Control[num].RxMessage);	// �����¼���Ϣ
}
static void DCANRxErr(INT8U num)//���ݽ��ճ�����ʱ�Դ���ϸ����
{
	DCAN_Control[num].RxMessage.MsgID = BSP_MSGID_DCAN_RXERR;	    // ������Ϣ
	DCAN_Control[num].RxMessage.DivNum = num;	                    // CAN�˿ں�
	DCAN_Control[num].RxMessage.pData = (INT8U *)DCAN_Config[num].pRxBuffer;	// �����������ָ��
	DCAN_Control[num].RxMessage.DataLen = DCAN_Control[num].RxLen;	// ����������ݳ���
	SYSPost(DCAN_Config[num].pEvent,&DCAN_Control[num].RxMessage);	// �����¼���Ϣ
}
/***********************************************************************************************
* Function	    : BSP_DCANWrite 
* Description	: CAN���ͺ���
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   ��ҫ
***********************************************************************************************/
void BSP_DCANWrite(_BSPDCAN_CONFIG canconfig)
{  
    /* Configure a transmit message object */
    CANMsgObjectConfig(DCAN_REGS_Table[canconfig.num], &canconfig.entry);
}
/***********************************************************************************************
* Function	    : BSP_DCANParityIRQHandler 
* Description	: CAN����żУ���жϴ������
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   ��ҫ
***********************************************************************************************/
#define DCAN_ERROR_OCCURED                (0x8000u)
void BSP_DCANParityIRQHandler(INT8U num)
{
    INT32U errVal;
  
    //��ȡ�Ĵ���״̬λ
    if(DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT) == DCAN_ERROR_OCCURED)
    {
        /* Check the status of DCAN Status and error register */
        errVal = DCANErrAndStatusRegInfoGet(DCAN_REGS_Table[num]);
        if(errVal & DCAN_PARITY_ERR_DETECTED)    
        {
            //������Ϣ��ʱ����INT32U���ݽ���ʾ�������Ϣ���ⲿ�û�ʹ��
            /* Read the word number where parity error got detected */
            DCAN_Config[num].pRxBuffer[0] = DCANParityErrCdRegStatusGet(DCAN_REGS_Table[num], DCAN_PARITY_ERR_WRD_NUM);
            /* Read the message number where parity error got detected */
            DCAN_Config[num].pRxBuffer[1] = DCANParityErrCdRegStatusGet(DCAN_REGS_Table[num],  DCAN_PARITY_ERR_MSG_NUM);
            //������Ϣ
            DCANRxErr(num);
       }
    }
}
/***********************************************************************************************
* Function	    : BSP_DCANIRQHandler
* Description	: CAN�������շ��жϴ�����
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   ��ҫ
***********************************************************************************************/
#define DCAN_NO_INT_PENDING               (0x00000000u)
void BSP_DCANIRQHandler(INT8U num)
{

    INT32U errval,msgNum;
    INT32U IDnum;
    
    //��ȡ�Ĵ���״̬
    while(DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT))
    {
        //CAN�������
        if(DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT) == DCAN_ERROR_OCCURED)
        {
            /* ������Ϣ�˶� */
            errval = DCANErrAndStatusRegInfoGet(DCAN_REGS_Table[num]);  
            
            if(errval & DCAN_MOD_IN_BUS_OFF_STATE)//����ûʹ��
            {
                /*����澯���ƵĴ����£�ʹ������һ��*/
                DCANAutoBusOnControl(DCAN_REGS_Table[num], DCAN_AUTO_BUS_ON_ENABLE);
            }
            //Atleast one of the error counters have reached the error warning limit
            if(errval & DCAN_ERR_WARN_STATE_RCHD)
            {
                //������Ϣ
                DCANRxErr(num);   
            }
        }
        //�����޴���
        if((DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT) != DCAN_NO_INT_PENDING) 
           && (DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT) != DCAN_ERROR_OCCURED))
        {
            /* ��ȡ���ĸ���ϢĿ��message object�����ж� */
            msgNum = DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT);
    
            /* С��CAN_NUM_OF_MSG_OBJS/2��CAN���� */
            if(msgNum < (CAN_NUM_OF_MSG_OBJS/2))
            {
                /* Clear the Interrupt pending status */
                CANClrIntPndStat(DCAN_REGS_Table[num], msgNum, DCAN_IF1_REG);              
            }
            /* �����ж��ǽ��� */
            if((msgNum >= (CAN_NUM_OF_MSG_OBJS/2)) && (msgNum < CAN_NUM_OF_MSG_OBJS))
            {
                /* Read a received message from message RAM to interface register */
                CANReadMsgObjData(DCAN_REGS_Table[num], msgNum, DCAN_Config[num].pRxBuffer, DCAN_IF2_REG);
                //��ȡ֡ID
                CANReadIDObjData(DCAN_REGS_Table[num], msgNum, &IDnum, DCAN_IF2_REG);
                IDnum = (IDnum>>18);
                IDnum = IDnum&0x1FFF;
                IDnum = IDnum<<5;
                DCAN_Config[num].entry.id = IDnum;
                //��¼������Ϣ
                DCAN_Control[num].RxLen = (DCANIFMsgCtlStatusGet(SOC_DCAN_0_REGS, DCAN_IF2_REG) & DCAN_DAT_LEN_CODE_READ);
                if((DCANIFArbStatusGet(SOC_DCAN_0_REGS, DCAN_IF2_REG) & DCAN_EXT_ID_READ) == DCAN_29_BIT_ID)//����֡
                {
                     DCANEXTRxOver(num);
                }
                else//��׼֡
                {
                    DCANSTDRxOver(num);
                }
                /* Clear the Interrupt pending status */
                CANClrIntPndStat(DCAN_REGS_Table[num], msgNum, DCAN_IF2_REG);
           }   
        }
    }
}
void (*DCAN_IRQ_PFUNC[2])(void)=
{
    DCAN0_IRQHandler,
    DCAN1_IRQHandler
};
void (*DCAN_ParityIRQ_PFUNC[2])(void)=
{
    DCAN0_ParityIRQHandler,
    DCAN1_ParityIRQHandler
};
/***********************************************************************************************
* Function	    : BSP_DCANHW_Init 
* Description	: CANӲ�����ʼ��
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   ��ҫ
***********************************************************************************************/
void BSP_DCANHW_Init(INT8U num)
{
    /* Enable the DCAN module clock */
    DCANModuleClkConfig();
    /* Perform the pinmux for DCAN */
    DCANPinMuxSetUp(num);
    /* Initialize the DCAN message RAM */
    DCANMsgRAMInit(num);        
}
/***********************************************************************************************
* Function	    : BSP_DCANInit 
* Description	: CAN��ʼ��������
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   ��ҫ
***********************************************************************************************/
void BSP_DCANInit(INT8U num,_BSPDCAN_CONFIG *pConfig)
{
    INT8U index;
    /*Ӳ����ʼ��*/
    BSP_DCANHW_Init(num);
    /*CAN�շ������жϹ���ʹ��*/
    /* Register the ISR in the Interrupt Vector Table.*/
    BSP_IntVectReg(DCAN_IRQS_Table[num], DCAN_IRQ_PFUNC[num]);
    IntPrioritySet(DCAN_IRQS_Table[num], 0, AINTC_HOSTINT_ROUTE_IRQ );
    /* Enable the System Interrupts for AINTC.*/
    IntSystemEnable(DCAN_IRQS_Table[num]);
    
    /*CAN��żУ�鴦���жϹ���ʹ��*/
    /* Register the ISR in the Interrupt Vector Table.*/
    BSP_IntVectReg(DCAN_ParityIRQS_Table[num], DCAN_ParityIRQ_PFUNC[num]);
    IntPrioritySet(DCAN_ParityIRQS_Table[num], 0, AINTC_HOSTINT_ROUTE_IRQ );
    /* Enable the System Interrupts for AINTC.*/
    IntSystemEnable(DCAN_ParityIRQS_Table[num]); 
    
    /* Reset the DCAN module */
    DCANReset(DCAN_REGS_Table[num]);
    /* Enter the Initialization mode of CAN controller */
    DCANInitModeSet(DCAN_REGS_Table[num]);
    /* Enable the write access to the DCAN configuration registers */
    DCANConfigRegWriteAccessControl(DCAN_REGS_Table[num], DCAN_CONF_REG_WR_ACCESS_ENABLE);
    /* Configure the bit timing values for CAN communication */
    CANSetBitTiming(DCAN_REGS_Table[num], DCAN_IN_CLK, DCAN_BIT_RATE);
    /*
    ** This feature will automatically get the CAN bus to bus-on
    ** state once the error counters are below the error warning
    ** limit.
    */
    DCANAutoBusOnTimeValSet(DCAN_REGS_Table[num], DCAN_IN_CLK / 100);
    DCANAutoBusOnControl(DCAN_REGS_Table[num], DCAN_AUTO_BUS_ON_ENABLE);
    /* Disable the write access to the DCAN configuration registers */
    DCANConfigRegWriteAccessControl(DCAN_REGS_Table[num], DCAN_CONF_REG_WR_ACCESS_DISABLE);
    //��������
    memcpy(&DCAN_Config[num],pConfig,sizeof(_BSPDCAN_CONFIG));
    
    index = CAN_NUM_OF_MSG_OBJS;
    while(index)
    {
        /* Invalidate all message objects in the message RAM */
        CANInValidateMsgObject(DCAN_REGS_Table[num], index, DCAN_IF2_REG);
        index--;
    }  
    DCAN_Config[num].entry.flag = (CAN_DATA_FRAME | CAN_MSG_DIR_RX);
    DCAN_Config[num].entry.id = 0;
    /* 
    ** Configure a receive message object to accept CAN 
    ** frames with standard ID.
    */
    CANMsgObjectConfig(DCAN_REGS_Table[num], &DCAN_Config[num].entry);
    DCAN_Config[num].entry.flag = (CAN_EXT_FRAME | CAN_MSG_DIR_RX | CAN_DATA_FRAME);
    DCAN_Config[num].entry.id = 0;
    /*
    ** Configure a receive message object to accept CAN
    ** frames with extended ID.
    */
    CANMsgObjectConfig(DCAN_REGS_Table[num], &DCAN_Config[num].entry);
    /* Start the CAN transfer */
    DCANNormalModeSet(DCAN_REGS_Table[num]);
    /* Enable the error interrupts */
    DCANIntEnable(DCAN_REGS_Table[num], DCAN_ERROR_INT);
    /* Enable the interrupt line 0 of DCAN module */
    DCANIntLineEnable(DCAN_REGS_Table[num], DCAN_INT_LINE0);
}

/************************(C)COPYRIGHT 2010 �㽭��̩*****END OF FILE****************************/
