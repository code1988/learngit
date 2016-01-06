/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName		    : bsp_led.h
* Author		    : Wang WenGang
* Date First Issued	: 2014-7-10 8:39:49   
* Version		    : v1
* Description		: 小LED数码管
*----------------------------------------历史版本信息-------------------------------------------
* History		    :
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_LED_H_
#define	__BSP_LED_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "hsi2c.h"
/* Private define-----------------------------------------------------------------------------*/
#define NUM_SHOW_INTERVAL 	0x05  // led 循环显示间隔 ms

// kou
#define  BSP_LED_SEL_1          0x00
#define  BSP_LED_SEL_2          0x01
#define  BSP_LED_SEL_3          0x02
// 显示类型

#define  BSP_LED_DISP_DIGIT     0x00u
#define  BSP_LED_DISP_LINE      0x01u
#define  BSP_LED_DISP_DLINE     0x02u
#define  BSP_LED_DISP_ERR       0x03u
#define  BSP_LED_DISP_DIGIT_PRE 0x04u
#define  BSP_LED_DISP_NONE      0x10u

#define  BSP_LED_CTL_FLASH_OFF  0x00u
#define  BSP_LED_CTL_FLASH_ON   0x01u


// led 消息类型
typedef struct
{
    INT8U  MsgType[3];		// 类型
    INT8U  LedSel;       // 哪一个小灯
    INT8U  LedArg[3];		// LED显示的类型
    INT16U LedNum[3];		// 显示的数据  
}_LED_CONTROL;
/*
typedef struct
{
	_LED_MESSAGE Message;						// 保存消息
	OS_EVENT *Event;							// 键值传递用事件指针
}_LED_CONTROL;
*/
//#define LED_UPWARDS

/*	I2C instance	*/

#endif
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
extern const INT8U NCODE[15];
extern VINT8U   dataToSlave[4];
extern INT8U    dataFromSlave[4];
extern VINT32U  tCount;
extern VINT32U  rCount;
extern VINT8U   numFlagt;
extern INT32U   numFlashtime;
extern _LED_CONTROL LED_Control;				// key模块控制变量定义	
extern OS_EVENT *   LEDEvent;

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/

void vfd_wrtime();
void led_din_gpio_init(void);
void led_clock_gpio_init(void);
void led_stb_gpio_init(void);
void led_din_gpio_init(void);
/****************************** End Of File *********************************/

