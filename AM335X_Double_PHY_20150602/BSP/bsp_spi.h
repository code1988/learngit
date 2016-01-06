/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: bsp_spi.h
* Author			: ��ҫ
* Date First Issued	: 10/29/2013
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2013		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_SPI_H_
#define	__BSP_SPI_H_
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/

/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
// spi����
//ʹ�õ�SPI���ˣ�����NUM����������������壬�ⲿ�û��������׸����˴����
#define BSPAT_SPINUM         0u  //spi0��ATʹ��
#define BSPW550A_SPINUM      1u  //SPI0��W550Aʹ��
#define BSPW550B_SPINUM      2u  //SPI1��W550Bʹ��



#define	BSPSPI_MAX_NUMBER		3u				// SPI��(0-1��)
#define	BSP_MSGID_SPI_RXOVER	0x12			// SPI�������
#define	BSP_MSGID_SPI_RXERR		0x14			// SPI���մ���
#define BSP_MSGID_SPI_TXOVER	0x15			// SPI�������

#ifdef __BSP_SPI_C_
// ���ſ��ƽṹ
#endif	//__BSP_SPI_C_
/* Private typedef----------------------------------------------------------------------------*/
// ����,��Ҫ�޸����¶���
// ������λ
typedef enum
{
	BSPSPI_FIRSTBIT_MSB=0,						// ��λ��ǰ
	BSPSPI_FIRSTBIT_LSB,						// ��λ��ǰ
}_BSPSPI_FIRSTBIT;
// ʱ����λ
typedef enum
{
	BSPSPI_CLK_PHASE_1EDGE=0,					// �����ӵ�һ��ʱ�ӱ��ؿ�ʼ
	BSPSPI_CLK_PHASE_2EDGE,						// �����ӵڶ���ʱ�ӱ��ؿ�ʼ
}_BSPSPI_CLK_PHASE;
// ʱ�Ӽ���
typedef enum
{
	BSPSPI_CLK_POLARITY_LOW=0,					// ����ʱ��
	BSPSPI_CLK_POLARITY_HIGH,					// ����ʱ��
}_BSPSPI_CLK_POLARITY;
// ����ģʽ����
typedef enum
{
	BSPSPI_WORKMODE_MASTER=0,					// ��ģʽ
	BSPSPI_WORKMODE_SLAVE,						// ��ģʽ
}_BSPSPI_WORKMODE;
// ��Ӧģʽ����
typedef enum
{
	BSPSPI_RESPOND_NORMAL=0u,					// ��ѯ��ʽ(Ĭ��)
	BSPSPI_RESPOND_INT,							// �жϷ�ʽ(���ͽ��ն����жϷ�ʽ)
	BSPSPI_RESPOND_DMA,							// DMA��ʽ(���ͽ��ն���DMA�жϷ�ʽ)
}_BSPSPI_RESPOND;

// ״̬λ
typedef union
{
	INT8U byte;
	struct
	{
		unsigned bEn:1;							// �豸ʹ��
		unsigned bTxBusy:1;						// ������
		unsigned bRxBusy:1;						// ������
		unsigned bRxOver:1;						// ���յ�һ֡����,��û����
		unsigned bRxErr:1;						// ���ճ���
	};
}_BSPSPI_STATE;

//ͨ��
#define	BSPSPI_CHN0 0
#define	BSPSPI_CHN1 1
#define	BSPSPI_CHN2 2
#define	BSPSPI_CHN3 3

// ���ò���
typedef struct
{
	INT8U num;									// SPI�˿ں� 0��1
	OS_EVENT *pEvent;							// �¼�ָ��,����������Ϣ�¼�
	INT32U channel;								// SPIͨ��(Ƭѡ��)ѡ��
//	INT32U Baudrate;							// ������
//	_BSPSPI_FIRSTBIT FirstBit;					// ������λ
//	_BSPSPI_CLK_PHASE Phase;					// ʱ����λ
//	_BSPSPI_CLK_POLARITY Polarity;				// ʱ�Ӽ���
	_BSPSPI_WORKMODE Mode;						// ����ģʽ
	_BSPSPI_RESPOND TxRespond;					// ������Ӧģʽ(��ѯ,INT,DMA)
	INT16U MaxTxBuffer;							// ����ͻ���,��ѯģʽ��Ч
	INT16U MaxRxBuffer;							// �����ջ���,��ѯģʽ��Ч
	INT8U *pTxBuffer;							// ���ͻ���,��ѯģʽ��Ч
	INT8U *pRxBuffer;							// ���ջ���,��ѯģʽ��Ч
	INT16U RxOvertime;							// ���ճ�ʱ,��ѯģʽ��Ч
}_BSPSPI_CONFIG;

// ���Ʋ���
typedef struct
{
	volatile _BSPSPI_STATE State;				// ����״̬
	
	INT16U TxAddr;								// ��ǰ���͵�ַ(���͹����е���,����SendLen˵�����ݷ������)
	INT16U TxLen;								// ���������ܳ���(���͹����в���,����������)
	
	INT16U RxStart;								// ������ʼλ��
	INT16U RxLen;								// ���ݳ���

	INT16U RxOvertime;							// �������һ�ν��յ�����ʱ��ʱ��,����������õĳ�ʱʱ��,����Ϊ��ʱ
	_BSP_MESSAGE RxMessage;						// ������ս�����Ϣ����
}_BSPSPI_CONTROL;    



void BSP_SPIIRQHandler(INT8U num);
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function Name	: BSP_SPIOverTime
* Description	: ��ʱ�ж�,ϵͳ��ʱ������,�û�������
* Input			: 
* Return		: 
* Note(s)		: ���պͷ��ͳ�ʱ����һ����,������ֳ�ʱ����Ϊ����,���״̬
* Contributor	: 090507	wangyao
***********************************************************************************************/
void BSP_SPIOverTime(void);
/***********************************************************************************************
* Function Name	: BSP_SPIClear
* Description	: SPI���
* Input			: num:�˿ں�
* Return		: 
* Note(s)		: ���SPI����,׼�����¿�ʼ,�жϻ���DMAģʽ��,�û������ڴ�����һ֡���ݺ���ô˺���
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_SPIClear(INT8U num);
/***********************************************************************************************
* Function Name	: BSP_SPIWrite
* Description	: SPI����д
* Input			: num:�˿ں�
* Return		: 
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_SPIWrite(INT8U num,INT8U *pData,INT16U len);
/***********************************************************************************************
* Function Name	: BSP_SPIRead
* Description	: SPI���ݶ�
* Input			: num:�˿ں�
* Return		: 
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_SPIRead(INT8U num,INT8U *pData,INT16U len);
/***********************************************************************************************
* Function Name	: BSP_SPIConfig
* Description	: SPI����
* Input			: num:�˿ں�(0 ~ BSPSPI_MAX_NUMBER-1)
				  *pConfig:���ò���(_BSPSPI_CONFIG)
					pConfig->pEvent,�¼�ָ��
					pConfig->Baudrate,������,ʵ�ʲ����ʿ��ܺ�����ֵ��ͬ(���18MHz)
					pConfig->FirstBit,������λ(_BSPSPI_FIRSTBIT)
						BSPSPI_FIRSTBIT_LOW,��λ��ǰ
						BSPSPI_FIRSTBIT_HIGH,��λ��ǰ
					pConfig->Phase,ʱ����λ(_BSPSPI_CLK_PHASE)
						BSPSPI_CLK_PHASE_1EDGE,�����ӵ�һ��ʱ�ӱ��ؿ�ʼ
						BSPSPI_CLK_PHASE_2EDGE,�����ӵڶ���ʱ�ӱ��ؿ�ʼ
					pConfig->Polarity,ʱ�Ӽ���(_BSPSPI_CLK_POLARITY)
						BSPSPI_CLK_POLARITY_LOW,����ʱ��
						BSPSPI_CLK_POLARITY_HIGH,����ʱ��
					pConfig->LineMode,��ģʽ(_BSPSPI_LINEMODE)
						BSPSPI_LINEMODE_FULLDUPLEX,˫��ȫ˫��
						BSPSPI_LINEMODE_2LINERX,˫��ֻ����
						BSPSPI_LINEMODE_1LINERXTX,���߽��շ���,������Լ��ӵ�,��ʼ����,�Զ��л�����
						BSPSPI_LINEMODE_1LINERX,���߽���
						BSPSPI_LINEMODE_1LINETX,���߷���
					pConfig->Mode,����ģʽ(_BSPSPI_WORKMODE)
						BSPSPI_WORKMODE_MASTER,��ģʽ
						BSPSPI_WORKMODE_SLAVE,��ģʽ
					pConfig->TxRespond,������Ӧģʽ(��ѯ,INT,DMA)(_BSPSPI_RESPOND)
						BSPSPI_RESPOND_NORMAL,��ѯ��ʽ(Ĭ��)
						BSPSPI_RESPOND_INT,�жϷ�ʽ(���ͽ��ն����жϷ�ʽ)(δ����)
						BSPSPI_RESPOND_DMA,DMA��ʽ(���ͽ��ն���DMA�жϷ�ʽ)(δ����)
					pConfig->MaxTxBuffer,����ͻ���
					pConfig->MaxRxBuffer,�����ջ���
					pConfig->TxBuffer,���ͻ���ָ��
					pConfig->RxBuffer,���ջ���ָ��
					pConfig->RecOvertime,���ճ�ʱ
* Return		: TRUE/FALSE
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_SPIConfig(_BSPSPI_CONFIG *pConfig);
/***********************************************************************************************
* Function Name	: BSP_SPIInit
* Description	: SPI��ʼ��,ʹ��Ĭ������
* Input			: num:�˿ں�(0 ~ BSPSPI_MAX_NUMBER-1)
* Return		: TRUE/FALSE
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_SPIInit(INT8U num);
void BSP_SPICSEnable(INT8U num);
void BSP_SPICSDisEnable(INT8U num);
void WriteToFlash(void);
void ReadFromFlash(void);

#endif	//__BSP_SPI_H_
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/

