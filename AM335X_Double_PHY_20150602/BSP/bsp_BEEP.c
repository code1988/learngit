/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_BEEP.c
**硬          件: am335x
**创    建    人: wangyao
**创  建  日  期: 2013-09-25
**最  新  版  本: V0.1
**描          述: BEEP控制程序
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/

/* Includes-----------------------------------------------------------------------------------*/
#include "include.h"
#include "bsp_beep.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
#include "gpio.h"
#include "gpio_v2.h"
/* Private define-----------------------------------------------------------------------------*/
// BEEP引脚定义
#define BSP_GPIO_INSTANCE_ADDRESS        (SOC_GPIO_1_REGS)
#define BSP_GPIO_INSTANCE_PIN_NUMBER     (23)
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static INT16U BeepKeepTicks;
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function	    : BSP_BEEPWrite
* Description	: BEEP写,改变BEEP状态
* Input		    : state:BEEP状态,_BSPBEEP_STATE
*		            BSPBEEP_STATE_CLOSE		关
*		            BSPBEEP_STATE_OPEN		开
*		            BSPBEEP_STATE_OVERTURN	翻转
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   王耀
***********************************************************************************************/
void BSP_BEEPWrite(_BSPBEEP_STATE state)
{
	static INT8U saveState;
	
	if(state == BSPBEEP_STATE_OVERTURN)
		saveState = !saveState;
	else
		saveState = state;
	if(saveState)	
        GPIOPinWrite(BSP_GPIO_INSTANCE_ADDRESS,BSP_GPIO_INSTANCE_PIN_NUMBER,GPIO_PIN_HIGH);	
	else
        GPIOPinWrite(BSP_GPIO_INSTANCE_ADDRESS,BSP_GPIO_INSTANCE_PIN_NUMBER,GPIO_PIN_LOW);	
}

/***********************************************************************************************
* Function		: BSP_BEEPTimeTick
* Description	: 系统调用，每个Tick调用一次。
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2010-12-11   王耀
***********************************************************************************************/
void BSP_BEEPTimeTick(void)
{
	if(BeepKeepTicks)
	{
		BeepKeepTicks--;	
		if(BeepKeepTicks == 0)
		{
			BSP_BEEPWrite(BSPBEEP_STATE_CLOSE);
		}
	}	
}

/***********************************************************************************************
* Function		: BSP_BEEPStart
* Description	: BEEP响声，若BEEP已处于响声状态，以最后一次的时间为准。
* Input			: BeepTime_Tick：响声时间，Tick单位。BeepTime_Tick后Beep自动关闭。
* Output		: 
* Note(s)		: 
* Contributor	: 2010-12-11   王耀
***********************************************************************************************/
void BSP_BEEPStart(INT16U BeepTime_Tick)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
	OS_ENTER_CRITICAL();
	if(BeepTime_Tick)
	{
		BSP_BEEPWrite(BSPBEEP_STATE_OPEN);
		BeepKeepTicks = BeepTime_Tick;
	}
	OS_EXIT_CRITICAL();    
}

/***********************************************************************************************
* Function		: BSP_BEEPInit
* Description	: 驱动初始化
* Input			: 
* Output		: 
* Note(s)		: 暂时不需要，管脚初始化全部由BSP.C中实现了。
* Contributor	: 2013-09-25  王耀
***********************************************************************************************/
void BSP_BEEPInit(void)
{
    /* Configuring the system clocks for GPIO1 instance. */
    GPIO1ModuleClkConfig();
    /* Enabling IRQ in CPSR of ARM processor. */
    //IntMasterIRQEnable();
    /* Initializing the ARM Interrupt Controller. */
    //IntAINTCInit();
    /* Configure a DMTimer instance. */
    //DelayTimerSetup();

    /* Selecting the pin GPIO1[23] to control the Audio Buzzer. */
    GPIO1Pin23PinMuxSetup();
    /* Enabling the GPIO module. */
    GPIOModuleEnable(BSP_GPIO_INSTANCE_ADDRESS);
    /* Perform a module reset of the GPIO module. */
    GPIOModuleReset(BSP_GPIO_INSTANCE_ADDRESS);
    /* Set the specified pin as an output pin. */
    GPIODirModeSet(BSP_GPIO_INSTANCE_ADDRESS,
                   BSP_GPIO_INSTANCE_PIN_NUMBER,
                   GPIO_DIR_OUTPUT);
}
/************************(C)COPYRIGHT 2010 浙江方泰*****END OF FILE****************************/
