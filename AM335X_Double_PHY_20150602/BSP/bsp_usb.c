/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_usb.c
* Author			:
* Date First Issued	: 130722
* Version			: V
* Description		: USB的驱动文件，主要负责USBLib与AM335X的衔接，比如硬件的初始化，与OS的结合等
*                     这里的USB以USB0一路为主，本驱动也暂时只做一路使用:HOST\OTG
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013		    : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "include.h"
#include "interrupt.h"
#include "bsp.h"
#include "soc_AM335x.h"
#include "evmAM335x.h"
#include "gpio.h"
#include "gpio_v2.h"
#include "bsp_int.h"
#include "usb.h"
#include "am335x_irq.h"
#include "cppi41dma.h"
//要調用的USBLib中的接口函数
#include "usblib.h"
#include "usbhost.h"//host irq处理函数
#include "usbhmsc.h"//host MSC处理

/* Private define-----------------------------------------------------------------------------*/
#define USBMSC_DRIVE_RETRYNUM     4   //重连次数
#define USB_TIMEOUT_MILLISECS  3000

/* Private typedef----------------------------------------------------------------------------*/
typedef struct
{
	_BSPUSB_STATE State;				// USB状态
    _BSPUSB_STATE LastState;            // USB上一次状态备份
    INT32U  USBRetryNum;                // USB重连次数
    _BSP_MESSAGE RxMessage;				// 保存接收结束消息内容

}_BSPUSB_CONTROL;

/* Private macro------------------------------------------------------------------------------*/

tUSBHTimeOut *USBHTimeOut = NULL;//给应用的Timeout相关的指针
volatile INT32U USBTicks = 0;
// 声明USB Events驱动接口.
DECLARE_EVENT_DRIVER(g_sUSBEventDriver, 0, 0, USBHCDEvents);
//MSC设备驱动APP接口调用
static tUSBHostClassDriver const * const g_ppHostClassDrivers[] =
{
    &g_USBHostMSCClassDriver,
    &g_sUSBEventDriver
};

#ifdef DMA_MODE
//USB使用专门的CPP
endpointInfo epInfo[]=
{
    {
        USB_EP_TO_INDEX(USB_EP_1),
        CPDMA_DIR_RX,
        CPDMA_MODE_SET_TRANSPARENT,
    },
    {
        USB_EP_TO_INDEX(USB_EP_1),
        CPDMA_DIR_TX,
        CPDMA_MODE_SET_TRANSPARENT,
    },
    {
        USB_EP_TO_INDEX(USB_EP_2),
        CPDMA_DIR_RX,
        CPDMA_MODE_SET_TRANSPARENT,
    },
    {
        USB_EP_TO_INDEX(USB_EP_2),
        CPDMA_DIR_TX,
        CPDMA_MODE_SET_TRANSPARENT,
    }

};
#define NUMBER_OF_ENDPOINTS 4

#endif
//*****************************************************************************
//
// This global holds the number of class drivers in the g_ppHostClassDrivers
// list.
//*****************************************************************************
//
#define NUM_CLASS_DRIVERS       (sizeof(g_ppHostClassDrivers) /  \
                                 sizeof(g_ppHostClassDrivers[0]))
/* Private variables--------------------------------------------------------------------------*/
static INT8U UsedPortNum = 0; // USB当前号USB0:0 USB1:1暂时找不到一个合适的控制来保存当前使用的USB号
                              // 这样做的坏处是这个驱动只能一路USB使用。到时候看看ulInstance与端口号是否存在联系
//IAR对齐方式
//#ifdef __IAR_SYSTEMS_ICC__
//#pragma data_alignment=32
static _BSPUSB_CONFIG  USB_Config[2];   //USB的配置信息保存
static _BSPUSB_CONTROL USB_Control[2];  //USB控制体
//#endif //__IAR_SYSTEMS_ICC__
// The size of the host controller's memory pool in bytes.
#define HCD_MEMORY_SIZE         128
// The memory pool to provide to the Host controller driver.
unsigned char g_pHCDPool[HCD_MEMORY_SIZE];
INT32U g_ulMSCInstance = 0; //
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: USBPost
* Description	: USB  BSP层给application层发送消息
* Input			:
* Return		:
* Note(s)		:
* Contributor	:130722   wangyao
***********************************************************************************************/
static void USBPost(INT8U num , _BSPUSB_STATE State)
{
	USB_Control[num].RxMessage.MsgID = State;
	USB_Control[num].RxMessage.DivNum = num;
	SYSPost(USB_Config[num].pEvent,&USB_Control[num].RxMessage);
}
/***********************************************************************************************
* Function		: BSP_USB_IRQInit
* Description	: USB的中断初始化
* Input			:
* Return		:
* Note(s)		:
* Contributor	:130722   wangyao
***********************************************************************************************/
void BSP_USB_IRQInit(void)
{
    //中断向量表，这里的中断处理函数直接调用USBLib中的枚举过程函数usbhostenum.c
    BSP_IntVectReg(USB0_INT_NUM, USB0HostIntHandler);
    //优先级设置
    IntPrioritySet(USB0_INT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ);
    //使能中断for AINTC
    IntSystemEnable(USB0_INT_NUM);
#ifdef DMA_MODE
    //中断向量表，这里也用同一个中断处理，处理函数中已经分DMA_MODE
    BSP_IntVectReg(USB_SSINT_NUM, USB0HostIntHandler);
    //优先级设置
    IntPrioritySet(USB_SSINT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ);
    //使能中断for AINTC
    IntSystemEnable(USB_SSINT_NUM);
#endif //DMA_MODE
}
/***********************************************************************************************
* Function		: MSCCallback
* Description	: MSC设备的操作回调函数
* Input			: param ulInstance is the driver instance which is needed when communicating
                  with the driver.
                  param ulEvent is one of the events defined by the driver.
                  param pvData is a pointer to data passed into the initial call to register
                  the callback.
* Return		:
* Note(s)		: 大设备删除，枚举时的回调函数，MSC设备拔掉后的回调函数
* Contributor	: 130722   wangyao
***********************************************************************************************/
void MSCCallback(unsigned int ulInstance, unsigned int ulEvent, void *pvData)
{

    // Determine the event.
    switch(ulEvent)
    {
        // Called when the device driver has successfully enumerated an MSC device.
        case MSC_EVENT_OPEN:
        {
            // Proceed to the enumeration state.
            USB_Control[UsedPortNum].State = STATE_DEVICE_ENUM;
            break;
        }
        // Called when the device driver has been unloaded due to error or
        // the device is no longer present.
        case MSC_EVENT_CLOSE:
        {
            // Go back to the "no device" state and wait for a new connection.
            USB_Control[UsedPortNum].State = STATE_NO_DEVICE;
            break;
        }
        default:
            break;
    }
}
/***********************************************************************************************
* Function		: USBHCDEvents
* Description	: 该函数被宏定义DECLARE_EVENT_DRIVER调用，当MSC设备插拔，连接等动作发生调用此函数
* Input			: pvData is actually a pointer to a tEventInfo structure.
* Return		:
* Note(s)		: This function is required when the g_USBGenericEventDriver is included in
                  the host controller driver array that is passed in to the
                  USBHCDRegisterDrivers() function.
* Contributor	: 130722   wangyao
***********************************************************************************************/
tEventInfo *g_pEventInfo;
void USBHCDEvents(void *pvData)//这里状态先用USB1
{
    // Cast this pointer to its actual type.
    g_pEventInfo = (tEventInfo *)pvData;
    switch(g_pEventInfo->ulEvent)
    {
        // New keyboard detected.
        case USB_EVENT_CONNECTED:
        {
            // An unknown device was detected.
            USB_Control[UsedPortNum].State = STATE_UNKNOWN_DEVICE;
            break;
        }
        // Keyboard has been unplugged.
        case USB_EVENT_DISCONNECTED:
        {
            // Unknown device has been removed.
            USB_Control[UsedPortNum].State = STATE_NO_DEVICE;
            break;
        }
        case USB_EVENT_POWER_FAULT:
        {
            // No power means no device is present.
            USB_Control[UsedPortNum].State = STATE_POWER_FAULT;
            break;
        }
        case USB_EVENT_BABBLE_ERROR:
        {
            // No power means no device is present.
            USB_Control[UsedPortNum].State = STATE_BABBLE_INT;
            break;
        }
        default:
        {
            break;
        }
    }
}
/***********************************************************************************************
* Function		: BSP_USBStateControl
* Description	:
* Input			:
* Return		:
* Note(s)		: 非外部用户调用。
* Contributor	: 130722   wangyao
***********************************************************************************************/
void BSP_USBStateControl(INT8U num)
{
    _BSPUSB_STATE usbstate;

    //USB的例行函数
    USBHCDMain(num, g_ulMSCInstance);
    if(USB_Control[num].State == STATE_DEVICE_ENUM)//正在枚举
    {
        if(USBHMSCDriveReady(g_ulMSCInstance) != 0)//
        {
             OSTimeDlyHMSM(0,0,0,200);//延时100MS,再进行check是不是再一次重连
        }
        else//成功
        {
           USB_Control[num].State = STATE_DEVICE_READY;//枚举成功
        }
    }
    if((USBHTimeOut->Status.slEP0)||
    (USBHTimeOut->Status.slNonEP0))
    {
        USB_Control[UsedPortNum].USBRetryNum--;
    }
    if(!USB_Control[UsedPortNum].USBRetryNum)
    {
        USB_Control[num].State = STATE_TIMEDOUT;//超时
        USB_Control[UsedPortNum].USBRetryNum = USBMSC_DRIVE_RETRYNUM;
        USBHTimeOut->Value.slEP0 = USB_EP0_TIMEOUT_MILLISECS;
        USBHTimeOut->Value.slNonEP0= USB_NONEP0_TIMEOUT_MILLISECS;
        USBHTimeOut->Status.slEP0 = 0;
        USBHTimeOut->Status.slNonEP0= 0;
    }
    usbstate = USB_Control[num].LastState;
    if(USB_Control[num].State != usbstate)
    {
        switch(USB_Control[num].State)
        {
            case STATE_NO_DEVICE:
                if(USB_Control[num].LastState == STATE_UNKNOWN_DEVICE)
                {
                    //nUnknown device disconnected
                    USBPost(num,STATE_UNKNOWN_PULLOUT);
                }
                else//MSC设备拔除
                {
                   //nMass storage device disconnected
                    USBPost(num,STATE_PULLOUT);//设备拔除
                }
                break;
            case STATE_DEVICE_ENUM://正在枚举
                break;
            case STATE_DEVICE_READY:
                //nMass storage device connected.
                //no break;
            case STATE_UNKNOWN_DEVICE:
                //nUnknown device connected.
                //no break;
            case STATE_POWER_FAULT:
                //nPower fault
                //no break;
            case STATE_BABBLE_INT:
                //nBabble fault
                //no break;
            case STATE_TIMEDOUT:
                //nDEVICE TIMEDOUT ...!!! \n IF NOT"
                USBPost(num, USB_Control[num].State);
                break;
            default:
                //nUnhandled fault.\n");
                break;
        }
        USB_Control[num].LastState = USB_Control[num].State;//保存状态
    }

}
/***********************************************************************************************
* Function	    : TaskUSBHCD
* Description	:
* Input		    :
* Return	    :
* Note(s)	    :
* Contributor	: 130722   wangyao
***********************************************************************************************/
void TaskUSBHCD(void *pdata)
{
    OSTimeDlyHMSM(0,0,0,500);
    while(1)
    {
        OSTimeDlyHMSM(0,0,0,500);
        BSP_USBStateControl(0);  // USB0监测
    }
}
/***********************************************************************************************
* Function		: BSP_USBInit
* Description	: USB的初始化
* Input			: num--- USB0 : 0  USB1: 1
* Return		:
* Note(s)		:
* Contributor	: 130722   wangyao
***********************************************************************************************/
//这是USBHCDmain任务的堆栈空间的分配以及优先级的配置
#define PRI_USBHCD               38   //暂时定为38，这个不能跟app_cfg.h中的冲突
#define TASK_STK_SIZE_USBHCD    0x100
OS_STK TaskUSBHCDStk[TASK_STK_SIZE_USBHCD];

void BSP_USBInit(INT8U num, _BSPUSB_CONFIG *pusbconfig)
{
    //使能clocking USB controller.
    USB0ModuleClkConfig();
    //USB中断初始化
    BSP_USB_IRQInit();

    if(pusbconfig->Mode == BSPUSB_WORKMODE_HOST)//如果是host模式
    {
        // Register the host class drivers.
        //注册主设备类驱动
        USBHCDRegisterDrivers(num, g_ppHostClassDrivers, NUM_CLASS_DRIVERS);
        // Open an instance of the mass storage class driver.
        //打开一个大容量存储设备
        g_ulMSCInstance = USBHMSCDriveOpen(num, 0, MSCCallback);
        // Initialize the power configuration.  This sets the power enable signal
        // to be active high and does not enable the power fault.
        //初始化电源配置
        USBHCDPowerConfigInit(num, USBHCD_VBUS_AUTO_HIGH);
#ifdef DMA_MODE
        Cppi41DmaInit(num, epInfo, NUMBER_OF_ENDPOINTS);
#endif
        //Initialize the host controller.
        //初始化主设备操作的USB控制器
        USBHCDInit(num, g_pHCDPool, HCD_MEMORY_SIZE);
        SET_CONNECT_RETRY(num, USBMSC_DRIVE_RETRYNUM);
        //超时机制的控制
        USBHCDTimeOutHook(num, &USBHTimeOut);
    }
    //保存初始化参数
    UsedPortNum = num;
    memcpy(&USB_Config[num],pusbconfig,sizeof(_BSPUSB_CONFIG));
    USB_Control[UsedPortNum].State = STATE_NO_DEVICE;
    USB_Control[UsedPortNum].LastState = STATE_NO_DEVICE;
    USB_Control[UsedPortNum].USBRetryNum = USBMSC_DRIVE_RETRYNUM;
    //USB的管理用一个任务在执行，这样能做到MSC的插拔，连接等信息，
    //用任务做的原因是hostMSC USBHCD的处理过程中使用了timer7,并且里面有100MS级别的延时，
    //而systick定了1ms.显然不能用扫描方式自动监控，所以用一个独立于App的任务做，这样也方便后期更多功能的扩张。
    //创建任务,USBHCDmain
    OSTaskCreateExt(TaskUSBHCD,
                    (void *)0,
                    &TaskUSBHCDStk[TASK_STK_SIZE_USBHCD-1],
                    PRI_USBHCD,
                    1,
                    &TaskUSBHCDStk[0],
                    TASK_STK_SIZE_USBHCD,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
}

/***********************************************************************************************
* Function		: BSP_USBCardCountTick
* Description	: 系统调用，每个Tick调用一次。测试U盘时间用
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2010-12-11   王耀
***********************************************************************************************/
void BSP_USBCountTick(void)
{
    USBTicks++;
}

/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/