/*********************************Copyright(c)***********************************
*****************************�㽭��̩********************************************
**-------------------------------------�ļ���Ϣ----------------------------------
**��   ��   ��: menustruct.c
**��   ��   ��: wangyao
**�� �� ��  ��: 2014.02.20
**�� �� ��  ��: V1.0
**��        ��: ����˵��ṹ,�����б�����
**-----------------------------------��ʷ�汾��Ϣ---------------------------------
** �޸���:  ������
** �ա���:  ������
** ��  ���� ��������
** �衡��:  ��������

***********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "sysconfig.h"
#include "display.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*����ָ���б�
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

//2���˵�
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
//һ���˵�  �˵���ҳ
*******************************************************************************/
ST_Menu* gpMenu0Kids[8] = {&gsMenu0_1,&gsMenu0_2,&gsMenu0_3,&gsMenu0_4,\
                           &gsMenu0_5,&gsMenu0_6,&gsMenu0_7,&gsMenu0_8} ;
const char* pMenu0_String[8] ={"����","�ֳ�","�鿴","���",\
                               "����","�ܳ�","����","��ӡ"};
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
/*****************************�����˵�******************************************/
ST_Menu* gpMenu0_2Kids[8] = {&gsMenu0_2_1,&gsMenu0_2_2,&gsMenu0_2_3,&gsMenu0_2_4,&gsMenu0_2_5};
const char* pMenu0_2String[8] ={"�㳮���","�嵥ģʽ","����������","�ܼ���","�ռ���"};
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
const char* pMenu0_3String[8] ={"�ܳ�ģʽ","�㳮�ٶ�","�ۼ�","���ֺ�",
                                "Ԥ����","��ʾ��","��ӡ��","����"};
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
/*************2014****@COPYRIGHT*********�㽭��̩*******END FILE***********************/
