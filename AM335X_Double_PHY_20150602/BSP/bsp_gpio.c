/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_gpio.c
**Ӳ          ��: am335x
**��    ��    ��: code
**��  ��  ��  ��: 141115
**��  ��  ��  ��: V0.1
**��          ��: 

**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: 
**��          ��: 
**��          ��: 
**��          ��: 20150305 - ����BSP_GPIOConfig������GPIOModuleReset������������ö���ⲿ�ж�ʱ����ʧЧ���� by code
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "bsp.h"
#include "gpio.h"
#include "Gpio_v2.h"
/* Private define-----------------------------------------------------------------------------*/
// GPIO���˿ڵĻ���ַ
const INT32U GPIO_REGS_Table[4] = {
    SOC_GPIO_0_REGS,
    SOC_GPIO_1_REGS,
    SOC_GPIO_2_REGS,
    SOC_GPIO_3_REGS,
};

// GPIO���ⲿ�ж�ʱ���жϺ�
const INT32U GPIO_IRQS_Table[4][2] = {
    SYS_INT_GPIOINT0A,SYS_INT_GPIOINT0B,
    SYS_INT_GPIOINT1A,SYS_INT_GPIOINT1B,
    SYS_INT_GPIOINT2A,SYS_INT_GPIOINT2B,
    SYS_INT_GPIOINT3A,SYS_INT_GPIOINT3B,    
};

// GPIO���ⲿ�ж�ʱ���жϷ�����
void (*GPIO_IRQSHander_Table[4][2])(void) = {
    GPIOINT0A_IRQHandler,GPIOINT0B_IRQHandler,
    GPIOINT1A_IRQHandler,GPIOINT1B_IRQHandler,
    GPIOINT2A_IRQHandler,GPIOINT2B_IRQHandler,
    GPIOINT3A_IRQHandler,GPIOINT3B_IRQHandler,
};

// GPIO���ò�����
static _BSPGPIO_CONFIG GPIO_Config[4][32];
/***********************************************************************************************
* Function Name	: BSP_GPIOINTnIRQHandler
* Description	: �ⲿ�жϷ�����
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
    // ��ȡָ���ж����ϵ�ָ���ܽ�״̬
    IntSt = GPIOPinIntStatus(GPIO_REGS_Table[portnum],intLine,pinnum);

    if(IntSt)
    {
        if((portnum == 1) && (pinnum == 20))    // W5500A�ж���
        {
            ret = BSP_W5500GetIR(W5500A);       // ��ȡ�ж�ԭ��
            Gpio_Msg[W5500A].MsgID = ret;
            Gpio_Msg[W5500A].DivNum = W5500A;
            SYSPost(GPIO_Config[portnum][pinnum].pEvent,&Gpio_Msg[W5500A]);
        }

        if((portnum == 1) && (pinnum == 22))    // W5500B�ж���
        {
            ret = BSP_W5500GetIR(W5500B);       // ��ȡ�ж�ԭ��
            Gpio_Msg[W5500B].MsgID = ret;
            Gpio_Msg[W5500B].DivNum = W5500B;
            SYSPost(GPIO_Config[portnum][pinnum].pEvent,&Gpio_Msg[W5500B]);
        }
        
#ifdef SDCARD_ENABLE
        if((portnum == 3) && (pinnum == 14))    // SD���ж���
        {
            SDSTAT = GPIOPinRead(SOC_GPIO_3_REGS,14);
            SDINIT_message.MsgID = BSP_SDINIT_COMFROM_GPIO; //���ɹ������ļ���С��Ϣ���ݹ�ȥ
            SDINIT_message.DataLen = 1;
            SDINIT_message.pData = &SDSTAT;
            SYSPost(MMCSDEvent,&SDINIT_message);
            OSTimeDlyHMSM(0,0,0,5);
        }
#endif //SDCARD_HOTPLUG
        // ���ж�
        GPIOPinIntClear(GPIO_REGS_Table[portnum],intLine,pinnum);
    }
    
}

/***********************************************************************************************
* Function Name	: BSP_GPIOConfig
* Description	: ��ʼ��GPIO
* Input			: 
* Return		: 
* Note(s)		: �ⲿ�жϹ���Ҳ�������ʼ����
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

    // ����������룬Ĭ�Ͼ������ⲿ�ж���
    if((GPIO_DIR_INPUT == pConfig->Dir)&&(GPIO_INT_TYPE_NO != pConfig->IntType))
    {
        GPIOPinIntEnable(GPIO_REGS_Table[pConfig->PortNum],pConfig->IntLine,pConfig->PinNum);
        GPIOIntTypeSet(GPIO_REGS_Table[pConfig->PortNum],pConfig->PinNum,pConfig->IntType);

        // ��������ע�� GPIOINT3A_IRQHandler 
        IntRegister(GPIO_IRQS_Table[pConfig->PortNum][pConfig->IntLine],GPIO_IRQSHander_Table[pConfig->PortNum][pConfig->IntLine]);

        // �����ж����ȼ�
        IntPrioritySet(GPIO_IRQS_Table[pConfig->PortNum][pConfig->IntLine], 0u, AINTC_HOSTINT_ROUTE_IRQ);

        // ��AINTC��ʹ��GPIOINTn�ж�
        IntSystemEnable(GPIO_IRQS_Table[pConfig->PortNum][pConfig->IntLine]);
    }


    // ����GPIO���ò�����
    memcpy(&GPIO_Config[pConfig->PortNum][pConfig->PinNum],pConfig,sizeof(_BSPGPIO_CONFIG));
	
    return TRUE;
}

/***********************************************************************************************
* Function Name	: BSP_GPIOConfig
* Description	: ��ʼ��GPIO
* Input			: 
* Return		: 
* Note(s)		: �ⲿ�жϹ���Ҳ�������ʼ����
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



/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/

