/*****************************************Copyright(C)******************************************
*******************************************浙江万马*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_lcd.h
* Author			: 王耀
* Date First Issued	: 10/12/2010
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_GUI_H_
#define	__BSP_GUI_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "grlib.h"
#include "hsi2c.h"
#include "widget.h"
#include "canvas.h"
#include "pushbutton.h"
#include "checkbox.h"
#include "radiobutton.h"
#include "container.h"
#include "slider.h"
#include "interrupt.h"
#include "cache.h"
/* Private define-----------------------------------------------------------------------------*/
#define PALETTE_SIZE   32
#define LCD_WIDTH      480
#define LCD_HEIGHT     272
#define NUM_ICONS 4
#define PB	0
#define CB	1
#define RB1  2
#define RB2  3

#define	PB_X_MIN	45
#define	PB_Y_MIN	45
#define	PB_X_MAX	(45+75-1)
#define	PB_Y_MAX	(45+75-1)

#define	CB_X_MIN	175
#define	CB_Y_MIN	45
#define	CB_X_MAX	(175+75-1)
#define	CB_Y_MAX	(45+75-1)

#define	RB1_X_MIN	275
#define	RB1_Y_MIN	45
#define	RB1_X_MAX	(275+150-1)
#define	RB1_Y_MAX	(45+35-1)

#define	RB2_X_MIN	275
#define	RB2_Y_MIN	80
#define	RB2_X_MAX	(275+150-1)
#define	RB2_Y_MAX	(80+35-1)
enum button_state
{
	PRESSED = 0,
	RELEASED = 1
};

extern const tFont g_sFontCH24;
/* Private typedef----------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BSP_GUIInit
* Description	: GUI初始化,带LCD初始化
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void BSP_GUIInit(tContext *pContext);
void LCDIRQHandler(void);
/***********************************************************************************************
* Function	: BSP_UpdataLCD
* Description	: 刷新屏幕
* Input		: 
* Output	: 
* Note(s)	: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void BSP_UpdataLCD(void);

#endif	//__BSP_GUI_H_
/************************(C)COPYRIGHT 2010 浙江万马*****END OF FILE****************************/
