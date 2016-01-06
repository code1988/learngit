/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_KEY.h
**创    建    人: wangyao
**创  建  日  期: 141203
**最  新  版  本: V0.1
**描          述: KEY的头文件
	使用步骤:
		1 在需要使用key的模块定义事件指针(邮箱),并创建该邮箱
		2 调用BSP_KEYInit()函数初始化key,传递上面定义的事件指针
		3 等待事件
		4 在出现合法按键后,驱动会发送一个事件消息,消息的内容就是键值(_BSPKEY_VALUE中定义),
		应用程序相应该事件,并根据消息内容处理按键.
		注:消息内容使用统一的结构_BSP_MESSAGE,但是只使用了DivNum项,用来保存键值
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述: 移植到AM335x
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_KEY_H_
#define	__BSP_KEY_H_
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/

// key常量定义
#define	KEY_MAX_NUM				6				// 按键引脚个数(按键个数)
#define	KEY_DELAY				20				// 去抖时间
#define	KEY_DELAY_LONG			70				// 长按键延时时间
#define	KEY_ENABLE_LONG			0				// 1:允许长按键(按键释放响应);0:只有短按键(按键按下响应,这个响应快点)
/* Private typedef----------------------------------------------------------------------------*/
//按键定义
#if 0
#define	BSPKEY_VALUE_ONE     0xFFDF			
#define	BSPKEY_VALUE_TOW     0xFFBF		
#define	BSPKEY_VALUE_THREE   0xFF7F			
#define	BSPKEY_VALUE_FOUR    0xFEFF
#define BSPKEY_VALUE_FIVE    0xBFFF				
#define	BSPKEY_VALUE_SIX     0xDFFF					
#define	BSPKEY_VALUE_SEVEN   0x6FFF				
#define	BSPKEY_VALUE_EIGHT   0xF7FF						
#define	BSPKEY_VALUE_NINE    0xFDFF					
#define	BSPKEY_VALUE_TEN     0xFBFF					
#define	BSPKEY_VALUE_ELEVEN  0x7FFF
#define	BSPKEY_VALUE_TWELVE  0x0004
#endif

#define	BSPKEY_VALUE_ONE     0xFE			
#define	BSPKEY_VALUE_TOW     0xFD		
#define	BSPKEY_VALUE_THREE   0xFB			
#define	BSPKEY_VALUE_FOUR    0xF7
#define BSPKEY_VALUE_FIVE    0x7F				
#define	BSPKEY_VALUE_SIX     0xBF					
#define	BSPKEY_VALUE_SEVEN   0xDF					
#define	BSPKEY_VALUE_EIGHT   0xEF							
#define	BSPKEY_VALUE_NINE    0x07					
#define	BSPKEY_VALUE_TEN     0x0B					
#define	BSPKEY_VALUE_ELEVEN  0x0D
#define	BSPKEY_VALUE_TWELVE  0x0E
/***********************************************************************************************
* Function		: BSP_165Init,BSP_165Read
* Description	: 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 140701 Wang WenGang
***********************************************************************************************/
INT8U KEYRead(void);

/***********************************************************************************************
* Function		: BSP_KEYScan
* Description	: 按键扫描程序,检测到按键后发送信号量.
	目前不能处理多个按键的情况,可能的按键值在_BSPKEY_VALUE中定义
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 081202	
***********************************************************************************************/
void BSP_KEYScan(void);
/***********************************************************************************************
* Function		: BSP_KEYHWInit
* Description	: 按键硬件初始化
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 
***********************************************************************************************/
void BSP_KEYHWInit(void);
/***********************************************************************************************
* Function		: BSP_KEYInit
* Description	: 驱动初始化,主要是引脚配置
* Input			: *Event:事件(邮箱)指针,使用key模块的用户必须指定一个用来传递按键信息和按键值的事件,
* Output		: 
* Note(s)		: 
* Contributor	: 081202	
***********************************************************************************************/
void BSP_KEYInit(OS_EVENT *Event);

void G1PinMuxSetup(unsigned int pinNo);
#endif	//__BSP_KEY_H_
/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/
