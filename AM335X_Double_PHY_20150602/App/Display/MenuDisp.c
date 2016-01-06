/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: main.c
* Author			: 
* Date First Issued	: 130722   
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			: 
* //2013	        : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "app.h"
#include "MenuDisp.h"
#include "DwinPotocol.h"
#include "display.h"
#include "Dispkey.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*******************************************************************************
//一级菜单  主菜单
*******************************************************************************/
//主菜单子菜单
ST_Menu* gpMenu0Kids[8] = {&gsMenu0_1,&gsMenu0_2,&gsMenu0_3,&gsMenu0_4,\
                           &gsMenu0_5,&gsMenu0_6,&gsMenu0_7,&gsMenu0_8} ;

//菜单界面的子菜单
ST_Menu* gpMenu8Kids[8] = {NULL,NULL,NULL,NULL,\
                           NULL,&gsMenu0_11,&gsMenu_41,NULL} ;

//主菜单
ST_Menu gsMenu0 = 
{
    NULL,
    gpMenu0Kids,
    8,
    0,
    0,
    NULL,
    DisplayMianMenu,
    0,
    KeyEvent_0,
    NULL,
};

/*****************************二级菜单******************************************/
//币种菜单
ST_Menu gsMenu0_1 =
{
    &gsMenu0,
    NULL,
    5,
    0,
    1,
    NULL,
    DisplayCommonMenu, 
    1,
    KeyEvent_1,
    NULL,
};

//批量菜单
ST_Menu gsMenu0_2 =
{
    &gsMenu0,
    NULL,
    5,
    0,
    1,
    NULL,
    DisplayCommonMenu, 
    2,
    KeyEvent_2,
    NULL,
};

//混点明细菜单
ST_Menu gsMenu0_3 = 
{
    &gsMenu0,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    3,
    KeyEvent_3,
    NULL,
};

//退钞口查看子菜单
ST_Menu gsMenu0_4 = 
{
    &gsMenu0,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    4,
    KeyEvent_4,
    NULL,
};

//接钞口查看子菜单
ST_Menu gsMenu0_5 = 
{
    &gsMenu0,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    5,
    KeyEvent_5,
    NULL,
};

//清分等级菜单
ST_Menu gsMenu0_6 = 
{
    &gsMenu0,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    6,
    KeyEvent_6,
    NULL,
};

//鉴伪等级
ST_Menu gsMenu0_7 = 
{
    &gsMenu0,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    7,
    KeyEvent_7,
    NULL,
};

//菜单界面
ST_Menu gsMenu0_8 = 
{
    &gsMenu0,
    gpMenu8Kids,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    8,
    KeyEvent_8,
    NULL,
};

//参数查看菜单
ST_Menu gsMenu0_11 = 
{
    &gsMenu0_8,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    11,
    KeyEvent_11,
    NULL,
};

//参数修改菜单
ST_Menu gsMenu0_12 = 
{
    &gsMenu_40,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    12,
    KeyEvent_12,
    NULL,
};

//操作员签到菜单
ST_Menu gsMenu_14 = 
{
    &gsMenu0_8,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    14,
    KeyEvent_14,
    NULL,
};

//操作员签退菜单
ST_Menu gsMenu_15 = 
{
    &gsMenu0_8,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    15,
    KeyEvent_15,
    NULL,
};

//交易号输入
ST_Menu gsMenu_16 = 
{
    &gsMenu0_8,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    16,
    KeyEvent_16,
    NULL,
};

//信息查看1
ST_Menu gsMenu_17 = 
{
    &gsMenu0_8,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    17,
    KeyEvent_17,
    NULL,
};

//信息查看2
ST_Menu gsMenu_18 = 
{
    &gsMenu0_8,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    18,
    KeyEvent_18,
    NULL,
};

//网点号查看
ST_Menu gsMenu_19 = 
{
    &gsMenu_17,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    19,
    KeyEvent_19,
    NULL,
};

//时间查看
ST_Menu gsMenu_20 = 
{
    &gsMenu_18,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    20,
    KeyEvent_20,
    NULL,
};

//版本信息1
ST_Menu gsMenu_21 = 
{
    &gsMenu0,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    21,
    KeyEvent_21,
    NULL,
};

//版本信息2
ST_Menu gsMenu_22 = 
{
    &gsMenu0,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    22,
    KeyEvent_22,
    NULL,
};

//黑名单设置菜单
ST_Menu gsMenu_23 = 
{
    &gsMenu0_8,
    NULL,
    0,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    23,
    KeyEvent_23,
    NULL,
};

//版本信息3
ST_Menu gsMenu_24 = 
{
    &gsMenu0_8,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    17,
    KeyEvent_24,
    NULL,
};

//机器号
ST_Menu gsMenu_25 = 
{
    &gsMenu_18,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    25,
    KeyEvent_25,
    NULL,
};

//MAC
ST_Menu gsMenu_26 = 
{
    &gsMenu_18,
    NULL,
    0,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    26,
    KeyEvent_26,
    NULL,
};

//开机出错信息
ST_Menu gsMenu_27 = 
{
    NULL,
    NULL,
    0,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    27,
    KeyEvent_27,
    NULL,
};

//黑名单输入
ST_Menu gsMenu_28 = 
{
    &gsMenu_23,
    NULL,
    0,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    14,
    KeyEvent_28,
    NULL,
};

//IP
ST_Menu gsMenu_50 = 
{
    &gsMenu_17,
    NULL,
    0,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    25,
    KeyEvent_50,
    NULL,
};

//MASK
ST_Menu gsMenu_51 = 
{
    &gsMenu_24,
    NULL,
    0,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    25,
    KeyEvent_51,
    NULL,
};

//GW
ST_Menu gsMenu_52 = 
{
    &gsMenu_24,
    NULL,
    0,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    25,
    KeyEvent_52,
    NULL,
};

//本地端口
ST_Menu gsMenu_53 = 
{
    &gsMenu_24,
    NULL,
    0,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    25,
    KeyEvent_53,
    NULL,
};

//远端端口
ST_Menu gsMenu_54 = 
{
    &gsMenu_24,
    NULL,
    5,
    0,
    3,
    NULL,
    NULL, 
    25,
    KeyEvent_54,
    NULL,
};

//远端IP
ST_Menu gsMenu_55 = 
{
    &gsMenu_24,
    NULL,
    5,
    0,
    3,
    NULL,
    NULL, 
    25,
    KeyEvent_55,
    NULL,
};

//禁止网发时间
ST_Menu gsMenu_56 = 
{
    &gsMenu_18,
    NULL,
    5,
    0,
    25,
    NULL,
    NULL, 
    8,
    KeyEvent_56,
    NULL,
};

//调试维护菜单
ST_Menu gsMenu_40 = 
{
    &gsMenu0,
    NULL,
    5,
    0,
    3,
    NULL,
    NULL, 
    40,
    KeyEvent_40,
    NULL,
};

//密码修改菜单
ST_Menu gsMenu_41 = 
{
    &gsMenu0_8,
    NULL,
    5,
    0,
    3,
    NULL,
    NULL, 
    41,
    KeyEvent_41,
    NULL,
};

//马达信息调试
ST_Menu gsMenu_79 = 
{
    &gsMenu_40,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    79,
    KeyEvent_79,
    NULL,
};

//速度调试菜单
ST_Menu gsMenu_80 = 
{
    &gsMenu_79,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    80,
    KeyEvent_80,
    NULL,
};

//CS校验
ST_Menu gsMenu_81 = 
{
    &gsMenu_79,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    81,
    KeyEvent_81,
    NULL,
};

//红外调试菜单
ST_Menu gsMenu_82 = 
{
    &gsMenu_79,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    82,
    KeyEvent_82,
    NULL,
};

//进钞接钞AD
ST_Menu gsMenu_83 = 
{
    &gsMenu_79,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    83,
    KeyEvent_83,
    NULL,
};

//升级菜单
ST_Menu gsMenu_84 = 
{
    &gsMenu_40,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    84,
    KeyEvent_84,
    NULL,
};

//大小磁头
ST_Menu gsMenu_85 = 
{
    &gsMenu_79,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    85,
    KeyEvent_82,
    NULL,
};

//荧光
ST_Menu gsMenu_86 = 
{
    &gsMenu_79,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    86,
    KeyEvent_82,
    NULL,
};

//走钞红外
ST_Menu gsMenu_87 = 
{
    &gsMenu_79,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    87,
    KeyEvent_87,
    NULL,
};

//走钞MT
ST_Menu gsMenu_88 = 
{
    &gsMenu_79,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    88,
    KeyEvent_88,
    NULL,
};

//走钞MG
ST_Menu gsMenu_89 = 
{
    &gsMenu_79,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    89,
    KeyEvent_89,
    NULL,
};

//老化校验
ST_Menu gsMenu_90 = 
{
    &gsMenu_40,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    90,
    KeyEvent_90,
    NULL,
};

//MT数据查看
ST_Menu gsMenu_91 = 
{
    &gsMenu_40,
    NULL,
    5,
    0,
    3,
    NULL,
    DisplayCommonMenu, 
    91,
    KeyEvent_91,
    NULL,
};

/************************(C)COPYRIGHT 2014 浙江方泰*****END OF FILE****************************/