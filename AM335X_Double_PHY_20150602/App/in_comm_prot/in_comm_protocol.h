/***************�ļ���Ϣ********************************************************************************
**��   ��   ��: in_comm_protocol.h
**˵        ��:
**��   ��   ��: hxj
**��   �� ����: 2014-12-24 14:42
*******************************************************************************************************/
#ifndef	__INC_IN_COMM_PROTOCOL_H
#define	__INC_IN_COMM_PROTOCOL_H

#include "def_config.h"



#define PARA_FOTA_FRAME_HEAD    0x02A0          //֡ͷ
#define PARA_FOTA_FRAME_END     0x16            //֡β


#define FOTA_LEN_HEAD_SUM       1               //֡ͷУ��ͳ���
#define FOTA_LEN_DTAT_ID        2               //������ID�ĳ���
#define FOTA_LEN_END_SUM        1               //֡βУ��ͳ���
#define FOTA_LEN_END_FLAG       1               //֡β��־����



#define PARA_DSP_TO_ARM         0xA0            //DSP�����ARM
#define PARA_ARM_TO_DSP         0x80            //ARM�����DSP
#define PARA_DIR_MASK           0xE0            //��������
#define PARA_FUN_MASK           0x1F            //��������


#define PARA_FCODE_SPI          0x01            //spi ��������
#define PARA_FCODE_COM          0x02            //com ��������
#define PARA_FCODE_NET          0x03            //net ��������

//�����붨��
#define PARA_FUN_RD             0x01            //������
#define PARA_FUN_WT             0x02            //д����
#define PARA_FUN_CH             0x03            //��·����



//ACK ����
#define ACK_NEED_RET            0xFF            //��ҪӦ��
#define ACK_NOT_RET             0xBB            //����ҪӦ��


#define ACK_OK                  0x00            //��ȷ
#define ACK_ERR_HEAD_SUM        0x01            //֡ͷУ��ͳ��� (û��)
#define ACK_ERR_ALL_SUM         0x02            //��֡У��ͳ���
#define ACK_ERR_INVALID_ID      0x03            //��֧�ֵ�ID
#define ACK_ERR_ID_DATA         0x04            //ID_DATA ���ݲ���
#define ACK_ERR_FUN_CODE        0x05            //��֧�ֵĹ�����
#define ACK_ERR_DIR             0x06            //�����
#define ACK_ERR_PRO_VER         0x07            //Э��汾��ƥ�� (û��)
#define ACK_ERR_FCODE           0x08            //�������
#define ACK_ERR_ID_OPT          0x09            //��֧��ID �Ĳ���,���IDֻ֧�ֶ�������֧��д����,���յ������ID��д����ָ��
#define ACK_ERR_END_FLAG        0x0A            //֡β����
#define ACK_ERR_ACK             0x0B            //���ͷ�ack���
#define ACK_ERR_OHTER           0x77            //��������

















//========================================================================================
//֡����N����
typedef	struct	_strOneFrameData_N_
{
	uint8       frame_flag;					//֡��Ч��־
	uint32      frame_point;				//ָ֡��
	uint32 		frame_len;					//֡����
	uint32 		frame_data_size;		    //֡��С
	uint8       *frame_data;		        //һ֡����
}strOneFrameData_N;




//���ջ��λ�����N����
typedef	struct	_strRecvCycBuf_N_
{
	uint32 		write_point;			    //дָ��
	uint32		read_point;				    //��ָ��
	uint32		pre_read_point;				//Ԥ��ָ��
	uint8		full_flag;				    //����־(1-��,0-��)
	uint32      rec_buf_size;               //���ջ��λ�������С
	uint8		*rec_buf;	                //���ջ��λ�����
	strOneFrameData_N frameData;            //һ֡����
}strRecvCycBuf_N;



//----------------------------------------------------------------------------------------
#define   INIT_DATA_ID(ID,len,wlen,ch_en,ret_len,ret_ch_en,rd_fun,wr_fun,read_ret_ck,read_ret_len,write_ret_ck,write_ret_len,ret_rd_fun,ret_wr_fun)		    #ID,ID,(len),(wlen),(ch_en),(ret_len),(ret_ch_en),(rd_fun),(wr_fun),\
                                                                                                                                                            (read_ret_ck),(read_ret_len),(write_ret_ck),(write_ret_len),(ret_rd_fun),(ret_wr_fun),


#define   INIT_END_DATA_ID()		                                                NULL,0,0,0,0,0,0,NULL,NULL,0,0,0,0,NULL,NULL,


//��д���ȼ��
#define   CK_ON	                	                                                1
#define   CK_OF		                                                                0

//�����س��ȼ��
#define   RT_ON	                	                                                1
#define   RT_OF		                                                                0




typedef	struct	_strCmdIdList_
{
    char       *id_des;                     //id����
	uint16 		id_data;			        //����id
	uint32 		id_len;			            //id ���ݳ���,������id������2�ֽ�,����������
	uint32 		id_wlen;			        //id ���ݳ���,������id������2�ֽ�,д��������
    char        check_len_en;               //��ⳤ�ȱ�ʹ��,1-��ⳤ��,0-����ⳤ��
	uint32 		id_ret_len;			        //id ���ݳ���,������id������2�ֽ�,���صĳ���
    char        check_ret_len_en;           //��ⷵ�س��ȱ�ʹ��,1-��ⳤ��,0-����ⳤ��
	int         ( *get_param ) ( strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );    //������
	int         ( *set_param ) ( strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );    //д����

    //ret read write
    char        check_read_ret_len_en;      //�������س��ȱ�ʹ��,1-��ⳤ��,0-����ⳤ��
    uint32 		id_read_ret_len;			//id �����ݳ���,������id������2�ֽ�,�����صĳ���

    char        check_write_ret_len_en;     //���д���س��ȱ�ʹ��,1-��ⳤ��,0-����ⳤ��
    uint32 		id_write_ret_len;			//id д���ݳ���,������id������2�ֽ�,д���صĳ���

    int         ( *self_get_param ) ( strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );    //�Լ��Ķ����ز���
    int         ( *self_set_param ) ( strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );    //�Լ���д���ز���

}strCmdIdList;






//========================================================================================


#pragma   pack(1)

//----------------------------------------------------------------------------------------

//fota frame head (size 17)
typedef	struct	_strRecvFotaFrameHead_
{
	uint16 		h_head;		    //֡ͷ          //a0 02
	uint8		h_fun;		    //������        //D7 D6 D5 �����䷽�� D4 D3 D2 D1 D0 �������룩
	uint8		h_code;		    //������        //0x03 - ���紫������
	uint16 		h_index;		//֡���        //
	uint8		h_ver;			//Э��汾��    //01
	uint8		h_ack;			//Ӧ���־      //ff �� bb
	uint32 		h_data_len;		//�����򳤶�    //����������ID������������
	uint32 		h_rev;			//�����ֽ�      //00 00 00 00
	uint8       h_sum;          //֡ͷУ���
}strRecvFotaFrameHead;



//fota frame
typedef	struct	_strRecvFotaFrame_
{
    strRecvFotaFrameHead head_data;
    //������ʼ
    uint16      frame_id;           //������ID
	uint8       frame_data[1024];   //����������
	//���������
	uint8		frame_sum_all;		//��֡У���
	uint8		frame_end;		    //֡β
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

