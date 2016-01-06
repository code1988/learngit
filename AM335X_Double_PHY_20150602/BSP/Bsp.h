/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp.h
* Author			: 王耀
* Date First Issued	        : 10/29/2013
* Version			: V
* Description		        : bsp的头文件,主要是统一包涵所有的驱动程序,本来想整个模块使能的宏定义的,但是
现在编译器比较聪明就不需要这样了,全包涵就行了
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_H_
#define	__BSP_H_
/* Includes-----------------------------------------------------------------------------------*/
//#include "bsp_conf.h"
#include "Include.h"
#include "bsp_debug.h"

#ifdef _BSP_LCD_ENABLE
#include "bsp_lcd.h"
#endif //_BSP_LCD_ENABLE

#ifdef _BSP_BEEP_ENABLE
#include "bsp_beep.h"
#endif //_BSP_BEEP_ENABLE

#ifdef _BSP_NUM_ENABLE
#include "bsp_led.h"
#endif

#ifdef _BSP_KEY_ENABLE
#include "bsp_KEY.h"
#endif //_BSP_KEY_ENABLE

#ifdef _BSP_ETHERNET_ENABLE
#include "bsp_ethernet.h"
#endif //_BSP_ETHERNET_ENABLE

#ifdef _BSP_MMCSD_ENABLE
#include "bsp_mmcsd.h"
//#include "ff.h"
#endif //_BSP_MMCSD_ENABLE

#ifdef _BSP_SPI_ENABLE
#include "bsp_spi.h"
#endif //_BSP_SPI_ENABLE

#ifdef _BSP_USB_ENALBE
#include "bsp_usb.h"
#endif //_BSP_USB_ENABLE 

#ifdef _BSP_UART_ENABLE
#include "bsp_uart.h"
#endif //_BSP_UART_ENABLE

#ifdef _BSP_DCAN_ENABLE
#include "bsp_can.h"
#endif //_BSP_DCAN_ENABLE

#ifdef _BSP_I2C0_ENABLE
#include "bsp_IIC.h"
#endif //_BSP_I2C0_ENABLE

#ifdef _BSP_NAND_ENABLE
#include "bsp_nand.h"
#endif //_BSP_NAND_ENABLE

#ifdef _BSP_AT45ENABLE
#include "bsp_flash.h"
#endif //_BSP_AT45ENABLE

#ifdef _BSP_EDMA_ENABLE
#include "bsp_edma.h"
#endif //_BSP_EDMA_ENABLE

#ifdef _BSP_W5500_ENABLE
#include "BSP_W5500.h"
#endif //_BSP_W5500_ENABLE

#ifdef _BSP_GPIO_ENABLE
#include "bsp_gpio.h"
#endif // _BSP_GPIO_ENABLE

#ifdef _BSP_COM2410_ENABLE
#include "ComWith2410.h"
#endif

/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BSP_Init 
* Description	        : Initialize all the peripherals that required OS services (OS initialized)
* Input			: none
* Output		: none
* Note(s)		: 1) This function SHOULD be called before any other BSP function is called.
* Contributor	        : 10/12/2010	wangyao
***********************************************************************************************/
void  BSP_Init (void);

void  OS_CPU_TickInit (void);

//CPU_INT32U   BSP_CPU_ClkGet              (void);
#endif	//__BSP_H_
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/

