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


// DSP初始化结果
#define DSP_INIT_ERR    0
#define DSP_INIT_SUS    1
#define DSP_NO_APP      2

// DSP程序升级结果
#define DSP_UPDT_ERR        3
#define DSP_UPDT_SUS        4
#define DSP_UPDT_NSPORT     5
#define FPGA_UPDT_ERR       6
#define FPGA_UPDT_SUS       7
#define FPGA_UPDT_NSPORT    8
#define CNY_UPDT_ERR        9
#define CNY_UPDT_SUS        10
#define CNY_UPDT_NSPORT     11

// DSP校正结果
#define ADJUST_ERR          12
#define ADJUST_SUS          13
#define ADJUST_FST_SUS      14
#define ADJUST_SEC_STRT     15
#define ADJUST_SEC_SUS      16

// 纸币信息
#define MONEY_INFO          17

// DSP版本信息
#define VERION_INFO         18

// DSP设备号
#define DSP1    BSPW550A_SPINUM
#define DSP2    BSPW550B_SPINUM

// CPSW PORT NUM
#define CPSW_PORT0  0
#define CPSW_PORT1  1
// 程序升级对象
#define UPDATE_FOR_DSP      0x01
#define UPDATE_FOR_FPGA     0x02
#define UPDATE_FOR_CNY      0x03

#define SUB_ID_UPDATE       0x00 // 升级命令
#define SUB_ID_BKLST        0x01 // 黑名单命令
#define SUB_ID_ADJ          0x40 // 校验命令
#define SUB_ID_VER          0x80 // 读版本命令
#define SUB_ID_BIG_IMAGE    0xC0 // 大图查看

#define BIG_PIC_SIZE        1769493
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

// 帧头结构
typedef struct
{
    INT16U  fheader;    // 帧头
    INT8U   fattr;      // 帧属性
    INT8U   fcode;      // 特征码
    INT16U  findex;     // 帧序号
    INT8U   pver;       // 协议版本号
    INT8U   ack;        // 应答
    INT32U  datalens;   // 数据长度
    INT8U   reserve[4]; // 保留字节
    INT8U   fhCheckSum; // 帧头校验和
    INT16U  fcmdID;     // 帧ID
}FrameHeader_S;
#pragma pack()

// 环形缓冲控制块
typedef struct
{
	INT8U 			*ptr;
	INT32U			tot_len;
	INT32U			len;
	INT32U			p_w;
	INT32U			p_r;
	OS_EVENT		*mutex;
}XBUFFER_S;



#define TX_BUF_SIZE         4096        // 发送缓冲尺寸
#define RX_BUF_SIZE         16*1024     // 接收缓冲尺寸
#define XBUFFER_SIZE        1024*1024*3 // 环形缓冲尺寸



// socket控制块队列
typedef struct socket_s
{
    struct socket_s *next;  // 上一个处在连接状态的socket控制块
    struct socket_s *prev;  // 下一个处在连接状态的socket控制块
    int socket;             // socket
    INT8U *tpbuf;           // 发送缓冲
    INT8U *rpbuf;           // 接收缓冲
    XBUFFER_S *xbuf;        // 环形缓冲控制块
}Socket_S;

// 帧参数控制块
typedef struct
{
    INT8U *pbuf;        // 数据域，没有填NULL
    INT32U datalens;    // 数据域长度，没有填0
    INT8U ack;          // 应答方式 0xFF/0xBB
    INT8U wr;           // 读写位，帧属性
    INT16U cmdID;       // 帧ID
}FramePara_S;

// LWIP服务端控制块
typedef struct
{
    LWIP_IF *t_lwip;    // 指向lwip网卡控制块
    int     socket;     // 服务器socket
    INT16U port;        // 服务器端口
    INT8U s_num;        // socket连接数量
    INT8U reserve[1];   // 保留
    OS_EVENT *lwipEvent;// 消息控制块
    Socket_S *pnode;    // socket控制块
}Typ_Config_Lwip;

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


extern _BSP_NET_TYPE NetChipUse;
extern OS_EVENT *DSPSndEvent;
extern void *DSPSndOSQ[10];
extern INT32U TickVal;
extern INT8U Pic_Md;

extern OS_EVENT *PHYEvent;
extern void *PHYOSQ[4];

extern void RecDSP_UpdtRT(INT8U *pFrame);
extern XBUFFER_S *xBuffer_Create(INT32U size,INT8U prio);
extern INT8U xBuffer_Free(XBUFFER_S *p);
extern INT8U xBuffer_Write(XBUFFER_S *p, INT8U *ptr, INT32U size);
extern INT8U xBuffer_Read(XBUFFER_S *p, INT8U *ptr, INT32U size);
extern INT8U xBuffer_Occupied_Size(XBUFFER_S *p, INT32U *size);

//#if USE_PHY
extern int APP_Socket_Send(INT8U *buf,INT32U len);
extern INT8U APP_Inside_Net_Status_Get(void);
//#else
extern int send_dsp1_data(uint8 *buf,uint32 len);
extern int send_dsp2_data(uint8 *buf,uint32 len);
extern void TaskComWithDSP(void *pdata);
extern int send_dsp_data_by_dev(uint8 devnum,uint8 *buf,uint32 len);
//#endif

//test
//int TEST_SET_ID_DSP_BASIC_INFO(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );


#endif
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/























