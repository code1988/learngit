/***************文件信息********************************************************************************
**文   件   名: net_server.h
**说        明:
**创   建   人: hxj
**创   建 日期: 2014-12-17 8:55
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
#define RECV_DAT_FILE_BUF_MAX               100         //接收dat文件的缓冲数
#define PARA_ONE_TIME_MONEY_MAX_NUM         300         //一次发送钞的最大张数


//========================================================================================
#pragma   pack(1)



//请求发送结构
typedef struct _NET_SIMPLE_
{
   uint32 start_head;           //起始标志      0x40404A4C
   uint32 msg_length;           //消息包总长度  定长
   uint16 version;              //协议版本      10
   uint16 cmd;                  //命令          0x00A1
   uint16 MachineNo[14];        //机具号        ASCII
   uint8  traceNumber[6];       //批次号        ASCII 动态增加 不足前面补'0' //同一机器同一天内批次号不能重复
   uint8  tranDate[8];          //验钞启动日期  ASCII
   uint8  tranTime[6];          //验钞启动时刻  ASCII
   uint16 totalRequestCounts;   //请求总次数    1
   uint16 requestNo;            //当前请求数    1
   uint8  bankNumber[8];        //8位网点编号   ASCII 不足前面补 '0'        (有)
   uint8  MachineType[2];       //设备类型      ASCII AD：A 类点钞机
   uint8  operatorId[12];       //操作员号      ASCII 不足右补 0x00         (无)
   uint8  company[4];           //4位厂商简称   ASCII FOTA
   uint16 packageIndex;         //包序号        0x0000
   uint16 reserve;              //保留字        0x0000
}NET_SIMPLE;


//结构体长度 94
typedef struct _NET_SIMPLE_C_
{
   uint8 start_head[4];         //起始标志      0x40404A4C
   uint8 msg_length[4];         //消息包总长度  定长
   uint8 version[2];            //协议版本      10
   uint8 cmd[2];                //命令          0x00A1
   uint8 MachineNo[28];         //机具号        ASCII
   uint8 traceNumber[6];        //批次号        ASCII 动态增加 不足前面补'0' //同一机器同一天内批次号不能重复
   uint8 tranDate[8];           //验钞启动日期  ASCII
   uint8 tranTime[6];           //验钞启动时刻  ASCII
   uint8 totalRequestCounts[2]; //请求总次数    1
   uint8 requestNo[2];          //当前请求数    1
   uint8 bankNumber[8];         //8位网点编号   ASCII 不足前面补 '0'        (有)
   uint8 MachineType[2];        //设备类型      ASCII AD：A 类点钞机
   uint8 operatorId[12];        //操作员号      ASCII 不足右补 0x00         (无)
   uint8 company[4];            //4位厂商简称   ASCII FOTA
   uint8 packageIndex[2];       //包序号        0x0000
   uint8 reserve[2];            //保留字        0x0000
}NET_SIMPLE_C;



//========================================================================================
//发送数据结构
typedef struct _NET_UP_
{
     uint32 start_head;         //起始标志      0x40404A4C
     uint32 msg_length;         //消息包总长度  动长 82+fileBody_len+2
     uint16 version;            //协议版本      10
     uint16 cmd;                //命令          0x0006 为号码记录
     uint16 MachineNo[14];      //机具号        ASCII
     uint8  code[20];           //业务类型      HM：号码记录，无业务时应为此类型  不足右补0x0       (有)
     uint16 packageIndex;       //文件分次发送时序号 0x0000
     uint8  reserve[20];        //保留位        0x00 ... 0x00(20个)
     uint8  fileBody[5000];     //报文体数据    动长 FSN格式
     uint16 checksum;           //校验码,(保留) 0x0000
}NET_UP;


//结构体长度 84
typedef struct _NET_UP_C_
{
     uint8 start_head[4];       //起始标志      0x40404A4C
     uint8 msg_length[4];       //消息包总长度  动长 82+fileBody_len+2
     uint8 version[2];          //协议版本      10
     uint8 cmd[2];              //命令          0x0006 为号码记录
     uint8 MachineNo[28];       //机具号        ASCII
     uint8 code[20];            //业务类型      HM：号码记录，无业务时应为此类型  不足右补0x0       (有)
     uint8 packageIndex[2];     //文件分次发送时序号 0x0000
     uint8 reserve[20];         //保留位        0x00 ... 0x00(20个)
     uint8 fileBody[2];         //报文体数据    动长 FSN格式
//     uint8 checksum[2];         //校验码,(保留) 0x0000
}NET_UP_C;




//========================================================================================
//发送结束后,服务器不再回复,服务器断开连接
//请求结束结构

typedef struct _NET_CLOSE_
{
    uint32 start_head;          //起始标志      0x40404A4C
    uint32 msg_length;          //消息包总长度  定长
    uint16 version;             //协议版本      10
    uint16 cmd;                 //命令          0x00A2
    uint16 MachineNo[14];       //机具号        ASCII
    uint16 packageIndex;        //文件分次发送时序号 0x0000
    uint16 reserve;             //保留位        0x0000
}NET_CLOSE;


//结构体长度 44
typedef struct _NET_CLOSE_C_
{
    uint8 start_head[4];        //起始标志      0x40404A4C
    uint8 msg_length[4];        //消息包总长度  定长
    uint8 version[2];           //协议版本      10
    uint8 cmd[2];               //命令          0x00A2
    uint8 MachineNo[28];        //机具号        ASCII
    uint8 packageIndex[2];      //文件分次发送时序号 0x0000
    uint8 reserve[2];           //保留位        0x0000

}NET_CLOSE_C;





//========================================================================================
//帧头结构 28
typedef struct _NET_HEAD_
{
    uint8  start_head[4];        //起始标志      "SPDB"
    uint32 msg_length;           //消息包总长度  总帧长
    uint8  fun_code[4];          //功能代码
    uint8  date_time[14];        //发送时间      格式为"YYYYMMDDhhmmss"
    uint16 domain;               //域个数
}NET_HEAD;



//帧头结构 28
typedef struct _NET_HEAD_C_
{
    uint8 start_head[4];        //起始标志      "SPDB"
    uint8 msg_length[4];        //消息包总长度  总帧长
    uint8 fun_code[4];          //功能代码
    uint8 date_time[14];        //发送时间      格式为"YYYYMMDDhhmmss"
    uint8 domain[2];            //域个数
}NET_HEAD_C;



//应答帧头结构 32
typedef struct _NET_HEAD_ACK_
{
    uint8  start_head[4];        //起始标志      "SPDB"
    uint32 msg_length;           //消息包总长度  总帧长
    uint8  fun_code[4];          //功能代码
    uint8  date_time[14];        //发送时间      格式为"YYYYMMDDhhmmss"
    uint8  err_code[4];          //错误代码      AAAA-成功,E001-E006 E101-E103
    uint16 domain;               //域个数
}NET_HEAD_ACK;


//应答帧头结构 32
typedef struct _NET_HEAD_ACK_C_
{
    uint8 start_head[4];        //起始标志      "SPDB"
    uint8 msg_length[4];        //消息包总长度  总帧长
    uint8 fun_code[4];          //功能代码
    uint8 date_time[14];        //发送时间      格式为"YYYYMMDDhhmmss"
    uint8 err_code[4];          //错误代码      AAAA-成功,E001-E006 E101-E103
    uint8 domain[2];            //域个数
}NET_HEAD_ACK_C;




//帧结构 28+正文
typedef struct _NET_FRAME_
{
    uint8 start_head[4];        //起始标志      "SPDB"
    uint32 msg_length;          //消息包总长度  总帧长
    uint16 fun_code;            //功能代码
    uint8 date_time[14];        //发送时间      格式为"YYYYMMDDhhmmss"
    uint16 domain;              //域个数
    uint8 data[1024];           //正文数据
}NET_FRAME;



//帧结构 28+正文
typedef struct _NET_FRAME_C_
{
    uint8 start_head[4];        //起始标志      "SPDB"
    uint8 msg_length[4];        //消息包总长度  总帧长
    uint8 fun_code[4];          //功能代码
    uint8 date_time[14];        //发送时间      格式为"YYYYMMDDhhmmss"
    uint8 domain[2];            //域个数
    uint8 data[1024];           //正文数据
}NET_FRAME_C;




//应答帧结构 28+正文
typedef struct _NET_FRAME_ACK_
{
    uint8 start_head[4];        //起始标志      "SPDB"
    uint32 msg_length;          //消息包总长度  总帧长
    uint8 fun_code[4];          //功能代码
    uint8 date_time[14];        //发送时间      格式为"YYYYMMDDhhmmss"
    uint8 err_code[4];          //错误代码      AAAA-成功,E001-E006 E101-E103
    uint8 domain[2];            //域个数
    uint8 data[1024];           //正文数据
}NET_FRAME_ACK;




//应答帧结构 28+正文
typedef struct _NET_FRAME_ACK_C_
{
    uint8 start_head[4];        //起始标志      "SPDB"
    uint8 msg_length[4];        //消息包总长度  总帧长
    uint8 fun_code[4];          //功能代码
    uint8 date_time[14];         //发送时间      格式为"YYYYMMDDhhmmss"
    uint8 err_code[4];          //错误代码       AAAA-成功,E001-E006 E101-E103
    uint8 domain[2];            //域个数
    uint8 data[1024];           //正文数据
}NET_FRAME_ACK_C;


//正文帧结构
typedef struct _NET_BODY_
{
    uint8  body_head[6];            //正文标志
    uint32 body_data_length;        //正文长度
    uint8  body_data[200];          //正文数据
}NET_BODY;



//正文帧头结构 10
typedef struct _NET_BODY_HEAD_C_
{
    uint8  body_head[6];            //正文标志
    uint8  body_data_length[4];     //正文长度
}NET_BODY_HEAD_C;




//正文帧结构
typedef struct _NET_BODY_C_
{
    uint8  body_head[6];            //正文标志
    uint8  body_data_length[4];     //正文长度
    uint8  body_data[200];          //正文数据
}NET_BODY_C;



//正文帧结构
typedef struct _MACH_INFO_C_
{
    uint8  mach_code[25];               //机具编号
    uint8  g_time_setup[15];            //启动时间
    uint8  g_local_sn[4];               //地区号(4)
    uint8  g_sub_dank_sn[4];            //支行号(4)
    uint8  g_net_sn[4];                 //网点号(4)

    uint8  SN_NetSend_MachineNo[8];
    uint8  opt_sn[8];                   //操作员号
    uint8  atm_transation[20];          //atm交易码
    uint32 not_download_file_num;       //用于网发的没有下载的文件数
    uint32 arm_not_download_file_num;   //ARM返回的没有下载的文件数

    uint8  arm_ack_not_download_file_num;   //ARM确认的没有下载的文件数
    uint8  arm_ack_opt_sign_in;         //ARM确认操作员签到
    uint8  arm_ack_opt_sign_out;        //ARM确认操作员退签
    uint8  arm_ack_trade_start;         //ARM确认启动交易点钞
    uint8  arm_ack_trade_end;           //ARM确认结束交易点钞
    uint8  arm_ack_file_download_ok;    //ARM确认文件已经下载
    uint8  arm_ack_save_file;           //ARM确认立即生成文件

    uint8  cur_client_over_file;        //当前的客户端结束标志


}MACH_INFO_C;


//正文帧结构
typedef struct _DSP_DATE_
{
    char   valid_flag;                  //有效标志
    char  date_str[15];                 //当前时间  14长度,格式为"YYYYMMDDhhmmss"
    uint32 cur_s_cnt;                   //获取  llTimerGetTime(NULL); 1s
    uint16 year;
    uint16 month;
    uint16 day;
    uint16 time_h;
    uint16 time_m;
    uint16 time_s;

}DSP_DATE;



//正文帧结构
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
    char  mach_code_str[24+1];               //机具编号
    char  opt_sn_str[8+1];                   //操作员号
    char  atm_transation_str[20+1];          //atm交易码
    char  dat_file_time_str[14+1];             //当前时间  14长度,格式为"YYYYMMDDhhmmss"
    //add
    uint8 local_sn[4];      //地区号(4)
    uint8 sub_dank_sn[4];   //支行号(4)
    uint8 net_sn[4];        //网点号(4)


}DSP_upload_head;


//========================================================================================
//spi frame
typedef struct _SPI_CMD_EX_C_
{
    uint8  data_src_flag[4];            //数据源标识
    uint8  frame_sn[2];                 //包序
    uint8  ack_flag;                    //应答标志
    uint8  rw_flag;                     //读写标志(1B) (0-读，1-写)
    uint8  sub_cmd[2];                  //子命令
    uint8  sub_data_len[4];             //子数据长度
    uint8  sub_data[100];               //子数据
}SPI_CMD_EX_C;


//========================================================================================
//原 fsn 定义
typedef struct {
	Uint16  HeadStart[4];
	Uint16  HeadString[6];
	Uint32  Counter;
	Uint16  HeadEnd[4];
} FSN_HEAD, *P_FSN_HEAD;

typedef struct {
  Uint32 Data[32];// 图像冠字号码点阵数据
} TImgSNoData;

typedef struct {
  Int16 Num;			//字符数
  Int16 height, width;	//每个图像字符高度和宽度
  Uint16 Reserve2;       //保留字2
  TImgSNoData SNo[12];
} TImageSNo;

typedef struct
{
  Uint16 Date;		  	//验钞启动日期
  Uint16 Time;		  	//验钞启动时间
  Uint16 tfFlag;		//真、假、残和旧币标志
  Uint16 ErrorCode[3];  //错误码(3个)
  Uint16 MoneyFlag[4]; 	//货币标志
  Uint16 Ver;           //版本号
  Uint16 Valuta;		//币值
  Uint16 CharNUM;	  	//冠字号码字符数
  Uint16 SNo[12];		//冠字号码
  Uint16 MachineSNo[24];//机具编号
  Uint16 Reserve1;       //保留字1
  TImageSNo ImageSNo;//图像冠字号码
} FSN_DATA, *P_FSN_DATA;





//========================================================================================
//fsn 文件结构定义 (由于fsn的结构在DSP中,刚好都是对齐的)

// 单个图像号码结构
//结构体长度 128
typedef struct _STR_ONE_SN_IMAGE_DATA_
{
    uint32 Data[32];        // 图像冠字号码点阵数据
} ATT_BYTE_NO_ALIGNED
STR_ONE_SN_IMAGE_DATA;


//图像冠字号码结构，最多12位号码
//结构体长度 1544
typedef struct _STR_FSN_BODY_BODY_
{
    uint16 Num;             //字符数
    uint16 height;          //每个图像字符高度
    uint16 width;           //每个图像字符宽度
    uint16 Reserve2;        //保留字2
    STR_ONE_SN_IMAGE_DATA SNo[12];
} ATT_BYTE_NO_ALIGNED
STR_FSN_BODY_BODY;


//结构体长度 100
typedef struct _STR_FSN_BODY_HEAD_
{
    uint16 Date;            //验钞启动日期
    uint16 Time;            //验钞启动时间
    uint16 tfFlag;          //真、假、残和旧币标志
    uint16 ErrorCode[3];    //错误码(3个)
    uint16 MoneyFlag[4];    //货币标志 CNY
    uint16 Ver;             //版本号,0-1990,1-1999,2-2005,9999
    uint16 Valuta;          //币值,1,5,10,20,50,100
    uint16 CharNUM;         //冠字号码字符数,10
    uint16 SNo[12];         //冠字号码
    uint16 MachineSNo[24];  //机具编号
    uint16 Reserve1;        //保留字     1
}ATT_BYTE_NO_ALIGNED
STR_FSN_BODY_HEAD;


//结构体长度 1644
typedef struct _STR_FSN_BODY_
{
	STR_FSN_BODY_HEAD 	data_head;
    STR_FSN_BODY_BODY 	data_body;     //图像冠字号码
}ATT_BYTE_NO_ALIGNED
STR_FSN_BODY;


//结构体长度 32
typedef struct _STR_FSN_HEAD_
{
    uint16  HeadStart[4];       //20,10,7,26
    uint16  HeadString[6];      //0x00 0x01 (0x2e/0x2f) 'S' 'N' 'o'
    uint32  Counter;            //
    uint16  HeadEnd[4];         //0x00 0x01 0x02 0x03
}ATT_BYTE_NO_ALIGNED
STR_FSN_HEAD;


//结构体长度 493232=32+1644*300
typedef struct _STR_FSN_FILE_
{
	STR_FSN_HEAD	fsn_head;
	STR_FSN_BODY	fsn_body[PARA_ONE_TIME_MONEY_MAX_NUM];
}ATT_BYTE_NO_ALIGNED
STR_FSN_FILE;



//========================================================================================
//dat 文件结构定义

// size 250
typedef struct _DAT_DATA_INFO_C_
{
	uint8 res_1[5];				//保留5,没用
	uint8 money_sn[10];			//10字节为冠字号
	uint8 money_val;			//币值 DAT文件中直接存1-100的值,DSP发送过来的是1~6分别对应:1,5,10,20,50,100元
	uint8 money_ver;			//版本,0-1990,1-1999,2-2005
	uint8 money_flag;			//真假，真为1
	uint8 money_sn_pic[232];	//232字节的抠图(压缩过的)
}DAT_DATA_INFO_C;


//size 48
typedef struct _DAT_HEAD_ADD_C_
{
	//add head
    uint8  opt_sn[8];                   //操作员号
    uint8  atm_transation[20];          //atm交易码
	//uint8  res_7[20];                   //add 用了12个,地区号(4),支行号(4),网点号(4)
    uint8 local_sn[4];                  //地区号(4)
    uint8 sub_dank_sn[4];               //支行号(4)
    uint8 net_sn[4];                    //网点号(4)
    uint8  res_8[8];

}DAT_HEAD_ADD_C;


//size 65
typedef struct _DAT_HEAD_OLD_C_
{
	//old head
    uint8  head_flag;          		//头标志,固定0x01，表示头数据
	uint8  res_1[4];          		//保留4,没用
	uint8  res_2[11]; 				//保留11,没用
	uint8  money_val;				//保留1,没用
	uint8  all_num[2];			    //总张数
	uint8  mach_code[24];           //机具编号
	uint8  time_setup[14];			//点钞时间,14字节为时间
	uint8  res_4[6];				//保留6,没用
	uint8  res_5[1];				//保留1,没用
	uint8  res_6[1];				//保留1,没用

}DAT_HEAD_OLD_C;



typedef struct _DAT_FILE_FORMAT_C_
{
    DAT_HEAD_OLD_C head_old;            //之前的头
	//add head
	DAT_HEAD_ADD_C head_add;	        //增加的头
	//data
	DAT_DATA_INFO_C data_info[PARA_ONE_TIME_MONEY_MAX_NUM];     //钱的具体信息

}DAT_FILE_FORMAT_C;


//定义接收dat文件的缓冲结构
typedef struct _DAT_FILE_BUF_
{
volatile    uint8  dat_valid_flag;      //有效标志  //0-空的,1-填好数据还没有发送,2-填好的数据正在发送,3-由DSP确认,但ARM没确认
    uint32 dat_len;             		//dat 长度
    uint32 money_cnt;                   //钱张数
    int    change_time;                 //转换的参考时间
    uint32 send_time_out_dds;           //发送超时时间
    char   time[15];                    //dat 文件时间,从dat文件中获得，14字符有效
    DAT_FILE_FORMAT_C dat_buf; 	        //dat 內容
}DAT_FILE_BUF;


typedef struct _DAT_TIME_FIND_
{
    int    i_val;
    int    change_time;                 //转换的参考时间
}DAT_TIME_FIND;



//====================================

//busi 头信息

//结构体长度 60
typedef struct _busi_head_
{
    uint8 local_sn[4];      //地区号(4)
    uint8 sub_dank_sn[4];   //支行号(4)
    uint8 net_sn[4];        //网点号(4)
    uint8 fact_sn[2];       //厂家编号(2) "10(2)"
    uint8 mach_sn[10];      //机器编号(10) "0000A00850(10)"

    uint8 busi_type[2];     //业务类型  "11"
    uint8 mach_type[2];     //设备类型  //03-清分机,//05-A类机
    uint8 card_sn[19];      //卡号
    uint8 Transaction_code[5];      //交易代码
    uint8 mach_s_num[6];            //端机流水号
    uint8 rh_busi_type[2];          //人行业务类型 "GM"

}ATT_BYTE_NO_ALIGNED
busi_head;


//busi 头信息

//结构体长度 52
typedef struct _busi_info_
{
    uint8 time[14];             //记录时间(YYYYMMDDhhmmss)
    uint8 SNo[10];              //冠字号码
    uint8 MoneyFlag[3];         //货币标志 "CNY"
    uint8 Valuta[3];            //币值             "050"
    uint8 Ver[4];               //版本号       '''1999'
    uint8 tfFlag;               //真、假、残和旧币标志
    uint8 new_old[2];           //成色
    uint8 cnt_num[5];           //序号
    uint8 res[10];              //备用
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
//内部用
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

