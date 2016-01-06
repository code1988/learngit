/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_can.h
**硬          件: am3352
**创    建    人: wangyao
**创  建  日  期: 2013-11-11
**最  新  版  本: V0.2
**描          述: 
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_DCAN_H_
#define	__BSP_DCAN_H_
/* Includes-----------------------------------------------------------------------------------*/
#include	"bsp_conf.h"
#include    "dcan_frame.h"
/* Private define-----------------------------------------------------------------------------*/
//外部用户要调用的帧属性的配置:
//帧类型定义标准数据帧,扩张数据帧
#define BSP_CANDATA_STDFRAME               CAN_DATA_FRAME      // 标准数据帧            
#define BSP_CANDATA_EXIFRAME               CAN_EXT_FRAME       // 标准扩张帧            
//帧的传输方向：发送，接收
#define BSP_CANMSG_DIR_TX                  CAN_MSG_DIR_TX      // 发送帧            
#define BSP_CANMSG_DIR_RX                  CAN_MSG_DIR_RX      // 接收帧           
//扩张帧，标准的的ID号，此处根据用户需求可配置
#define BSP_CANTX_MSG_EXTD_ID                (0x1000u)
#define BSP_CANTX_MSG_STD_ID                 (0x3660u)
//外部用户构建task传输过程用到的标志
#define BSP_MSGID_DCAN_STDRXOVER           0x10 // CAN标准帧接收完成
#define BSP_MSGID_DCAN_EXTRXOVER           0x11 // CAN扩张帧接收完成
#define	BSP_MSGID_DCAN_STDTXOVER	       0x12 // CAN标准帧发送完成
#define	BSP_MSGID_DCAN_EXTTXOVER	       0x13 // CAN扩张帧发送完成
#define	BSP_MSGID_DCAN_RXERR	           0x14 // CAN接收错误
#define	BSP_MSGID_DCAN_TXERR	           0x15 // CAN发送错误
//can的总路数
#define BSPCAN_MAX_NUMBER                  2   //总共2路CAN
/* Private typedef----------------------------------------------------------------------------*/
typedef struct
{
    _OS_EVENT *pEvent;							// 事件指针,用来传递消息事件       
    INT8U num;                                  // can端口号
    INT32U *pTxBuffer;                          // 发送缓存
    INT32U *pRxBuffer;                          // 接收缓存,当有Parity错误时候，此为错误内容
    can_frame entry;                            // 帧的配置
}_BSPDCAN_CONFIG;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function	    : BSP_DCANWrite 
* Description	: CAN发送函数
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   王耀
***********************************************************************************************/
void BSP_DCANWrite(_BSPDCAN_CONFIG canconfig);
/***********************************************************************************************
* Function	    : BSP_DCANParityIRQHandler 
* Description	: CAN的奇偶校验中断处理程序
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   王耀
***********************************************************************************************/
void BSP_DCANParityIRQHandler(INT8U num);
/***********************************************************************************************
* Function	    : BSP_DCANIRQHandler
* Description	: CAN的数据收发中断处理函数
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   王耀
***********************************************************************************************/
void BSP_DCANIRQHandler(INT8U num);
/***********************************************************************************************
* Function	    : BSP_DCANInit 
* Description	: CAN初始化函数，
* Input		    : num: 0:DCAN0 1:DCAN1
* Output	    : 
* Note(s)	    : 
* Contributor	: 2013-12-11   王耀
***********************************************************************************************/
void BSP_DCANInit(INT8U num,_BSPDCAN_CONFIG *pConfig);
#endif	//__BSP_DCAN_H_
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
