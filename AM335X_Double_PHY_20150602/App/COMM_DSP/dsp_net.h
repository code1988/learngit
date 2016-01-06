/***************文件信息********************************************************************************
**文   件   名: dsp_net.h
**说        明:
**创   建   人: hxj
**创   建 日期: 2014-12-24 14:41
*******************************************************************************************************/
#ifndef	__INC_DSP_NET_H
#define	__INC_DSP_NET_H

#include "def_config.h"
#include "in_comm_protocol.h"

//========================================================================================
//

#define CS_CHECK_START 0x02

//========================================================================================

#pragma   pack(1)


typedef	struct	_strRecvDSPIMG_
{
	uint16 index;               //抠图序号(2B)
    uint8 img_data[180];        //抠图(180B)
	uint8 money_sn[10];			//10字节为冠字号
	uint8 money_val;			//币值,DSP发送过来的是1~6分别对应:1,5,10,20,50,100元
	uint8 money_ver;			//版本,(0,1,2)0-1990,1-1999,2-2005
	uint8 money_flag;			//真假，真为1

}strRecvDSPIMG;

// size 90
typedef	struct	_strRecvBasicInfo_
{
    uint8 sys_time[14];             //同步时间
	uint8 mach_sn[24];			    //机具编号
	uint8 user_name[20];		    //用户名
	uint8 user_passwd[20];		    //密码
    uint8 local_sn[4];              //地区号(4)
    uint8 sub_dank_sn[4];           //支行号(4)
    uint8 net_sn[4];                //网点号(4)

}strRecvBasicInfo;


//size 26
typedef	struct	_strIPPara_
{
    uint8 ip[4];                //ip
	uint8 mk[4];			    //mask
    uint8 gw[4];                //gw
    uint16 port;                //port
    uint8 mip[4];               //mip
    uint16 mport;               //mport
    uint8 mac[6];               //mac

}strIPPara;






#pragma   pack()

//========================================================================================







extern strRecvCycBuf_N *g_p_dsp_net_recv_cbuf;
extern strCmdIdList dsp_id_wr_list[];
extern VAL_VOL uint8 g_send_big_img_flag;
extern uint8 g_update_a8_success_flag;
extern uint8 g_sd_res_alarm_thr;
extern uint8 g_sd_res_alarm_up_status;
extern uint32 g_sd_all_size;
extern uint32 g_sd_res_size;
extern uint32 g_sd_send_ok_del_day;      //单位天
extern uint32 g_sd_send_err_del_day;     //单位天




extern int init_dat_file(void);
extern void dsp_net_init_buf(void);
extern void dsp_net_recv_data_to_cyc_buf_n (uint8 *buf, uint32 len );
extern void dsp_net_recv_data_to_cyc_buf_n2 (uint8 *buf, uint32 len );
extern void dsp_net_find_frame(void);
extern void task_dsp_net_server_time(void *pdata);


extern int get_sd_res_alarm_thr(uint8 *tmp_val);
extern int save_sd_res_alarm_thr(uint8 tmp_val);
extern int get_sd_size_info(uint32 *tmp_all_size_val,uint32 *tmp_res_size_val);





extern int SET_ID_DSP_IMG ( strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_CNT_MONEY_START(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_CNT_MONEY_END(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_BASIC_INFO(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );

extern int GET_ID_DSP_VER(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );




extern int GET_ID_DSP_NET_PARA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_NET_PARA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int GET_ID_DSP_NET_SEND_PROTOCOL(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_NET_SEND_PROTOCOL(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int GET_ID_DSP_NET_SEND_SWITCH(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_NET_SEND_SWITCH(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int GET_ID_DSP_NET_SEND_MASK_TIME(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_NET_SEND_MASK_TIME(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_BIG_IMG_START(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_BIG_IMG_DATA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_BIG_IMG_END(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int GET_ID_DSP_UPDATE_A8(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int GET_ID_DSP_SYNC_TIME(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int GET_ID_DSP_SD_RES_ALARM_THR(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_SD_RES_ALARM_THR(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int GET_ID_DSP_SD_RES_ALARM_UP(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int GET_ID_DSP_SD_RES_SIZE(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_SD_DEL_DATA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int GET_ID_DSP_SD_OPT_PARA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_SD_OPT_PARA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_CHECK_CS(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );



#if FZ1500
extern int SET_RET_ID_DSP_CHECK_CS_ACK(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int SET_ID_DSP_UPDATE_STATUS(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
extern int GET_RET_ID_DSP_VER(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len );
#endif




extern void TEST_SET_ID_DSP_BASIC_INFO(void);



#endif
/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

