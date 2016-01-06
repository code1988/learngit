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
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __BSP_GPIO_H_
#define __BSP_GPIO_H_

/* Private macro------------------------------------------------------------------------------*/

// GPIO配置结构体
typedef struct{
    INT8U PortNum;              // 端口号
    INT32U PinNum;              // 管脚号
    INT32U Dir;                 // I/O方向
    INT32U IntLine;             // 中断线号
    INT32U IntType;             // 中断触发类型
    OS_EVENT *pEvent;			// 事件指针,用来传递消息事件
}_BSPGPIO_CONFIG;


#define PORT0 0x00
#define PORT1 0x01
#define PORT2 0x02
#define PORT3 0x03

#define GPIO_DIR_I 0x01
#define GPIO_DIR_O 0x00

#define GPIO_INTA   0x00
#define GPIO_INTB   0x01

#define GPIO_INT_TYPE_NO    0x01
#define GPIO_INT_TYPE_L     0x04
#define GPIO_INT_TYPE_H     0x08
#define GPIO_INT_TYPE_BOTH  0x0C

//#define SDCARD_HOT_START
#ifdef SDCARD_HOT_START
#define BSP_SDINIT_COMFROM_GPIO 0xC1
#endif //SDCARD_HOT_START

void BSP_GPIOINTnIRQHandler(unsigned int portnum,unsigned int pinnum,unsigned int intLine);
INT8U BSP_GPIOConfig(_BSPGPIO_CONFIG *pConfig);
INT8U BSP_GPIOInit(INT8U num);



#endif
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/

