/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: main.h
* Author			: 王耀
* Date First Issued	: 10/09/2013
* Version			: V
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
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

#define	__DEBUG_TASK_TEST_ENABLE        0	    // 测试任务使能

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
void TaskMMCSD(void *pdata);//SD卡处理任务
void TaskFlash(void *pdata);//FLASH处理任务
void TaskUSB(void *pdata);//USB任务
void TaskComWithMC(void *pdata);
void TaskDisplay(void *pdata);
void TaskComWithDSP1(void *pdata);	// DSP1通讯任务
void TaskComWithDSP2(void *pdata);	// DSP2通讯任务
void TaskSDMaster(void *pdata);  // SD卡存取主任务
void TaskSDMinor(void *pdata);  // SD卡存取辅助任务
void Task_PHY_DSP(void *pdata); // A8_PHY_DSP通讯任务

/* Private functions--------------------------------------------------------------------------*/
#endif
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
