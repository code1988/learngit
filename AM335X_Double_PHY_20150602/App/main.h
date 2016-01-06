/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: main.h
* Author			: ��ҫ
* Date First Issued	: 10/09/2013
* Version			: V
* Description		:
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2010		: V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __MAIN_H_
#define __MAIN_H_
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
#ifdef __DEBUG
#endif

#define	__DEBUG_TASK_TEST_ENABLE        0	    // ��������ʹ��

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
void TaskStart(void *pdata);
#if __DEBUG_TASK_TEST_ENABLE
	void TaskTest(void *pdata);
#endif	//__DEBUG_TASK_TEST_ENABLE
static void init_task_core(void *pdata);
void TaskLED(void *pdata);
void TaskWDG(void *pdata);
void TaskMMCSD(void *pdata);//SD����������
void TaskFlash(void *pdata);//FLASH��������
void TaskUSB(void *pdata);//USB����
void TaskComWithMC(void *pdata);
void TaskDisplay(void *pdata);
void TaskComWithDSP1(void *pdata);	// DSP1ͨѶ����
void TaskComWithDSP2(void *pdata);	// DSP2ͨѶ����
void TaskSDMaster(void *pdata);  // SD����ȡ������
void TaskSDMinor(void *pdata);  // SD����ȡ��������
void Task_PHY_DSP(void *pdata); // A8_PHY_DSPͨѶ����

/* Private functions--------------------------------------------------------------------------*/
#endif
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/
