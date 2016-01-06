/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_ethernet.c
**硬          件: am335x
**创    建    人: wangyao
**创  建  日  期: 131029
**最  新  版  本: V0.1
**描          述: ethernet的硬件初始化函数，
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "os_cpu.h"
#include "dmtimer.h"
#include "lib_def.h"
#include "interrupt.h"
#include "soc_AM335x.h"
#include "interrupt.h"
#include "evmAM335x.h"
#include "am335x_irq.h"
#include "bsp_int.h"
//#include "lwiplib.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
void  (*ETH_PostSem)(void) = NULL;
/***********************************************************************************************
* Function		: BSP_CPSWIntrSetUp 
* Description	        : 网络控制器中断初始化
* Input			: none
* Output		: none
* Note(s)		: 1) This function SHOULD be called before any other BSP function is called.
* Contributor	        : 10/12/2010	wangyao
***********************************************************************************************/
static void BSP_CPSWIntrSetUp(void)
{
    /* Enable IRQ for ARM (in CPSR)*/
    //IntMasterIRQEnable();中断使能初始化由于定时器2系统已启动就需要使能，所以放在系统Stick里
    //IntAINTCInit();
    /*使用与OS结合的中断初始化，优先级设置函数*/
    /* Register the Receive ISR for Core 0 */
    BSP_IntVectReg(SYS_INT_3PGSWRXINT0, CPSWCore0Rx_IRQHandler);
    /* Register the Transmit ISR for Core 0 */
    BSP_IntVectReg(SYS_INT_3PGSWTXINT0, CPSWCore0Tx_IRQHandler);
    
    /* Set the priority */
    IntPrioritySet(SYS_INT_3PGSWTXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);
    IntPrioritySet(SYS_INT_3PGSWRXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enable the system interrupt */
    IntSystemEnable(SYS_INT_3PGSWTXINT0);
    IntSystemEnable(SYS_INT_3PGSWRXINT0);
    
}
/***********************************************************************************************
* Function		: BSP_EtherInit 
* Description	        : 网络口的初始化
* Input			: none
* Output		: none
* Note(s)		: 1) 该函数本来要集成在LWIP的初始化中，考虑到整体风格先独立由BSP调用
* Contributor	        : 10/12/2010	wangyao
***********************************************************************************************/
void  BSP_EtherInit(void)
{
    CPSWPinMuxSetup();          // 管脚片选
    CPSWClkEnable();            // 时钟初始化  
    BSP_CPSWIntrSetUp();        // 中断管理初始化
    EVMPortRGMIIModeSelect();   // PHY接口模式选择
}

/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/