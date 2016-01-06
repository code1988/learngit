/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName		: bsp_conf.h
* Author		: 王耀
* Date First Issued	: 2013-07-22
* Version		: V
* Description		: bsp的全局配置文件,主要定义了一些共用的结构类型
*----------------------------------------历史版本信息-------------------------------------------
* History		:
* //2010		: V
* Description		: 20141112  _BSP_MESSAGE结构体中datalen被定义为INT32U
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_CONF_H_
#define	__BSP_CONF_H_
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
//硬件外围驱动的启动
//#define  _BSP_LCD_ENABLE
#define  _BSP_BEEP_ENABLE
#define  _BSP_ETHERNET_ENABLE
#define  _BSP_MMCSD_ENABLE
#define  _BSP_SPI_ENABLE
#define  _BSP_USB_ENALBE 
#define  _BSP_UART_ENABLE
#define  _BSP_DCAN_ENABLE
#define  _BSP_KEY_ENABLE
#define  _BSP_NUM_ENABLE
#define  _BSP_I2C0_ENABLE
#define  _BSP_AT45ENABLE
#define  _BSP_NAND_ENABLE
#define  _BSP_EDMA_ENABLE
#define  _BSP_W5500_ENABLE
#define  _BSP_GPIO_ENABLE
//#define _BSP_COM2410_ENABLE
// 使用OS
#define	BSP_OS					1
#define	BSP_UCOSII				1
// OS类型定义
#if	(BSP_OS && BSP_UCOSII)
typedef	OS_EVENT				_OS_EVENT;
#else
typedef	INT8U					_OS_EVENT;
#endif	//BSP_OS && BSP_UCOSII
typedef void (_FUNC_VOID)();
// 状态类型,一般用于位控制
#define	BSP_STATE_RESET			0u				// 清0
#define	BSP_STATE_SET			1u				// 置1
#define	BSP_STATE_OVERTURN		2u				// 翻转
#define	BSP_STATE_CLOSE			0u				// 关
#define	BSP_STATE_OPEN			1u				// 开
/* Private typedef----------------------------------------------------------------------------*/
// 消息ID定义,对不同的消息源进行定义(不要为0)
typedef enum
{
	BSP_MSGID_OUTTIME = 0x00,			//超时	

	BSP_MSGID_UART_RXOVER = 0x10,		// 串口接收完成
	BSP_MSGID_UART_TXOVER = 0x11,		// 串口发送完成
	
	BSP_MSGID_CYRF_RXOVER=0x12,			// CYRF接收完成
	BSP_MSGID_CYRF_TXOVER,				// CYRF发送完成
	BSP_MSGID_CYRF_RX_OVERTIME,			// CYRF接收超时
	BSP_MSGID_CYRF_TX_OVERTIME,			// CYRF发送超时

	BSP_MSGID_KEY = 0x20,				// 按键

	BSP_MSGID_RWCD_CLRDATA = 0x28,		// RWCD,开始清除数据
	BSP_MSGID_RWCD_CLRPARA = 0x29,		// RWCD,开始清除参数

	BSP_CLOCK_ONE_MIN_MSG = 0x35,		//1分钟计时消息

}_BSP_MSGID;

/*******************************************************************************
* 事件消息结构,所有线程(系统)间消息传递时的数据结构。
*
* 线程(系统)间消息传递Convention (如C语言调用汇编函数那样的Convention)
* 1)由OSMboxPend(或OSQPend)线程，管理归零。归零函数OSMboxPost(或OSQPost)线程
*   提供。
* 
* 2)OSMboxPost(或OSQPost)线程为免“用同一个BSP_MESSAGE变量Msg1存储向某个事件发送的
*   不同消息，当前一个消息未处理时，发送后一个消息就会改变前一个消息的值。接收线
*   程处理该事件接收的消息时，会认为连续收到两个相同的消息。”的情况，OSMboxPost
*   (或OSQPost)线程可以设立Msg1、Msg2，BSP_MESSAGE变量。归零时Msgx.DataLen统一设
*   为0xFFFF，发送消息时可以通过Msgx.DataLen判断是否已使用。所有Msgx已使用则不发
*   送消息，等待归零。
* 
*  以上Convention可以保证线程(系统)间可靠、安全、简便的传递消息。
********************************************************************************/
typedef struct
{
    INT8U MsgID;		// 消息ID,原本想用_BSP_MSGID定义所有消息ID。但考虑到目前只是在各个功能驱动开发阶段暂时不统一管理
    INT8U DivNum;		// 设备号
    INT32U DataLen;		// 数据长度, 操作注意见以上说明
    INT8U *pData;		// 数据指针, 操作注意见以上说明
}_BSP_MESSAGE;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
// 发送事件消息
#if	BSP_OS
void BSP_OSPost(_OS_EVENT *pEvent,_BSP_MESSAGE *pMsg);
#endif	//BSP_OS
#define	SYSPost(e,m)			BSP_OSPost(e,m)	// 发送事件函数重定义
#define	DIS_INT				OS_ENTER_CRITICAL	// 禁止中断
#define	EN_INT				OS_EXIT_CRITICAL	// 使能中断
#define	DIS_SCHED			OSSchedLock		// 禁止调度
#define	EN_SCHED			OSSchedUnlock	// 使能调度
#define	SYSTimeDly(t)			OSTimeDly(t)	// 系统延时(单位:系统tick)
void BSPDelay(INT32U delay);

#endif	//__BSP_CONF_H_
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
