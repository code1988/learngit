/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: am335x_irq.c
**硬          件: am335x
**创    建    人: wangyao
**创  建  日  期: 131029
**最  新  版  本: V0.1
**描          述: am335x的中断处理接口。所有的中断函数入口由该文件统一管理。这里的函数名定义还
不完全跟启动文件中的中断函数定义一致。使用到的时候，就统一函数名。
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人:
**日          期:
**版          本:
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "ucos_ii.h"
#include "soc_AM335x.h"
#include "bsp.h"
#include "dmtimer.h"
#include "lwiplib.h"

/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/


/*******************************************************************************
* Function Name  : OS_CPU_TickISR_Handler
* Description    : 定时器2的中断处理程序，这里被用来做，系统的时钟节拍，命名上暂时跟OS
*                  风格保持一致。
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
INT8U TimerCount = 0;
void  OS_CPU_TickISR_Handler (void)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    /* Disable the DMTimer interrupts */
    //DMTimerIntDisable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);
    /* Clear the status of the interrupt flags */
    DMTimerIntStatusClear(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_IT_FLAG);

    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    OSTimeTick();        /* Call uC/OS-II's OSTimeTick()系统节拍服务程序*/
    BSP_BEEPTimeTick();		//beep超时时间逐减，机制类似于ECB等待超时
    BSP_UARTOverTime();		//uart超时时间逐减
//    BSP_SPIOverTime();
    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
    /* Enable the DMTimer interrupts */
    //DMTimerIntEnable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);
}
/*******************************************************************************
* Function Name  : Timer6_IRQHandler
* Description    : 定时器6的中断处理程序
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  Timer6_IRQHandler (void)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    /* Disable the DMTimer interrupts */
    //DMTimerIntDisable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);
    /* Clear the status of the interrupt flags */
    DMTimerIntStatusClear(SOC_DMTIMER_6_REGS, DMTIMER_INT_OVF_IT_FLAG);

    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    BSP_KEYScan();//按键扫描

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
    /* Enable the DMTimer interrupts */
    //DMTimerIntEnable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);
}
/***********************************************************************************************
* Function		: CPSWCore0Rx_IRQHandler
* Description	: 网络接收中断处理函数
* Input			: none
* Output		: none
* Note(s)		:
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void CPSWCore0Rx_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    lwIPRxIntHandler(0);

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */


}

/***********************************************************************************************
* Function		: CPSWCore0Tx_IRQHandler
* Description	: 网络发送中断处理函数
* Input			: none
* Output		: none
* Note(s)		:
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void CPSWCore0Tx_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    lwIPTxIntHandler(0);

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */

}

/*******************************************************************************
* Function Name  : LCD_IRQHandler
* Description    : This function handles LCD global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3   /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

//    LCDIRQHandler();  //LCD raster中断处理

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}

/*******************************************************************************
* Function Name  : EDMA3COMPLETION_IRQHandler
* Description    : This function handles EDMA3COMPLETION global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EDMA3COMPLETION_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    Edma3CompletionIsr();  //Edma3Completion中断处理函数，SD卡用

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}

/*******************************************************************************
* Function Name  : EDMA3ERROR_IRQHandler
* Description    : This function handles EDMA3ERROR global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EDMA3ERROR_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3 /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    Edma3CCErrorIsr();  //Edma3CCError中断处理函数，SD卡用

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : HSMMCSD_IRQHandler
* Description    : This function handles HSMMCSD global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HSMMCSD_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    HSMMCSDIsr();  //HSMMCSD中断处理函数

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : MCSPI0_IRQHandler
* Description    : This function handles MCSPI0 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCSPI0_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    BSP_SPIIRQHandler(0);  //MCSPI0中断处理函数

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : MCSPI1_IRQHandler
* Description    : This function handles MCSPI1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCSPI1_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    BSP_SPIIRQHandler(1);  //MCSPI1中断处理函数

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : UART0_IRQHandler
* Description    : This function handles UART0 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART0_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    BSP_UARTIRQHandler(0);


    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : UART1_IRQHandler
* Description    : This function handles UART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART1_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    BSP_UARTIRQHandler(1);


    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : UART2_IRQHandler
* Description    : This function handles UART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART2_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

#if FZ2000
    BSP_UARTIRQHandler_two(2);
#else
    BSP_UARTIRQHandler(2);
#endif


    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : UART3_IRQHandler
* Description    : This function handles UART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART3_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    BSP_UARTIRQHandler(3);


    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : UART4_IRQHandler
* Description    : This function handles UART4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART4_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    BSP_UARTIRQHandler(4);


    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : UART5_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART5_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    BSP_UARTIRQHandler(5);


    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : DCAN0_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DCAN0_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    BSP_DCANIRQHandler(0);
    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : DCAN1_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DCAN1_IRQHandler(void)
{
    #if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    BSP_DCANIRQHandler(1);
    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : DCAN0_ParityIRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DCAN0_ParityIRQHandler(void)
{
    #if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    BSP_DCANParityIRQHandler(0);
    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : DCAN1_ParityIRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DCAN1_ParityIRQHandler(void)
{
    #if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    BSP_DCANParityIRQHandler(1);
    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}
/*******************************************************************************
* Function Name  : I2C0_IRQHandler
* Description    : This function handles I2C0 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C0_IRQHandler(void)
{

#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    BSP_I2CIRQHandler();

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}

/*******************************************************************************
* Function Name  : GPIOINT0A_IRQHandler
* Description    : This function handles GPIOINT0A global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIOINT0A_IRQHandler(void)
{
    #if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
    #endif

    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    BSP_GPIOINTnIRQHandler(3,19,0);

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}

/*******************************************************************************
* Function Name  : GPIOINT0B_IRQHandler
* Description    : This function handles GPIOINT0B global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIOINT0B_IRQHandler(void)
{
    #if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
    #endif

    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    BSP_GPIOINTnIRQHandler(3,19,0);

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}

/*******************************************************************************
* Function Name  : GPIOINT1A_IRQHandler
* Description    : This function handles GPIOINT1A global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIOINT1A_IRQHandler(void)
{
    #if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
    #endif

    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    BSP_GPIOINTnIRQHandler(1,20,0);

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}

/*******************************************************************************
* Function Name  : GPIOINT1B_IRQHandler
* Description    : This function handles GPIOINT1B global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIOINT1B_IRQHandler(void)
{
    #if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
    #endif

    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    BSP_GPIOINTnIRQHandler(1,22,1);

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}

/*******************************************************************************
* Function Name  : GPIOINT2A_IRQHandler
* Description    : This function handles GPIOINT2A global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIOINT2A_IRQHandler(void)
{
    #if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
    #endif

    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    BSP_GPIOINTnIRQHandler(2,26,0);

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}

/*******************************************************************************
* Function Name  : GPIOINT2B_IRQHandler
* Description    : This function handles GPIOINT2B global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIOINT2B_IRQHandler(void)
{
    #if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
    #endif

    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

//    BSP_GPIOINTnIRQHandler(2,19,0);

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}

/*******************************************************************************
* Function Name  : GPIOINT3A_IRQHandler
* Description    : This function handles GPIOINT3A global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIOINT3A_IRQHandler(void)
{
    #if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
    #endif

    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    BSP_GPIOINTnIRQHandler(3,14,0);

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}

/*******************************************************************************
* Function Name  : GPIOINT3B_IRQHandler
* Description    : This function handles GPIOINT3B global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIOINT3B_IRQHandler(void)
{
    #if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
    #endif

    OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

//    BSP_GPIOINTnIRQHandler(3,19,0);

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}



/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/


