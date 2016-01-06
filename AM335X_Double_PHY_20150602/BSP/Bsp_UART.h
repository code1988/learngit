/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_uart.h
**��    ��    ��: wangyao
**��  ��  ��  ��: 131211
**��  ��  ��  ��: V0.1
**��          ��: ����֧��
	(0 ~ BSPUART_MAX_NUMBER-1)�Ĵ��ں�.
	ʹ�÷���:
	1. ��ʼ������,�ȵ���BSP_UARTInit()��ʼ������,Ȼ����һ��_BSPUART_CONFIG���͵Ľṹ����,���øñ���,
Ȼ�����BSP_UARTConfig()���ô���.
	2. ��������
		��ѯģʽ��(BSPUART_RESPOND_NORMAL)(δʵ��):�û�ͨ������BSP_UARTWrite()��������,����һֱ�ں����ڵȴ�������ɺ󷵻�
		�ж�ģʽ��(BSPUART_RESPOND_INT):�û�ͨ������BSP_UARTWrite()��������,����Ҫ���͵����ݸ��Ƶ�����buf�󷵻�,
			����ʼ���ݴ���.�û��ȴ����������Ϣ�¼�(BSP_MSGID_UART_TXOVER)
		DMAģʽ��(BSPUART_RESPOND_DMA):�û�ͨ������BSP_UARTWrite()��������,����Ҫ���͵����ݸ��Ƶ�����buf�󷵻�,
			����ʼ���ݴ���.�û��ȴ������������(BSP_MSGID_UART_TXOVER)
	3. ��������
		��ѯģʽ��(BSPUART_RESPOND_NORMAL)(δʵ��):�û�����BSP_UARTRead()����ȴ���������,�����յ�ָ���������ݺ󷵻�
		�ж�ģʽ��(BSPUART_RESPOND_INT):����ʱ,�û��������ý��ճ�ʱʱ��(RxOverTime)��Ŀ������ֽ�(RxOverLen)����,
			�ȴ����������Ϣ�¼�(BSP_MSGID_UART_RXOVER),�յ���Ϣ��,���Ը�����Ϣ���ݶ�ȡ��������,Ȼ��������BSP_UARTRxClear()����,
			����ϴ�����,׼��������֡.���߿���ֱ��ͨ��BSP_UARTRead()������ȡ���ݵ��û�����.
		DMAģʽ��(BSPUART_RESPOND_DMA):ͬ�ж�ģʽ
	���ж�ģʽ��,��Ҫ���ý��ճ�ʱʱ��(RxOverTime��Ϊ0,��λ��ϵͳtickʱ��),���ʱ�䲻��̫��ȷ,
����1����λ��ƫ��,������ô���2.�������յ�һ���������ݺ�,��ʼ��ʱ,ÿ�յ������ݶ����¿�ʼ��ʱ,����ʱʱ��
����(RxOverTime)��û��������,����Ϊ���ճ�ʱ,���ͽ��ճ�ʱ����(BSPUART_EVENT_RX_OVERTIME).
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: wangyao
**��          ��:
**�� �� ǰ �� ��:
**��          ��:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __BSP_UART_H_
#define __BSP_UART_H_


#include "def_config.h"


/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
// ��������
#define	BSPUART_MAX_NUMBER		6u				// ������(0-5��)
#define UART0					0
#define UART1					1
#define UART2					2
#define UART3					3
#define UART4					4
#define UART5					5

#define	BSP_MSGID_UART_RXOVER	0x10			// ���ڽ������
#define	BSP_MSGID_UART_TXOVER	0x11			// ���ڷ������
#define	BSP_MSGID_UART_RXERR	0x12			// ���ڽ��մ���
#define	BSP_MSGID_UART_TXERR	0x13			// ���ڷ��ʹ���

/* Private typedef----------------------------------------------------------------------------*/
// ��������,��Ҫ�޸����¶���
// У��λ
typedef enum
{
	BSPUART_PARITY_NO=0,						// ��У��(Ĭ��)
	BSPUART_PARITY_ODD,							// ��У��
	BSPUART_PARITY_EVEN,						// żУ��
}_BSPUART_PARITY;
// ֹͣλ
typedef enum
{
	BSPUART_STOPBITS_1=0,						// 1��ֹͣλ(Ĭ��)
	BSPUART_STOPBITS_1_5,						// 1.5��ֹͣλ(ԭ������ļ���֧��)
	BSPUART_STOPBITS_2,							// 2��ֹͣλ
}_BSPUART_STOPBITS;
// ����λ��,ע��,��������У��λ���,��Ȼ���������Ľ��
// ����,���У����BSPUART_PARITY_ODD,����λ��BSPUART_WORDLENGTH_8D,�������7λ������У��
typedef enum
{
	BSPUART_WORDLENGTH_5D=0,					// 5λ����
	BSPUART_WORDLENGTH_6D,						// 6λ����
	BSPUART_WORDLENGTH_7D,						// 7λ����
    BSPUART_WORDLENGTH_8D                       // 8λ����(Ĭ��)
}_BSPUART_WORDLENGTH;
// ���ڹ���ģʽ����
typedef enum
{
	BSPUART_WORK_FULLDUPLEX=0u,					// Ĭ��,ȫ˫��
	BSPUART_WORK_HALFDUPLEX,					// ��˫��
	BSPUART_WORK_IRDA,							// ����
	BSPUART_WORK_IRDA_LOWPOWER,					// ����͹���
}_BSPUART_WORK;
// ������Ӧģʽ����
typedef enum
{
	BSPUART_RESPOND_NORMAL=0u,					// ��ѯ��ʽ(Ĭ��)
	BSPUART_RESPOND_INT,						// �жϷ�ʽ(���ͽ��ն����жϷ�ʽ)
	BSPUART_RESPOND_DMA,						// DMA��ʽ(����DMA��ʽ,�������жϷ�ʽ)
}_BSPUART_RESPOND;
// ���ڳ�ʼ������
typedef struct
{
	_OS_EVENT *pEvent;							// �¼�ָ��,����������Ϣ�¼�
	INT32U Baudrate;							// ������
	_BSPUART_PARITY Parity;						// У��
	_BSPUART_STOPBITS StopBits;					// ֹͣλ
	_BSPUART_WORDLENGTH WordLength;				// ����λ��
	_BSPUART_WORK Work;							// ����ģʽ
	_BSPUART_RESPOND TxRespond;					// ��Ӧģʽ(��ѯ,INT,DMA)
	INT16U MaxTxBuffer;							// ����ͻ���
	INT16U MaxRxBuffer;							// �����ջ���
	INT8U *pTxBuffer;							// ���ͻ����׵�ַ
	INT8U *pRxBuffer;							// ���ջ����׵�ַ
	INT16U TxSpacetime;							// ���ʱ��,����֡��С���
	INT16U RxOvertime;							// ��ʱʱ��,�����ʱ�����ղ���������,˵��һ֡���
}_BSPUART_CONFIG;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function Name	: BSP_UARTTxClear,BSP_UARTRxClear
* Description	: UART���
* Input			: num:�˿ں�
* Return		:
* Note(s)		: ���UART����,׼����ʼ�����շ�����
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_UARTTxClear(INT8U num);
void BSP_UARTRxClear(INT8U num);
/***********************************************************************************************
* Function Name	: BSP_UARTOverTime
* Description	: ��ʱ�ж�,ϵͳ��ʱ������,�û�������
* Input			:
* Return		:
* Note(s)		:
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_UARTOverTime(void);
/***********************************************************************************************
* Function Name	: BSP_UARTWrite
* Description	: UART����д
* Input			: num:�˿ں�
				  *pData:����������ָ��
				  len:���ݳ���
* Return		: TRUE/FALSE
* Note(s)		: ���ú�,�������ݵ����ͻ���
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_UARTWrite(INT8U num,const INT8U *pData,INT16U len);
/***********************************************************************************************
* Function Name	: BSP_UARTRead
* Description	: UART���ݶ�
* Input			: num:�˿ں�
				  *pData:���ݶ�ȡ����ָ��
				  len:���ݳ���
* Return		: ʵ�ʶ�ȡ�����ݳ���
* Note(s)		: ���ƴ��ڽ��ջ�������ݵ�pData,Ȼ��׼��������һ֡����,Ҳ����˵����û�ʹ�����������
��ȡ��������,��ô�Ͳ���Ҫ����BSP_UARTRxClear()������.
	����û�ָ�������ݳ���Ϊ0,��ʹ�ý�����Ϣ(RxMessage)�б�������ݳ���
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT16U BSP_UARTRead(INT8U num,INT8U *pData,INT16U len);
/***********************************************************************************************
* Function Name	: BSPUARTInit
* Description	: ���ڳ�ʼ������
* Input			: num:���ں�0 ~ BSPUART_MAX_NUMBER-1
				  *pConfig:���ò���(_BSPUART_CONFIG)
					pConfig->pEvent:�¼�ָ��,����������Ϣ�¼�
					pConfig->Baudrate:������,(���1MHz)
					pConfig->Parity:У��λ
						BSPUART_PARITY_NO:��У��(Ĭ��)
						BSPUART_PARITY_ODD:��У��
						BSPUART_PARITY_EVEN:ż����
					pConfig->StopBits:ֹͣλ
						BSPUART_STOPBITS_1:1λ(Ĭ��)
						BSPUART_STOPBITS_0_5:0.5λ(STR9��֧��)
						BSPUART_STOPBITS_1_5:1.5λ(STR9��֧��)
						BSPUART_STOPBITS_2:2λ
					pConfig->WordLength:����λ
						BSPUART_WORDLENGTH_8D:8λ����(Ĭ��)
						BSPUART_WORDLENGTH_7DP:7λ����+У��λ
						BSPUART_WORDLENGTH_8DP:8λ����+У��λ
					pConfig->Work:����ģʽ
						BSPUART_WORK_FULLDUPLEX:ȫ˫��(Ĭ��)
						BSPUART_WORK_HALFDUPLEX:��˫��(δʵ��)
						BSPUART_WORK_IRDA:����(δʵ��)
						BSPUART_WORK_IRDA_LOWPOWER:����͹���(δʵ��)
					pConfig->TxRespond:������Ӧ��ʽ
						BSPUART_RESPOND_NORMAL:��ѯ��ʽ(Ĭ��)(δʵ��)
						BSPUART_RESPOND_INT:�жϷ�ʽ
						BSPUART_RESPOND_DMA:DMA��ʽ(ֻ�з�����DMA,���ջ����жϷ�ʽ)
					pConfig->MaxTxBuffer,����ͻ���
					pConfig->MaxRxBuffer,�����ջ���
					pConfig->TxBuffer,���ͻ���ͷָ��
					pConfig->RxBuffer,���ջ���ͷָ��
					pConfig->RxOverLen,֪ͨ���ո���,���������ٸ�����,����֪ͨ.Ϊ0ʱ,ֻ���յ�һ֡���ݺ�֪ͨ
					pConfig->TxSpacetime,���ʱ��,����֡��С���
					pConfig->RxOvertime,��ʱʱ��,�����ʱ�����ղ���������,˵��һ֡���
* Return		: TRUE/FALSE
* Note(s)		:
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_UARTConfig(INT8U num,_BSPUART_CONFIG *pConfig);
/***********************************************************************************************
* Function Name	: BSP_UARTIRQHandler
* Description	: UART�жϷ������
* Input			: num:�˿ں�
* Return		:
* Note(s)		:
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_UARTIRQHandler(INT8U num);

/***********************************************************************************************
* Function		: BSP_UARTCfgBaud
* Description	:  configure baudrate
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2014-12-3 by Wang WenGang
***********************************************************************************************/
void BSP_UARTCfgBaud(INT8U num, INT32U baud);

INT8U BSP_UARTWrite_two(INT8U num,const INT8U *pData,INT16U len);


extern void BSP_UARTIRQHandler_two(INT8U num);
extern void uart_tx_int_enable(uint8 num);
extern void uart_tx_int_disable(uint8 num);





#endif	//__BSP_UART_H_
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/
