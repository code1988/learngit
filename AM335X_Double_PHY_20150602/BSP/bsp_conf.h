/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName		: bsp_conf.h
* Author		: ��ҫ
* Date First Issued	: 2013-07-22
* Version		: V
* Description		: bsp��ȫ�������ļ�,��Ҫ������һЩ���õĽṹ����
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History		:
* //2010		: V
* Description		: 20141112  _BSP_MESSAGE�ṹ����datalen������ΪINT32U
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_CONF_H_
#define	__BSP_CONF_H_
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
//Ӳ����Χ����������
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
// ʹ��OS
#define	BSP_OS					1
#define	BSP_UCOSII				1
// OS���Ͷ���
#if	(BSP_OS && BSP_UCOSII)
typedef	OS_EVENT				_OS_EVENT;
#else
typedef	INT8U					_OS_EVENT;
#endif	//BSP_OS && BSP_UCOSII
typedef void (_FUNC_VOID)();
// ״̬����,һ������λ����
#define	BSP_STATE_RESET			0u				// ��0
#define	BSP_STATE_SET			1u				// ��1
#define	BSP_STATE_OVERTURN		2u				// ��ת
#define	BSP_STATE_CLOSE			0u				// ��
#define	BSP_STATE_OPEN			1u				// ��
/* Private typedef----------------------------------------------------------------------------*/
// ��ϢID����,�Բ�ͬ����ϢԴ���ж���(��ҪΪ0)
typedef enum
{
	BSP_MSGID_OUTTIME = 0x00,			//��ʱ	

	BSP_MSGID_UART_RXOVER = 0x10,		// ���ڽ������
	BSP_MSGID_UART_TXOVER = 0x11,		// ���ڷ������
	
	BSP_MSGID_CYRF_RXOVER=0x12,			// CYRF�������
	BSP_MSGID_CYRF_TXOVER,				// CYRF�������
	BSP_MSGID_CYRF_RX_OVERTIME,			// CYRF���ճ�ʱ
	BSP_MSGID_CYRF_TX_OVERTIME,			// CYRF���ͳ�ʱ

	BSP_MSGID_KEY = 0x20,				// ����

	BSP_MSGID_RWCD_CLRDATA = 0x28,		// RWCD,��ʼ�������
	BSP_MSGID_RWCD_CLRPARA = 0x29,		// RWCD,��ʼ�������

	BSP_CLOCK_ONE_MIN_MSG = 0x35,		//1���Ӽ�ʱ��Ϣ

}_BSP_MSGID;

/*******************************************************************************
* �¼���Ϣ�ṹ,�����߳�(ϵͳ)����Ϣ����ʱ�����ݽṹ��
*
* �߳�(ϵͳ)����Ϣ����Convention (��C���Ե��û�ຯ��������Convention)
* 1)��OSMboxPend(��OSQPend)�̣߳�������㡣���㺯��OSMboxPost(��OSQPost)�߳�
*   �ṩ��
* 
* 2)OSMboxPost(��OSQPost)�߳�Ϊ�⡰��ͬһ��BSP_MESSAGE����Msg1�洢��ĳ���¼����͵�
*   ��ͬ��Ϣ����ǰһ����Ϣδ����ʱ�����ͺ�һ����Ϣ�ͻ�ı�ǰһ����Ϣ��ֵ��������
*   �̴�����¼����յ���Ϣʱ������Ϊ�����յ�������ͬ����Ϣ�����������OSMboxPost
*   (��OSQPost)�߳̿�������Msg1��Msg2��BSP_MESSAGE����������ʱMsgx.DataLenͳһ��
*   Ϊ0xFFFF��������Ϣʱ����ͨ��Msgx.DataLen�ж��Ƿ���ʹ�á�����Msgx��ʹ���򲻷�
*   ����Ϣ���ȴ����㡣
* 
*  ����Convention���Ա�֤�߳�(ϵͳ)��ɿ�����ȫ�����Ĵ�����Ϣ��
********************************************************************************/
typedef struct
{
    INT8U MsgID;		// ��ϢID,ԭ������_BSP_MSGID����������ϢID�������ǵ�Ŀǰֻ���ڸ����������������׶���ʱ��ͳһ����
    INT8U DivNum;		// �豸��
    INT32U DataLen;		// ���ݳ���, ����ע�������˵��
    INT8U *pData;		// ����ָ��, ����ע�������˵��
}_BSP_MESSAGE;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
// �����¼���Ϣ
#if	BSP_OS
void BSP_OSPost(_OS_EVENT *pEvent,_BSP_MESSAGE *pMsg);
#endif	//BSP_OS
#define	SYSPost(e,m)			BSP_OSPost(e,m)	// �����¼������ض���
#define	DIS_INT				OS_ENTER_CRITICAL	// ��ֹ�ж�
#define	EN_INT				OS_EXIT_CRITICAL	// ʹ���ж�
#define	DIS_SCHED			OSSchedLock		// ��ֹ����
#define	EN_SCHED			OSSchedUnlock	// ʹ�ܵ���
#define	SYSTimeDly(t)			OSTimeDly(t)	// ϵͳ��ʱ(��λ:ϵͳtick)
void BSPDelay(INT32U delay);

#endif	//__BSP_CONF_H_
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/
