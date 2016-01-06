/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: am335x_irq.h
**硬          件: s3c2410
**创    建    人: wangyao
**创  建  日  期: 130722
**最  新  版  本: V0.1
**描          述: am335x的中断处理接口。所有的中断函数入口由该文件统一管理。这里的函数名定义还
不完全跟启动文件中的中断函数定义一致。使用到的时候，就统一函数命
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__AM335X_IRQ_H_
#define	__AM335X_IRQ_H_
/* Includes-----------------------------------------------------------------------------------*/
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
void  OS_CPU_TickISR_Handler (void);
/*******************************************************************************
* Function Name  : Timer6_IRQHandler
* Description    : 定时器6的中断处理程序
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  Timer6_IRQHandler (void);
/***********************************************************************************************
* Function		: CPSWCore0Rx_IRQHandler 
* Description	        : 网络接收中断处理函数
* Input			: none
* Output		: none
* Note(s)		: 
* Contributor	        : 10/12/2010	wangyao
***********************************************************************************************/
void CPSWCore0Rx_IRQHandler(void);

/***********************************************************************************************
* Function		: CPSWCore0Tx_IRQHandler 
* Description	        : 网络发送中断处理函数
* Input			: none
* Output		: none
* Note(s)		: 
* Contributor	        : 10/12/2010	wangyao
***********************************************************************************************/
void CPSWCore0Tx_IRQHandler(void);
/*******************************************************************************
* Function Name  : LCD_IRQHandler
* Description    : This function handles LCD global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_IRQHandler(void);
/*******************************************************************************
* Function Name  : EDMA3COMPLETION_IRQHandler
* Description    : This function handles EDMA3COMPLETION global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EDMA3COMPLETION_IRQHandler(void);
/*******************************************************************************
* Function Name  : EDMA3ERROR_IRQHandler
* Description    : This function handles EDMA3ERROR global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EDMA3ERROR_IRQHandler(void);
/*******************************************************************************
* Function Name  : HSMMCSD_IRQHandler
* Description    : This function handles HSMMCSD global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HSMMCSD_IRQHandler(void);
/*******************************************************************************
* Function Name  : MCSPI0_IRQHandler
* Description    : This function handles HSMMCSD global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCSPI0_IRQHandler(void);
/*******************************************************************************
* Function Name  : MCSPI1_IRQHandler
* Description    : This function handles HSMMCSD global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCSPI1_IRQHandler(void);
/*******************************************************************************
* Function Name  : UART0_IRQHandler
* Description    : This function handles UART0 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART0_IRQHandler(void);
/*******************************************************************************
* Function Name  : UART1_IRQHandler
* Description    : This function handles UART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART1_IRQHandler(void);
/*******************************************************************************
* Function Name  : UART2_IRQHandler
* Description    : This function handles UART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART2_IRQHandler(void);
/*******************************************************************************
* Function Name  : UART3_IRQHandler
* Description    : This function handles UART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART3_IRQHandler(void);
/*******************************************************************************
* Function Name  : UART4_IRQHandler
* Description    : This function handles UART4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART4_IRQHandler(void);
/*******************************************************************************
* Function Name  : UART5_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART5_IRQHandler(void);
/*******************************************************************************
* Function Name  : DCAN0_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DCAN0_IRQHandler(void);
/*******************************************************************************
* Function Name  : DCAN1_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DCAN1_IRQHandler(void);
/*******************************************************************************
* Function Name  : DCAN0_ParityIRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DCAN0_ParityIRQHandler(void);
/*******************************************************************************
* Function Name  : DCAN1_ParityIRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DCAN1_ParityIRQHandler(void);
/*******************************************************************************
* Function Name  : I2C0_IRQHandler
* Description    : This function handles I2C0 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C0_IRQHandler(void);
/*******************************************************************************
* Function Name  : Timer6_IRQHandler
* Description    : This function handles Timer6 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Timer6_IRQHandler(void);
void GPIOINT0A_IRQHandler(void);
void GPIOINT0B_IRQHandler(void);
void GPIOINT1A_IRQHandler(void);
void GPIOINT1B_IRQHandler(void);
void GPIOINT2A_IRQHandler(void);
void GPIOINT2B_IRQHandler(void);
void GPIOINT3A_IRQHandler(void);
void GPIOINT3B_IRQHandler(void);


#endif	//__AM335X_IRQ_H_
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
