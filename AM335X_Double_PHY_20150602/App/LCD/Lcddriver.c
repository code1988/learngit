/*****************************************Copyright(C)******************************************
*******************************************浙江万马*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: lcddriver.c
* Author			: 王耀
* Date First Issued	: 12/10/2010
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "app.h"
#include "bsp.h"
#include "appinterfacefun.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
void displayOSTime(void);
/* Private functions--------------------------------------------------------------------------*/

/*****************************************************************************
* Function          : FirstLine 
* Description       : 显示第一行
* Input             : 无
* Output            : 无
* Contributor       : wangyao
* Date First Issued : 2010/12/12
******************************************************************************/
void FirstLine(void)
{
    displayOSTime();
}
/*****************************************************************************
* Function          : displayOSTime 
* Description       : 显示系统时间
* Input             : 无
* Output            : 无
* Contributor       : wangyao
* Date First Issued : 2010/12/12
******************************************************************************/
void displayOSTime(void)
{
    INT8U ypos=0;
    INT16U hour, minute, second;

	hour=gRtcTime.Hour;
	minute=gRtcTime.Minute;
	second=gRtcTime.Second;
    BSP_GUISprint(32,ypos,"2xZ",(INT8U *)&hour);
	BSP_GUIDispChars(34,ypos,1,":");
	BSP_GUISprint(35,ypos,"2xZ",(INT8U *)&minute);
	BSP_GUIDispChars(37,ypos,1,":");
	BSP_GUISprint(38,ypos,"2xZ",(INT8U *)&second);
	BSP_GUIHLine(0,16,39,1);//画一条线
	BSP_GUIHLine(0,192,39,1);//画一条线

}
/***********************************************************************************************
* Function		: LasttowLine
* Description	: 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
INT8U  GetElectrifyFlag(void);
void LasttowLine(void)
{
	INT8U flag=0;
	INT8U data[2]={0};
	INT8U offsetx=0;
	InterfaceStruct powerdata;
	flag=GetElectrifyFlag();  //获取当前充电模式
	if(flag)
	{
		BSP_GUIFill(0,193,319,239,0);
		BSP_GUIDispChars(0,193,29,"当前A相电压:");
		powerdata.RWChoose = READ_ORDER;//读
		powerdata.DataID = 0xB611;
		powerdata.len    = 2;
		powerdata.ptr = data;
		DataAppfunction(&powerdata);
		offsetx = BSP_GUISprint(12, 193,"4.1pL",data);
		BSP_GUIDispChars(12+offsetx,193,1,"V");
		BSP_GUIDispChars(0,209,22,"当前A相电流:");
		powerdata.DataID = 0xB621;
		powerdata.len    = 2;
		powerdata.ptr = data;
		DataAppfunction(&powerdata);
		offsetx = BSP_GUISprint(12, 209,"4.2pL",data);
		BSP_GUIDispChars(12+offsetx,209,1,"A");
	}
	else
	{
		BSP_GUIFill(0,193,39,239,0);
		BSP_GUIDispChars(0,193,29,"公司网址:http://www.wmxny.com");
		BSP_GUIDispChars(0,209,22,"联系电话:0571-61078251");
	}
}
/************************(C)COPYRIGHT 2010 浙江万马****END OF FILE****************************/
