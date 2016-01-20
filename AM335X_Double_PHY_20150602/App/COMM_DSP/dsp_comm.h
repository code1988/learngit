/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: dsp_comm.h
* Author			:
* Date First Issued	: 141030
* Version			: V
* Description		:
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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


#define UPDATEHEADLEN       17                                  // ��������Ĺ̶�ͷ��
#define FRAMEHEADLEN        17                                  // ֡ͷ��
#define ARM_RD_DSP          0x81                                // arm �� dsp
#define ARM_WR_DSP          0x82                                // arm д dsp
#define REQ_ACK             0xFF                                // Ҫ����շ�Ӧ��
#define NREQ_ACK            0xBB                                // ��Ҫ����շ�Ӧ��


#define FRAME_FIX_INFO(ptr)     *(INT16U *)(ptr+FRAME_HEAD_OFFSET)=0x02A0;\
                                *(ptr+FRAME_FEATCODE_OFFSET)=0x03;\
                                *(ptr+FRAME_VERSION_OFFSET)=0x01;\
                                *(INT32U *)(ptr+FRAME_RESERVE_OFFSET)=0x00000000


// DSP��ʼ�����
#define DSP_INIT_ERR    0
#define DSP_INIT_SUS    1
#define DSP_NO_APP      2

// DSP�����������
#define DSP_UPDT_ERR        3
#define DSP_UPDT_SUS        4
#define DSP_UPDT_NSPORT     5
#define FPGA_UPDT_ERR       6
#define FPGA_UPDT_SUS       7
#define FPGA_UPDT_NSPORT    8
#define CNY_UPDT_ERR        9
#define CNY_UPDT_SUS        10
#define CNY_UPDT_NSPORT     11

// DSPУ�����
#define ADJUST_ERR          12
#define ADJUST_SUS          13
#define ADJUST_FST_SUS      14
#define ADJUST_SEC_STRT     15
#define ADJUST_SEC_SUS      16

// ֽ����Ϣ
#define MONEY_INFO          17

// DSP�汾��Ϣ
#define VERION_INFO         18

// DSP�豸��
#define DSP1    BSPW550A_SPINUM
#define DSP2    BSPW550B_SPINUM

// CPSW PORT NUM
#define CPSW_PORT0  0
#define CPSW_PORT1  1
// ������������
#define UPDATE_FOR_DSP      0x01
#define UPDATE_FOR_FPGA     0x02
#define UPDATE_FOR_CNY      0x03

#define SUB_ID_UPDATE       0x00 // ��������
#define SUB_ID_BKLST        0x01 // ����������
#define SUB_ID_ADJ          0x40 // У������
#define SUB_ID_VER          0x80 // ���汾����
#define SUB_ID_BIG_IMAGE    0xC0 // ��ͼ�鿴

#define BIG_PIC_SIZE        1769493
// �ṹ���ֽڶ���
#pragma pack(1)
// ��������̶�ͷ
typedef struct
{
    INT8U   Targt;      // ����Ŀ��
    INT32U  TotlSeg;    // �ܶ���
    INT32U  TotlLen;    // �ܳ�
    INT32U  CurSeg;     // ��ǰ�����
    INT32U  CurLen;     // ��ǰ�γ���
}_UpdtFixFmt;

// ֡ͷ�ṹ
typedef struct
{
    INT16U  fheader;    // ֡ͷ
    INT8U   fattr;      // ֡����
    INT8U   fcode;      // ������
    INT16U  findex;     // ֡���
    INT8U   pver;       // Э��汾��
    INT8U   ack;        // Ӧ��
    INT32U  datalens;   // ���ݳ���
    INT8U   reserve[4]; // �����ֽ�
    INT8U   fhCheckSum; // ֡ͷУ���
    INT16U  fcmdID;     // ֡ID
}FrameHeader_S;
#pragma pack()

// ���λ�����ƿ�
typedef struct
{
	INT8U 			*ptr;
	INT32U			tot_len;
	INT32U			len;
	INT32U			p_w;
	INT32U			p_r;
	OS_EVENT		*mutex;
}XBUFFER_S;



#define TX_BUF_SIZE         4096        // ���ͻ���ߴ�
#define RX_BUF_SIZE         16*1024     // ���ջ���ߴ�
#define XBUFFER_SIZE        1024*1024*3 // ���λ���ߴ�



// socket���ƿ����
typedef struct socket_s
{
    struct socket_s *next;  // ��һ����������״̬��socket���ƿ�
    struct socket_s *prev;  // ��һ����������״̬��socket���ƿ�
    int socket;             // socket
    INT8U *tpbuf;           // ���ͻ���
    INT8U *rpbuf;           // ���ջ���
    XBUFFER_S *xbuf;        // ���λ�����ƿ�
}Socket_S;

// ֡�������ƿ�
typedef struct
{
    INT8U *pbuf;        // ������û����NULL
    INT32U datalens;    // �����򳤶ȣ�û����0
    INT8U ack;          // Ӧ��ʽ 0xFF/0xBB
    INT8U wr;           // ��дλ��֡����
    INT16U cmdID;       // ֡ID
}FramePara_S;

// LWIP����˿��ƿ�
typedef struct
{
    LWIP_IF *t_lwip;    // ָ��lwip�������ƿ�
    int     socket;     // ������socket
    INT16U port;        // �������˿�
    INT8U s_num;        // socket��������
    INT8U reserve[1];   // ����
    OS_EVENT *lwipEvent;// ��Ϣ���ƿ�
    Socket_S *pnode;    // socket���ƿ�
}Typ_Config_Lwip;

typedef enum
{
    _ABANDON,           // ����(����Ӧ��)
    _NO_ERR=0x00,       // ��ȷ
    _SUM_CHK_ERR=0x01,  // ��֡У��ʹ���
    _TAIL_ERR=0x02,     // ֡β����
    _ID_ERR=03,         // ID����
    _ID_DATA_ERR=0x04,  // ID_DATA���ݴ���
    _FCODE_ERR=0x05,    // ���������
    _OP_ID_ERR=0x06     // ID��Ӧ�Ĳ�������(��Ӧ��ID��֧�ֶ�/д)
}_err_no;               // Ӧ���־

typedef enum
{
    _ASWR_FRAME,        // Ӧ��֡
    _NOR_FRAME_ACK,     // ��ͨ֡Ҫ��Ӧ��
    _NOR_FRAM_NACK      // ��ͨ֡��Ҫ��Ӧ��
}_Frame_Cat;            // ֡���


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
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/























