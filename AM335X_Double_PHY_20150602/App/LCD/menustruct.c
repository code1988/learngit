/*********************************Copyright(c)***********************************
*****************************浙江方泰********************************************
**-------------------------------------文件信息----------------------------------
**文   件   名: menustruct.c
**创   建   人: wangyao
**创 建 日  期: 2014.02.20
**最 新 版  本: V1.0
**描        述: 定义菜单结构,函数列表数组
**-----------------------------------历史版本信息---------------------------------
** 修改人:  ×××
** 日　期:  ×××
** 版  本： ××××
** 描　述:  ××××

***********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "sysconfig.h"
#include "display.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*函数指针列表
INT32S (*functiondata[100])(INT8U *MeterNo,INT32U type)={

};
*/
/* Private function prototypes -----------------------------------------------*/
ST_Menu gsMenu0;
ST_Menu gsMenu0_1;
ST_Menu gsMenu0_2;
ST_Menu gsMenu0_3;
ST_Menu gsMenu0_4;
ST_Menu gsMenu0_5;
ST_Menu gsMenu0_6;
ST_Menu gsMenu0_7;
ST_Menu gsMenu0_8;

//2级菜单
ST_Menu gsMenu0_2_1;
ST_Menu gsMenu0_2_2;
ST_Menu gsMenu0_2_3;
ST_Menu gsMenu0_2_4;
ST_Menu gsMenu0_2_5;

ST_Menu gsMenu0_3_1;
ST_Menu gsMenu0_3_2;
ST_Menu gsMenu0_3_3;
ST_Menu gsMenu0_3_4;
ST_Menu gsMenu0_3_5;
ST_Menu gsMenu0_3_6;
ST_Menu gsMenu0_3_7;
ST_Menu gsMenu0_3_8;
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
//一级菜单  菜单首页
*******************************************************************************/
ST_Menu* gpMenu0Kids[8] = {&gsMenu0_1,&gsMenu0_2,&gsMenu0_3,&gsMenu0_4,\
                           &gsMenu0_5,&gsMenu0_6,&gsMenu0_7,&gsMenu0_8} ;
const char* pMenu0_String[8] ={"货币","分钞","查看","清分",\
                               "操作","拒钞","进阶","打印"};
ST_Menu gsMenu0 = 
{
    NULL,
    gpMenu0Kids,
	NULL,
	NULL,
    pMenu0_String,
	8,
    0,
	1,
	NULL,
	DisplayAll,
};
/*****************************二级菜单******************************************/
ST_Menu* gpMenu0_2Kids[8] = {&gsMenu0_2_1,&gsMenu0_2_2,&gsMenu0_2_3,&gsMenu0_2_4,&gsMenu0_2_5};
const char* pMenu0_2String[8] ={"点钞结果","清单模式","各钞口数量","总计数","日计数"};
ST_Menu gsMenu0_2 =
{
    &gsMenu0,
    gpMenu0_2Kids,
	NULL,
	NULL,
    pMenu0_2String,
	5,
    0,
	1,
	NULL,
	DisplayAll,    
};
ST_Menu* gpMenu0_3Kids[8] = {&gsMenu0_3_1,&gsMenu0_3_2,&gsMenu0_3_3,&gsMenu0_3_4,
                             &gsMenu0_3_5,&gsMenu0_3_6,&gsMenu0_3_7,&gsMenu0_3_8};
const char* pMenu0_3String[8] ={"跑钞模式","点钞速度","累加","冠字号",
                                "预置数","警示音","打印机","语言"};
ST_Menu gsMenu0_3 = 
{
    &gsMenu0,
    gpMenu0_3Kids,
	NULL,
	NULL,
    pMenu0_3String,
	8,
    0,
	1,
	NULL,
	DisplayAll,    
};
/*************2014****@COPYRIGHT*********浙江方泰*******END FILE***********************/
