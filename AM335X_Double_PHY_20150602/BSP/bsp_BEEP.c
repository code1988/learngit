/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_BEEP.c
**Ӳ          ��: am335x
**��    ��    ��: wangyao
**��  ��  ��  ��: 2013-09-25
**��  ��  ��  ��: V0.1
**��          ��: BEEP���Ƴ���
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: 
**��          ��: 
**��          ��: 
**��          ��:
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
// BEEP���Ŷ���
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
* Description	: BEEPд,�ı�BEEP״̬
* Input		    : state:BEEP״̬,_BSPBEEP_STATE
*		            BSPBEEP_STATE_CLOSE		��
*		            BSPBEEP_STATE_OPEN		��
*		            BSPBEEP_STATE_OVERTURN	��ת
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   ��ҫ
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
* Description	: ϵͳ���ã�ÿ��Tick����һ�Ρ�
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2010-12-11   ��ҫ
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
* Description	: BEEP��������BEEP�Ѵ�������״̬�������һ�ε�ʱ��Ϊ׼��
* Input			: BeepTime_Tick������ʱ�䣬Tick��λ��BeepTime_Tick��Beep�Զ��رա�
* Output		: 
* Note(s)		: 
* Contributor	: 2010-12-11   ��ҫ
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
* Description	: ������ʼ��
* Input			: 
* Output		: 
* Note(s)		: ��ʱ����Ҫ���ܽų�ʼ��ȫ����BSP.C��ʵ���ˡ�
* Contributor	: 2013-09-25  ��ҫ
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
/************************(C)COPYRIGHT 2010 �㽭��̩*****END OF FILE****************************/
