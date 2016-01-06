/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: main.c
* Author			: 
* Date First Issued	: 130722   
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
//һ���˵�  ���˵�
*******************************************************************************/
//���˵��Ӳ˵�
ST_Menu* gpMenu0Kids[8] = {&gsMenu0_1,&gsMenu0_2,&gsMenu0_3,&gsMenu0_4,\
                           &gsMenu0_5,&gsMenu0_6,&gsMenu0_7,&gsMenu0_8} ;

//�˵�������Ӳ˵�
ST_Menu* gpMenu8Kids[8] = {NULL,NULL,NULL,NULL,\
                           NULL,&gsMenu0_11,&gsMenu_41,NULL} ;

//���˵�
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

/*****************************�����˵�******************************************/
//���ֲ˵�
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

//�����˵�
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

//�����ϸ�˵�
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

//�˳��ڲ鿴�Ӳ˵�
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

//�ӳ��ڲ鿴�Ӳ˵�
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

//��ֵȼ��˵�
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

//��α�ȼ�
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

//�˵�����
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

//�����鿴�˵�
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

//�����޸Ĳ˵�
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

//����Աǩ���˵�
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

//����Աǩ�˲˵�
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

//���׺�����
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

//��Ϣ�鿴1
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

//��Ϣ�鿴2
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

//����Ų鿴
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

//ʱ��鿴
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

//�汾��Ϣ1
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

//�汾��Ϣ2
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

//���������ò˵�
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

//�汾��Ϣ3
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

//������
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

//����������Ϣ
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

//����������
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

//���ض˿�
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

//Զ�˶˿�
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

//Զ��IP
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

//��ֹ����ʱ��
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

//����ά���˵�
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

//�����޸Ĳ˵�
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

//�����Ϣ����
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

//�ٶȵ��Բ˵�
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

//CSУ��
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

//������Բ˵�
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

//�����ӳ�AD
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

//�����˵�
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

//��С��ͷ
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

//ӫ��
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

//�߳�����
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

//�߳�MT
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

//�߳�MG
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

//�ϻ�У��
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

//MT���ݲ鿴
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

/************************(C)COPYRIGHT 2014 �㽭��̩*****END OF FILE****************************/