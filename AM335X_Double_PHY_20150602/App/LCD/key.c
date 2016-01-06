/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: key.c
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
#include "key.h"
#include "display.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: Key_zero
* Description	: 0按键功能函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_zero(INT8U keyval)
{
	keyval&=0x0f;
	if(keyval>9)
		return;
	if((gps_CurMenu->Menu_Flag==21)||\
	   (gps_CurMenu->Menu_Flag==22)||\
	   (gps_CurMenu->Menu_Flag==23)||\
	   (gps_CurMenu->Menu_Flag==25))
	{
	}
	return;
}
/***********************************************************************************************
* Function		: Key_one
* Description	: 1按键功能函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_one(INT8U keyval)
{
	keyval&=0x0f;
	if(keyval>9)
		return;
	if((gps_CurMenu->Menu_Flag==0)||\
	   (gps_CurMenu->Menu_Flag==1)||\
	   (gps_CurMenu->Menu_Flag==3))//显示菜单
	{
    	/* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
    	if(gps_CurMenu->Menu_paKidsMenu == NULL)  
    		return;
    	gps_CurMenu->function(gps_CurMenu->Menu_paKidsMenu[keyval],0);
		return;
	}
	else 	if((gps_CurMenu->Menu_Flag==21)||\
	   (gps_CurMenu->Menu_Flag==22)||\
	   (gps_CurMenu->Menu_Flag==23)||\
	   (gps_CurMenu->Menu_Flag==25))
	{
		
	}
	return;
}
/***********************************************************************************************
* Function		: Key_two
* Description	: 1按键功能函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_two(INT8U keyval)
{
	keyval&=0x0f;
	if(keyval>9)
		return;
	if((gps_CurMenu->Menu_Flag==0)||\
	   (gps_CurMenu->Menu_Flag==1)||\
	   (gps_CurMenu->Menu_Flag==3))//显示菜单
	{
    	/* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
    	if(gps_CurMenu->Menu_paKidsMenu == NULL)  
    		return;
    	gps_CurMenu->function(gps_CurMenu->Menu_paKidsMenu[keyval],0);
		return;
	}
	else 	if((gps_CurMenu->Menu_Flag==21)||\
	   (gps_CurMenu->Menu_Flag==22)||\
	   (gps_CurMenu->Menu_Flag==23)||\
	   (gps_CurMenu->Menu_Flag==25))

	{
		
	}
	return;
}
/***********************************************************************************************
* Function		: Key_three
* Description	: 1按键功能函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_three(INT8U keyval)
{
	keyval&=0x0f;
	if(keyval>9)
		return;
	if((gps_CurMenu->Menu_Flag==0)||\
	   (gps_CurMenu->Menu_Flag==1)||\
	   (gps_CurMenu->Menu_Flag==3))//显示菜单
	{
    	/* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
    	if(gps_CurMenu->Menu_paKidsMenu == NULL)  
    		return;

    	gps_CurMenu->function(gps_CurMenu->Menu_paKidsMenu[keyval-1],0);
		return;

	}
	else 	if((gps_CurMenu->Menu_Flag==21)||\
	   (gps_CurMenu->Menu_Flag==22)||\
	   (gps_CurMenu->Menu_Flag==23)||\
	   (gps_CurMenu->Menu_Flag==25))

	{
		
	}
	return;
}
/***********************************************************************************************
* Function		: Key_four
* Description	: 4按键功能函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_four(INT8U keyval)
{
	keyval&=0x0f;
	if(keyval>9)
		return;
	if((gps_CurMenu->Menu_Flag==0)||\
	   (gps_CurMenu->Menu_Flag==1)||\
	   (gps_CurMenu->Menu_Flag==3))//显示菜单
	{
    	/* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
    	if(gps_CurMenu->Menu_paKidsMenu == NULL)  
    		return;
    	gps_CurMenu->function(gps_CurMenu->Menu_paKidsMenu[keyval-1],0);
		return;
	}
	else 	if((gps_CurMenu->Menu_Flag==21)||\
	   (gps_CurMenu->Menu_Flag==22)||\
	   (gps_CurMenu->Menu_Flag==23)||\
	   (gps_CurMenu->Menu_Flag==25))
	{
		
	}

	return;
}
/***********************************************************************************************
* Function		: Key_five
* Description	: 4按键功能函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_five(INT8U keyval)
{
	keyval&=0x0f;
	if(keyval>9)
		return;
	if((gps_CurMenu->Menu_Flag==0)||\
	   (gps_CurMenu->Menu_Flag==1)||\
	   (gps_CurMenu->Menu_Flag==3))//显示菜单
	{
    	/* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
    	if(gps_CurMenu->Menu_paKidsMenu == NULL)  
    		return;
    	gps_CurMenu->function(gps_CurMenu->Menu_paKidsMenu[keyval-1],0);
		return;
	}
	else 	if((gps_CurMenu->Menu_Flag==21)||\
	   (gps_CurMenu->Menu_Flag==22)||\
	   (gps_CurMenu->Menu_Flag==23)||\
	   (gps_CurMenu->Menu_Flag==25))
	{
		
	}
	return;
}
/***********************************************************************************************
* Function		: Key_six
* Description	: 6按键功能函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_six(INT8U keyval)
{
	keyval&=0x0f;
	if(keyval>9)
		return;
	if((gps_CurMenu->Menu_Flag==0)||\
	   (gps_CurMenu->Menu_Flag==1)||\
	   (gps_CurMenu->Menu_Flag==3))//显示菜单
	{
    	/* 已经是最底层菜单，直接返回或执行其他操作，取决于function */
    	if(gps_CurMenu->Menu_paKidsMenu == NULL)  
    		return;
    	gps_CurMenu->function(gps_CurMenu->Menu_paKidsMenu[keyval-1],0);
		return;
	}
	else if((gps_CurMenu->Menu_Flag==21)||\
	   (gps_CurMenu->Menu_Flag==22)||\
	   (gps_CurMenu->Menu_Flag==23)||\
	   (gps_CurMenu->Menu_Flag==25))
	{
		
	}
	return;
}
/***********************************************************************************************
* Function		: Key_seven
* Description	: 7按键功能函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_seven(INT8U keyval)
{
	keyval&=0x0f;
	if(keyval>9)
		return;
	if((gps_CurMenu->Menu_Flag==21)||\
	   (gps_CurMenu->Menu_Flag==22)||\
	   (gps_CurMenu->Menu_Flag==23)||\
	   (gps_CurMenu->Menu_Flag==25))
	{
		
	}
	return;
}
/***********************************************************************************************
* Function		: Key_eight
* Description	: 暂时做返回键功能用
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_eight(INT8U keyval)
{
	keyval&=0x0f;
	if(keyval>9)
		return;
    
	if(gps_CurMenu == MENU_TOPLAYER)//菜单首页
	{
		return;
	}
	else
	{
		if(gps_CurMenu->Menu_pFather!=NULL)//退出测量点清零
		{
			DisplayAll(gps_CurMenu->Menu_pFather,0);
		}
	}
}
/***********************************************************************************************
* Function		: Key_nigh
* Description	: 9按键功能函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2010	wangyao
***********************************************************************************************/
void Key_nigh(INT8U keyval)
{
	keyval&=0x0f;
	if(keyval>9)
		return;
	if((gps_CurMenu->Menu_Flag==21)||\
	   (gps_CurMenu->Menu_Flag==22)||\
	   (gps_CurMenu->Menu_Flag==23)||\
	   (gps_CurMenu->Menu_Flag==25))
	{
		
	}
	return;
}
/************************(C)COPYRIGHT 2010 浙江方泰****END OF FILE****************************/
