/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: bsp_usb.c
* Author			:
* Date First Issued	: 130722
* Version			: V
* Description		: USB�������ļ�����Ҫ����USBLib��AM335X���νӣ�����Ӳ���ĳ�ʼ������OS�Ľ�ϵ�
*                     �����USB��USB0һ·Ϊ����������Ҳ��ʱֻ��һ·ʹ��:HOST\OTG
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
//Ҫ�{�õ�USBLib�еĽӿں���
#include "usblib.h"
#include "usbhost.h"//host irq������
#include "usbhmsc.h"//host MSC����

/* Private define-----------------------------------------------------------------------------*/
#define USBMSC_DRIVE_RETRYNUM     4   //��������
#define USB_TIMEOUT_MILLISECS  3000

/* Private typedef----------------------------------------------------------------------------*/
typedef struct
{
	_BSPUSB_STATE State;				// USB״̬
    _BSPUSB_STATE LastState;            // USB��һ��״̬����
    INT32U  USBRetryNum;                // USB��������
    _BSP_MESSAGE RxMessage;				// ������ս�����Ϣ����

}_BSPUSB_CONTROL;

/* Private macro------------------------------------------------------------------------------*/

tUSBHTimeOut *USBHTimeOut = NULL;//��Ӧ�õ�Timeout��ص�ָ��
volatile INT32U USBTicks = 0;
// ����USB Events�����ӿ�.
DECLARE_EVENT_DRIVER(g_sUSBEventDriver, 0, 0, USBHCDEvents);
//MSC�豸����APP�ӿڵ���
static tUSBHostClassDriver const * const g_ppHostClassDrivers[] =
{
    &g_USBHostMSCClassDriver,
    &g_sUSBEventDriver
};

#ifdef DMA_MODE
//USBʹ��ר�ŵ�CPP
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
static INT8U UsedPortNum = 0; // USB��ǰ��USB0:0 USB1:1��ʱ�Ҳ���һ�����ʵĿ��������浱ǰʹ�õ�USB��
                              // �������Ļ������������ֻ��һ·USBʹ�á���ʱ�򿴿�ulInstance��˿ں��Ƿ������ϵ
//IAR���뷽ʽ
//#ifdef __IAR_SYSTEMS_ICC__
//#pragma data_alignment=32
static _BSPUSB_CONFIG  USB_Config[2];   //USB��������Ϣ����
static _BSPUSB_CONTROL USB_Control[2];  //USB������
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
* Description	: USB  BSP���application�㷢����Ϣ
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
* Description	: USB���жϳ�ʼ��
* Input			:
* Return		:
* Note(s)		:
* Contributor	:130722   wangyao
***********************************************************************************************/
void BSP_USB_IRQInit(void)
{
    //�ж�������������жϴ�����ֱ�ӵ���USBLib�е�ö�ٹ��̺���usbhostenum.c
    BSP_IntVectReg(USB0_INT_NUM, USB0HostIntHandler);
    //���ȼ�����
    IntPrioritySet(USB0_INT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ);
    //ʹ���ж�for AINTC
    IntSystemEnable(USB0_INT_NUM);
#ifdef DMA_MODE
    //�ж�����������Ҳ��ͬһ���жϴ������������Ѿ���DMA_MODE
    BSP_IntVectReg(USB_SSINT_NUM, USB0HostIntHandler);
    //���ȼ�����
    IntPrioritySet(USB_SSINT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ);
    //ʹ���ж�for AINTC
    IntSystemEnable(USB_SSINT_NUM);
#endif //DMA_MODE
}
/***********************************************************************************************
* Function		: MSCCallback
* Description	: MSC�豸�Ĳ����ص�����
* Input			: param ulInstance is the driver instance which is needed when communicating
                  with the driver.
                  param ulEvent is one of the events defined by the driver.
                  param pvData is a pointer to data passed into the initial call to register
                  the callback.
* Return		:
* Note(s)		: ���豸ɾ����ö��ʱ�Ļص�������MSC�豸�ε���Ļص�����
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
* Description	: �ú������궨��DECLARE_EVENT_DRIVER���ã���MSC�豸��Σ����ӵȶ����������ô˺���
* Input			: pvData is actually a pointer to a tEventInfo structure.
* Return		:
* Note(s)		: This function is required when the g_USBGenericEventDriver is included in
                  the host controller driver array that is passed in to the
                  USBHCDRegisterDrivers() function.
* Contributor	: 130722   wangyao
***********************************************************************************************/
tEventInfo *g_pEventInfo;
void USBHCDEvents(void *pvData)//����״̬����USB1
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
* Note(s)		: ���ⲿ�û����á�
* Contributor	: 130722   wangyao
***********************************************************************************************/
void BSP_USBStateControl(INT8U num)
{
    _BSPUSB_STATE usbstate;

    //USB�����к���
    USBHCDMain(num, g_ulMSCInstance);
    if(USB_Control[num].State == STATE_DEVICE_ENUM)//����ö��
    {
        if(USBHMSCDriveReady(g_ulMSCInstance) != 0)//
        {
             OSTimeDlyHMSM(0,0,0,200);//��ʱ100MS,�ٽ���check�ǲ�����һ������
        }
        else//�ɹ�
        {
           USB_Control[num].State = STATE_DEVICE_READY;//ö�ٳɹ�
        }
    }
    if((USBHTimeOut->Status.slEP0)||
    (USBHTimeOut->Status.slNonEP0))
    {
        USB_Control[UsedPortNum].USBRetryNum--;
    }
    if(!USB_Control[UsedPortNum].USBRetryNum)
    {
        USB_Control[num].State = STATE_TIMEDOUT;//��ʱ
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
                else//MSC�豸�γ�
                {
                   //nMass storage device disconnected
                    USBPost(num,STATE_PULLOUT);//�豸�γ�
                }
                break;
            case STATE_DEVICE_ENUM://����ö��
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
        USB_Control[num].LastState = USB_Control[num].State;//����״̬
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
        BSP_USBStateControl(0);  // USB0���
    }
}
/***********************************************************************************************
* Function		: BSP_USBInit
* Description	: USB�ĳ�ʼ��
* Input			: num--- USB0 : 0  USB1: 1
* Return		:
* Note(s)		:
* Contributor	: 130722   wangyao
***********************************************************************************************/
//����USBHCDmain����Ķ�ջ�ռ�ķ����Լ����ȼ�������
#define PRI_USBHCD               38   //��ʱ��Ϊ38��������ܸ�app_cfg.h�еĳ�ͻ
#define TASK_STK_SIZE_USBHCD    0x100
OS_STK TaskUSBHCDStk[TASK_STK_SIZE_USBHCD];

void BSP_USBInit(INT8U num, _BSPUSB_CONFIG *pusbconfig)
{
    //ʹ��clocking USB controller.
    USB0ModuleClkConfig();
    //USB�жϳ�ʼ��
    BSP_USB_IRQInit();

    if(pusbconfig->Mode == BSPUSB_WORKMODE_HOST)//�����hostģʽ
    {
        // Register the host class drivers.
        //ע�����豸������
        USBHCDRegisterDrivers(num, g_ppHostClassDrivers, NUM_CLASS_DRIVERS);
        // Open an instance of the mass storage class driver.
        //��һ���������洢�豸
        g_ulMSCInstance = USBHMSCDriveOpen(num, 0, MSCCallback);
        // Initialize the power configuration.  This sets the power enable signal
        // to be active high and does not enable the power fault.
        //��ʼ����Դ����
        USBHCDPowerConfigInit(num, USBHCD_VBUS_AUTO_HIGH);
#ifdef DMA_MODE
        Cppi41DmaInit(num, epInfo, NUMBER_OF_ENDPOINTS);
#endif
        //Initialize the host controller.
        //��ʼ�����豸������USB������
        USBHCDInit(num, g_pHCDPool, HCD_MEMORY_SIZE);
        SET_CONNECT_RETRY(num, USBMSC_DRIVE_RETRYNUM);
        //��ʱ���ƵĿ���
        USBHCDTimeOutHook(num, &USBHTimeOut);
    }
    //�����ʼ������
    UsedPortNum = num;
    memcpy(&USB_Config[num],pusbconfig,sizeof(_BSPUSB_CONFIG));
    USB_Control[UsedPortNum].State = STATE_NO_DEVICE;
    USB_Control[UsedPortNum].LastState = STATE_NO_DEVICE;
    USB_Control[UsedPortNum].USBRetryNum = USBMSC_DRIVE_RETRYNUM;
    //USB�Ĺ�����һ��������ִ�У�����������MSC�Ĳ�Σ����ӵ���Ϣ��
    //����������ԭ����hostMSC USBHCD�Ĵ��������ʹ����timer7,����������100MS�������ʱ��
    //��systick����1ms.��Ȼ������ɨ�跽ʽ�Զ���أ�������һ��������App��������������Ҳ������ڸ��๦�ܵ����š�
    //��������,USBHCDmain
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
* Description	: ϵͳ���ã�ÿ��Tick����һ�Ρ�����U��ʱ����
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2010-12-11   ��ҫ
***********************************************************************************************/
void BSP_USBCountTick(void)
{
    USBTicks++;
}

/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/