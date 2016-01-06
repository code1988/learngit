/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: lcdmain.c
**硬          件: am335x
**创    建    人: wangyao
**创  建  日  期: 131029
**最  新  版  本: V0.1
**描          述: LCD显示的任务主文件
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "guisprint.h"
#include "display.h"
#include "key.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
typedef enum
{
	//0~9数字键
	BSP_KEY_0,	    
	BSP_KEY_1,		
	BSP_KEY_2,
	BSP_KEY_3,
	BSP_KEY_4,
	BSP_KEY_5,
	BSP_KEY_6,
	BSP_KEY_7,
	BSP_KEY_8,//作为返回键使用
	BSP_KEY_9,
}_BSP_KEYMSG;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
tContext sContext;//文本
extern const unsigned char g_pucImage[];
extern const unsigned char g_TILogo[];
extern const unsigned char key2[];

OS_EVENT *LCDDisplay_prevent;	//定义液晶显示邮箱
void *LCDDisplay[8];		    //8个消息可以存储
//static FILINFO FileInfo;
//static FILINFO FileInfo1;
/* Private function prototypes----------------------------------------------------------------*/
void  KeyMessageDealWith(INT8U *pdata);
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: DummyKeyInit 
* Description	: 虚拟按键，用串口代替
* Input			: none
* Output		: none
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
INT8U TxKeyBuff[10]={0};
INT8U RxKeyBuff[10]={0};
void DummyKeyInit(void)
{
    _BSPUART_CONFIG UART_Config;

    LCDDisplay_prevent = OSQCreate(LCDDisplay,8);
    UART_Config.Baudrate = 115200;		            // 波特率
    UART_Config.Parity = BSPUART_PARITY_NO;			// 校验位
    UART_Config.StopBits = BSPUART_STOPBITS_1;		// 停止位
    UART_Config.WordLength = BSPUART_WORDLENGTH_8D;	// 数据位数
    UART_Config.Work = BSPUART_WORK_FULLDUPLEX;		// 工作模式
    UART_Config.TxRespond = BSPUART_RESPOND_INT;	// 中断模式
    UART_Config.pEvent = LCDDisplay_prevent;	    // 消息事件
    UART_Config.MaxTxBuffer = 10;	                // 发送缓冲容量
    UART_Config.MaxRxBuffer = 10;	                // 接收缓冲容量
    UART_Config.pTxBuffer = TxKeyBuff;	// 发送缓冲指针
    UART_Config.pRxBuffer = RxKeyBuff;	// 接收缓冲指针
    UART_Config.TxSpacetime = SYS_DELAY_20ms;	// 发送帧间隔
    UART_Config.RxOvertime = SYS_DELAY_50ms;	// 接收帧间隔

    BSP_UARTConfig(0,&UART_Config);    
}
/*****************************************************************   
函数名称：INT32U IntHexToBcd(INT32U Source)
输入参数：Source-待转化的Hex数据   
输出参数：转化后的整型BCD码数据
约束条件：无
功能描述：将整型的十六进制数转换成BCD码
作    者：                     日期:  
修    改：                     日期:  
版    本：  
*****************************************************************/
INT32U IntHexToBcd(INT32U Source)
{
	INT32U Result, tBase[8], Temp1, Temp2;
	int i, j;

	tBase[0] = 10000000;
	tBase[1] = 1000000;
	tBase[2] = 100000;
	tBase[3] = 10000;
	tBase[4] = 1000;
	tBase[5] = 100;
	tBase[6] = 10;
	tBase[7] = 1;

	Result = 0;		
	for(i = 0; i < 8; i++)
	{
		Temp1 = 0;  
		Temp2 = 0;
		for(j = 0; j < 10; j++)
		{
			Temp1 += tBase[i];
			if(Source < Temp1)
			{
				Result = (Result << 4);
				Result = (Result | j);
				Source -= Temp2;
				break;
			}
			Temp2 = Temp1;			
		}
	}		
	return Result;
}

/***********************************************************************************************
* Function		: void  KeyMessageDealWith(INT8U *pdata)
* Description	: 按键消息到来处理相应按键功能
* Input			: *pdata 按键值
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void  KeyMessageDealWith(INT8U *pdata)
{
    _BSP_KEYMSG keyval;
    
	keyval = (_BSP_KEYMSG)(*(pdata)&0x0F);//十进制十六进制发送过来都可以
	switch(keyval)
	{
		case BSP_KEY_0:
			break;	    
		case BSP_KEY_1:
            Key_one(keyval);
			break;		
		case BSP_KEY_2:
            Key_two(keyval);
			break;
		case BSP_KEY_3:
			break;
		case BSP_KEY_4:
			break;
		case BSP_KEY_5:
			break;
		case BSP_KEY_6:
			break;
		case BSP_KEY_7:
			break;
        case BSP_KEY_8:
            Key_eight(keyval);
			break;    
		default:
			break;
	}
}

/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/