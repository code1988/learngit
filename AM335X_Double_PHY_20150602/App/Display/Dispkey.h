/*****************************************Copyright(C)******************************************
*******************************************浙江万马*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: key.h
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
#ifndef	__KEY_H_
#define	__KEY_H_
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
//主函数中处理相关接收到信息的主处理函数
void DealWithKey(INT8U *key);
/***********************************************************************************************
* Function		: Key_one
* Description	: 0~9按键功能函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 19/11/2014	songchao
***********************************************************************************************/
void KeyEvent_0(INT8U *keyval);
void KeyEvent_1(INT8U *keyval);
void KeyEvent_2(INT8U *keyval);
void KeyEvent_3(INT8U *keyval);
void KeyEvent_4(INT8U *keyval);
void KeyEvent_5(INT8U *keyval);
void KeyEvent_6(INT8U *keyval);
void KeyEvent_7(INT8U *keyval);
void KeyEvent_8(INT8U *keyval);
void KeyEvent_9(INT8U *keyval);
void KeyEvent_10(INT8U *keyval);
void KeyEvent_11(INT8U *keyval);
void KeyEvent_12(INT8U *keyval);
void KeyEvent_14(INT8U *keyval);
void KeyEvent_15(INT8U *keyval);
void KeyEvent_16(INT8U *keyval);
void KeyEvent_17(INT8U *keyval);
void KeyEvent_18(INT8U *keyval);
void KeyEvent_19(INT8U *keyval);
void KeyEvent_20(INT8U *keyval);
void KeyEvent_21(INT8U *keyval);
void KeyEvent_22(INT8U *keyval);
void KeyEvent_23(INT8U *keyval);
void KeyEvent_24(INT8U *keyval);
void KeyEvent_25(INT8U *keyval);
void KeyEvent_26(INT8U *keyval);
void KeyEvent_27(INT8U *keyval);
void KeyEvent_28(INT8U *keyval);
void KeyEvent_40(INT8U *keyval);
void KeyEvent_41(INT8U *keyval);
void KeyEvent_50(INT8U *keyval);
void KeyEvent_51(INT8U *keyval);
void KeyEvent_52(INT8U *keyval);
void KeyEvent_53(INT8U *keyval);
void KeyEvent_54(INT8U *keyval);
void KeyEvent_55(INT8U *keyval);
void KeyEvent_56(INT8U *keyval);
void KeyEvent_79(INT8U *keyval);
void KeyEvent_80(INT8U *keyval);
void KeyEvent_81(INT8U *keyval);
void KeyEvent_82(INT8U *keyval);
void KeyEvent_83(INT8U *keyval);
void KeyEvent_84(INT8U *keyval);
void KeyEvent_87(INT8U *keyval);
void KeyEvent_88(INT8U *keyval);
void KeyEvent_89(INT8U *keyval);
void KeyEvent_90(INT8U *keyval);
void KeyEvent_91(INT8U *keyval);



void DealCursor(INT8U Cursor);
void DealKeyBoard(INT8U Cursor);
void Dealblacklist(INT8U Cursor);
void DeleteBlack(void);
/***********************************************************************************************
* Function		: void Key_return(void)
* Description	: 返回键
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_return(void);
void Key_Clear(void);
/***********************************************************************************************
* Function		: Key_Enter(void)
* Description	: 确认键控制
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_Enter(void);
void Key_Down(void);
void Key_UP(void);
#endif	//__KEY_H_
/************************(C)COPYRIGHT 2010 浙江万马*****END OF FILE****************************/
