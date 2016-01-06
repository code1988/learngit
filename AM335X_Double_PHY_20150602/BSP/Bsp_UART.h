/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_uart.h
**创    建    人: wangyao
**创  建  日  期: 131211
**最  新  版  本: V0.1
**描          述: 串口支持
	(0 ~ BSPUART_MAX_NUMBER-1)的串口号.
	使用方法:
	1. 初始化串口,先调用BSP_UARTInit()初始化串口,然后定义一个_BSPUART_CONFIG类型的结构变量,配置该变量,
然后调用BSP_UARTConfig()配置串口.
	2. 发送数据
		查询模式下(BSPUART_RESPOND_NORMAL)(未实现):用户通过调用BSP_UARTWrite()发送数据,程序一直在函数内等待发送完成后返回
		中断模式下(BSPUART_RESPOND_INT):用户通过调用BSP_UARTWrite()发送数据,程序将要发送的数据复制到发送buf后返回,
			并开始数据传输.用户等待发送完成消息事件(BSP_MSGID_UART_TXOVER)
		DMA模式下(BSPUART_RESPOND_DMA):用户通过调用BSP_UARTWrite()发送数据,程序将要发送的数据复制到发送buf后返回,
			并开始数据传输.用户等待发送完成邮箱(BSP_MSGID_UART_TXOVER)
	3. 接收数据
		查询模式下(BSPUART_RESPOND_NORMAL)(未实现):用户调用BSP_UARTRead()程序等待接收数据,当接收到指定长度数据后返回
		中断模式下(BSPUART_RESPOND_INT):配置时,用户必须设置接收超时时间(RxOverTime)或目标接收字节(RxOverLen)非零,
			等待接收完成消息事件(BSP_MSGID_UART_RXOVER),收到消息后,可以根据消息内容读取处理数据,然后必须调用BSP_UARTRxClear()函数,
			清除上次数据,准备接收下帧.或者可以直接通过BSP_UARTRead()函数读取数据到用户缓冲.
		DMA模式下(BSPUART_RESPOND_DMA):同中断模式
	在中断模式下,需要设置接收超时时间(RxOverTime不为0,单位是系统tick时间),这个时间不会太精确,
至少1个单位的偏差,所以最好大于2.这样在收到一个串口数据后,开始计时,每收到新数据都重新开始计时,当计时时间
到达(RxOverTime)仍没有新数据,则认为接收超时,发送接收超时邮箱(BSPUART_EVENT_RX_OVERTIME).
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: wangyao
**日          期:
**修 改 前 版 本:
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __BSP_UART_H_
#define __BSP_UART_H_


#include "def_config.h"


/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
// 串口配置
#define	BSPUART_MAX_NUMBER		6u				// 串口数(0-5号)
#define UART0					0
#define UART1					1
#define UART2					2
#define UART3					3
#define UART4					4
#define UART5					5

#define	BSP_MSGID_UART_RXOVER	0x10			// 串口接收完成
#define	BSP_MSGID_UART_TXOVER	0x11			// 串口发送完成
#define	BSP_MSGID_UART_RXERR	0x12			// 串口接收错误
#define	BSP_MSGID_UART_TXERR	0x13			// 串口发送错误

/* Private typedef----------------------------------------------------------------------------*/
// 串口设置,不要修改以下定义
// 校验位
typedef enum
{
	BSPUART_PARITY_NO=0,						// 无校验(默认)
	BSPUART_PARITY_ODD,							// 奇校验
	BSPUART_PARITY_EVEN,						// 偶校验
}_BSPUART_PARITY;
// 停止位
typedef enum
{
	BSPUART_STOPBITS_1=0,						// 1个停止位(默认)
	BSPUART_STOPBITS_1_5,						// 1.5个停止位(原库对外文件不支持)
	BSPUART_STOPBITS_2,							// 2个停止位
}_BSPUART_STOPBITS;
// 数据位数,注意,这个必须和校验位配合,不然会出现意外的结果
// 比如,如果校验是BSPUART_PARITY_ODD,数据位是BSPUART_WORDLENGTH_8D,结果会变成7位数据奇校验
typedef enum
{
	BSPUART_WORDLENGTH_5D=0,					// 5位数据
	BSPUART_WORDLENGTH_6D,						// 6位数据
	BSPUART_WORDLENGTH_7D,						// 7位数据
    BSPUART_WORDLENGTH_8D                       // 8位数据(默认)
}_BSPUART_WORDLENGTH;
// 串口工作模式定义
typedef enum
{
	BSPUART_WORK_FULLDUPLEX=0u,					// 默认,全双工
	BSPUART_WORK_HALFDUPLEX,					// 半双工
	BSPUART_WORK_IRDA,							// 红外
	BSPUART_WORK_IRDA_LOWPOWER,					// 红外低功耗
}_BSPUART_WORK;
// 串口响应模式定义
typedef enum
{
	BSPUART_RESPOND_NORMAL=0u,					// 查询方式(默认)
	BSPUART_RESPOND_INT,						// 中断方式(发送接收都是中断方式)
	BSPUART_RESPOND_DMA,						// DMA方式(发送DMA方式,接收是中断方式)
}_BSPUART_RESPOND;
// 串口初始化参数
typedef struct
{
	_OS_EVENT *pEvent;							// 事件指针,用来传递消息事件
	INT32U Baudrate;							// 波特率
	_BSPUART_PARITY Parity;						// 校验
	_BSPUART_STOPBITS StopBits;					// 停止位
	_BSPUART_WORDLENGTH WordLength;				// 数据位数
	_BSPUART_WORK Work;							// 工作模式
	_BSPUART_RESPOND TxRespond;					// 响应模式(查询,INT,DMA)
	INT16U MaxTxBuffer;							// 最大发送缓冲
	INT16U MaxRxBuffer;							// 最大接收缓冲
	INT8U *pTxBuffer;							// 发送缓冲首地址
	INT8U *pRxBuffer;							// 接收缓冲首地址
	INT16U TxSpacetime;							// 间隔时间,发送帧最小间隔
	INT16U RxOvertime;							// 超时时间,在这个时间里收不到新数据,说明一帧完成
}_BSPUART_CONFIG;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function Name	: BSP_UARTTxClear,BSP_UARTRxClear
* Description	: UART清除
* Input			: num:端口号
* Return		:
* Note(s)		: 清除UART数据,准备开始重新收发数据
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_UARTTxClear(INT8U num);
void BSP_UARTRxClear(INT8U num);
/***********************************************************************************************
* Function Name	: BSP_UARTOverTime
* Description	: 超时判断,系统定时器调用,用户不关心
* Input			:
* Return		:
* Note(s)		:
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_UARTOverTime(void);
/***********************************************************************************************
* Function Name	: BSP_UARTWrite
* Description	: UART数据写
* Input			: num:端口号
				  *pData:待发送数据指针
				  len:数据长度
* Return		: TRUE/FALSE
* Note(s)		: 调用后,复制数据到发送缓冲
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_UARTWrite(INT8U num,const INT8U *pData,INT16U len);
/***********************************************************************************************
* Function Name	: BSP_UARTRead
* Description	: UART数据读
* Input			: num:端口号
				  *pData:数据读取缓冲指针
				  len:数据长度
* Return		: 实际读取的数据长度
* Note(s)		: 复制串口接收缓冲的数据到pData,然后准备接收下一帧数据,也就是说如果用户使用了这个函数
读取串口数据,那么就不需要调用BSP_UARTRxClear()函数了.
	如果用户指定的数据长度为0,则使用接收消息(RxMessage)中保存的数据长度
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT16U BSP_UARTRead(INT8U num,INT8U *pData,INT16U len);
/***********************************************************************************************
* Function Name	: BSPUARTInit
* Description	: 串口初始化程序
* Input			: num:串口号0 ~ BSPUART_MAX_NUMBER-1
				  *pConfig:配置参数(_BSPUART_CONFIG)
					pConfig->pEvent:事件指针,用来传递消息事件
					pConfig->Baudrate:波特率,(最大1MHz)
					pConfig->Parity:校验位
						BSPUART_PARITY_NO:无校验(默认)
						BSPUART_PARITY_ODD:奇校验
						BSPUART_PARITY_EVEN:偶检验
					pConfig->StopBits:停止位
						BSPUART_STOPBITS_1:1位(默认)
						BSPUART_STOPBITS_0_5:0.5位(STR9不支持)
						BSPUART_STOPBITS_1_5:1.5位(STR9不支持)
						BSPUART_STOPBITS_2:2位
					pConfig->WordLength:数据位
						BSPUART_WORDLENGTH_8D:8位数据(默认)
						BSPUART_WORDLENGTH_7DP:7位数据+校验位
						BSPUART_WORDLENGTH_8DP:8位数据+校验位
					pConfig->Work:工作模式
						BSPUART_WORK_FULLDUPLEX:全双工(默认)
						BSPUART_WORK_HALFDUPLEX:半双工(未实现)
						BSPUART_WORK_IRDA:红外(未实现)
						BSPUART_WORK_IRDA_LOWPOWER:红外低功耗(未实现)
					pConfig->TxRespond:发送响应方式
						BSPUART_RESPOND_NORMAL:查询方式(默认)(未实现)
						BSPUART_RESPOND_INT:中断方式
						BSPUART_RESPOND_DMA:DMA方式(只有发送是DMA,接收还是中断方式)
					pConfig->MaxTxBuffer,最大发送缓冲
					pConfig->MaxRxBuffer,最大接收缓冲
					pConfig->TxBuffer,发送缓冲头指针
					pConfig->RxBuffer,接收缓冲头指针
					pConfig->RxOverLen,通知接收个数,接收满多少个数据,发送通知.为0时,只在收到一帧数据后通知
					pConfig->TxSpacetime,间隔时间,发送帧最小间隔
					pConfig->RxOvertime,超时时间,在这个时间里收不到新数据,说明一帧完成
* Return		: TRUE/FALSE
* Note(s)		:
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_UARTConfig(INT8U num,_BSPUART_CONFIG *pConfig);
/***********************************************************************************************
* Function Name	: BSP_UARTIRQHandler
* Description	: UART中断服务程序
* Input			: num:端口号
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
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
