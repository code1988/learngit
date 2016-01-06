/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName		    : bsp_led.h
* Author		    : Wang WenGang
* Date First Issued	: 2014-7-10 8:39:49   
* Version		    : v1
* Description		: СLED�����
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History		    :
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_LED_H_
#define	__BSP_LED_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "hsi2c.h"
/* Private define-----------------------------------------------------------------------------*/
#define NUM_SHOW_INTERVAL 	0x05  // led ѭ����ʾ��� ms

// kou
#define  BSP_LED_SEL_1          0x00
#define  BSP_LED_SEL_2          0x01
#define  BSP_LED_SEL_3          0x02
// ��ʾ����

#define  BSP_LED_DISP_DIGIT     0x00u
#define  BSP_LED_DISP_LINE      0x01u
#define  BSP_LED_DISP_DLINE     0x02u
#define  BSP_LED_DISP_ERR       0x03u
#define  BSP_LED_DISP_DIGIT_PRE 0x04u
#define  BSP_LED_DISP_NONE      0x10u

#define  BSP_LED_CTL_FLASH_OFF  0x00u
#define  BSP_LED_CTL_FLASH_ON   0x01u


// led ��Ϣ����
typedef struct
{
    INT8U  MsgType[3];		// ����
    INT8U  LedSel;       // ��һ��С��
    INT8U  LedArg[3];		// LED��ʾ������
    INT16U LedNum[3];		// ��ʾ������  
}_LED_CONTROL;
/*
typedef struct
{
	_LED_MESSAGE Message;						// ������Ϣ
	OS_EVENT *Event;							// ��ֵ�������¼�ָ��
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
extern _LED_CONTROL LED_Control;				// keyģ����Ʊ�������	
extern OS_EVENT *   LEDEvent;

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/

void vfd_wrtime();
void led_din_gpio_init(void);
void led_clock_gpio_init(void);
void led_stb_gpio_init(void);
void led_din_gpio_init(void);
/****************************** End Of File *********************************/

