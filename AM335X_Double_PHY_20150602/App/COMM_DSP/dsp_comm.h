/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: dsp_comm.h
* Author			:
* Date First Issued	: 141030
* Version			: V
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013	        : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __DSP_COMM_H_
#define __DSP_COMM_H_

#include "bsp.h"
#include "def_config.h"

/* Private macro------------------------------------------------------------------------------*/
#define FRAME_HEAD_OFFSET           0
#define FRAME_ATTRIBUTE_OFFSET      2
#define FRAME_FEATCODE_OFFSET       3
#define FRAME_INDEX_OFFSET          4
#define FRAME_VERSION_OFFSET        6
#define FRAME_ACK_OFFSET            7
#define FRAME_DATALEN_OFFSET        8
#define FRAME_RESERVE_OFFSET        12
#define FRAME_HEADSUM_OFFSET        16
#define FRAME_CMD_ID_OFFSET         17
#define FRAME_DATA_OFFSET           19


#define UPDATEHEADLEN       17                                  // 升级命令的固定头长
#define FRAMEHEADLEN        17                                  // 帧头长
#define ARM_RD_DSP          0x81                                // arm 读 dsp
#define ARM_WR_DSP          0x82                                // arm 写 dsp
#define REQ_ACK             0xFF                                // 要求接收方应答
#define NREQ_ACK            0xBB                                // 不要求接收方应答


#define FRAME_FIX_INFO(ptr)     *(INT16U *)(ptr+FRAME_HEAD_OFFSET)=0x02A0;\
                                *(ptr+FRAME_FEATCODE_OFFSET)=0x03;\
                                *(ptr+FRAME_VERSION_OFFSET)=0x01;\
                                *(INT32U *)(ptr+FRAME_RESERVE_OFFSET)=0x00000000

// 纸币信息
#define MONEY_INFO          17

// DSP版本信息
#define VERION_INFO         18

// DSP设备号
#define DSP1    W5500A
#define DSP2    W5500B

// 程序升级对象
#define UPDATE_FOR_DSP      0x01
#define UPDATE_FOR_FPGA     0x02
#define UPDATE_FOR_CNY      0x03

#define MAIN_ID_MASK        0x3F                // 主ID
#define SUB_ID_MASK         0xC0                // 子ID                                  
#define SUB_ID_UPDATE       0x00 // 升级命令
#define SUB_ID_ADJ          0x40 // 校验命令
#define SUB_ID_VER          0x80 // 读版本命令
#define SUB_ID_BIG_IMAGE    0xC0 // 大图查看
                                  
// 结构体字节对齐
#pragma pack(1)
// 升级命令固定头
typedef struct
{
    INT8U   Targt;      // 升级目标
    INT32U  TotlSeg;    // 总段数
    INT32U  TotlLen;    // 总长
    INT32U  CurSeg;     // 当前段序号
    INT32U  CurLen;     // 当前段长度
}_UpdtFixFmt;
#pragma pack()

typedef enum
{
    _ABANDON,           // 丢弃(不做应答)
    _NO_ERR=0x00,       // 正确
    _SUM_CHK_ERR=0x01,  // 整帧校验和错误
    _TAIL_ERR=0x02,     // 帧尾错误
    _ID_ERR=03,         // ID错误
    _ID_DATA_ERR=0x04,  // ID_DATA内容错误
    _FCODE_ERR=0x05,    // 功能码错误
    _OP_ID_ERR=0x06     // ID对应的操作错误(对应的ID不支持读/写)
}_err_no;               // 应答标志

typedef enum
{
    _ASWR_FRAME,        // 应答帧
    _NOR_FRAME_ACK,     // 普通帧要求应答
    _NOR_FRAM_NACK      // 普通帧不要求应答
}_Frame_Cat;            // 帧类别

#if DBG_DSP_RECV_TWO_EN
extern OS_EVENT *W5500AEvent;          // W5500A事件控制块
extern void *W5500OAOSQ[4];             // W5500A消息队列
#endif

extern OS_EVENT *W5500BEvent;          // W5500B事件控制块
extern void *W5500OBOSQ[4];             // W5500B消息队列


INT8U Deal_DSPData(INT8U DevNum,_BSP_MESSAGE *pMsg);
_Frame_Cat Judge_Ack(INT8U *pFrame);
#if FZ1500
void RecDSP_UpdtRT(INT8U *pFrame);
void SndDSP_UpdtCMD(INT8U *pFrame,_BSP_MESSAGE *UpdtMsg);
void SndDSP_VersionGet(INT8U *pFrame,_BSP_MESSAGE *UpdtMsg);
void SndDSP_CheckCSCMD(INT8U *pFrame,_BSP_MESSAGE *VerMsg);
void SndDSP_BIGIMAGECMD(INT8U *pFrame,_BSP_MESSAGE *VerMsg);
#endif

extern int send_dsp1_data(uint8 *buf,uint32 len);
extern int send_dsp2_data(uint8 *buf,uint32 len);
extern void TaskComWithDSP(void *pdata);

//test
//int TEST_SET_ID_DSP_BASIC_INFO(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );


#endif
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/























