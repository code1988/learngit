/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_can.c
**硬          件: am335x
**创    建    人: wangyao
**创  建  日  期: 2013-09-25
**最  新  版  本: V0.1
**描          述: DCAN驱动程序
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
#include "bsp_int.h"
#include "dcan_frame.h"
#include "dcan.h"
#include "interrupt.h"
#include "am335x_irq.h"
/* Private define-----------------------------------------------------------------------------*/
#define DCAN_IN_CLK                       (24000000u)
#define DCAN_BIT_RATE                     (20000u)//(1000000u) //位速率20k
/* Private typedef----------------------------------------------------------------------------*/
typedef struct
{
    INT32U TxLen; //发送长度   
    INT32U RxLen; //接收长度
    _BSP_MESSAGE TxMessage;						// 保存发送结束消息内容
	_BSP_MESSAGE RxMessage;						// 保存接收结束消息内容    
}_BSPDCAN_CONTROL;
static _BSPDCAN_CONTROL DCAN_Control[BSPCAN_MAX_NUMBER];	// CAN控制参数
static _BSPDCAN_CONFIG  DCAN_Config[BSPCAN_MAX_NUMBER];     // CAN配置参数
/* Private macro------------------------------------------------------------------------------*/
//CAN寄存器
const INT32U DCAN_REGS_Table[] = {SOC_DCAN_0_REGS, SOC_DCAN_1_REGS};
//CAN的中断号
const INT8U DCAN_IRQS_Table[] = {SYS_INT_DCAN0_INT0, SYS_INT_DCAN1_INT0};//这里驱动暂只用LINE0。
//CAN的奇偶校验中断号
const INT8U DCAN_ParityIRQS_Table[] = {SYS_INT_DCAN0_PARITY, SYS_INT_DCAN1_PARITY};
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
static void DCANSTDRxOver(INT8U num)//标准帧接收完成
{
	DCAN_Control[num].RxMessage.MsgID = BSP_MSGID_DCAN_STDRXOVER;	// 标准帧接收消息
	DCAN_Control[num].RxMessage.DivNum = num;	                    // CAN端口号
	DCAN_Control[num].RxMessage.pData = (INT8U *)DCAN_Config[num].pRxBuffer;	// 保存接收数据指针
	DCAN_Control[num].RxMessage.DataLen = DCAN_Control[num].RxLen;	// 保存接收数据长度
	SYSPost(DCAN_Config[num].pEvent,&DCAN_Control[num].RxMessage);	// 发送事件消息
}
static void DCANEXTRxOver(INT8U num)//扩张帧接收完成
{
	DCAN_Control[num].RxMessage.MsgID = BSP_MSGID_DCAN_EXTRXOVER;	// 扩张帧接收消息
	DCAN_Control[num].RxMessage.DivNum = num;	                    // CAN端口号
	DCAN_Control[num].RxMessage.pData = (INT8U *)DCAN_Config[num].pRxBuffer;	// 保存接收数据指针
	DCAN_Control[num].RxMessage.DataLen = DCAN_Control[num].RxLen;	// 保存接收数据长度
	SYSPost(DCAN_Config[num].pEvent,&DCAN_Control[num].RxMessage);	// 发送事件消息
}
static void DCANRxErr(INT8U num)//数据接收出错，暂时对错误不细化。
{
	DCAN_Control[num].RxMessage.MsgID = BSP_MSGID_DCAN_RXERR;	    // 错误消息
	DCAN_Control[num].RxMessage.DivNum = num;	                    // CAN端口号
	DCAN_Control[num].RxMessage.pData = (INT8U *)DCAN_Config[num].pRxBuffer;	// 保存接收数据指针
	DCAN_Control[num].RxMessage.DataLen = DCAN_Control[num].RxLen;	// 保存接收数据长度
	SYSPost(DCAN_Config[num].pEvent,&DCAN_Control[num].RxMessage);	// 发送事件消息
}
/***********************************************************************************************
* Function	    : BSP_DCANWrite 
* Description	: CAN发送函数
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   王耀
***********************************************************************************************/
void BSP_DCANWrite(_BSPDCAN_CONFIG canconfig)
{  
    /* Configure a transmit message object */
    CANMsgObjectConfig(DCAN_REGS_Table[canconfig.num], &canconfig.entry);
}
/***********************************************************************************************
* Function	    : BSP_DCANParityIRQHandler 
* Description	: CAN的奇偶校验中断处理程序
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   王耀
***********************************************************************************************/
#define DCAN_ERROR_OCCURED                (0x8000u)
void BSP_DCANParityIRQHandler(INT8U num)
{
    INT32U errVal;
  
    //获取寄存器状态位
    if(DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT) == DCAN_ERROR_OCCURED)
    {
        /* Check the status of DCAN Status and error register */
        errVal = DCANErrAndStatusRegInfoGet(DCAN_REGS_Table[num]);
        if(errVal & DCAN_PARITY_ERR_DETECTED)    
        {
            //错误消息的时候，两INT32U数据将表示错误的信息供外部用户使用
            /* Read the word number where parity error got detected */
            DCAN_Config[num].pRxBuffer[0] = DCANParityErrCdRegStatusGet(DCAN_REGS_Table[num], DCAN_PARITY_ERR_WRD_NUM);
            /* Read the message number where parity error got detected */
            DCAN_Config[num].pRxBuffer[1] = DCANParityErrCdRegStatusGet(DCAN_REGS_Table[num],  DCAN_PARITY_ERR_MSG_NUM);
            //发送消息
            DCANRxErr(num);
       }
    }
}
/***********************************************************************************************
* Function	    : BSP_DCANIRQHandler
* Description	: CAN的数据收发中断处理函数
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   王耀
***********************************************************************************************/
#define DCAN_NO_INT_PENDING               (0x00000000u)
void BSP_DCANIRQHandler(INT8U num)
{

    INT32U errval,msgNum;
    INT32U IDnum;
    
    //获取寄存器状态
    while(DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT))
    {
        //CAN传输出错
        if(DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT) == DCAN_ERROR_OCCURED)
        {
            /* 错误信息核对 */
            errval = DCANErrAndStatusRegInfoGet(DCAN_REGS_Table[num]);  
            
            if(errval & DCAN_MOD_IN_BUS_OFF_STATE)//总线没使能
            {
                /*错误告警限制的次数下，使能总线一次*/
                DCANAutoBusOnControl(DCAN_REGS_Table[num], DCAN_AUTO_BUS_ON_ENABLE);
            }
            //Atleast one of the error counters have reached the error warning limit
            if(errval & DCAN_ERR_WARN_STATE_RCHD)
            {
                //发送消息
                DCANRxErr(num);   
            }
        }
        //传输无错误
        if((DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT) != DCAN_NO_INT_PENDING) 
           && (DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT) != DCAN_ERROR_OCCURED))
        {
            /* 获取是哪个信息目标message object引起中断 */
            msgNum = DCANIntRegStatusGet(DCAN_REGS_Table[num], DCAN_INT_LINE0_STAT);
    
            /* 小于CAN_NUM_OF_MSG_OBJS/2，CAN发送 */
            if(msgNum < (CAN_NUM_OF_MSG_OBJS/2))
            {
                /* Clear the Interrupt pending status */
                CANClrIntPndStat(DCAN_REGS_Table[num], msgNum, DCAN_IF1_REG);              
            }
            /* 其他中断是接收 */
            if((msgNum >= (CAN_NUM_OF_MSG_OBJS/2)) && (msgNum < CAN_NUM_OF_MSG_OBJS))
            {
                /* Read a received message from message RAM to interface register */
                CANReadMsgObjData(DCAN_REGS_Table[num], msgNum, DCAN_Config[num].pRxBuffer, DCAN_IF2_REG);
                //获取帧ID
                CANReadIDObjData(DCAN_REGS_Table[num], msgNum, &IDnum, DCAN_IF2_REG);
                IDnum = (IDnum>>18);
                IDnum = IDnum&0x1FFF;
                IDnum = IDnum<<5;
                DCAN_Config[num].entry.id = IDnum;
                //记录长度信息
                DCAN_Control[num].RxLen = (DCANIFMsgCtlStatusGet(SOC_DCAN_0_REGS, DCAN_IF2_REG) & DCAN_DAT_LEN_CODE_READ);
                if((DCANIFArbStatusGet(SOC_DCAN_0_REGS, DCAN_IF2_REG) & DCAN_EXT_ID_READ) == DCAN_29_BIT_ID)//扩张帧
                {
                     DCANEXTRxOver(num);
                }
                else//标准帧
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
* Description	: CAN硬件层初始化
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   王耀
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
* Description	: CAN初始化函数，
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   王耀
***********************************************************************************************/
void BSP_DCANInit(INT8U num,_BSPDCAN_CONFIG *pConfig)
{
    INT8U index;
    /*硬件初始化*/
    BSP_DCANHW_Init(num);
    /*CAN收发数据中断管理使能*/
    /* Register the ISR in the Interrupt Vector Table.*/
    BSP_IntVectReg(DCAN_IRQS_Table[num], DCAN_IRQ_PFUNC[num]);
    IntPrioritySet(DCAN_IRQS_Table[num], 0, AINTC_HOSTINT_ROUTE_IRQ );
    /* Enable the System Interrupts for AINTC.*/
    IntSystemEnable(DCAN_IRQS_Table[num]);
    
    /*CAN奇偶校验处理中断管理使能*/
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
    //保存配置
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

/************************(C)COPYRIGHT 2010 浙江方泰*****END OF FILE****************************/
