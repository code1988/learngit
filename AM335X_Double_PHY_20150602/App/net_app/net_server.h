/***************�ļ���Ϣ********************************************************************************
**��   ��   ��: net_server.h
**˵        ��:
**��   ��   ��: hxj
**��   �� ����: 2014-12-17 8:55
*******************************************************************************************************/
#ifndef	__INC_NET_SERVER_H
#define	__INC_NET_SERVER_H



#include "def_config.h"



#if 0 /*hxj amend,date 2014-5-27 10:32*/
#define ONE_TIMEOUT_END_VAL                 20          //s
#define ONE_TIMEOUT_RECV_VAL                20          //s

#else
#define ONE_TIMEOUT_END_VAL                 0          //s
#define ONE_TIMEOUT_RECV_VAL                0          //s
#endif


#define ONE_SEND_LEN                        1460
#define RECV_DAT_FILE_BUF_MAX               100         //����dat�ļ��Ļ�����
#define PARA_ONE_TIME_MONEY_MAX_NUM         300         //һ�η��ͳ����������


//========================================================================================
#pragma   pack(1)



//�����ͽṹ
typedef struct _NET_SIMPLE_
{
   uint32 start_head;           //��ʼ��־      0x40404A4C
   uint32 msg_length;           //��Ϣ���ܳ���  ����
   uint16 version;              //Э��汾      10
   uint16 cmd;                  //����          0x00A1
   uint16 MachineNo[14];        //���ߺ�        ASCII
   uint8  traceNumber[6];       //���κ�        ASCII ��̬���� ����ǰ�油'0' //ͬһ����ͬһ�������κŲ����ظ�
   uint8  tranDate[8];          //�鳮��������  ASCII
   uint8  tranTime[6];          //�鳮����ʱ��  ASCII
   uint16 totalRequestCounts;   //�����ܴ���    1
   uint16 requestNo;            //��ǰ������    1
   uint8  bankNumber[8];        //8λ������   ASCII ����ǰ�油 '0'        (��)
   uint8  MachineType[2];       //�豸����      ASCII AD��A ��㳮��
   uint8  operatorId[12];       //����Ա��      ASCII �����Ҳ� 0x00         (��)
   uint8  company[4];           //4λ���̼��   ASCII FOTA
   uint16 packageIndex;         //�����        0x0000
   uint16 reserve;              //������        0x0000
}NET_SIMPLE;


//�ṹ�峤�� 94
typedef struct _NET_SIMPLE_C_
{
   uint8 start_head[4];         //��ʼ��־      0x40404A4C
   uint8 msg_length[4];         //��Ϣ���ܳ���  ����
   uint8 version[2];            //Э��汾      10
   uint8 cmd[2];                //����          0x00A1
   uint8 MachineNo[28];         //���ߺ�        ASCII
   uint8 traceNumber[6];        //���κ�        ASCII ��̬���� ����ǰ�油'0' //ͬһ����ͬһ�������κŲ����ظ�
   uint8 tranDate[8];           //�鳮��������  ASCII
   uint8 tranTime[6];           //�鳮����ʱ��  ASCII
   uint8 totalRequestCounts[2]; //�����ܴ���    1
   uint8 requestNo[2];          //��ǰ������    1
   uint8 bankNumber[8];         //8λ������   ASCII ����ǰ�油 '0'        (��)
   uint8 MachineType[2];        //�豸����      ASCII AD��A ��㳮��
   uint8 operatorId[12];        //����Ա��      ASCII �����Ҳ� 0x00         (��)
   uint8 company[4];            //4λ���̼��   ASCII FOTA
   uint8 packageIndex[2];       //�����        0x0000
   uint8 reserve[2];            //������        0x0000
}NET_SIMPLE_C;



//========================================================================================
//�������ݽṹ
typedef struct _NET_UP_
{
     uint32 start_head;         //��ʼ��־      0x40404A4C
     uint32 msg_length;         //��Ϣ���ܳ���  ���� 82+fileBody_len+2
     uint16 version;            //Э��汾      10
     uint16 cmd;                //����          0x0006 Ϊ�����¼
     uint16 MachineNo[14];      //���ߺ�        ASCII
     uint8  code[20];           //ҵ������      HM�������¼����ҵ��ʱӦΪ������  �����Ҳ�0x0       (��)
     uint16 packageIndex;       //�ļ��ִη���ʱ��� 0x0000
     uint8  reserve[20];        //����λ        0x00 ... 0x00(20��)
     uint8  fileBody[5000];     //����������    ���� FSN��ʽ
     uint16 checksum;           //У����,(����) 0x0000
}NET_UP;


//�ṹ�峤�� 84
typedef struct _NET_UP_C_
{
     uint8 start_head[4];       //��ʼ��־      0x40404A4C
     uint8 msg_length[4];       //��Ϣ���ܳ���  ���� 82+fileBody_len+2
     uint8 version[2];          //Э��汾      10
     uint8 cmd[2];              //����          0x0006 Ϊ�����¼
     uint8 MachineNo[28];       //���ߺ�        ASCII
     uint8 code[20];            //ҵ������      HM�������¼����ҵ��ʱӦΪ������  �����Ҳ�0x0       (��)
     uint8 packageIndex[2];     //�ļ��ִη���ʱ��� 0x0000
     uint8 reserve[20];         //����λ        0x00 ... 0x00(20��)
     uint8 fileBody[2];         //����������    ���� FSN��ʽ
//     uint8 checksum[2];         //У����,(����) 0x0000
}NET_UP_C;




//========================================================================================
//���ͽ�����,���������ٻظ�,�������Ͽ�����
//��������ṹ

typedef struct _NET_CLOSE_
{
    uint32 start_head;          //��ʼ��־      0x40404A4C
    uint32 msg_length;          //��Ϣ���ܳ���  ����
    uint16 version;             //Э��汾      10
    uint16 cmd;                 //����          0x00A2
    uint16 MachineNo[14];       //���ߺ�        ASCII
    uint16 packageIndex;        //�ļ��ִη���ʱ��� 0x0000
    uint16 reserve;             //����λ        0x0000
}NET_CLOSE;


//�ṹ�峤�� 44
typedef struct _NET_CLOSE_C_
{
    uint8 start_head[4];        //��ʼ��־      0x40404A4C
    uint8 msg_length[4];        //��Ϣ���ܳ���  ����
    uint8 version[2];           //Э��汾      10
    uint8 cmd[2];               //����          0x00A2
    uint8 MachineNo[28];        //���ߺ�        ASCII
    uint8 packageIndex[2];      //�ļ��ִη���ʱ��� 0x0000
    uint8 reserve[2];           //����λ        0x0000

}NET_CLOSE_C;





//========================================================================================
//֡ͷ�ṹ 28
typedef struct _NET_HEAD_
{
    uint8  start_head[4];        //��ʼ��־      "SPDB"
    uint32 msg_length;           //��Ϣ���ܳ���  ��֡��
    uint8  fun_code[4];          //���ܴ���
    uint8  date_time[14];        //����ʱ��      ��ʽΪ"YYYYMMDDhhmmss"
    uint16 domain;               //�����
}NET_HEAD;



//֡ͷ�ṹ 28
typedef struct _NET_HEAD_C_
{
    uint8 start_head[4];        //��ʼ��־      "SPDB"
    uint8 msg_length[4];        //��Ϣ���ܳ���  ��֡��
    uint8 fun_code[4];          //���ܴ���
    uint8 date_time[14];        //����ʱ��      ��ʽΪ"YYYYMMDDhhmmss"
    uint8 domain[2];            //�����
}NET_HEAD_C;



//Ӧ��֡ͷ�ṹ 32
typedef struct _NET_HEAD_ACK_
{
    uint8  start_head[4];        //��ʼ��־      "SPDB"
    uint32 msg_length;           //��Ϣ���ܳ���  ��֡��
    uint8  fun_code[4];          //���ܴ���
    uint8  date_time[14];        //����ʱ��      ��ʽΪ"YYYYMMDDhhmmss"
    uint8  err_code[4];          //�������      AAAA-�ɹ�,E001-E006 E101-E103
    uint16 domain;               //�����
}NET_HEAD_ACK;


//Ӧ��֡ͷ�ṹ 32
typedef struct _NET_HEAD_ACK_C_
{
    uint8 start_head[4];        //��ʼ��־      "SPDB"
    uint8 msg_length[4];        //��Ϣ���ܳ���  ��֡��
    uint8 fun_code[4];          //���ܴ���
    uint8 date_time[14];        //����ʱ��      ��ʽΪ"YYYYMMDDhhmmss"
    uint8 err_code[4];          //�������      AAAA-�ɹ�,E001-E006 E101-E103
    uint8 domain[2];            //�����
}NET_HEAD_ACK_C;




//֡�ṹ 28+����
typedef struct _NET_FRAME_
{
    uint8 start_head[4];        //��ʼ��־      "SPDB"
    uint32 msg_length;          //��Ϣ���ܳ���  ��֡��
    uint16 fun_code;            //���ܴ���
    uint8 date_time[14];        //����ʱ��      ��ʽΪ"YYYYMMDDhhmmss"
    uint16 domain;              //�����
    uint8 data[1024];           //��������
}NET_FRAME;



//֡�ṹ 28+����
typedef struct _NET_FRAME_C_
{
    uint8 start_head[4];        //��ʼ��־      "SPDB"
    uint8 msg_length[4];        //��Ϣ���ܳ���  ��֡��
    uint8 fun_code[4];          //���ܴ���
    uint8 date_time[14];        //����ʱ��      ��ʽΪ"YYYYMMDDhhmmss"
    uint8 domain[2];            //�����
    uint8 data[1024];           //��������
}NET_FRAME_C;




//Ӧ��֡�ṹ 28+����
typedef struct _NET_FRAME_ACK_
{
    uint8 start_head[4];        //��ʼ��־      "SPDB"
    uint32 msg_length;          //��Ϣ���ܳ���  ��֡��
    uint8 fun_code[4];          //���ܴ���
    uint8 date_time[14];        //����ʱ��      ��ʽΪ"YYYYMMDDhhmmss"
    uint8 err_code[4];          //�������      AAAA-�ɹ�,E001-E006 E101-E103
    uint8 domain[2];            //�����
    uint8 data[1024];           //��������
}NET_FRAME_ACK;




//Ӧ��֡�ṹ 28+����
typedef struct _NET_FRAME_ACK_C_
{
    uint8 start_head[4];        //��ʼ��־      "SPDB"
    uint8 msg_length[4];        //��Ϣ���ܳ���  ��֡��
    uint8 fun_code[4];          //���ܴ���
    uint8 date_time[14];         //����ʱ��      ��ʽΪ"YYYYMMDDhhmmss"
    uint8 err_code[4];          //�������       AAAA-�ɹ�,E001-E006 E101-E103
    uint8 domain[2];            //�����
    uint8 data[1024];           //��������
}NET_FRAME_ACK_C;


//����֡�ṹ
typedef struct _NET_BODY_
{
    uint8  body_head[6];            //���ı�־
    uint32 body_data_length;        //���ĳ���
    uint8  body_data[200];          //��������
}NET_BODY;



//����֡ͷ�ṹ 10
typedef struct _NET_BODY_HEAD_C_
{
    uint8  body_head[6];            //���ı�־
    uint8  body_data_length[4];     //���ĳ���
}NET_BODY_HEAD_C;




//����֡�ṹ
typedef struct _NET_BODY_C_
{
    uint8  body_head[6];            //���ı�־
    uint8  body_data_length[4];     //���ĳ���
    uint8  body_data[200];          //��������
}NET_BODY_C;



//����֡�ṹ
typedef struct _MACH_INFO_C_
{
    uint8  mach_code[25];               //���߱��
    uint8  g_time_setup[15];            //����ʱ��
    uint8  g_local_sn[4];               //������(4)
    uint8  g_sub_dank_sn[4];            //֧�к�(4)
    uint8  g_net_sn[4];                 //�����(4)

    uint8  SN_NetSend_MachineNo[8];
    uint8  opt_sn[8];                   //����Ա��
    uint8  atm_transation[20];          //atm������
    uint32 not_download_file_num;       //����������û�����ص��ļ���
    uint32 arm_not_download_file_num;   //ARM���ص�û�����ص��ļ���

    uint8  arm_ack_not_download_file_num;   //ARMȷ�ϵ�û�����ص��ļ���
    uint8  arm_ack_opt_sign_in;         //ARMȷ�ϲ���Աǩ��
    uint8  arm_ack_opt_sign_out;        //ARMȷ�ϲ���Ա��ǩ
    uint8  arm_ack_trade_start;         //ARMȷ���������׵㳮
    uint8  arm_ack_trade_end;           //ARMȷ�Ͻ������׵㳮
    uint8  arm_ack_file_download_ok;    //ARMȷ���ļ��Ѿ�����
    uint8  arm_ack_save_file;           //ARMȷ�����������ļ�

    uint8  cur_client_over_file;        //��ǰ�Ŀͻ��˽�����־


}MACH_INFO_C;


//����֡�ṹ
typedef struct _DSP_DATE_
{
    char   valid_flag;                  //��Ч��־
    char  date_str[15];                 //��ǰʱ��  14����,��ʽΪ"YYYYMMDDhhmmss"
    uint32 cur_s_cnt;                   //��ȡ  llTimerGetTime(NULL); 1s
    uint16 year;
    uint16 month;
    uint16 day;
    uint16 time_h;
    uint16 time_m;
    uint16 time_s;

}DSP_DATE;



//����֡�ṹ
typedef struct _NET_MASK_TIME_
{
    uint8 time_h;
    uint8 time_m;
}NET_MASK_TIME;


typedef struct _NET_MASK_ONE_TIME_
{
    NET_MASK_TIME time_s;
    NET_MASK_TIME time_e;
}NET_MASK_ONE_TIME;


typedef struct _NET_MASK_ONE_TIME_DS_
{
    uint32 time_s_ds;
    uint32 time_e_ds;
}NET_MASK_ONE_TIME_DS;



//
typedef struct _DSP_upload_head_
{
    char  mach_code_str[24+1];               //���߱��
    char  opt_sn_str[8+1];                   //����Ա��
    char  atm_transation_str[20+1];          //atm������
    char  dat_file_time_str[14+1];             //��ǰʱ��  14����,��ʽΪ"YYYYMMDDhhmmss"
    //add
    uint8 local_sn[4];      //������(4)
    uint8 sub_dank_sn[4];   //֧�к�(4)
    uint8 net_sn[4];        //�����(4)


}DSP_upload_head;


//========================================================================================
//spi frame
typedef struct _SPI_CMD_EX_C_
{
    uint8  data_src_flag[4];            //����Դ��ʶ
    uint8  frame_sn[2];                 //����
    uint8  ack_flag;                    //Ӧ���־
    uint8  rw_flag;                     //��д��־(1B) (0-����1-д)
    uint8  sub_cmd[2];                  //������
    uint8  sub_data_len[4];             //�����ݳ���
    uint8  sub_data[100];               //������
}SPI_CMD_EX_C;


//========================================================================================
//ԭ fsn ����
typedef struct {
	Uint16  HeadStart[4];
	Uint16  HeadString[6];
	Uint32  Counter;
	Uint16  HeadEnd[4];
} FSN_HEAD, *P_FSN_HEAD;

typedef struct {
  Uint32 Data[32];// ͼ����ֺ����������
} TImgSNoData;

typedef struct {
  Int16 Num;			//�ַ���
  Int16 height, width;	//ÿ��ͼ���ַ��߶ȺͿ��
  Uint16 Reserve2;       //������2
  TImgSNoData SNo[12];
} TImageSNo;

typedef struct
{
  Uint16 Date;		  	//�鳮��������
  Uint16 Time;		  	//�鳮����ʱ��
  Uint16 tfFlag;		//�桢�١��к;ɱұ�־
  Uint16 ErrorCode[3];  //������(3��)
  Uint16 MoneyFlag[4]; 	//���ұ�־
  Uint16 Ver;           //�汾��
  Uint16 Valuta;		//��ֵ
  Uint16 CharNUM;	  	//���ֺ����ַ���
  Uint16 SNo[12];		//���ֺ���
  Uint16 MachineSNo[24];//���߱��
  Uint16 Reserve1;       //������1
  TImageSNo ImageSNo;//ͼ����ֺ���
} FSN_DATA, *P_FSN_DATA;





//========================================================================================
//fsn �ļ��ṹ���� (����fsn�Ľṹ��DSP��,�պö��Ƕ����)

// ����ͼ�����ṹ
//�ṹ�峤�� 128
typedef struct _STR_ONE_SN_IMAGE_DATA_
{
    uint32 Data[32];        // ͼ����ֺ����������
} ATT_BYTE_NO_ALIGNED
STR_ONE_SN_IMAGE_DATA;


//ͼ����ֺ���ṹ�����12λ����
//�ṹ�峤�� 1544
typedef struct _STR_FSN_BODY_BODY_
{
    uint16 Num;             //�ַ���
    uint16 height;          //ÿ��ͼ���ַ��߶�
    uint16 width;           //ÿ��ͼ���ַ����
    uint16 Reserve2;        //������2
    STR_ONE_SN_IMAGE_DATA SNo[12];
} ATT_BYTE_NO_ALIGNED
STR_FSN_BODY_BODY;


//�ṹ�峤�� 100
typedef struct _STR_FSN_BODY_HEAD_
{
    uint16 Date;            //�鳮��������
    uint16 Time;            //�鳮����ʱ��
    uint16 tfFlag;          //�桢�١��к;ɱұ�־
    uint16 ErrorCode[3];    //������(3��)
    uint16 MoneyFlag[4];    //���ұ�־ CNY
    uint16 Ver;             //�汾��,0-1990,1-1999,2-2005,9999
    uint16 Valuta;          //��ֵ,1,5,10,20,50,100
    uint16 CharNUM;         //���ֺ����ַ���,10
    uint16 SNo[12];         //���ֺ���
    uint16 MachineSNo[24];  //���߱��
    uint16 Reserve1;        //������     1
}ATT_BYTE_NO_ALIGNED
STR_FSN_BODY_HEAD;


//�ṹ�峤�� 1644
typedef struct _STR_FSN_BODY_
{
	STR_FSN_BODY_HEAD 	data_head;
    STR_FSN_BODY_BODY 	data_body;     //ͼ����ֺ���
}ATT_BYTE_NO_ALIGNED
STR_FSN_BODY;


//�ṹ�峤�� 32
typedef struct _STR_FSN_HEAD_
{
    uint16  HeadStart[4];       //20,10,7,26
    uint16  HeadString[6];      //0x00 0x01 (0x2e/0x2f) 'S' 'N' 'o'
    uint32  Counter;            //
    uint16  HeadEnd[4];         //0x00 0x01 0x02 0x03
}ATT_BYTE_NO_ALIGNED
STR_FSN_HEAD;


//�ṹ�峤�� 493232=32+1644*300
typedef struct _STR_FSN_FILE_
{
	STR_FSN_HEAD	fsn_head;
	STR_FSN_BODY	fsn_body[PARA_ONE_TIME_MONEY_MAX_NUM];
}ATT_BYTE_NO_ALIGNED
STR_FSN_FILE;



//========================================================================================
//dat �ļ��ṹ����

// size 250
typedef struct _DAT_DATA_INFO_C_
{
	uint8 res_1[5];				//����5,û��
	uint8 money_sn[10];			//10�ֽ�Ϊ���ֺ�
	uint8 money_val;			//��ֵ DAT�ļ���ֱ�Ӵ�1-100��ֵ,DSP���͹�������1~6�ֱ��Ӧ:1,5,10,20,50,100Ԫ
	uint8 money_ver;			//�汾,0-1990,1-1999,2-2005
	uint8 money_flag;			//��٣���Ϊ1
	uint8 money_sn_pic[232];	//232�ֽڵĿ�ͼ(ѹ������)
}DAT_DATA_INFO_C;


//size 48
typedef struct _DAT_HEAD_ADD_C_
{
	//add head
    uint8  opt_sn[8];                   //����Ա��
    uint8  atm_transation[20];          //atm������
	//uint8  res_7[20];                   //add ����12��,������(4),֧�к�(4),�����(4)
    uint8 local_sn[4];                  //������(4)
    uint8 sub_dank_sn[4];               //֧�к�(4)
    uint8 net_sn[4];                    //�����(4)
    uint8  res_8[8];

}DAT_HEAD_ADD_C;


//size 65
typedef struct _DAT_HEAD_OLD_C_
{
	//old head
    uint8  head_flag;          		//ͷ��־,�̶�0x01����ʾͷ����
	uint8  res_1[4];          		//����4,û��
	uint8  res_2[11]; 				//����11,û��
	uint8  money_val;				//����1,û��
	uint8  all_num[2];			    //������
	uint8  mach_code[24];           //���߱��
	uint8  time_setup[14];			//�㳮ʱ��,14�ֽ�Ϊʱ��
	uint8  res_4[6];				//����6,û��
	uint8  res_5[1];				//����1,û��
	uint8  res_6[1];				//����1,û��

}DAT_HEAD_OLD_C;



typedef struct _DAT_FILE_FORMAT_C_
{
    DAT_HEAD_OLD_C head_old;            //֮ǰ��ͷ
	//add head
	DAT_HEAD_ADD_C head_add;	        //���ӵ�ͷ
	//data
	DAT_DATA_INFO_C data_info[PARA_ONE_TIME_MONEY_MAX_NUM];     //Ǯ�ľ�����Ϣ

}DAT_FILE_FORMAT_C;


//�������dat�ļ��Ļ���ṹ
typedef struct _DAT_FILE_BUF_
{
volatile    uint8  dat_valid_flag;      //��Ч��־  //0-�յ�,1-������ݻ�û�з���,2-��õ��������ڷ���,3-��DSPȷ��,��ARMûȷ��
    uint32 dat_len;             		//dat ����
    uint32 money_cnt;                   //Ǯ����
    int    change_time;                 //ת���Ĳο�ʱ��
    uint32 send_time_out_dds;           //���ͳ�ʱʱ��
    char   time[15];                    //dat �ļ�ʱ��,��dat�ļ��л�ã�14�ַ���Ч
    DAT_FILE_FORMAT_C dat_buf; 	        //dat ����
}DAT_FILE_BUF;


typedef struct _DAT_TIME_FIND_
{
    int    i_val;
    int    change_time;                 //ת���Ĳο�ʱ��
}DAT_TIME_FIND;



//====================================

//busi ͷ��Ϣ

//�ṹ�峤�� 60
typedef struct _busi_head_
{
    uint8 local_sn[4];      //������(4)
    uint8 sub_dank_sn[4];   //֧�к�(4)
    uint8 net_sn[4];        //�����(4)
    uint8 fact_sn[2];       //���ұ��(2) "10(2)"
    uint8 mach_sn[10];      //�������(10) "0000A00850(10)"

    uint8 busi_type[2];     //ҵ������  "11"
    uint8 mach_type[2];     //�豸����  //03-��ֻ�,//05-A���
    uint8 card_sn[19];      //����
    uint8 Transaction_code[5];      //���״���
    uint8 mach_s_num[6];            //�˻���ˮ��
    uint8 rh_busi_type[2];          //����ҵ������ "GM"

}ATT_BYTE_NO_ALIGNED
busi_head;


//busi ͷ��Ϣ

//�ṹ�峤�� 52
typedef struct _busi_info_
{
    uint8 time[14];             //��¼ʱ��(YYYYMMDDhhmmss)
    uint8 SNo[10];              //���ֺ���
    uint8 MoneyFlag[3];         //���ұ�־ "CNY"
    uint8 Valuta[3];            //��ֵ             "050"
    uint8 Ver[4];               //�汾��       '''1999'
    uint8 tfFlag;               //�桢�١��к;ɱұ�־
    uint8 new_old[2];           //��ɫ
    uint8 cnt_num[5];           //���
    uint8 res[10];              //����
}ATT_BYTE_NO_ALIGNED
busi_info;

typedef struct _STR_BUFI_FILE_
{
    busi_head   busi_hd;
    busi_info   busi_body[PARA_ONE_TIME_MONEY_MAX_NUM];
}ATT_BYTE_NO_ALIGNED
STR_BUFI_FILE;


//====================================




#pragma   pack()

//========================================================================================




extern SK_TYPE g_sk_big_img;
extern SK_TYPE  g_sk_wz;
extern uint32   g_connect_wz_timeout_s;
extern int      g_connect_wz_start_flag;

extern char      *p_g_fsn_data;
extern int      fsn_data_max_len;

extern char     *p_g_busi_txt;
extern int      busi_data_max_len;
extern NET_MASK_ONE_TIME g_not_send_time[10];
extern MACH_INFO_C cur_arm_mac_info;
extern VAL_VOL uint8   g_get_net_date_flag;
extern uint8   g_mac[6];



extern char  g_init_tcp_ip_data_buf_over_flag;

extern uint8 g_prot_type;
extern uint8 g_prot_mode;
extern uint8 g_net_send_switch;


extern char g_local_ip[20];
extern char g_local_mk[20];
extern char g_local_gw[20];
extern uint16 g_local_port;

extern char g_server_ip[20];
extern uint16 g_server_port;









//========================================================================================
//�ڲ���
extern int creat_fsn_by_dat(STR_FSN_FILE *p_fsn_file,int fsn_buf_len,int *p_ret_fsn_len,DAT_FILE_FORMAT_C *p_dat_file,int dat_file_len);
extern void clear_dat_buf(int point);
extern int send_to_arm_by_spi(SK_TYPE skt,uint16 fun_code,char *buf,int len);
extern int set_flag_dat_buf(int point,uint8 set_valid_flag);
extern int get_upload_head_info_by_dat_file(DSP_upload_head *p_ret_upload_head,DAT_FILE_FORMAT_C *p_dat_file,int dat_file_len);





//========================================================================================
extern void init_tcp_server_data(void);
extern void send_dat_buf(void);
extern int get_os_sys_date(char *date);


extern int get_flag_icbc_fsn_net();
extern int open_flag_icbc_fsn_net();
extern int close_flag_icbc_fsn_net();
extern int get_gbl_net_timeout();
extern int set_gbl_net_timeout(int second);
extern uint16 get_server_port(void);
extern int get_server_ip(char *str_ip,uint8 *p_ip);
extern void get_local_mac(char *str_mac,uint8 *p_mac);
extern int get_local_ip_info(char *str_ip,char *str_mk,char *str_gw,uint8 *p_ip,uint8 *p_mk,uint8 *p_gw);

extern void init_net_protocol_para(void);

extern int is_ok_date(char *date);
extern int set_os_sys_date(char *date);
extern void sys_ddms(void);
extern uint32 get_sys_dds(void);
extern uint32 get_sys_ddms(void);
extern void task_net_server(void *pdata);
extern void task_net_server_time(void *pdata);

extern int save_dat_file_to_send_buf(char *sub_data_buf,uint32 sub_data_len);


extern int save_local_ip_info(uint8 *p_ip,uint8 *p_mk,uint8 *p_gw);
extern int save_server_ip(uint8 *p_ip);
extern int save_server_port(uint16 port);
extern int get_net_protocol_info(uint8 *prot_type,uint8 *prot_mode);
extern int save_net_protocol_info(uint8 prot_type,uint8 prot_mode);
extern int get_net_switch(uint8 *tmp_switch);
extern int save_net_switch(uint8 tmp_switch);
extern int get_net_stop_time(NET_MASK_ONE_TIME *tmp_not_send_time,int num);
extern int save_net_stop_time(NET_MASK_ONE_TIME *tmp_not_send_time,int num);
extern int net_send_big_img(char *buf,int len,int recv_data_flag);
extern int set_recv_dat_file_flag_by_index(int index,uint8 tmp_valid_flag);
extern void get_save_para(void);
extern int change_ip_str_to_val(char *ipstr,uint8 *val);
extern int save_local_port(uint16 port);
extern uint16 get_local_port(void);






//========================================================================================







#endif
/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

