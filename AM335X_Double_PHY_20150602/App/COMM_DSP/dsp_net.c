/***************文件信息********************************************************************************
**文   件   名: dsp_net.c
**说        明:
**创   建   人: hxj
**创   建 日期: 2014-12-24 14:41
*******************************************************************************************************/
#include "dsp_net.h"
#include "def_config.h"
#include "in_comm_protocol.h"
#include "dsp_comm.h"
#include "net_server.h"
#include "mmcsdmain.h"
#include "compile_time.h"
#include "version.h"
#include "bsp_conf.h"
#include "save_para.h"
#include "app.h"







strRecvCycBuf_N *g_p_dsp_net_recv_cbuf=NULL;
strRecvCycBuf_N *g_p_dsp_net_recv_cbuf2=NULL;

DAT_FILE_FORMAT_C *g_p_dat_file=NULL;
DAT_FILE_FORMAT_C *g_p_dat_file_cp=NULL;

uint8 g_data_info_flag[PARA_ONE_TIME_MONEY_MAX_NUM]={0};
VAL_VOL int g_dsp_send_money_status=0;
VAL_VOL char g_recv_dsp_send_basic_info_flag=0;

VAL_VOL uint8 g_send_big_img_flag=0;
VAL_VOL uint8 g_send_big_img_stop_fsn_send_flag=0;
uint32  g_send_big_img_delay_cnt=0;     //单位为 1s

uint32 g_big_img_cnt=0;

uint8 g_update_a8_success_flag=0;

uint8 g_sd_res_alarm_thr=0;         //1-99百分比
uint8 g_sd_res_alarm_up_status=0;   //0-正常,1-告警
uint32 g_sd_all_size=0;             //单位M
uint32 g_sd_res_size=0;             //单位M

uint32 g_sd_send_ok_del_day=0;      //单位天
uint32 g_sd_send_err_del_day=0;     //单位天






strCmdIdList dsp_id_wr_list[] =
{

    //------------------------------------------------------------------------------------------------------------------------
#if FZ1500
    {INIT_DATA_ID(ID_DSP_UPDATE_STATUS,0,2,CK_ON,1,RT_ON,NULL,SET_ID_DSP_UPDATE_STATUS,CK_OF,0,CK_OF,0,NULL,NULL)},                                                          //0x3201          //app版本
#endif

    {INIT_DATA_ID(ID_DSP_UPDATE_A8,0,0,CK_ON,1,RT_ON,GET_ID_DSP_UPDATE_A8,NULL,CK_OF,0,CK_OF,0,NULL,NULL)},                                                          //0x3201          //app版本

    //ver id
#if FZ1500
    {INIT_DATA_ID(ID_DSP_VER,0,0,CK_ON,44,RT_ON,GET_ID_DSP_VER,NULL,CK_ON,44,CK_OF,0,GET_RET_ID_DSP_VER,NULL)},
    {INIT_DATA_ID(ID_DSP_CHECK_CS,0,1,CK_ON,0,RT_OF,NULL,SET_ID_DSP_CHECK_CS,CK_OF,0,CK_OF,0,NULL,NULL)},
#else
    {INIT_DATA_ID(ID_DSP_VER,0,0,CK_ON,44,RT_ON,GET_ID_DSP_VER,NULL,CK_OF,44,CK_OF,0,NULL,NULL)},
#endif

    //0x3201          //app版本
    //------------------------------------------------------------------------------------------------------------------------
    //net para
    {INIT_DATA_ID(ID_DSP_NET_PARA,0,sizeof(strIPPara),CK_ON,sizeof(strIPPara),RT_ON,GET_ID_DSP_NET_PARA,SET_ID_DSP_NET_PARA,CK_OF,0,CK_OF,0,NULL,NULL)},  //0x3301          //网络参数
    //------------------------------------------------------------------------------------------------------------------------
    //net send
    {INIT_DATA_ID(ID_DSP_NET_SEND_PROTOCOL,0,2,CK_ON,2,RT_ON,GET_ID_DSP_NET_SEND_PROTOCOL,SET_ID_DSP_NET_SEND_PROTOCOL,CK_OF,0,CK_OF,0,NULL,NULL)},       //0x3401          //网络协议,0-工行协议,1-光荣协议,2-维融协议,3-中信协议
    {INIT_DATA_ID(ID_DSP_NET_SEND_SWITCH,0,1,CK_ON,1,RT_ON,GET_ID_DSP_NET_SEND_SWITCH,SET_ID_DSP_NET_SEND_SWITCH,CK_OF,0,CK_OF,0,NULL,NULL)},             //0x3402          //网发开关,0-网发关,1-网发开
    {INIT_DATA_ID(ID_DSP_NET_SEND_MASK_TIME,0,40,CK_ON,40,RT_ON,GET_ID_DSP_NET_SEND_MASK_TIME,SET_ID_DSP_NET_SEND_MASK_TIME,CK_OF,0,CK_OF,0,NULL,NULL)},  //0x3403          //不网发时段
    //------------------------------------------------------------------------------------------------------------------------
    //img id
    {INIT_DATA_ID(ID_DSP_CNT_MONEY_START,0,14,CK_ON,0,RT_ON,NULL,SET_ID_DSP_CNT_MONEY_START,CK_OF,0,CK_OF,0,NULL,NULL)},              //0x3501          //点钞起始时间
    {INIT_DATA_ID(ID_DSP_CNT_MONEY_END,0,0,CK_ON,0,RT_ON,NULL,SET_ID_DSP_CNT_MONEY_END,CK_OF,0,CK_OF,0,NULL,NULL)},                   //0x3502          //点钞结束
    {INIT_DATA_ID(ID_DSP_IMG,0,sizeof(strRecvDSPIMG),CK_ON,0,RT_ON,NULL,SET_ID_DSP_IMG,CK_OF,0,CK_OF,0,NULL,NULL)},                   //0x3503          //实时抠图


    {INIT_DATA_ID(ID_DSP_SYNC_TIME,0,0,CK_ON,14,RT_ON,GET_ID_DSP_SYNC_TIME,NULL,CK_OF,0,CK_OF,0,NULL,NULL)},                          //0x3505          //同步時間

    {INIT_DATA_ID(ID_DSP_BASIC_INFO,0,sizeof(strRecvBasicInfo),CK_ON,0,RT_ON,NULL,SET_ID_DSP_BASIC_INFO,CK_OF,0,CK_OF,0,NULL,NULL)},  //0x3504          //开机基本信息
    {INIT_DATA_ID(ID_DSP_BIG_IMG_START,0,0,CK_OF,0,RT_OF,NULL,SET_ID_DSP_BIG_IMG_START,CK_OF,0,CK_OF,0,NULL,NULL)},                   //0x3506          //大图开始
    {INIT_DATA_ID(ID_DSP_BIG_IMG_DATA,0,0,CK_OF,0,RT_OF,NULL,SET_ID_DSP_BIG_IMG_DATA,CK_OF,0,CK_OF,0,NULL,NULL)},                     //0x3507          //大图数据
    {INIT_DATA_ID(ID_DSP_BIG_IMG_END,0,0,CK_OF,0,RT_OF,NULL,SET_ID_DSP_BIG_IMG_END,CK_OF,0,CK_OF,0,NULL,NULL)},                       //0x3508          //大图结束



    //------------------------------------------------------------------------------------------------------------------------
    //sd id
    {INIT_DATA_ID(ID_DSP_SD_RES_ALARM_THR,0,1,CK_ON,1,RT_ON,GET_ID_DSP_SD_RES_ALARM_THR,SET_ID_DSP_SD_RES_ALARM_THR,CK_OF,0,CK_OF,0,NULL,NULL)},                                  //0x3601          //SD卡剩余容量告警值,百分比(1-99)
    {INIT_DATA_ID(ID_DSP_SD_RES_ALARM_UP,0,0,CK_ON,1,RT_ON,GET_ID_DSP_SD_RES_ALARM_UP,NULL,CK_OF,0,CK_OF,0,NULL,NULL)},                                   //0x3602          //SD卡剩余容量告警,百分比(1-99)
    {INIT_DATA_ID(ID_DSP_SD_RES_SIZE,0,0,CK_ON,8,RT_ON,GET_ID_DSP_SD_RES_SIZE,NULL,CK_OF,0,CK_OF,0,NULL,NULL)},                                       //0x3603          //SD余容量,单位为 M，
    {INIT_DATA_ID(ID_DSP_SD_DEL_DATA,0,0,CK_ON,0,RT_ON,NULL,SET_ID_DSP_SD_DEL_DATA,CK_OF,0,CK_OF,0,NULL,NULL)},                                       //0x3604          //删除SD数据
    {INIT_DATA_ID(ID_DSP_SD_OPT_PARA,0,8,CK_ON,8,RT_ON,GET_ID_DSP_SD_OPT_PARA,SET_ID_DSP_SD_OPT_PARA,CK_OF,0,CK_OF,0,NULL,NULL)},                                       //0x3605          //SD卡操作参数
    //------------------------------------------------------------------------------------------------------------------------

    //end flag
    {INIT_END_DATA_ID()},


};



void dsp_net_init_buf(void)
{

#if 0==DBG_MASK_DSP_NET1_EN
    g_p_dsp_net_recv_cbuf=malloc_cycbuf_n(PARA_SIZE_DSP_RECV_CBUF,PARA_SIZE_DSP_RECV_ONE_FRAME);
    if(NULL == g_p_dsp_net_recv_cbuf)
    {
        DBG_SYS_ERROR("in dsp_net_init_buf 1_1:malloc g_p_dsp_net_recv_cbuf ok\r\n");
    }
    else
    {
        DBG_PRO("in dsp_net_init_buf 1_2:malloc ok\r\n");
    }
#endif

#if 0==DBG_MASK_DSP_NET2_EN
    g_p_dsp_net_recv_cbuf2=malloc_cycbuf_n(PARA_SIZE_DSP_RECV_CBUF,PARA_SIZE_DSP_RECV_ONE_FRAME);
    if(NULL == g_p_dsp_net_recv_cbuf2)
    {
        DBG_SYS_ERROR("in dsp_net_init_buf 1_3:malloc g_p_dsp_net_recv_cbuf ok\r\n");
    }
    else
    {
        DBG_PRO("in dsp_net_init_buf 1_4:malloc ok\r\n");
    }
#endif


    g_p_dat_file=(DAT_FILE_FORMAT_C *)malloc(sizeof(DAT_FILE_FORMAT_C)*2);
    if(NULL==g_p_dat_file)
    {
        DBG_SYS_ERROR("in dsp_net_init_buf 2_1:malloc g_p_dat_file ok\r\n");
    }
    else
    {
        memset((char *)g_p_dat_file,0,sizeof(DAT_FILE_FORMAT_C)*2);
    }

    init_dat_file();

    display_id_list(dsp_id_wr_list,"dsp_id_wr_list");

}




void dsp_net_recv_data_to_cyc_buf_n (uint8 *buf, uint32 len )
{
    recv_data_to_cyc_buf_n (g_p_dsp_net_recv_cbuf,buf,len);
}


void dsp_net_recv_data_to_cyc_buf_n2 (uint8 *buf, uint32 len )
{
    recv_data_to_cyc_buf_n (g_p_dsp_net_recv_cbuf2,buf,len);
}


void dsp_net_find_frame(void)
{
    if(NULL == g_p_dsp_net_recv_cbuf) return;
    find_frame_by_prot_fota ( g_p_dsp_net_recv_cbuf,1);
}


void dsp_net_find_frame2(void)
{
    if(NULL == g_p_dsp_net_recv_cbuf2) return;
    find_frame_by_prot_fota ( g_p_dsp_net_recv_cbuf2,2);
}




uint16 get_index_dsp_net_frame_num(void)
{
    static uint16 index=0x7fff;

    index++;
    if(index<0x8000) index=0x8000;
    return index;
}




int dsp_net_send_data(uint8 *buf,uint32 len)
{
    if(NULL== buf) return -1;
    if(0== len)    return 0;

#if DISPLAY_DSP_SEND_DATA_EN
    DBG_PRO("in dsp_net_send_data 1:send len=%d \r\n",len);
    DBG_BUF_PRO(buf,len,"in dsp_net_send_data 2:");
#endif

    return send_dsp1_data(buf,len);

}



int dsp_net_send_data2(uint8 *buf,uint32 len)
{
    if(NULL== buf) return -1;
    if(0== len)    return 0;

#if DISPLAY_DSP_SEND_DATA_EN
    DBG_PRO("in dsp_net_send_data2 1:send len=%d \r\n",len);
    DBG_BUF_PRO(buf,len,"in dsp_net_send_data2 2:");
#endif

    return send_dsp2_data(buf,len);

}



void dsp_net_frame_manage(void)
{
    if(NULL == g_p_dsp_net_recv_cbuf) return;

    strOneFrameData_N *pframeData=&g_p_dsp_net_recv_cbuf->frameData;
    strCmdIdList *p_tmp_id_wr_list=dsp_id_wr_list;
    P_FUN_SEND send_fun=dsp_net_send_data;
    strRecvFotaFrame *p_buf_c=NULL;
    int ret_pre=0;
    int i;
    int call_ret;
    uint8 fun_code=0;
    uint32 msg_length=0;
    int recv_data_len=0;


    strOneFrameData_N tmp_one_frame;
    static uint8 *tmp_one_frame_frame_data=NULL;
    if(NULL==tmp_one_frame_frame_data)
    {
        tmp_one_frame_frame_data=(uint8 *)malloc(PARA_SIZE_DSP_SEND_ONE_FRAME);
        if(NULL==tmp_one_frame_frame_data)
        {
            DBG_ALARM("malloc err\r\n");
            return;
        }
    }

    tmp_one_frame.frame_data_size=PARA_SIZE_DSP_SEND_ONE_FRAME;
    tmp_one_frame.frame_data=tmp_one_frame_frame_data;

    p_buf_c=(strRecvFotaFrame *)pframeData->frame_data;
    msg_length=pframeData->frame_len;
    recv_data_len=msg_length-(sizeof(strRecvFotaFrameHead)+FOTA_LEN_DTAT_ID+0+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG);

    if(1==pframeData->frame_flag)
    {
        ret_pre=fota_frame_pre_manage(pframeData,p_tmp_id_wr_list,send_fun,0x8000,0xffff);
        if(1==ret_pre)
        {
            fun_code=p_buf_c->head_data.h_fun & PARA_FUN_MASK;
            for(i=0;NULL != p_tmp_id_wr_list[i].id_des;++i)
            {
                if( p_buf_c->frame_id == p_tmp_id_wr_list[i].id_data )
                {

                    if(ACK_OK != p_buf_c->head_data.h_ack)
                    {
                        //对方的读写判断

                        if(PARA_FUN_RD == fun_code)
                        {
                            if(NULL !=  p_tmp_id_wr_list[i].get_param)
                            {
                                call_ret=p_tmp_id_wr_list[i].get_param(pframeData,&tmp_one_frame,p_buf_c->frame_id,p_tmp_id_wr_list[i].id_ret_len);
                                if(ACK_OK==call_ret)
                                {
                                    create_ack_frame_by_read(pframeData,&tmp_one_frame);
                                    if(RT_ON==p_tmp_id_wr_list[i].check_ret_len_en)
                                    {
                                        int check_msg_length=sizeof(strRecvFotaFrameHead)+(FOTA_LEN_DTAT_ID+p_tmp_id_wr_list[i].id_ret_len)+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG;
                                        if(check_msg_length != tmp_one_frame.frame_len)
                                        {
                                            DBG_PRO("in dsp_net_frame_manage 2_0:read return id data len err,id=0x%04x\r\n",p_tmp_id_wr_list[i].id_data);
                                            DBG_PRO("in dsp_net_frame_manage 2_1:check_msg_length=%d,tmp_one_frame.frame_len=%d\r\n",check_msg_length,tmp_one_frame.frame_len);
                                            break;
                                        }
                                    }
                                    send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                                }
                                else
                                {
                                    DBG_PRO("in dsp_net_frame_manage 2_2:get id(0x%04X),call_ret=%d \r\n",p_buf_c->frame_id,call_ret);
                                }
                            }
                        }
                        else if(PARA_FUN_WT== fun_code)
                        {
                            if(NULL != p_tmp_id_wr_list[i].set_param )
                            {
                                call_ret=p_tmp_id_wr_list[i].set_param(pframeData,&tmp_one_frame,p_buf_c->frame_id,recv_data_len);
                                if(ACK_OK==call_ret)
                                {
                                    if(ACK_NOT_RET != p_buf_c->head_data.h_ack)
                                    {
                                        create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_OK);
                                        send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                                    }
                                }
                                else if(ACK_ERR_ID_DATA==call_ret)
                                {
                                    DBG_PRO("in dsp_net_frame_manage 3_1:id data len err,id=0x%04x\r\n",p_tmp_id_wr_list[i].id_data);
                                    create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_ID_DATA);
                                    send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                                }
                                else if(ACK_ERR_OHTER==call_ret)
                                {
                                    if(ACK_NOT_RET != p_buf_c->head_data.h_ack)
                                    {
                                        create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_OHTER);
                                        send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                                    }
                                }
                                else
                                {
                                    DBG_PRO("in dsp_net_frame_manage 3_2:set id(0x%04X),call_ret=%d \r\n",p_buf_c->frame_id,call_ret);
                                }
                            }

                        }

                    }
                    else
                    {
                        //自己的读写返回帧处理

                        if(PARA_FUN_RD == fun_code)
                        {
                            if(NULL !=  p_tmp_id_wr_list[i].self_get_param)
                            {
                                call_ret=p_tmp_id_wr_list[i].self_get_param(pframeData,&tmp_one_frame,p_buf_c->frame_id,recv_data_len);
                                if(ACK_OK!=call_ret)
                                {
                                    DBG_ALARM("in dsp_net_frame_manage 4_1:return read manage err,id=0x%04x,call_ret=%d\r\n",p_tmp_id_wr_list[i].id_data,call_ret);
                                }
                            }
                        }
                        else if(PARA_FUN_WT== fun_code)
                        {
                            if(NULL != p_tmp_id_wr_list[i].self_set_param )
                            {
                                call_ret=p_tmp_id_wr_list[i].self_set_param(pframeData,&tmp_one_frame,p_buf_c->frame_id,recv_data_len);
                                if(ACK_OK!=call_ret)
                                {
                                    DBG_ALARM("in dsp_net_frame_manage 4_2:return write manage err,id=0x%04x,call_ret=%d\r\n",p_tmp_id_wr_list[i].id_data,call_ret);
                                }

                            }

                        }

                    }

                    break;

                }

            }

        }

        pframeData->frame_flag=0;

    }

    return;

}







void dsp_net_frame_manage2(void)
{
    if(NULL == g_p_dsp_net_recv_cbuf2) return;

    strOneFrameData_N *pframeData=&g_p_dsp_net_recv_cbuf2->frameData;
    strCmdIdList *p_tmp_id_wr_list=dsp_id_wr_list;
    P_FUN_SEND send_fun=dsp_net_send_data2;
    strRecvFotaFrame *p_buf_c=NULL;
    int ret_pre=0;
    int i;
    int call_ret;
    uint8 fun_code=0;
    uint32 msg_length=0;
    int recv_data_len=0;


    strOneFrameData_N tmp_one_frame;
    static uint8 *tmp_one_frame_frame_data=NULL;
    if(NULL==tmp_one_frame_frame_data)
    {
        tmp_one_frame_frame_data=(uint8 *)malloc(PARA_SIZE_DSP_SEND_ONE_FRAME);
        if(NULL==tmp_one_frame_frame_data)
        {
            DBG_ALARM("malloc err\r\n");
            return;
        }
    }

    tmp_one_frame.frame_data_size=PARA_SIZE_DSP_SEND_ONE_FRAME;
    tmp_one_frame.frame_data=tmp_one_frame_frame_data;

    p_buf_c=(strRecvFotaFrame *)pframeData->frame_data;
    msg_length=pframeData->frame_len;
    recv_data_len=msg_length-(sizeof(strRecvFotaFrameHead)+FOTA_LEN_DTAT_ID+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG);

    if(1==pframeData->frame_flag)
    {
        ret_pre=fota_frame_pre_manage(pframeData,p_tmp_id_wr_list,send_fun,0x8000,0xffff);
        if(1==ret_pre)
        {
            fun_code=p_buf_c->head_data.h_fun & PARA_FUN_MASK;
            for(i=0;NULL != p_tmp_id_wr_list[i].id_des;++i)
            {
                if( p_buf_c->frame_id == p_tmp_id_wr_list[i].id_data )
                {

                    if(ACK_OK != p_buf_c->head_data.h_ack)
                    {
                        //对方的读写判断

                        if(PARA_FUN_RD == fun_code)
                        {
                            if(NULL !=  p_tmp_id_wr_list[i].get_param)
                            {
                                call_ret=p_tmp_id_wr_list[i].get_param(pframeData,&tmp_one_frame,p_buf_c->frame_id,p_tmp_id_wr_list[i].id_ret_len);
                                if(ACK_OK==call_ret)
                                {
                                    create_ack_frame_by_read(pframeData,&tmp_one_frame);
                                    if(RT_ON==p_tmp_id_wr_list[i].check_ret_len_en)
                                    {
                                        int check_msg_length=sizeof(strRecvFotaFrameHead)+(FOTA_LEN_DTAT_ID+p_tmp_id_wr_list[i].id_ret_len)+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG;
                                        if(check_msg_length != tmp_one_frame.frame_len)
                                        {
                                            DBG_PRO("in dsp_net_frame_manage2 2_0:read return id data len err,id=0x%04x\r\n",p_tmp_id_wr_list[i].id_data);
                                            break;
                                        }
                                    }

                                    send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                                }
                                else
                                {
                                    DBG_PRO("in dsp_net_frame_manage2 2_2:get id(0x%04X),call_ret=%d \r\n",p_buf_c->frame_id,call_ret);
                                }
                            }
                        }
                        else if(PARA_FUN_WT== fun_code)
                        {
                            if(NULL != p_tmp_id_wr_list[i].set_param )
                            {
                                call_ret=p_tmp_id_wr_list[i].set_param(pframeData,&tmp_one_frame,p_buf_c->frame_id,recv_data_len);
                                if(ACK_OK==call_ret)
                                {
                                    if(ACK_NOT_RET != p_buf_c->head_data.h_ack)
                                    {
                                        create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_OK);
                                        send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                                    }
                                }
                                else if(ACK_ERR_ID_DATA==call_ret)
                                {
                                    DBG_PRO("in dsp_net_frame_manage2 3_1:id data len err,id=0x%04x\r\n",p_tmp_id_wr_list[i].id_data);
                                    create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_ID_DATA);
                                    send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                                }
                                else if(ACK_ERR_OHTER==call_ret)
                                {
                                    if(ACK_NOT_RET != p_buf_c->head_data.h_ack)
                                    {
                                        create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_OHTER);
                                        send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                                    }
                                }
                                else
                                {
                                    DBG_PRO("in dsp_net_frame_manage2 3_2:set id(0x%04X),call_ret=%d \r\n",p_buf_c->frame_id,call_ret);
                                }
                            }

                        }

                    }
                    else
                    {
                        //自己的读写返回帧处理

                        if(PARA_FUN_RD == fun_code)
                        {
                            if(NULL !=  p_tmp_id_wr_list[i].self_get_param)
                            {
                                call_ret=p_tmp_id_wr_list[i].self_get_param(pframeData,&tmp_one_frame,p_buf_c->frame_id,recv_data_len);
                                if(ACK_OK!=call_ret)
                                {
                                    DBG_ALARM("in dsp_net_frame_manage 4_1:return read manage err,id=0x%04x,call_ret=%d\r\n",p_tmp_id_wr_list[i].id_data,call_ret);
                                }
                            }
                        }
                        else if(PARA_FUN_WT== fun_code)
                        {
                            if(NULL != p_tmp_id_wr_list[i].self_set_param )
                            {
                                call_ret=p_tmp_id_wr_list[i].self_set_param(pframeData,&tmp_one_frame,p_buf_c->frame_id,recv_data_len);
                                if(ACK_OK!=call_ret)
                                {
                                    DBG_ALARM("in dsp_net_frame_manage 4_2:return write manage err,id=0x%04x,call_ret=%d\r\n",p_tmp_id_wr_list[i].id_data,call_ret);
                                }

                            }

                        }

                    }

                    break;
                }

            }

        }

        pframeData->frame_flag=0;

    }

    return;

}















/****************************************************************************************************
**名称:void task_dsp_net_server_time(void *pdata)
**功能:线程任务
* 入口:无
* 出口:无
**auth:hxj, date: 2014-12-25 16:21
*****************************************************************************************************/
void task_dsp_net_server_time(void *pdata)
{
    DBG_DIS("in task_dsp_net_server_time 0:\r\n");
    MY_DELAY_X_S(1);


    MY_DELAY_X_MS(500);

    for(;;)
    {

#if 1 /*hxj amend,date 2014-12-25 16:56*/
        MY_DELAY_X_MS(1);

#if 0==DBG_MASK_DSP_NET1_EN
        dsp_net_find_frame();
        dsp_net_frame_manage();
#endif

#if 0==DBG_MASK_DSP_NET2_EN
        dsp_net_find_frame2();
        dsp_net_frame_manage2();
#endif


#else
        MY_DELAY_X_S(1);
        static uint32 t_cnt=0;

        t_cnt++;
        DBG_DIS("in task_dsp_net_server_time 1:t_cnt=%d,sysdds=%d,sys_ddms=%d\r\n",t_cnt,GET_SYS_DDS(NULL),GET_SYS_DDMS(NULL));
#endif



    }


}



int init_dat_file(void)
{
    if(NULL == g_p_dat_file) return 0;


    memset(&g_p_dat_file->head_old,0,sizeof(DAT_HEAD_OLD_C));

    g_p_dat_file->head_old.head_flag=0x02;
    g_p_dat_file->head_old.res_1[0]=1;
    g_p_dat_file->head_old.res_1[1]=2;
    g_p_dat_file->head_old.res_1[2]=3;
    g_p_dat_file->head_old.res_1[3]=4;

    memset(&g_p_dat_file->head_add,0,sizeof(DAT_HEAD_ADD_C));

    memset(g_data_info_flag,0,sizeof(g_data_info_flag));

    return 0;

}



uint16 get_index_data_info_end_flag(void)
{
    int i;
    for(i=0;i<GET_LIST_NUM(g_data_info_flag);++i)
    {
        if(g_data_info_flag[GET_LIST_NUM(g_data_info_flag)-1-i])
        {
            return (GET_LIST_NUM(g_data_info_flag)-1-i);
        }
    }

    return GET_LIST_NUM(g_data_info_flag);
}


uint16 get_data_info_flag_cnt(void)
{
    int i;
    uint16 cnt=0;

    for(i=0;i<GET_LIST_NUM(g_data_info_flag);++i)
    {
        if(1==g_data_info_flag[i])
        {
            cnt++;
        }
    }

    return cnt;
}


int add_dat_file_head_basic_info(uint8 *p_mach_sn,uint8 *p_time_setup,uint8 *p_local_sn,uint8 *p_sub_dank_sn,uint8 *p_net_sn)
{
    if(NULL == g_p_dat_file) return -1;

    if(NULL != p_time_setup)
    {
        memcpy(g_p_dat_file->head_old.time_setup,p_time_setup,14);
    }

    if(NULL != p_mach_sn)
    {
        memcpy(g_p_dat_file->head_old.mach_code,p_mach_sn,24);
        #if CUR_PROTOCOL!=PROTOCOL_ZX
            //重置年份
            g_p_dat_file->head_old.mach_code[3]=g_p_dat_file->head_old.time_setup[2];
            g_p_dat_file->head_old.mach_code[4]=g_p_dat_file->head_old.time_setup[3];
        #endif
    }

    if(NULL != p_local_sn)
    {
        memcpy(g_p_dat_file->head_add.local_sn,p_local_sn,4);
    }

    if(NULL != p_sub_dank_sn)
    {
        memcpy(g_p_dat_file->head_add.sub_dank_sn,p_sub_dank_sn,4);
    }

    if(NULL != p_net_sn)
    {
        memcpy(g_p_dat_file->head_add.net_sn,p_net_sn,4);
    }

    return 0;

}




/****************************************************************************************************
**名称:int add_one_money_info_to_dat_file(strRecvDSPIMG *p_dsp_img)
**功能:
* 入口:无
* 出口:成功返回当前的张数,出错返回负数
**auth:hxj, date: 2014-12-29 17:5
*****************************************************************************************************/
int add_one_money_info_to_dat_file(strRecvDSPIMG *p_dsp_img)
{
    if(NULL == g_p_dat_file) return -1;
    if(NULL == p_dsp_img)   return -2;

    #if DISPLAY_DSP_ADD_MONEY_INFO_EN
    DBG_PRO("in add_one_money_info_to_dat_file 0:\r\n");
    #endif

    uint16 cur_all_num=0;

    cur_all_num=get_data_info_flag_cnt();
    if(cur_all_num >= PARA_ONE_TIME_MONEY_MAX_NUM)
    {
        DBG_ALARM("in add_one_money_info_to_dat_file 1:cur_all_num(%d) >= PARA_ONE_TIME_MONEY_MAX_NUM(%d)\r\n",cur_all_num,PARA_ONE_TIME_MONEY_MAX_NUM);
        return -3;
    }
    //dsp send index minval 1
    p_dsp_img->index=p_dsp_img->index-1;

    if(p_dsp_img->index >= PARA_ONE_TIME_MONEY_MAX_NUM)
    {
        DBG_ALARM("in add_one_money_info_to_dat_file 2:p_dsp_img->index(%d) >= PARA_ONE_TIME_MONEY_MAX_NUM(%d)\r\n",p_dsp_img->index,PARA_ONE_TIME_MONEY_MAX_NUM);
        return -4;
    }

    if(1==g_data_info_flag[p_dsp_img->index])
    {
        DBG_ALARM("in add_one_money_info_to_dat_file 3:p_dsp_img->index(%d) had data\r\n",p_dsp_img->index);
        return -5;
    }


    DAT_DATA_INFO_C *p_data_info=&g_p_dat_file->data_info[p_dsp_img->index];

    //copy data
    memset(p_data_info->res_1,0,5);
    memcpy(p_data_info->money_sn,p_dsp_img->money_sn,10);


    uint16 tmp_money_val = 0;
    switch (p_dsp_img->money_val)
    {
        case 1:
        case 11:
            tmp_money_val = 1;
        break;
        case 2:
        case 12:
            tmp_money_val = 5;
        break;
        case 3:
        case 13:
            tmp_money_val = 10;
        break;
        case 4:
        case 14:
            tmp_money_val = 20;
        break;
        case 5:
        case 15:
            tmp_money_val = 50;
        break;
        case 6:
        case 16:
            tmp_money_val = 100;
        break;
        case 0:
            tmp_money_val = 0;
        break;
        default:
            tmp_money_val = 100;
        break;
    }

    p_data_info->money_val=tmp_money_val;
    p_data_info->money_ver=p_dsp_img->money_ver;
    p_data_info->money_flag=p_dsp_img->money_flag;



    memcpy(p_data_info->money_sn_pic,p_dsp_img->img_data,180);
    memset(&p_data_info->money_sn_pic[180],0,232-180);


    g_data_info_flag[p_dsp_img->index]=1;

    cur_all_num=get_data_info_flag_cnt();

    #if DISPLAY_DSP_ADD_MONEY_INFO_EN
    DBG_PRO("in add_one_money_info_to_dat_file 6:cur_all_num=%d,index=%d \r\n",cur_all_num,p_dsp_img->index);
    #endif

    return cur_all_num;

}



/****************************************************************************************************
**名称:int save_dat_file_to_sd(char *buf,int len)
**功能:保存文件到SD卡
* 入口:无
* 出口:成功返回0
**auth:hxj, date: 2014-12-30 9:40
*****************************************************************************************************/
int save_dat_file_to_sd(char *buf,int len)
{
    if(NULL==buf) return -1;
    if(len<=0)    return -2;

    DBG_DIS("in save_dat_file_to_sd 0:\r\n");

#if 1 /*hxj amend,date 2014-12-30 9:44*/
    static _BSP_MESSAGE pMsg;
    pMsg.MsgID =APP_SAVE_DAT;
    pMsg.pData = (uint8 *)buf;
    pMsg.DataLen =len;
    SYSPost(MMCSDEvent,&pMsg);
#endif

    DBG_DIS("in save_dat_file_to_sd 100:\r\n");

    return 0;

}



int save_dat_file_to_buf(void)
{
    if(NULL == g_p_dat_file) return 0;

    DBG_PRO("in save_dat_file_to_buf 0:\r\n");

    uint16 money_num=0;
    uint16 new_money_num=0;
    int cp_len=0;
    int i;
    uint16 get_index=0;
    int del_cnt=0;
    int deled_cnt=0;

    money_num=get_data_info_flag_cnt();

    DBG_PRO("in save_dat_file_to_buf 1:money_num=%d\r\n",money_num);
    if(0!=money_num)
    {
        if((0 == g_p_dat_file->head_old.mach_code[0]) ||(0==g_p_dat_file->head_old.time_setup[0]))
        {
            add_dat_file_head_basic_info(cur_arm_mac_info.mach_code,cur_arm_mac_info.g_time_setup,cur_arm_mac_info.g_local_sn,cur_arm_mac_info.g_sub_dank_sn,cur_arm_mac_info.g_net_sn);
        }

        new_money_num=money_num;
        get_index=get_index_data_info_end_flag();
        if(get_index < GET_LIST_NUM(g_data_info_flag))
        {
            new_money_num=1+get_index;
            if( new_money_num > money_num)
            {
                del_cnt=new_money_num-money_num;
                deled_cnt=0;
                DBG_ALARM("in save_dat_file_to_buf 2:need add %d money info,new_money_num=%d\r\n",del_cnt,new_money_num);

                for(i=0;i<GET_LIST_NUM(g_data_info_flag);++i)
                {
                    if(0==g_data_info_flag[i])
                    {
                        DAT_DATA_INFO_C *p_data_info=&g_p_dat_file->data_info[i];
                        //copy data
#if 0 /*hxj amend,date 2015-1-7 11:9*/
                        memset(p_data_info,0,sizeof(DAT_DATA_INFO_C));
#else
                        memset(p_data_info->money_sn,0,10);
                        p_data_info->money_val=0;
                        p_data_info->money_ver=3;
                        p_data_info->money_flag=0;
                        memset(p_data_info->money_sn_pic,0,232);
#endif
                        deled_cnt++;
                        if(deled_cnt>=del_cnt) break;

                    }
                }

            }
        }

        SET_2B_SMALLSN_VAL(g_p_dat_file->head_old.all_num,new_money_num,2);

        //---------------------------
        cp_len=sizeof(DAT_HEAD_OLD_C)+sizeof(DAT_HEAD_ADD_C)+GET_2B_SMALLSN_VAL(g_p_dat_file->head_old.all_num)*sizeof(DAT_DATA_INFO_C);
        memcpy(&g_p_dat_file[1],&g_p_dat_file[0],cp_len);
        #if DBG_NOT_SAVE_SD_AND_DIR_NET_SEND_EN
            //save_dat_file_to_sd((char *)&g_p_dat_file[1],cp_len);
            int ret_save=0;
            ret_save=save_dat_file_to_send_buf((char *)&g_p_dat_file[1],cp_len);
            if(0!=ret_save)
            {
                DBG_ALARM("in save_dat_file_to_buf 3:save_dat_file_to_send_buf err,ret_save=%d\r\n",ret_save);
            }

            save_dat_file_to_sd((char *)&g_p_dat_file[1],cp_len);
        #else
            save_dat_file_to_sd((char *)&g_p_dat_file[1],cp_len);
        #endif

        init_dat_file();

    }

    return 0;

}




/****************************************************************************************************
**名称:int send_big_img_manage(int send_big_img_step,char *buf,int len)
**功能:
* 入口:无
* 出口:成功返回0,出错返回-1
**auth:hxj, date: 2015-3-4 9:18
*****************************************************************************************************/
int send_big_img_manage(int send_big_img_step,char *buf,int len)
{
    int ret_val_net_send_big_img=10;
    int tmp_send_big_img_delay_cnt=30;  // 1s
    int i;
    int cnt_colse_fsn_send=20;           // unit 1s
    static char big_img_head_buf[100]={0}; //index=10 是张数,一个字节
    static int  big_img_head_len=0;
    static int  big_img_head_send_money_cnt=0;

#if 1 /*hxj amend,date 2015-1-16 10:46*/

    switch(send_big_img_step)
    {
        case 0: //init socket and send data
            g_big_img_cnt=0;
            g_send_big_img_delay_cnt=tmp_send_big_img_delay_cnt;
            g_send_big_img_stop_fsn_send_flag=0;
            g_send_big_img_flag=1;

            //关闭fsn上传,超时为5秒
            for(i=0;i<cnt_colse_fsn_send;++i)
            {
                if(1==g_send_big_img_stop_fsn_send_flag) break;
                MY_DELAY_X_S(1);
            }

            if(i>=cnt_colse_fsn_send)
            {
                goto err_big;
            }

            big_img_head_len=len;
            if(big_img_head_len>sizeof(big_img_head_buf))
            {
                big_img_head_len=sizeof(big_img_head_buf);
                DBG_DIS("big_img_head_len err\r\n");
            }
            memcpy(big_img_head_buf,buf,big_img_head_len);

            big_img_head_send_money_cnt=big_img_head_buf[10];
            if( INVALID_SOCKET != g_sk_big_img)
            {
                DBG_NORMAL("in send_big_img_manage 1_0:close socket,g_sk_big_img=0x%08x\r\n",(int)g_sk_big_img);
                SK_COLSE(g_sk_big_img);
                g_sk_big_img=INVALID_SOCKET;
            }

        break;
        case 1: //send data
            g_send_big_img_delay_cnt=tmp_send_big_img_delay_cnt;
            g_send_big_img_flag=1;
            g_big_img_cnt++;

            if(1==g_big_img_cnt)
            {
                DBG_DIS("in send_big_img_manage 1_1:big_img_head_send_money_cnt=%d\r\n",big_img_head_send_money_cnt);
                ret_val_net_send_big_img=net_send_big_img(big_img_head_buf,big_img_head_len,1);
                if(10 != ret_val_net_send_big_img)
                {
                   goto err_big;
                }
            }

            //一次发送失败,再重发送一次
            ret_val_net_send_big_img=net_send_big_img(buf,len,0);
            if(10 != ret_val_net_send_big_img)
            {
                if(-6 == ret_val_net_send_big_img)
                {
                    DBG_DIS("in send_big_img_manage 1_2:resend big img,g_big_img_cnt=%d\r\n",g_big_img_cnt);

                    //重发大图张数
                    big_img_head_buf[10]=big_img_head_send_money_cnt-g_big_img_cnt+1;
                    ret_val_net_send_big_img=net_send_big_img(big_img_head_buf,big_img_head_len,1);
                    if(10 != ret_val_net_send_big_img)
                    {
                       goto err_big;
                    }

                    ret_val_net_send_big_img=net_send_big_img(buf,len,0);
                    if(10 != ret_val_net_send_big_img)
                    {
                        goto err_big;
                    }
                }
                else
                {
                    goto err_big;
                }
            }

        break;
        case 2: //end
            ret_val_net_send_big_img=net_send_big_img(buf,len,2);
            if(10 != ret_val_net_send_big_img)
            {
                goto err_big;
            }

        break;
        default:
        break;
    }
    return 0;


err_big:

    if(10 != ret_val_net_send_big_img)
    {
        if( INVALID_SOCKET != g_sk_big_img)
        {
            DBG_NORMAL("in send_big_img_manage 2:close socket,g_sk_big_img=0x%08x\r\n",(int)g_sk_big_img);
            SK_COLSE(g_sk_big_img);
            g_sk_big_img=INVALID_SOCKET;
        }
    }

    g_big_img_cnt=0;
    g_send_big_img_flag=0;
    g_send_big_img_stop_fsn_send_flag=0;

    return -1;
#else
    //test
    g_send_big_img_flag=1;
    return 0;

#endif

}




int get_sd_res_alarm_thr(uint8 *tmp_val)
{

#if DBG_SAVE_A8_PARA_USE_ADD_EN
    read_eeprom_1byte(ADS_SD_REMAIN_SIZE_THR,tmp_val);
#else
    flash_para_opt_read(PARA_100F_ID_SD_REMAIN_SIZE_THR,tmp_val);
#endif

    return 0;
}


int save_sd_res_alarm_thr(uint8 tmp_val)
{
    write_eeprom_1byte(ADS_SD_REMAIN_SIZE_THR,tmp_val);
    g_sd_res_alarm_thr=tmp_val;

    return 0;
}



int get_sd_size_info(uint32 *tmp_all_size_val,uint32 *tmp_res_size_val)
{
    if(NULL != tmp_all_size_val)
    {
        *tmp_all_size_val=8*1024;
    }

    if(NULL != tmp_res_size_val)
    {
        *tmp_res_size_val=4*1024;
    }

    return 0;
}



int get_sd_opt_para(uint32 *tmp_send_ok_del_day,uint32 *tmp_send_err_del_day)
{
    if(NULL != tmp_send_ok_del_day)
    {
        read_eeprom_4byte(ADS_SD_SEND_OK_DAY,(uint8 *)tmp_send_ok_del_day);
    }

    if(NULL != tmp_send_err_del_day)
    {
        read_eeprom_4byte(ADS_SD_SEND_ERR_DAY,(uint8 *)tmp_send_err_del_day);
    }

    return 0;
}




int save_sd_opt_para(uint32 tmp_send_ok_del_day,uint32 tmp_send_err_del_day)
{
    write_eeprom_4byte(ADS_SD_SEND_OK_DAY,tmp_send_ok_del_day);
    write_eeprom_4byte(ADS_SD_SEND_ERR_DAY,tmp_send_err_del_day);
    return 0;
}








int sd_need_del_file(void)
{
    DBG_DIS("in sd_need_del_file 0:\r\n");

    return 0;
}



int save_cur_money_info(void)
{

    uint16 money_num=0;
    money_num=get_data_info_flag_cnt();
    if(0 != money_num )
    {
        //保存数据
        save_dat_file_to_buf();
    }
    else
    {
        //DBG_ALARM("in save_cur_money_info 1:money opt end,but not data need save,err\r\n");
    }

    g_dsp_send_money_status=1;

    return 0;
}




//0x2042          //实时抠图
int SET_ID_DSP_IMG(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    strRecvDSPIMG *p_frame_data=(strRecvDSPIMG *)p_fota_frame->frame_data;
    int ret_add=0;

    DBG_ID("in SET_ID_DSP_IMG 0:id=0x%04x,data_len=%d\r\n",id,len);

    if(0==g_recv_dsp_send_basic_info_flag)
    {
        DBG_ID("in SET_ID_DSP_IMG 1:return 0==g_recv_dsp_send_basic_info_flag\r\n");
        return ACK_ERR_ID_DATA;
    }


    if(1==g_dsp_send_money_status)
    {
        g_dsp_send_money_status=0;
        //开始第一次点钞

        char time[20]={0};
        if(0==get_os_sys_date(time))
        {
            //DBG_ID("in SET_ID_DSP_IMG 3_2:use net time as start time\r\n");
            memcpy(cur_arm_mac_info.g_time_setup,(char *)time,14);
        }
        else
        {
            DBG_ID("in SET_ID_DSP_IMG 3_3:get os time err,use is 20150130111617 \r\n");
            memcpy(cur_arm_mac_info.g_time_setup,"20150130111617",14);
        }

        DBG_ID("in SET_ID_DSP_IMG 3_4:start time=%s \r\n",cur_arm_mac_info.g_time_setup);

        init_dat_file();
        add_dat_file_head_basic_info(cur_arm_mac_info.mach_code,cur_arm_mac_info.g_time_setup,cur_arm_mac_info.g_local_sn,cur_arm_mac_info.g_sub_dank_sn,cur_arm_mac_info.g_net_sn);
    }


    //判断是否到300张,如果到了自动保存
    if(PARA_ONE_TIME_MONEY_MAX_NUM==p_frame_data->index)
    {
        ret_add=add_one_money_info_to_dat_file(p_frame_data);
        if(ret_add<0)
        {
            DBG_ALARM("in SET_ID_DSP_IMG 8:id=0x%04x,data_len=%d,add one money err,ret_add=%d \r\n",id,len,ret_add);
        }

        save_cur_money_info();
    }
    else if(p_frame_data->index>PARA_ONE_TIME_MONEY_MAX_NUM)
    {
        DBG_ALARM("in SET_ID_DSP_IMG 9:id=0x%04x,data_len=%d,recv index(%d) err\r\n",id,len,p_frame_data->index);
    }
    else
    {
        ret_add=add_one_money_info_to_dat_file(p_frame_data);
        if(ret_add<0)
        {
            DBG_ALARM("in SET_ID_DSP_IMG 10:id=0x%04x,data_len=%d,add one money err,ret_add=%d \r\n",id,len,ret_add);
        }
    }

    return ACK_OK;

}


int SET_ID_DSP_CNT_MONEY_START(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    //strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    //strRecvDSPIMG *p_frame_data=(strRecvDSPIMG *)p_fota_frame->frame_data;

    DBG_ID("in SET_ID_DSP_CNT_MONEY_START 0_1:id=0x%04x,data_len=%d\r\n",id,len);

    return ACK_OK;

}


int SET_ID_DSP_CNT_MONEY_END(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{

    DBG_ID("in SET_ID_DSP_CNT_MONEY_END 0:id=0x%04x,data_len=%d\r\n",id,len);
    save_cur_money_info();
    return ACK_OK;

}



int SET_ID_DSP_BASIC_INFO(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    strRecvBasicInfo *p_frame_data=(strRecvBasicInfo *)p_fota_frame->frame_data;

    DBG_ID("in SET_ID_DSP_BASIC_INFO 0:id=0x%04x,data_len=%d\r\n",id,len);

    char sys_time[15]={0};
    if(0==g_get_net_date_flag)
    {
        memcpy(sys_time,p_frame_data->sys_time,14);
        if(!is_ok_date(sys_time))
        {
            DBG_ID("in SET_ID_DSP_BASIC_INFO 1_1:recv arm time err[%s]\r\n",sys_time);
            DBG_BUF_PRO((uint8 *)p_frame_data, len, "in SET_ID_DSP_BASIC_INFO 1_2:");
            return ACK_ERR_ID_DATA;
        }
        else
        {
            DBG_ID("in SET_ID_DSP_BASIC_INFO 2_1:recv arm time ok[%s]\r\n",sys_time);
            if(0 != set_os_sys_date(sys_time))
            {
                DBG_BUF_PRO((uint8 *)p_frame_data, len, "in SET_ID_DSP_BASIC_INFO 2_2:");
                DBG_ALARM("in SET_ID_DSP_BASIC_INFO 2_3:set sys time err,recv sys time=%s\r\n",sys_time);
                return ACK_ERR_ID_DATA;
            }
            else
            {
                DBG_ID("in SET_ID_DSP_BASIC_INFO 2_4:set sys time ok\r\n");
            }
        }
    }
    else
    {
        DBG_PRO("in SET_ID_DSP_BASIC_INFO 3:use net time\r\n");
        if(0!=get_os_sys_date(sys_time))
        {
            DBG_PRO("in SET_ID_DSP_BASIC_INFO 4:but get os time err\r\n");
            memcpy(sys_time,p_frame_data->sys_time,14);
        }
    }

    memcpy(cur_arm_mac_info.g_local_sn,p_frame_data->local_sn,4);
    memcpy(cur_arm_mac_info.g_sub_dank_sn,p_frame_data->sub_dank_sn,4);
    memcpy(cur_arm_mac_info.g_net_sn,p_frame_data->net_sn,4);


    char tmp_mach_sn[25]={0};             //机具编号
    int  need_change_mach_sn_year_flag=1;

    memcpy(tmp_mach_sn,p_frame_data->mach_sn,24);


#if CUR_PROTOCOL==PROTOCOL_ZX /*hxj amend,date 2014-11-17 18:7*/
    tmp_mach_sn[0]=cur_arm_mac_info.g_sub_dank_sn[1];
    tmp_mach_sn[1]=cur_arm_mac_info.g_sub_dank_sn[2];
    tmp_mach_sn[2]=cur_arm_mac_info.g_sub_dank_sn[3];
    tmp_mach_sn[3]=cur_arm_mac_info.g_net_sn[0];
    tmp_mach_sn[4]=cur_arm_mac_info.g_net_sn[1];
    tmp_mach_sn[5]='/';
    tmp_mach_sn[6]='F';
    tmp_mach_sn[7]='O';
    tmp_mach_sn[8]='T';
    tmp_mach_sn[9]='A';
    tmp_mach_sn[10]='/';
    tmp_mach_sn[11]=cur_arm_mac_info.g_net_sn[2];
    tmp_mach_sn[12]=cur_arm_mac_info.g_net_sn[3];
    tmp_mach_sn[13]='0';
    tmp_mach_sn[14]='0';
    tmp_mach_sn[15]='0';
    memcpy(&tmp_mach_sn[16],cur_arm_mac_info.SN_NetSend_MachineNo,8);
    need_change_mach_sn_year_flag=0;

#elif CUR_PROTOCOL==PROTOCOL_GR
    tmp_mach_sn[0]='J';
    tmp_mach_sn[1]='C';
    tmp_mach_sn[2]='B';
    tmp_mach_sn[3]='1';
    tmp_mach_sn[4]='5';
    tmp_mach_sn[5]='/';
    tmp_mach_sn[6]='F';
    tmp_mach_sn[7]='O';
    tmp_mach_sn[8]='T';
    tmp_mach_sn[9]='A';
    tmp_mach_sn[10]='/';

#else
    tmp_mach_sn[0]='B';
    tmp_mach_sn[1]='O';
    tmp_mach_sn[2]='C';
    tmp_mach_sn[3]='1';
    tmp_mach_sn[4]='5';
    tmp_mach_sn[5]='/';
    tmp_mach_sn[6]='F';
    tmp_mach_sn[7]='O';
    tmp_mach_sn[8]='T';
    tmp_mach_sn[9]='A';
    tmp_mach_sn[10]='/';
#endif

    if(1==need_change_mach_sn_year_flag)
    {
        tmp_mach_sn[3]=sys_time[2];
        tmp_mach_sn[4]=sys_time[3];
    }

    memcpy(cur_arm_mac_info.mach_code,tmp_mach_sn,24);
    cur_arm_mac_info.mach_code[24]=0;

    g_recv_dsp_send_basic_info_flag=1;
    g_dsp_send_money_status=1;



    DBG_DIS("in SET_ID_DSP_BASIC_INFO 90:cur_arm_mac_info.mach_code=%s\r\n",cur_arm_mac_info.mach_code);
    DBG_PRO("in SET_ID_DSP_BASIC_INFO 100:get basic info ok\r\n");

    return ACK_OK;

}



int GET_ID_DSP_SYNC_TIME(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame_ret=(strRecvFotaFrame *)ret_frame->frame_data;
#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in GET_ID_DSP_SYNC_TIME 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    int  cp_len=0;
    int  cp_all_len=0;
    char buf[30]={0};

    if(1==g_get_net_date_flag)
    {
        get_os_sys_date(buf);
    }

    cp_len=14;
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],buf,cp_len);
    cp_all_len+=cp_len;

    if(cp_all_len != len)
    {
        DBG_PRO("in GET_ID_DSP_SYNC_TIME 1:cp len err\r\n");

    }

    p_fota_frame_ret->head_data.h_data_len=cp_all_len;

    p_fota_frame_ret->head_data.h_data_len+=FOTA_LEN_DTAT_ID;
    return ACK_OK;

}




int GET_ID_DSP_UPDATE_A8(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame_ret=(strRecvFotaFrame *)ret_frame->frame_data;
#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in GET_ID_DSP_UPDATE_A8 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    int  cp_len=0;
    int  cp_all_len=0;
    char buf[10];
    extern OS_EVENT *MMCSDEvent;
    int i;

    g_update_a8_success_flag=0;

    _BSP_MESSAGE tmp_send_message;

    tmp_send_message.MsgID = APP_COMFROM_UI;
    tmp_send_message.DataLen = 0x02;
    tmp_send_message.pData = NULL;
    OSQPost (MMCSDEvent,&tmp_send_message);

    for(i=0;i<60*10;++i)
    {
        if(0==g_update_a8_success_flag)
        {
            MY_DELAY_X_MS(100);
        }
        else
        {
            break;
        }
    }

    buf[0]=g_update_a8_success_flag;
    cp_len=1;

    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],buf,cp_len);
    cp_all_len+=cp_len;

    if(cp_all_len != len)
    {
        DBG_PRO("in GET_ID_DSP_UPDATE_A8 1:cp len err\r\n");
    }

    p_fota_frame_ret->head_data.h_data_len=cp_all_len;

    p_fota_frame_ret->head_data.h_data_len+=FOTA_LEN_DTAT_ID;
    return ACK_OK;

}









int GET_ID_DSP_VER(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame_ret=(strRecvFotaFrame *)ret_frame->frame_data;
#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in GET_ID_DSP_VER 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    int  cp_len=0;
    int  cp_all_len=0;

    char ver_info[30]={0};
    char comp_time[14]={0};
    int i;

    getVersion(ver_info);
    DBG_DIS("A8_ver_info:%s\r\n",ver_info);

    char time_st[30];
    get_compile_time_my_format (time_st );
    //2011-10-05 15:07:15  ->20111005150715
    memcpy(comp_time,time_st,4);
    for(i=0;i<5;++i)
    {
        comp_time[4+i*2]=time_st[5+i*3];
        comp_time[5+i*2]=time_st[6+i*3];
    }

    cp_len=sizeof(ver_info);
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],ver_info,cp_len);
    cp_all_len+=cp_len;

    cp_len=sizeof(comp_time);
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],comp_time,cp_len);
    cp_all_len+=cp_len;

    if(cp_all_len != len)
    {
        DBG_PRO("in GET_ID_DSP_VER 1:cp len err\r\n");

    }

    p_fota_frame_ret->head_data.h_data_len=cp_all_len;

    p_fota_frame_ret->head_data.h_data_len+=FOTA_LEN_DTAT_ID;
    return ACK_OK;

}


#if FZ1500
int GET_RET_ID_DSP_VER(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;

    DBG_ID("in GET_RET_ID_DSP_VER 0:id=0x%04x,data_len=%d\r\n",id,len);

    static char read_dsp_ver[31]={0};
    char tem_comp_time[15]={0};

    memcpy(read_dsp_ver,p_frame_data,30);
    memcpy(tem_comp_time,&p_frame_data[30],14);

    DBG_ID("in GET_RET_ID_DSP_VER 1:das_app_ver:%s,comp_time=%s\r\n",read_dsp_ver,tem_comp_time);

    static _BSP_MESSAGE VerMsg;
    if(NULL != DispTaskEvent)
    {
        DBG_ID("in GET_RET_ID_DSP_VER 3:send message to display\r\n");
        VerMsg.MsgID = APP_DISP_DSP_VER;
        VerMsg.pData = (void *)read_dsp_ver;
        VerMsg.DivNum = DSP2;
        VerMsg.DataLen = 25;

        MY_DELAY_X_MS(300);
        OSQPost(DispTaskEvent,&VerMsg);
        MY_DELAY_X_MS(300);

    }
    else
    {
        DBG_ID("in GET_RET_ID_DSP_VER 4:NULL == DispTaskEvent\r\n");
    }

    return ACK_OK;

}

#endif



int GET_ID_DSP_NET_PARA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame_ret=(strRecvFotaFrame *)ret_frame->frame_data;
#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in GET_ID_DSP_NET_PARA 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    int  cp_len=0;
    int  cp_all_len=0;

    strIPPara ip_info;

    change_ip_str_to_val(g_local_ip,ip_info.ip);
    change_ip_str_to_val(g_local_mk,ip_info.mk);
    change_ip_str_to_val(g_local_gw,ip_info.gw);
    ip_info.port=g_local_port;

    change_ip_str_to_val(g_server_ip,ip_info.mip);
    ip_info.mport=g_server_port;

    get_local_mac(NULL,ip_info.mac);

    cp_len=sizeof(strIPPara);
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],&ip_info,cp_len);
    cp_all_len+=cp_len;

    if(cp_all_len != len)
    {
        DBG_PRO("in GET_ID_DSP_NET_PARA 1:cp len err\r\n");

    }

    p_fota_frame_ret->head_data.h_data_len=cp_all_len;

    p_fota_frame_ret->head_data.h_data_len+=FOTA_LEN_DTAT_ID;
    return ACK_OK;

}




int SET_ID_DSP_NET_PARA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    strIPPara *p_frame_data=(strIPPara *)p_fota_frame->frame_data;

#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in SET_ID_DSP_NET_PARA 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif

    save_local_ip_info(p_frame_data->ip,p_frame_data->mk,p_frame_data->gw);
    save_local_port(p_frame_data->port);

    save_server_ip(p_frame_data->mip);
    save_server_port(p_frame_data->mport);

    return ACK_OK;

}





int GET_ID_DSP_NET_SEND_PROTOCOL(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame_ret=(strRecvFotaFrame *)ret_frame->frame_data;
#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in GET_ID_DSP_NET_SEND_PROTOCOL 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    int  cp_len=0;
    int  cp_all_len=0;
    uint8 buf[2];


    buf[0]=g_prot_type;
    buf[1]=g_prot_mode;
    cp_len=2;

    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],buf,cp_len);
    cp_all_len+=cp_len;

    if(cp_all_len != len)
    {
        DBG_PRO("in GET_ID_DSP_NET_SEND_PROTOCOL 1:cp len err\r\n");

    }

    p_fota_frame_ret->head_data.h_data_len=cp_all_len;

    p_fota_frame_ret->head_data.h_data_len+=FOTA_LEN_DTAT_ID;
    return ACK_OK;

}



int SET_ID_DSP_NET_SEND_PROTOCOL(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;

#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in SET_ID_DSP_NET_SEND_PROTOCOL 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif

    save_net_protocol_info(p_frame_data[0],p_frame_data[1]);
    return ACK_OK;
}








int GET_ID_DSP_SD_RES_ALARM_UP(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame_ret=(strRecvFotaFrame *)ret_frame->frame_data;
#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in GET_ID_DSP_SD_RES_ALARM_THR 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    int  cp_len=0;
    int  cp_all_len=0;
    uint8 buf[2];

    buf[0]=g_sd_res_alarm_up_status;
    cp_len=1;
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],buf,cp_len);
    cp_all_len+=cp_len;

    if(cp_all_len != len)
    {
        DBG_PRO("in GET_ID_DSP_SD_RES_ALARM_THR 1:cp len err\r\n");

    }

    p_fota_frame_ret->head_data.h_data_len=cp_all_len;

    p_fota_frame_ret->head_data.h_data_len+=FOTA_LEN_DTAT_ID;
    return ACK_OK;

}










int GET_ID_DSP_SD_OPT_PARA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame_ret=(strRecvFotaFrame *)ret_frame->frame_data;
#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in GET_ID_DSP_SD_OPT_PARA 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    int  cp_len=0;
    int  cp_all_len=0;
    uint8 buf[10];


    get_sd_opt_para(&g_sd_send_ok_del_day,&g_sd_send_err_del_day);

    memcpy(buf,(char *)&g_sd_send_ok_del_day,sizeof(g_sd_send_ok_del_day));
    cp_len=4;
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],buf,cp_len);
    cp_all_len+=cp_len;

    memcpy(buf,(char *)&g_sd_send_err_del_day,sizeof(g_sd_send_err_del_day));
    cp_len=4;
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],buf,cp_len);
    cp_all_len+=cp_len;


    if(cp_all_len != len)
    {
        DBG_PRO("in GET_ID_DSP_SD_OPT_PARA 1:cp len err\r\n");

    }

    p_fota_frame_ret->head_data.h_data_len=cp_all_len;

    p_fota_frame_ret->head_data.h_data_len+=FOTA_LEN_DTAT_ID;
    return ACK_OK;

}




int SET_ID_DSP_SD_OPT_PARA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;

#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in SET_ID_DSP_SD_RES_ALARM_THR 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    uint32 tmp_send_ok_del_day;
    uint32 tmp_send_err_del_day;

    memcpy((uint8 *)&tmp_send_ok_del_day,&p_frame_data[0],sizeof(tmp_send_ok_del_day));
    memcpy((uint8 *)&tmp_send_err_del_day,&p_frame_data[4],sizeof(tmp_send_err_del_day));

    save_sd_opt_para(tmp_send_ok_del_day,tmp_send_err_del_day);

    return ACK_OK;
}





int GET_ID_DSP_SD_RES_SIZE(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame_ret=(strRecvFotaFrame *)ret_frame->frame_data;
#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in GET_ID_DSP_SD_RES_SIZE 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    int  cp_len=0;
    int  cp_all_len=0;
    uint8 buf[10];


    get_sd_size_info(&g_sd_all_size,&g_sd_res_size);

    memcpy(buf,(char *)&g_sd_all_size,sizeof(g_sd_all_size));
    cp_len=4;
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],buf,cp_len);
    cp_all_len+=cp_len;

    memcpy(buf,(char *)&g_sd_res_size,sizeof(g_sd_res_size));
    cp_len=4;
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],buf,cp_len);
    cp_all_len+=cp_len;


    if(cp_all_len != len)
    {
        DBG_PRO("in GET_ID_DSP_SD_RES_SIZE 1:cp len err\r\n");

    }

    p_fota_frame_ret->head_data.h_data_len=cp_all_len;

    p_fota_frame_ret->head_data.h_data_len+=FOTA_LEN_DTAT_ID;
    return ACK_OK;

}




int SET_ID_DSP_SD_DEL_DATA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    //strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    //uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;

#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in SET_ID_DSP_SD_DEL_DATA 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif

    sd_need_del_file();

    return ACK_OK;
}








int GET_ID_DSP_SD_RES_ALARM_THR(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame_ret=(strRecvFotaFrame *)ret_frame->frame_data;
#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in GET_ID_DSP_SD_RES_ALARM_THR 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    int  cp_len=0;
    int  cp_all_len=0;
    uint8 buf[2];

    buf[0]=g_sd_res_alarm_thr;
    cp_len=1;
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],buf,cp_len);
    cp_all_len+=cp_len;

    if(cp_all_len != len)
    {
        DBG_PRO("in GET_ID_DSP_SD_RES_ALARM_THR 1:cp len err\r\n");

    }

    p_fota_frame_ret->head_data.h_data_len=cp_all_len;

    p_fota_frame_ret->head_data.h_data_len+=FOTA_LEN_DTAT_ID;
    return ACK_OK;

}



int SET_ID_DSP_SD_RES_ALARM_THR(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;

#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in SET_ID_DSP_SD_RES_ALARM_THR 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif

    save_sd_res_alarm_thr(p_frame_data[0]);

    return ACK_OK;
}










int GET_ID_DSP_NET_SEND_SWITCH(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame_ret=(strRecvFotaFrame *)ret_frame->frame_data;
#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in GET_ID_DSP_NET_SEND_SWITCH 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    int  cp_len=0;
    int  cp_all_len=0;
    uint8 buf[2];

    buf[0]=g_net_send_switch;
    cp_len=1;
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],buf,cp_len);
    cp_all_len+=cp_len;

    if(cp_all_len != len)
    {
        DBG_PRO("in GET_ID_DSP_NET_SEND_SWITCH 1:cp len err\r\n");

    }

    p_fota_frame_ret->head_data.h_data_len=cp_all_len;

    p_fota_frame_ret->head_data.h_data_len+=FOTA_LEN_DTAT_ID;
    return ACK_OK;

}




int SET_ID_DSP_NET_SEND_SWITCH(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;

#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in SET_ID_DSP_NET_SEND_SWITCH 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif

    save_net_switch(p_frame_data[0]);

    return ACK_OK;
}



#if FZ1500
int SET_ID_DSP_UPDATE_STATUS(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;

#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in SET_ID_DSP_UPDATE_STATUS 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif

    if(1==p_frame_data[1])
    {
        DBG_ID("in SET_ID_DSP_UPDATE_STATUS 1:dsp update ok \r\n");
    }
    else
    {
        DBG_ID("in SET_ID_DSP_UPDATE_STATUS 2:dsp update err\r\n");
    }

    RecDSP_UpdtRT(p_frame_data);

    return ACK_OK;
}
#endif


#if FZ1500
int SET_ID_DSP_CHECK_CS(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    static _BSP_MESSAGE pMsg;

    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;

#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in SET_ID_DSP_CHECK_CS 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif


    pMsg.MsgID = APP_DISP_DSP_CHECK_CS;	// 升级命令低八位
    pMsg.DivNum = DSP2;
    pMsg.pData = p_frame_data;
    pMsg.DataLen = 0x01;
    SYSPost(DispTaskEvent,&pMsg);

    return ACK_OK;
}
#endif

int GET_ID_DSP_NET_SEND_MASK_TIME(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame_ret=(strRecvFotaFrame *)ret_frame->frame_data;
#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in GET_ID_DSP_NET_SEND_MASK_TIME 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif
    int  cp_len=0;
    int  cp_all_len=0;

    cp_len=sizeof(g_not_send_time);
    memcpy(&p_fota_frame_ret->frame_data[cp_all_len],(char *)g_not_send_time,cp_len);
    cp_all_len+=cp_len;

    if(cp_all_len != len)
    {
        DBG_PRO("in GET_ID_DSP_NET_SEND_MASK_TIME 1:cp len err\r\n");

    }

    p_fota_frame_ret->head_data.h_data_len=cp_all_len;

    p_fota_frame_ret->head_data.h_data_len+=FOTA_LEN_DTAT_ID;
    return ACK_OK;

}




int SET_ID_DSP_NET_SEND_MASK_TIME(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;

#if DISPLAY_DSP_ID_INFO_EN
    DBG_ID("in SET_ID_DSP_NET_SEND_MASK_TIME 0:id=0x%04x,data_len=%d\r\n",id,len);
#endif

    NET_MASK_ONE_TIME *tmp_not_send_time;

    tmp_not_send_time=(NET_MASK_ONE_TIME *)p_frame_data;
    save_net_stop_time(tmp_not_send_time,10);
    return ACK_OK;

}



int SET_ID_DSP_BIG_IMG_START(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;
    int ret=0;

    DBG_ID("in SET_ID_DSP_BIG_IMG_START 0:id=0x%04x,data_len=%d\r\n",id,len);
    ret=send_big_img_manage(0,(char *)p_frame_data,len);
    DBG_PRO("in SET_ID_DSP_BIG_IMG_START 100:\r\n");

    if(0==ret)
    {
        DBG_PRO("in SET_ID_DSP_BIG_IMG_START 110:return ACK_OK\r\n");
        return ACK_OK;
    }
    else
    {
        DBG_PRO("in SET_ID_DSP_BIG_IMG_START 110:return ACK_OK\r\n");
        return ACK_ERR_OHTER;
    }

}



int SET_ID_DSP_BIG_IMG_DATA(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;
    int ret=0;

    DBG_ID("in SET_ID_DSP_BIG_IMG_DATA 0:id=0x%04x,data_len=%d\r\n",id,len);  //id=0x3507,data_len=1277952
    ret=send_big_img_manage(1,(char *)p_frame_data,len);
    DBG_PRO("in SET_ID_DSP_BIG_IMG_DATA 100:g_big_img_cnt=%d\r\n",g_big_img_cnt);


    if(0==ret)
    {
        DBG_PRO("in SET_ID_DSP_BIG_IMG_DATA 110:return ACK_OK\r\n");
        return ACK_OK;
    }
    else
    {
        DBG_PRO("in SET_ID_DSP_BIG_IMG_DATA 110:return ACK_ERR_OHTER\r\n");
        return ACK_ERR_OHTER;
    }

}



int SET_ID_DSP_BIG_IMG_END(strOneFrameData_N *recv_frame,strOneFrameData_N *ret_frame,uint16 id, uint32 len )
{
    strRecvFotaFrame *p_fota_frame=(strRecvFotaFrame *)recv_frame->frame_data;
    uint8 *p_frame_data=(uint8 *)p_fota_frame->frame_data;
    int ret=0;

    DBG_ID("in SET_ID_DSP_BIG_IMG_END 0:id=0x%04x,data_len=%d\r\n",id,len);
    ret=send_big_img_manage(2,(char *)p_frame_data,len);
    DBG_PRO("in SET_ID_DSP_BIG_IMG_END 100:\r\n");

    if(0==ret)
    {
        DBG_PRO("in SET_ID_DSP_BIG_IMG_END 110:return ACK_OK\r\n");
        return ACK_OK;
    }
    else
    {
        DBG_PRO("in SET_ID_DSP_BIG_IMG_END 110:return ACK_ERR_OHTER\r\n");
        return ACK_ERR_OHTER;
    }

}

void TEST_SET_ID_DSP_BASIC_INFO(void )
{
    char sys_time[15]={"20140822235959"};
    char sn[5] = {"0000"};
    
    memcpy(cur_arm_mac_info.g_local_sn,sn,4);
    memcpy(cur_arm_mac_info.g_sub_dank_sn,sn,4);
    memcpy(cur_arm_mac_info.g_net_sn,sn,4);
    
    char tmp_mach_sn[25]={0};             //机具编号
    int  need_change_mach_sn_year_flag=1;

    tmp_mach_sn[0]='B';
    tmp_mach_sn[1]='O';
    tmp_mach_sn[2]='C';
    tmp_mach_sn[3]='1';
    tmp_mach_sn[4]='5';
    tmp_mach_sn[5]='/';
    tmp_mach_sn[6]='F';
    tmp_mach_sn[7]='O';
    tmp_mach_sn[8]='T';
    tmp_mach_sn[9]='A';
    tmp_mach_sn[10]='/';

    if(1==need_change_mach_sn_year_flag)
    {
        tmp_mach_sn[3]=sys_time[2];
        tmp_mach_sn[4]=sys_time[3];
    }

    memcpy(cur_arm_mac_info.mach_code,tmp_mach_sn,24);
    cur_arm_mac_info.mach_code[24]=0;

    g_recv_dsp_send_basic_info_flag=1;
    g_dsp_send_money_status=1;



    DBG_DIS("in SET_ID_DSP_BASIC_INFO 90:cur_arm_mac_info.mach_code=%s\r\n",cur_arm_mac_info.mach_code);
    DBG_PRO("in SET_ID_DSP_BASIC_INFO 100:get basic info ok\r\n");

}






/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

