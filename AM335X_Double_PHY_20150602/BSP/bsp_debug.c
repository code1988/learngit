/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_debug.c
* Author			: wangyao
* Date First Issued	: 130722
* Version			: V
* Description		: debug管理，这里目前只做printf功能，后续将补充软件错误监控部分，
*                     默认的串口0为debug串口，供需要串口调试的开发人员使用
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013		    : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "include.h"
#include "bsp.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
#include "gpio.h"
#include "gpio_v2.h"
#include "bsp_int.h"
#include "am335x_irq.h"
#include "uart_irda_cir.h"
/* Private define-----------------------------------------------------------------------------*/
#define UART_CONSOLE_BASE                    (SOC_UART_0_REGS)
#define BAUD_RATE_115200                     (115200)
#define UART_MODULE_INPUT_CLK                (48000000)
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
char BSP_DEBUGBuff[256] = {0};
/* Private function prototypes----------------------------------------------------------------*/
static void UartFIFOConfigure(INT32U txTrigLevel,INT32U rxTrigLevel);
static void UartBaudRateSet(INT32U baudRate);
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		:
* Description	: 以下三个为初始化配置函数
* Input			:
* Return		:
* Note(s)		:
* Contributor	:130722   wangyao
***********************************************************************************************/
static void UARTStdioInitExpClk(INT32U baudRate,INT32U rxTrigLevel,INT32U txTrigLevel)
{
    /* Performing a module reset. */
    UARTModuleReset(UART_CONSOLE_BASE);

    /* Performing FIFO configurations. */
    UartFIFOConfigure(txTrigLevel, rxTrigLevel);

    /* Performing Baud Rate settings. */
    UartBaudRateSet(baudRate);

    /* Switching to Configuration Mode B. */
    UARTRegConfigModeEnable(UART_CONSOLE_BASE, UART_REG_CONFIG_MODE_B);

    /* Programming the Line Characteristics. */
    UARTLineCharacConfig(UART_CONSOLE_BASE,
                         (UART_FRAME_WORD_LENGTH_8 | UART_FRAME_NUM_STB_1),
                         UART_PARITY_NONE);

    /* Disabling write access to Divisor Latches. */
    UARTDivisorLatchDisable(UART_CONSOLE_BASE);

    /* Disabling Break Control. */
    UARTBreakCtl(UART_CONSOLE_BASE, UART_BREAK_COND_DISABLE);

    /* Switching to UART16x operating mode. */
    UARTOperatingModeSelect(UART_CONSOLE_BASE, UART16x_OPER_MODE);
}

/*
** A wrapper function performing FIFO configurations.
*/

static void UartFIFOConfigure(INT32U txTrigLevel,INT32U rxTrigLevel)
{
    INT32U fifoConfig = 0;

    /* Setting the TX and RX FIFO Trigger levels as 1. No DMA enabled. */
    fifoConfig = UART_FIFO_CONFIG(UART_TRIG_LVL_GRANULARITY_1,
                                  UART_TRIG_LVL_GRANULARITY_1,
                                  txTrigLevel,
                                  rxTrigLevel,
                                  1,
                                  1,
                                  UART_DMA_EN_PATH_SCR,
                                  UART_DMA_MODE_0_ENABLE);

    /* Configuring the FIFO settings. */
    UARTFIFOConfig(UART_CONSOLE_BASE, fifoConfig);
}

/*
** A wrapper function performing Baud Rate settings.
*/

static void UartBaudRateSet(INT32U baudRate)
{
    INT32U divisorValue = 0;

    /* Computing the Divisor Value. */
    divisorValue = UARTDivisorValCompute(UART_MODULE_INPUT_CLK,
                                         baudRate,
                                         UART16x_OPER_MODE,
                                         UART_MIR_OVERSAMPLING_RATE_42);

    /* Programming the Divisor Latches. */
    UARTDivisorLatchWrite(UART_CONSOLE_BASE, divisorValue);
}
/***********************************************************************************************
* Function		: BSP_DebugUARTInit
* Description	: 使用printf初始化
* Input			:
* Return		:
* Note(s)		:
* Contributor	:130722   wangyao
***********************************************************************************************/
void BSP_DebugUARTInit(void)
{
    /* Configuring the system clocks for UART0 instance. */
    UARTModuleClkConfig(0);
    /* Performing the Pin Multiplexing for UART0 instance. */
    UARTPinMuxSetup(0);
    UARTStdioInitExpClk(BAUD_RATE_115200, 1, 1);
}




/****************************************************************************************************
**名称:void BSP_DebugUARTInit_two(int baud_rate)
**功能:串口初始化
* 入口:无
* 出口:无
**auth:hxj, date: 2014-12-16 14:37
*****************************************************************************************************/
void BSP_DebugUARTInit_two(int baud_rate)
{
    /* Configuring the system clocks for UART0 instance. */
    UARTModuleClkConfig(0);
    /* Performing the Pin Multiplexing for UART0 instance. */
    UARTPinMuxSetup(0);
    UARTStdioInitExpClk(baud_rate, 1, 1);
}

/***********************************************************************************************
* Function		: BSPPrintf
* Description	: printf函数
* Input			:
* Return		:
* Note(s)		:
* Contributor	:130722   wangyao
***********************************************************************************************/
void BSPPrintf(char *pStr, ...)
{
    char *pbuff;

    pbuff = BSP_DEBUGBuff;
	va_list arg_ptr;
	va_start(arg_ptr,pStr);
	vsprintf(pbuff,pStr,arg_ptr);
    while(*pbuff)
    {
        UARTCharPut(UART_CONSOLE_BASE,*pbuff);
        pbuff++;
    }
	va_end(arg_ptr);
}



/****************************************************************************************************
**名称:int uart0_send(char *buf,int len)
**功能:串口发送数据
* 入口:无
* 出口:无
**auth:hxj, date: 2014-12-16 14:46
*****************************************************************************************************/
int uart0_send(char *buf,int len)
{
    if(NULL == buf) return-1;
    if(len<=0) return 0;
    int i;
    for(i=0;i<len;++i)
    {
        UARTCharPut(UART_CONSOLE_BASE,buf[i]);
    }
    return len;
}











/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
