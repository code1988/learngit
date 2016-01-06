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

// ֽ����Ϣ
#define MONEY_INFO          17

// DSP�汾��Ϣ
#define VERION_INFO         18

// DSP�豸��
#define DSP1    W5500A
#define DSP2    W5500B

// ������������
#define UPDATE_FOR_DSP      0x01
#define UPDATE_FOR_FPGA     0x02
#define UPDATE_FOR_CNY      0x03

#define MAIN_ID_MASK        0x3F                // ��ID
#define SUB_ID_MASK         0xC0                // ��ID                                  
#define SUB_ID_UPDATE       0x00 // ��������
#define SUB_ID_ADJ          0x40 // У������
#define SUB_ID_VER          0x80 // ���汾����
#define SUB_ID_BIG_IMAGE    0xC0 // ��ͼ�鿴
                                  
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
#pragma pack()

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

#if DBG_DSP_RECV_TWO_EN
extern OS_EVENT *W5500AEvent;          // W5500A�¼����ƿ�
extern void *W5500OAOSQ[4];             // W5500A��Ϣ����
#endif

extern OS_EVENT *W5500BEvent;          // W5500B�¼����ƿ�
extern void *W5500OBOSQ[4];             // W5500B��Ϣ����


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
/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/























