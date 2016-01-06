/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: bsp_usb.h
* Author			: 
* Date First Issued	: 130722   
* Version			: V
* Description		: USB�������ļ�����Ҫ����USBLib��AM335X���νӣ�����Ӳ���ĳ�ʼ������OS�Ľ�ϵ�
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
#define USB0_INT_NUM    SYS_INT_USB0       //USB0�жϼĴ������¶���
/* Private typedef----------------------------------------------------------------------------*/
//USB״̬����
typedef enum
{
    STATE_NO_DEVICE,      //���豸
    STATE_DEVICE_ENUM,    //MSC�豸ö��
    STATE_DEVICE_READY,   //MSC�豸׼������
    STATE_UNKNOWN_DEVICE, //�޷�ʶ����豸����
    STATE_POWER_FAULT,    //��Դ����
    STATE_BABBLE_INT,     //�쳣�жϷ���
    STATE_TIMEDOUT,       //�豸��ʱ
    STATE_PULLOUT,        //�豸�γ�
    STATE_UNKNOWN_PULLOUT //����ʶ���豸�γ�
}_BSPUSB_STATE;
//USB����ģʽ����
typedef enum
{
	BSPUSB_WORKMODE_HOST=0,					// host��ģʽ
	BSPUSB_WORKMODE_DEVICE,					// device�豸ģʽ
    BSPUSB_WORKMODE_OTG                     // OTGģʽ
}_BSPUSB_WORKMODE;
// ���ò���
typedef struct
{
	OS_EVENT *pEvent;							// �¼�ָ��,����������Ϣ�¼�
	_BSPUSB_WORKMODE Mode;						// ����ģʽ
    //Ĭ�ϲ���DMAģʽ��������ʱ����DMA,�ж�ģʽ���ù���
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
* Note(s)		: ���ⲿ�û����á�
* Contributor	: 130722   wangyao
***********************************************************************************************/
void BSP_USBStateControl(INT8U num);
/***********************************************************************************************
* Function		: BSP_USBInit
* Description	: USB�ĳ�ʼ��
* Input			: num--- USB0 : 0  USB1: 1
* Return		: 
* Note(s)		: 
* Contributor	: 130722   wangyao
***********************************************************************************************/
void BSP_USBInit(INT8U num, _BSPUSB_CONFIG *pusbconfig);

#endif	//__BSP_USB_H_
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/
