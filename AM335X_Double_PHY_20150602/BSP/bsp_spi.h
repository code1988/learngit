/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_spi.h
* Author			: 王耀
* Date First Issued	: 10/29/2013
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
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
// spi配置
//使用到SPI的人，定义NUM调用这里的三个定义，外部用户不能轻易更换此处标号
#define BSPAT_SPINUM         0u  //spi0供AT使用
#define BSPW550A_SPINUM      1u  //SPI0工W550A使用
#define BSPW550B_SPINUM      2u  //SPI1工W550B使用



#define	BSPSPI_MAX_NUMBER		3u				// SPI数(0-1号)
#define	BSP_MSGID_SPI_RXOVER	0x12			// SPI接收完成
#define	BSP_MSGID_SPI_RXERR		0x14			// SPI接收错误
#define BSP_MSGID_SPI_TXOVER	0x15			// SPI发送完成

#ifdef __BSP_SPI_C_
// 引脚控制结构
#endif	//__BSP_SPI_C_
/* Private typedef----------------------------------------------------------------------------*/
// 设置,不要修改以下定义
// 数据首位
typedef enum
{
	BSPSPI_FIRSTBIT_MSB=0,						// 高位在前
	BSPSPI_FIRSTBIT_LSB,						// 低位在前
}_BSPSPI_FIRSTBIT;
// 时钟相位
typedef enum
{
	BSPSPI_CLK_PHASE_1EDGE=0,					// 采样从第一个时钟边沿开始
	BSPSPI_CLK_PHASE_2EDGE,						// 采样从第二个时钟边沿开始
}_BSPSPI_CLK_PHASE;
// 时钟极性
typedef enum
{
	BSPSPI_CLK_POLARITY_LOW=0,					// 空闲时低
	BSPSPI_CLK_POLARITY_HIGH,					// 空闲时高
}_BSPSPI_CLK_POLARITY;
// 工作模式定义
typedef enum
{
	BSPSPI_WORKMODE_MASTER=0,					// 主模式
	BSPSPI_WORKMODE_SLAVE,						// 从模式
}_BSPSPI_WORKMODE;
// 响应模式定义
typedef enum
{
	BSPSPI_RESPOND_NORMAL=0u,					// 查询方式(默认)
	BSPSPI_RESPOND_INT,							// 中断方式(发送接收都是中断方式)
	BSPSPI_RESPOND_DMA,							// DMA方式(发送接收都是DMA中断方式)
}_BSPSPI_RESPOND;

// 状态位
typedef union
{
	INT8U byte;
	struct
	{
		unsigned bEn:1;							// 设备使能
		unsigned bTxBusy:1;						// 发送中
		unsigned bRxBusy:1;						// 接收中
		unsigned bRxOver:1;						// 接收到一帧数据,还没处理
		unsigned bRxErr:1;						// 接收出错
	};
}_BSPSPI_STATE;

//通道
#define	BSPSPI_CHN0 0
#define	BSPSPI_CHN1 1
#define	BSPSPI_CHN2 2
#define	BSPSPI_CHN3 3

// 配置参数
typedef struct
{
	INT8U num;									// SPI端口号 0，1
	OS_EVENT *pEvent;							// 事件指针,用来传递消息事件
	INT32U channel;								// SPI通道(片选号)选择
//	INT32U Baudrate;							// 波特率
//	_BSPSPI_FIRSTBIT FirstBit;					// 数据首位
//	_BSPSPI_CLK_PHASE Phase;					// 时钟相位
//	_BSPSPI_CLK_POLARITY Polarity;				// 时钟极性
	_BSPSPI_WORKMODE Mode;						// 工作模式
	_BSPSPI_RESPOND TxRespond;					// 发送响应模式(查询,INT,DMA)
	INT16U MaxTxBuffer;							// 最大发送缓冲,查询模式无效
	INT16U MaxRxBuffer;							// 最大接收缓冲,查询模式无效
	INT8U *pTxBuffer;							// 发送缓冲,查询模式无效
	INT8U *pRxBuffer;							// 接收缓冲,查询模式无效
	INT16U RxOvertime;							// 接收超时,查询模式无效
}_BSPSPI_CONFIG;

// 控制参数
typedef struct
{
	volatile _BSPSPI_STATE State;				// 串口状态
	
	INT16U TxAddr;								// 当前发送地址(发送过程中递增,等于SendLen说明数据发送完成)
	INT16U TxLen;								// 发送数据总长度(发送过程中不变,结束后清零)
	
	INT16U RxStart;								// 数据起始位置
	INT16U RxLen;								// 数据长度

	INT16U RxOvertime;							// 保存最后一次接收到数据时的时间,如果超过设置的超时时间,则认为超时
	_BSP_MESSAGE RxMessage;						// 保存接收结束消息内容
}_BSPSPI_CONTROL;    



void BSP_SPIIRQHandler(INT8U num);
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function Name	: BSP_SPIOverTime
* Description	: 超时判断,系统定时器调用,用户不关心
* Input			: 
* Return		: 
* Note(s)		: 接收和发送超时做在一起了,如果出现超时则认为出错,清除状态
* Contributor	: 090507	wangyao
***********************************************************************************************/
void BSP_SPIOverTime(void);
/***********************************************************************************************
* Function Name	: BSP_SPIClear
* Description	: SPI清除
* Input			: num:端口号
* Return		: 
* Note(s)		: 清除SPI数据,准备重新开始,中断或者DMA模式下,用户必须在处理完一帧数据后调用此函数
* Contributor	: 131211	wangyao
***********************************************************************************************/
void BSP_SPIClear(INT8U num);
/***********************************************************************************************
* Function Name	: BSP_SPIWrite
* Description	: SPI数据写
* Input			: num:端口号
* Return		: 
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_SPIWrite(INT8U num,INT8U *pData,INT16U len);
/***********************************************************************************************
* Function Name	: BSP_SPIRead
* Description	: SPI数据读
* Input			: num:端口号
* Return		: 
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_SPIRead(INT8U num,INT8U *pData,INT16U len);
/***********************************************************************************************
* Function Name	: BSP_SPIConfig
* Description	: SPI配置
* Input			: num:端口号(0 ~ BSPSPI_MAX_NUMBER-1)
				  *pConfig:配置参数(_BSPSPI_CONFIG)
					pConfig->pEvent,事件指针
					pConfig->Baudrate,波特率,实际波特率可能和设置值不同(最大18MHz)
					pConfig->FirstBit,数据首位(_BSPSPI_FIRSTBIT)
						BSPSPI_FIRSTBIT_LOW,低位在前
						BSPSPI_FIRSTBIT_HIGH,高位在前
					pConfig->Phase,时钟相位(_BSPSPI_CLK_PHASE)
						BSPSPI_CLK_PHASE_1EDGE,采样从第一个时钟边沿开始
						BSPSPI_CLK_PHASE_2EDGE,采样从第二个时钟边沿开始
					pConfig->Polarity,时钟极性(_BSPSPI_CLK_POLARITY)
						BSPSPI_CLK_POLARITY_LOW,空闲时低
						BSPSPI_CLK_POLARITY_HIGH,空闲时高
					pConfig->LineMode,线模式(_BSPSPI_LINEMODE)
						BSPSPI_LINEMODE_FULLDUPLEX,双线全双工
						BSPSPI_LINEMODE_2LINERX,双线只接收
						BSPSPI_LINEMODE_1LINERXTX,单线接收发送,这个是自己加的,初始接收,自动切换方向
						BSPSPI_LINEMODE_1LINERX,单线接收
						BSPSPI_LINEMODE_1LINETX,单线发送
					pConfig->Mode,工作模式(_BSPSPI_WORKMODE)
						BSPSPI_WORKMODE_MASTER,主模式
						BSPSPI_WORKMODE_SLAVE,从模式
					pConfig->TxRespond,发送响应模式(查询,INT,DMA)(_BSPSPI_RESPOND)
						BSPSPI_RESPOND_NORMAL,查询方式(默认)
						BSPSPI_RESPOND_INT,中断方式(发送接收都是中断方式)(未测试)
						BSPSPI_RESPOND_DMA,DMA方式(发送接收都是DMA中断方式)(未测试)
					pConfig->MaxTxBuffer,最大发送缓冲
					pConfig->MaxRxBuffer,最大接收缓冲
					pConfig->TxBuffer,发送缓冲指针
					pConfig->RxBuffer,接收缓冲指针
					pConfig->RecOvertime,接收超时
* Return		: TRUE/FALSE
* Note(s)		: 
* Contributor	: 131211	wangyao
***********************************************************************************************/
INT8U BSP_SPIConfig(_BSPSPI_CONFIG *pConfig);
/***********************************************************************************************
* Function Name	: BSP_SPIInit
* Description	: SPI初始化,使用默认配置
* Input			: num:端口号(0 ~ BSPSPI_MAX_NUMBER-1)
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
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/

