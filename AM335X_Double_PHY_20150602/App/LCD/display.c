/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: display.c
* Author			: 王耀
* Date First Issued	: 02/20/2014
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "guisprint.h"
#include "display.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
INT8U Setdata[30];//设置测量点号用全局变量
INT16U MeasNum=0;//输入测量点号
SetDec gps_data={64,1,16,8,1,0,0,0,Setdata};//设置页面用结构体
ST_Menu *gps_CurMenu;     //当前菜单
INT8U  gCursorInMenu;    //光标在当前菜单中的序号
ST_Cursor  gs_CursorInPage;   //当前页中的光标位置
//上下箭头数组前2个数据表示图形的长宽,后面为图形数组
//static const INT8U downdata[]={0x08,0x08,0x10,0x20,0x40,0xff,0x40,0x20,0x10,0x00};
//static const INT8U updata[]={0x08,0x08,0x08,0x04,0x02,0xff,0x02,0x04,0x08,0x00};
INT32S (*LcdPage)(INT16U *MeterNo,INT32U type);//页面显示函数指针
/* Private function prototypes -----------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/********************************************************************
* Function Name : DisplayAll()
* Description   : 显示函数入口,根据菜单属性不同显示菜单,页面,设置页面.
* Input         : -*pMenu,当前待显菜单结构. 
*                 -cursor,反显条序号
* Output        : None.
* Return        : None.
********************************************************************/
void DisplayAll(struct st_menu* pMenu, INT8U cursor)
{    
	INT8U i=0;
//	INT32S index=0;
    
    LcdPage=NULL;//函数指针初始化
    gps_CurMenu=pMenu;//当前菜单指针赋值
	LcdPage=*gps_CurMenu->funpage;//函数指针赋值
    gps_data.count=0;//计数清零

    if((pMenu->Menu_Flag==0)||(pMenu->Menu_Flag==1))//显示主页,或者子页面面菜单
	{	
		gCursorInMenu = cursor;//光标在当前菜单中的序号
    	if(cursor > (pMenu->Menu_Maxcursor-1)) //如果光标超出范围
    		return;
        BSP_GUIFill(0,0,LCD_WIDTH-1,LCD_HEIGHT-1,ClrDarkBlue);
    	gs_CursorInPage.YCoor = MENU_Y_START;//光标所在位置
    	gs_CursorInPage.XCoor = MENU_X_START;
    	//显示第该页
        GrContextBackgroundSet(&sContext,ClrDarkBlue);
        GrContextFontSet(&sContext, &g_sFontCH24);
        GrContextForegroundSet(&sContext, ClrWhite);
		for(i=0;i<gps_CurMenu->Menu_Maxcursor;i+=2)
		{
			GrStringDraw(&sContext,pMenu->Menu_pContent[i],4,MENU_X_START,MENU_Y_START+30*i,0);
            if((i+1)<=(gps_CurMenu->Menu_Maxcursor-1))
            {
                GrStringDraw(&sContext,pMenu->Menu_pContent[i+1],4,MENU_X_START+370,MENU_Y_START+30*i,0);
            }
		}
		BSP_UpdataLCD(); 
	}
    else
    {
        return;   
    }
	

}

/************************(C)COPYRIGHT 2010 浙江万马****END OF FILE****************************/
