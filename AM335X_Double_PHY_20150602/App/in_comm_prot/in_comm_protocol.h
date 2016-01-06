/***************文件信息********************************************************************************
**文   件   名: in_comm_protocol.h
**说        明:
**创   建   人: hxj
**创   建 日期: 2014-12-24 14:42
*******************************************************************************************************/
#ifndef	__INC_IN_COMM_PROTOCOL_H
#define	__INC_IN_COMM_PROTOCOL_H

#include "def_config.h"



#define PARA_FOTA_FRAME_HEAD    0x02A0          //帧头
#define PARA_FOTA_FRAME_END     0x16            //帧尾


#define FOTA_LEN_HEAD_SUM       1               //帧头校验和长度
#define FOTA_LEN_DTAT_ID        2               //数据域ID的长度
#define FOTA_LEN_END_SUM        1               //帧尾校验和长度
#define FOTA_LEN_END_FLAG       1               //帧尾标志长度



#define PARA_DSP_TO_ARM         0xA0            //DSP传输给ARM
#define PARA_ARM_TO_DSP         0x80            //ARM传输给DSP
#define PARA_DIR_MASK           0xE0            //方向掩码
#define PARA_FUN_MASK           0x1F            //功能掩码


#define PARA_FCODE_SPI          0x01            //spi 传输数据
#define PARA_FCODE_COM          0x02            //com 传输数据
#define PARA_FCODE_NET          0x03            //net 传输数据

//功能码定义
#define PARA_FUN_RD             0x01            //读功能
#define PARA_FUN_WT             0x02            //写功能
#define PARA_FUN_CH             0x03            //链路测试



//ACK 定义
#define ACK_NEED_RET            0xFF            //需要应答
#define ACK_NOT_RET             0xBB            //不需要应答


#define ACK_OK                  0x00            //正确
#define ACK_ERR_HEAD_SUM        0x01            //帧头校验和出错 (没用)
#define ACK_ERR_ALL_SUM         0x02            //整帧校验和出错
#define ACK_ERR_INVALID_ID      0x03            //不支持的ID
#define ACK_ERR_ID_DATA         0x04            //ID_DATA 内容不对
#define ACK_ERR_FUN_CODE        0x05            //不支持的功能码
#define ACK_ERR_DIR             0x06            //方向错
#define ACK_ERR_PRO_VER         0x07            //协议版本不匹配 (没用)
#define ACK_ERR_FCODE           0x08            //特征码错
#define ACK_ERR_ID_OPT          0x09            //不支持ID 的操作,如此ID只支持读操作不支持写操作,但收到了这个ID的写操作指令
#define ACK_ERR_END_FLAG        0x0A            //帧尾出错
#define ACK_ERR_ACK             0x0B            //发送方ack码错
#define ACK_ERR_OHTER           0x77            //其它错误

















//========================================================================================
//帧数据N定义
typedef	struct	_strOneFrameData_N_
{
	uint8       frame_flag;					//帧有效标志
	uint32      frame_point;				//帧指针
	uint32 		frame_len;					//帧长度
	uint32 		frame_data_size;		    //帧大小
	uint8       *frame_data;		        //一帧数据
}strOneFrameData_N;




//接收环形缓冲区N定义
typedef	struct	_strRecvCycBuf_N_
{
	uint32 		write_point;			    //写指针
	uint32		read_point;				    //读指针
	uint32		pre_read_point;				//预读指针
	uint8		full_flag;				    //满标志(1-满,0-空)
	uint32      rec_buf_size;               //接收环形缓冲区大小
	uint8		*rec_buf;	                //接收环形缓冲区
	strOneFrameData_N frameData;            //一帧数据
}strRecvCycBuf_N;



//----------------------------------------------------------------------------------------
#define   INIT_DATA_ID(ID,len,wlen,ch_en,ret_len,ret_ch_en,rd_fun,wr_fun,read_ret_ck,read_ret_len,write_ret_ck,write_ret_len,ret_rd_fun,ret_wr_fun)		    #ID,ID,(len),(wlen),(ch_en),(ret_len),(ret_ch_en),(rd_fun),(wr_fun),\
                                                                                                                                                            (read_ret_ck),(read_ret_len),(write_ret_ck),(write_ret_len),(ret_rd_fun),(ret_wr_fun),


#define   INIT_END_DATA_ID()		                                                NULL,0,0,0,0,0,0,NULL,NULL,0,0,0,0,NULL,NULL,


//读写长度检测
#define   CK_ON	                	                                                1
#define   CK_OF		                                                                0

//读返回长度检测
#define   RT_ON	                	                                                1
#define   RT_OF		                                                                0




typedef	struct	_strCmdIdList_
{
    char       *id_des;                     //id描述
	uint16 		id_data;			        //数据id
	uint32 		id_len;			            //id 数据长度,不包括id自身长度2字节,读操作长度
	uint32 		id_wlen;			        //id 数据长度,不包括id自身长度2字节,写操作长度
    char        check_len_en;               //检测长度标使能,1-检测长度,0-不检测长度
	uint32 		id_ret_len;			        //id 数据长度,不包括id自身长度2字节,返回的长度
    char        check_ret_len_en;           //检测返回长度标使能,1-检测长度,0-不检测长度
	int         ( *get_param ) ( strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );    //读操作
	int         ( *set_param ) ( strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );    //写操作

    //ret read write
    char        check_read_ret_len_en;      //检测读返回长度标使能,1-检测长度,0-不检测长度
    uint32 		id_read_ret_len;			//id 读数据长度,不包括id自身长度2字节,读返回的长度

    char        check_write_ret_len_en;     //检测写返回长度标使能,1-检测长度,0-不检测长度
    uint32 		id_write_ret_len;			//id 写数据长度,不包括id自身长度2字节,写返回的长度

    int         ( *self_get_param ) ( strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );    //自己的读返回操作
    int         ( *self_set_param ) ( strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );    //自己的写返回操作

}strCmdIdList;






//========================================================================================


#pragma   pack(1)

//----------------------------------------------------------------------------------------

//fota frame head (size 17)
typedef	struct	_strRecvFotaFrameHead_
{
	uint16 		h_head;		    //帧头          //a0 02
	uint8		h_fun;		    //功能码        //D7 D6 D5 （传输方向） D4 D3 D2 D1 D0 （功能码）
	uint8		h_code;		    //特征码        //0x03 - 网络传输数据
	uint16 		h_index;		//帧序号        //
	uint8		h_ver;			//协议版本号    //01
	uint8		h_ack;			//应答标志      //ff 或 bb
	uint32 		h_data_len;		//数据域长度    //包括数据域ID和数据域内容
	uint32 		h_rev;			//保留字节      //00 00 00 00
	uint8       h_sum;          //帧头校验和
}strRecvFotaFrameHead;



//fota frame
typedef	struct	_strRecvFotaFrame_
{
    strRecvFotaFrameHead head_data;
    //数据域开始
    uint16      frame_id;           //数据域ID
	uint8       frame_data[1024];   //数据域内容
	//数据域结束
	uint8		frame_sum_all;		//整帧校验和
	uint8		frame_end;		    //帧尾
}strRecvFotaFrame;








#pragma   pack()
//========================================================================================


typedef int(*P_FUN_SEND)(uint8 *buf,uint32 len);














extern void recv_data_to_cyc_buf_n ( strRecvCycBuf_N *recCycBufN, uint8 *buf, uint32 len );
extern int get_index_empty_frame_by_frameN(strOneFrameData_N *frameData,int frame_size);
extern uint8 calc_fota_frame_sum(uint8 *buf,int len);
extern uint32 get_prot_fota_head(void);
extern void init_cycbuf_n(strRecvCycBuf_N *recCycBuf);
extern strRecvCycBuf_N *malloc_cycbuf_n(uint32 cyc_buf_size,uint32 one_frame_size);
extern void find_frame_by_prot_fota ( strRecvCycBuf_N *recCycBufN ,uint8 index_rec_frame );
extern void create_ack_frame_by_err_ack(strOneFrameData_N *recv_frameData,strOneFrameData_N *ret_frameData,uint8 ack);
extern void display_id_list(strCmdIdList *p_id_wr_list,char *des);


extern int fota_frame_pre_manage(strOneFrameData_N *pframeData,strCmdIdList *p_tmp_id_wr_list,P_FUN_SEND send_fun,uint16 self_h_index_min,uint16 self_h_index_max);


extern void create_ack_frame_by_read(strOneFrameData_N *recv_frameData,strOneFrameData_N *ret_frameData);









#endif
/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

