/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_usb.h
* Author			: 
* Date First Issued	: 130722   
* Version			: V
* Description		: USB的驱动文件，主要负责USBLib与AM335X的衔接，比如硬件的初始化，与OS的结合等
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_USB_H_
#define	__BSP_USB_H_
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
#ifdef DMA_MODE
#define USB_SSINT_NUM   SYS_INT_USBSSINT   //USB subsystem register
#endif //DMA_MODE
#define USB0_INT_NUM    SYS_INT_USB0       //USB0中断寄存器重新定义
/* Private typedef----------------------------------------------------------------------------*/
//USB状态定义
typedef enum
{
    STATE_NO_DEVICE,      //无设备
    STATE_DEVICE_ENUM,    //MSC设备枚举
    STATE_DEVICE_READY,   //MSC设备准备就绪
    STATE_UNKNOWN_DEVICE, //无法识别的设备接入
    STATE_POWER_FAULT,    //电源出错
    STATE_BABBLE_INT,     //异常中断发生
    STATE_TIMEDOUT,       //设备超时
    STATE_PULLOUT,        //设备拔除
    STATE_UNKNOWN_PULLOUT //不认识的设备拔除
}_BSPUSB_STATE;
//USB工作模式定义
typedef enum
{
	BSPUSB_WORKMODE_HOST=0,					// host主模式
	BSPUSB_WORKMODE_DEVICE,					// device设备模式
    BSPUSB_WORKMODE_OTG                     // OTG模式
}_BSPUSB_WORKMODE;
// 配置参数
typedef struct
{
	OS_EVENT *pEvent;							// 事件指针,用来传递消息事件
	_BSPUSB_WORKMODE Mode;						// 工作模式
    //默认采用DMA模式，这里暂时不做DMA,中断模式配置功能
}_BSPUSB_CONFIG;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BSP_USBStateControl
* Description	: 
* Input			: 
* Return		: 
* Note(s)		: 非外部用户调用。
* Contributor	: 130722   wangyao
***********************************************************************************************/
void BSP_USBStateControl(INT8U num);
/***********************************************************************************************
* Function		: BSP_USBInit
* Description	: USB的初始化
* Input			: num--- USB0 : 0  USB1: 1
* Return		: 
* Note(s)		: 
* Contributor	: 130722   wangyao
***********************************************************************************************/
void BSP_USBInit(INT8U num, _BSPUSB_CONFIG *pusbconfig);

#endif	//__BSP_USB_H_
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
