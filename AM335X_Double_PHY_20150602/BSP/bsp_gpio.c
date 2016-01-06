/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_gpio.c
**硬          件: am335x
**创    建    人: code
**创  建  日  期: 141115
**最  新  版  本: V0.1
**描          述: 

**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述: 20150305 - 屏蔽BSP_GPIOConfig函数中GPIOModuleReset函数，解决配置多个外部中断时配置失效问题 by code
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "bsp.h"
#include "gpio.h"
#include "Gpio_v2.h"
/* Private define-----------------------------------------------------------------------------*/
// GPIO各端口的基地址
const INT32U GPIO_REGS_Table[4] = {
    SOC_GPIO_0_REGS,
    SOC_GPIO_1_REGS,
    SOC_GPIO_2_REGS,
    SOC_GPIO_3_REGS,
};

// GPIO做外部中断时的中断号
const INT32U GPIO_IRQS_Table[4][2] = {
    SYS_INT_GPIOINT0A,SYS_INT_GPIOINT0B,
    SYS_INT_GPIOINT1A,SYS_INT_GPIOINT1B,
    SYS_INT_GPIOINT2A,SYS_INT_GPIOINT2B,
    SYS_INT_GPIOINT3A,SYS_INT_GPIOINT3B,    
};

// GPIO做外部中断时的中断服务函数
void (*GPIO_IRQSHander_Table[4][2])(void) = {
    GPIOINT0A_IRQHandler,GPIOINT0B_IRQHandler,
    GPIOINT1A_IRQHandler,GPIOINT1B_IRQHandler,
    GPIOINT2A_IRQHandler,GPIOINT2B_IRQHandler,
    GPIOINT3A_IRQHandler,GPIOINT3B_IRQHandler,
};

// GPIO配置参数表
static _BSPGPIO_CONFIG GPIO_Config[4][32];
/***********************************************************************************************
* Function Name	: BSP_GPIOINTnIRQHandler
* Description	: 外部中断服务函数
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 141220	code
***********************************************************************************************/
_BSP_MESSAGE SDINIT_message;
void BSP_GPIOINTnIRQHandler(unsigned int portnum,unsigned int pinnum,unsigned int intLine)
{
    INT32U IntSt;
    INT8U ret;
    static _BSP_MESSAGE Gpio_Msg[2];
#ifdef SDCARD_HOTPLUG
    INT8U SDSTAT = 0;
#endif //SDCARD_HOTPLUG
    // 读取指定中断线上的指定管脚状态
    IntSt = GPIOPinIntStatus(GPIO_REGS_Table[portnum],intLine,pinnum);

    if(IntSt)
    {
        if((portnum == 1) && (pinnum == 20))    // W5500A中断线
        {
            ret = BSP_W5500GetIR(W5500A);       // 获取中断原因
            Gpio_Msg[W5500A].MsgID = ret;
            Gpio_Msg[W5500A].DivNum = W5500A;
            SYSPost(GPIO_Config[portnum][pinnum].pEvent,&Gpio_Msg[W5500A]);
        }

        if((portnum == 1) && (pinnum == 22))    // W5500B中断线
        {
            ret = BSP_W5500GetIR(W5500B);       // 获取中断原因
            Gpio_Msg[W5500B].MsgID = ret;
            Gpio_Msg[W5500B].DivNum = W5500B;
            SYSPost(GPIO_Config[portnum][pinnum].pEvent,&Gpio_Msg[W5500B]);
        }
        
#ifdef SDCARD_ENABLE
        if((portnum == 3) && (pinnum == 14))    // SD卡中断线
        {
            SDSTAT = GPIOPinRead(SOC_GPIO_3_REGS,14);
            SDINIT_message.MsgID = BSP_SDINIT_COMFROM_GPIO; //读成功，把文件大小信息传递过去
            SDINIT_message.DataLen = 1;
            SDINIT_message.pData = &SDSTAT;
            SYSPost(MMCSDEvent,&SDINIT_message);
            OSTimeDlyHMSM(0,0,0,5);
        }
#endif //SDCARD_HOTPLUG
        // 清中断
        GPIOPinIntClear(GPIO_REGS_Table[portnum],intLine,pinnum);
    }
    
}

/***********************************************************************************************
* Function Name	: BSP_GPIOConfig
* Description	: 初始化GPIO
* Input			: 
* Return		: 
* Note(s)		: 外部中断功能也在这里初始化了
* Contributor	: 141220	code
***********************************************************************************************/
INT8U BSP_GPIOConfig(_BSPGPIO_CONFIG *pConfig)
{
    /* Selecting GPIOn[i] pin for use. */
    GpioPinMuxSetup(pConfig->PortNum,pConfig->PinNum,PAD_FS_RXE_PU_PUPDE(7)); 

    /* Setting the GPIO pin as an input pin. */
    GPIODirModeSet(GPIO_REGS_Table[pConfig->PortNum],
                   pConfig->PinNum,
                   pConfig->Dir);

    // 如果是做输入，默认就是作外部中断用
    if((GPIO_DIR_INPUT == pConfig->Dir)&&(GPIO_INT_TYPE_NO != pConfig->IntType))
    {
        GPIOPinIntEnable(GPIO_REGS_Table[pConfig->PortNum],pConfig->IntLine,pConfig->PinNum);
        GPIOIntTypeSet(GPIO_REGS_Table[pConfig->PortNum],pConfig->PinNum,pConfig->IntType);

        // 向量表中注册 GPIOINT3A_IRQHandler 
        IntRegister(GPIO_IRQS_Table[pConfig->PortNum][pConfig->IntLine],GPIO_IRQSHander_Table[pConfig->PortNum][pConfig->IntLine]);

        // 设置中断优先级
        IntPrioritySet(GPIO_IRQS_Table[pConfig->PortNum][pConfig->IntLine], 0u, AINTC_HOSTINT_ROUTE_IRQ);

        // 在AINTC中使能GPIOINTn中断
        IntSystemEnable(GPIO_IRQS_Table[pConfig->PortNum][pConfig->IntLine]);
    }


    // 更新GPIO配置参数表
    memcpy(&GPIO_Config[pConfig->PortNum][pConfig->PinNum],pConfig,sizeof(_BSPGPIO_CONFIG));
	
    return TRUE;
}

/***********************************************************************************************
* Function Name	: BSP_GPIOConfig
* Description	: 初始化GPIO
* Input			: 
* Return		: 
* Note(s)		: 外部中断功能也在这里初始化了
* Contributor	: 141220	code
***********************************************************************************************/
INT8U BSP_GPIOInit(INT8U num)
{
    /* Enabling functional clocks for GPIOn instance. */
    GPIOModuleClkConfig(num);
    
    /* Enabling the GPIO module. */
    GPIOModuleEnable(GPIO_REGS_Table[num]);

    /* Resetting the GPIO module. */
    GPIOModuleReset(GPIO_REGS_Table[num]);

    return TRUE;
}



/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/

