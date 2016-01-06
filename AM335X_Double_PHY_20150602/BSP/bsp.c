/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp.c
**硬          件: am335x
**创    建    人: wangyao
**创  建  日  期: 131029
**最  新  版  本: V0.1
**描          述: 驱动主函数,包括RCC,GPIO,外部总线,系统时钟,中断等基本配置和所有需要初始化的外设
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "os_cpu.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
#include "dmtimer.h"
#include "lib_def.h"
#include "interrupt.h"
#include "bsp_int.h"
#include "am335x_irq.h"
#include "bsp.h"
#include "delay.h"

/* Private define-----------------------------------------------------------------------------*/
#define BSP_Timer7Init()  {DelayTimerSetup();}//定时器7,只给USB用，其他任务不建议调用
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BSP_Init 
* Description	: Initialize all the peripherals that required OS services (OS initialized)
* Input			: none
* Output		: none
* Note(s)		: 1) This function SHOULD be called before any other BSP function is called.
* Contributor	: 10/12/2013	wangyao
***********************************************************************************************/
void  BSP_Init (void)
{
    BSP_IntInit();//中断向量表初始化
    BSP_Timer7Init();

    BSP_GPIOInit(0);    // GPIO0模块初始化
    BSP_GPIOInit(1);    // GPIO1模块初始化
    BSP_GPIOInit(3);    // GPIO3模块初始化
    BSP_SPIInit(0);     // SPI0模块初始化
    BSP_SPIInit(2);     // SPI1模块初始化，注:这里2指向SPI1模块
    BSP_EtherInit();    // 网络模块初始化
        DBG_NET("in lwIPInit 1_0:call tcpip_init() start \r\n");
    tcpip_init(NULL, NULL);
    DBG_NET("in lwIPInit 1_1:call tcpip_init()   end \r\n");
    
#ifdef _BSP_MMCSD_ENABLE
//    BSP_MMCSDInit();               //SD卡硬件初始化,这里暂时包含EDMA3初始化
#endif //_BSP_MMCSD_ENABLE
    
#ifdef _BSP_DCANENABLE
    //BSP_DCANInit();
#endif //_BSP_DCANENABLE
 
#ifdef _BSP_AT45ENABLE
    BSP_FlashInitAT();
#endif //_BSP_AT45ENABLE
}
/***********************************************************************************************
* Function		: BSP_OSPost
* Description	: 
* Input			: pEvent 事件控制块;	 pMsg 消息指针
* Return		: 
* Note(s)		: 该函数对ucos内核中所有的OSxPost函数进行封装，统一管理
* Contributor	: 131211	wangyao
***********************************************************************************************/
// 发送事件消息
#if	BSP_OS
void BSP_OSPost(_OS_EVENT *pEvent,_BSP_MESSAGE *pMsg)
{
#if	BSP_UCOSII
	if(pEvent == NULL)
		return;
#if OS_Q_EN
	if(pEvent->OSEventType == OS_EVENT_TYPE_Q)
		OSQPost(pEvent,pMsg);
#endif	//OS_Q_EN
#if OS_MBOX_EN
	else if(pEvent->OSEventType == OS_EVENT_TYPE_MBOX)
		OSMboxPost(pEvent,pMsg);
#endif	//OS_MBOX_EN
#if OS_SEM_EN
	else if(pEvent->OSEventType == OS_EVENT_TYPE_SEM)
		OSSemPost(pEvent);
#endif	//OS_SEM_EN

#endif	//BSP_UCOSII
}
#endif	//BSP_OS
/*
*********************************************************************************************************
*********************************************************************************************************
**                                                 OS FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            OS_CPU_TickInit()
*
* Description : Initialize OS tick source for the V850E2/Jx4-L, Interval Timer M.
*
* Argument(s) : ticks_per_sec              Number of ticks per second.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/
/*
** Setup the timer for one-shot and compare mode.
*/
#define TIMER_INITIAL_COUNT             (0xFFFFFFFF - 24000)//(0xFF000000u)//
#define TIMER_RLD_COUNT                 (0xFFFFFFFF - 24000)//(0xFF000000u)//
/*
** Do the necessary DMTimer configurations on to AINTC.
*/
static void DMTimerAintcConfigure(void)
{
    /* Registering DMTimerIsr */
    BSP_IntVectReg(SYS_INT_TINT2, OS_CPU_TickISR_Handler);//定时器2做为uC/OS系统节拍服务函数. 
    
    /* Set the priority */
    IntPrioritySet(SYS_INT_TINT2, 20, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enable the system interrupt */
    IntSystemEnable(SYS_INT_TINT2);
}

/*
** Setup the timer for one-shot and compare mode.
*/
static void DMTimerSetUp(void)
{
    /* Load the counter with the initial count value */
    DMTimerCounterSet(SOC_DMTIMER_2_REGS, TIMER_INITIAL_COUNT);//计数器寄存器赋值

    /* Load the load register with the reload count value */
    DMTimerReloadSet(SOC_DMTIMER_2_REGS, TIMER_RLD_COUNT);

    /* Configure the DMTimer for Auto-reload and compare mode */
    DMTimerModeConfigure(SOC_DMTIMER_2_REGS, DMTIMER_AUTORLD_NOCMP_ENABLE);
}

/***********************************************************************************************
* Function		: OS_CPU_TickInit 
* Description	: uCOS系统时钟初始化
* Input			: none
* Output		: none
* Note(s)		: 采用定时器2作为uCOS系统时钟
* Contributor	: 08/07/2014	wangyao
***********************************************************************************************/
void  OS_CPU_TickInit (void)
{  
    /* This function will enable clocks for the DMTimer2 instance */
    DMTimer2ModuleClkConfig();
    /* Enable IRQ in CPSR */
    IntMasterIRQEnable();

    /* Register DMTimer2 interrupts on to AINTC */
    DMTimerAintcConfigure();

    /* Perform the necessary configurations for DMTimer */
    DMTimerSetUp();

    /* Enable the DMTimer interrupts */
    DMTimerIntEnable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);

    /* Start the DMTimer */
    DMTimerEnable(SOC_DMTIMER_2_REGS);      
}


/*
*********************************************************************************************************
*********************************************************************************************************
**                                       CPU CLOCK FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            BSP_CPU_ClkGet()
*
* Description : Gets the CPU clock frequency(Fclk).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/
CPU_INT32U  BSP_CPU_ClkGet (void)
{
    return (0u);
}
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/