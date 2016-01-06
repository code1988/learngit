/*****************************************Copyright(C)******************************************
*******************************************�㽭����*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			    : app_cfg.h
* Author			    : ��ҫ
* Date First Issued	    : 10/29/2013
* Version			    : V
* Description		    : Ӧ�ò���������ȼ������ļ�
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			    :
* //2013		        : V
* Description		    :
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
// ���ȼ�����
#define OS_TASK_ITSVC_PRIO 		0			        //ϵͳӲ�������������ȼ������
#define	PRI_SYSTEM_0			1			        //ϵͳ���������ȼ�
#define	PRI_SYSTEM_1			2			        //ϵͳ���������ȼ�
#define	PRI_SYSTEM_2			3			        //ϵͳ���������ȼ�
#define	PRI_SYSTEM_3			OS_LOWEST_PRIO-2	//ϵͳ���������ȼ�
#define	PRI_SYSTEM_4			OS_LOWEST_PRIO-1	//ϵͳ���������ȼ�
#define	PRI_SYSTEM_5			OS_LOWEST_PRIO		//ϵͳ���������ȼ�
// ϵͳ��ʼ���������ȼ�
#define OS_TASK_INIT_PRIO 		38
// �û��������ȼ�
#define	PRI_WDG				    5    	// ���Ź�����
#define PRI_TaskDisplay         6
#define PRI_SPI_MUTEX           7       // SPI���������ȼ��컨��
#define PRI_LCD                 18   	// LCD����
#define	PRI_TEST			    19   	// ��������
#define PRI_FLASH            	20   	// NAND����
#define PRI_USB                 21   	// USB����
#define PRI_TaskComWithMC       22      //
#define PRI_TaskComWithDSP1	    23		// DSPͨѶ����  
#define PRI_TaskComWithDSP2	    24		// DSPͨѶ����
#define PRI_2410			    25		// 2410 ͨ��

#define PRI_DSP_NET_SERVER_TIME 26		// dsp net server_time

#define PRI_NET_SERVER_TIME	    27		// net server_time
#define PRI_NET_SERVER		    28		// net server



#define PRI_ARM_UART            30		// arm uart

#define PRI_ARM_UART_SEND       31		// arm uart send

#define PRI_SD_MUTEX            32      // SD�����������ȼ��컨��
#define PRI_SD_MASTER           33      // SD����ȡ������
#define PRI_SD_MINOR           	34      // SD����ȡ��������

#define PRI_Task_PHY_DSP        35      // A8_PHY_DSPͨѶ����


#define	PRI_LED				    37   	// LEDָʾ������

//USBɨ��,��ʱ��Ϊ38��������ܸ�app_cfg.h�еĳ�ͻ
#define	PRI_USBHCD				38  	// usbɨ������



//LWIP��ֲ���õ����������ȼ���ethernet��ص�����ռ��10��17�ܹ�8���������ȼ�
//#define  APP_TASK_START_PRIO                               3
//#define  APP_TASK_KBD_PRIO                                 4
//#define  APP_TASK_USER_IF_PRIO                            14

#define  LWIP_TASK_START_PRIO                             10
#define  LWIP_TASK_END_PRIO                               17

//#define  LWIP_TCPSERVER_PRIO                              15
//#define  LWIP_UDPSERVER_PRIO                              16

#define  OS_PROBE_TASK_PRIO              (OS_LOWEST_PRIO - 3)
#define  OS_TASK_TMR_PRIO                (OS_LOWEST_PRIO - 2)

//#define  APP_ID                       8192u
//#define  ETHERNET_DRV_ID              APP_ID+1
//#define  ETHERNET_TSK_ID              APP_ID+2
//#define  LWIP_TSK_ID                  APP_ID+3

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_STK_SIZE                   128


/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#define  APP_TRACE_LEVEL                        TRACE_LEVEL_DBG
#define  APP_TRACE                              printf

#define  APP_TRACE_INFO(x)                      ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO) ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)                       ((APP_TRACE_LEVEL >= TRACE_LEVEL_DBG ) ? (void)(APP_TRACE x) : (void)0)
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/

#endif
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/
