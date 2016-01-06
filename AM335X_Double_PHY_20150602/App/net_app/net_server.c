/***************文件信息********************************************************************************
**文   件   名: net_server.c
**说        明:
**创   建   人: hxj
**创   建 日期: 2014-8-18 12:58
*******************************************************************************************************/
#include "def_config.h"
#include "net_server.h"
#include "bsp.h"
#include "in_comm_protocol.h"
#include "save_para.h"
#include "dsp_net.h"






//系统的日期
DSP_DATE    cur_rec_date;
uint64  g_sys_ddms=0;
uint32  g_sys_dds=0;


char   g_net_first_init_over_flag=0;
char   g_init_tcp_ip_data_buf_over_flag=0;
char   g_auto_negotiation_oked_flag=0;
char   g_app_net_up_flag=0;
uint32 g_app_net_up_delay=PARA_G_APP_NET_UP_DELAY;


VAL_VOL char    net_link_ok_flag=0;
SK_TYPE g_sk_wz=INVALID_SOCKET;
SK_TYPE g_sk_big_img=INVALID_SOCKET;

int     gbl_timeout_sec = 4*60;

uint32  g_connect_wz_timeout_s=0;
int     g_connect_wz_start_flag=0;

char   *p_g_fsn_data=NULL;
int     fsn_data_max_len=0;
char   *p_g_busi_txt=NULL;
char   *p_g_busi_txt_data=NULL;

int     busi_data_max_len=0;
DAT_FILE_BUF *p_dat_file_buf_data=NULL;
MACH_INFO_C cur_arm_mac_info;


int     dat_file_to_fsn_change_err_cnt=0;
int     flag_icbc_fsn_net_state = 1;
uint32  g_dsp_sync_time_delya_dds=0;
uint8   g_dsp_recv_arm_sync_time_ack_flag=0;
char    arm_busy_flag=0;

NET_MASK_ONE_TIME g_not_send_time[10]={0};
NET_MASK_ONE_TIME_DS g_not_send_time_ds[10]={0};

VAL_VOL uint8   g_get_net_date_flag=0;
uint8   g_mac[6]={0};


uint8 g_prot_type;
uint8 g_prot_mode;
uint8 g_net_send_switch;

char g_local_ip[20]={0};
char g_local_mk[20]={0};
char g_local_gw[20]={0};
uint16 g_local_port=0;

char g_server_ip[20]={0};
uint16 g_server_port=0;






//=============================================================================
//test


char dat_file_buf_1[]={0x01,0x01,0x02,0x03,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x06,0x01,0x00,0x42,0x4f,0x43,0x31,0x34,0x2f,0x46,0x4f,0x54,0x41,0x2f,0x30,0x31,0x34,
0x30,0x35,0x32,0x37,0x30,0x31,0x32,0x00,0x00,0x00,0x32,0x30,0x31,0x34,0x31,0x32,0x31,0x37,0x31,
0x33,0x34,0x34,0x30,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x31,0x32,0x33,0x34,0x35,0x35,0x36,0x36,0x37,0x37,0x38,0x38,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x00,0x00,0x44,0x39,0x39,0x44,0x30,0x38,0x33,0x38,0x32,0x34,
0x64,0x02,0x01,0xfc,0x1f,0xfc,0x3f,0x0c,0x30,0x0c,0x30,0x0c,0x18,0x1c,0x18,0xf8,0x1f,0xf0,0x07,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x9c,0x1f,0x8c,0x39,0x8e,0x30,0xcc,0x30,0x8c,0x3d,0xf8,
0x1f,0xf0,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x84,0x1f,0xce,0x1f,0xce,0x38,0xc6,0x30,0xc7,0x30,
0xce,0x39,0xfe,0x1f,0xfc,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x7f,0xff,0x7f,0x07,0x70,0x07,
0x70,0x07,0x70,0x07,0x30,0x0e,0x3c,0xfe,0x1f,0xf8,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0x0f,
0xfe,0x3f,0x1f,0x38,0x07,0x70,0x03,0x60,0x07,0x70,0x0f,0x38,0xfe,0x3f,0xf8,0x0f,0x00,0x00,0x00,
0x00,0x00,0x00,0x38,0x00,0xfc,0x3e,0xee,0x7f,0x87,0x73,0x87,0x63,0x87,0x63,0xc6,0x73,0xfe,0x3f,
0x7c,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1c,0x18,0x1e,0x38,0x06,0x70,0x87,0x71,0xc7,
0x71,0xc6,0x33,0xfe,0x3f,0xfc,0x1e,0x10,0x00,0x00,0x00,0x00,0x00,0x38,0x00,0xfc,0x1f,0xce,0x3f,
0x86,0x39,0x86,0x31,0x86,0x31,0xfe,0x3f,0xfc,0x1f,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0e,
0x0c,0x1e,0x1c,0x7e,0x38,0xe6,0x30,0xc6,0x31,0x86,0x3f,0x06,0x1f,0x00,0x04,0x00,0x00,0x00,0x00,
0x00,0x00,0x70,0x00,0xf0,0x01,0xb0,0x07,0x30,0x1f,0xfc,0x1f,0xfc,0x1f,0x30,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,};






char dat_file_buf_2[]={0x01,0x01,0x02,0x03,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x06,0x02,0x00,0x42,0x4f,0x43,0x31,0x34,0x2f,0x46,0x4f,0x54,0x41,0x2f,0x30,0x31,0x34,0x30,
0x35,0x32,0x37,0x30,0x31,0x32,0x00,0x00,0x00,0x32,0x30,0x31,0x34,0x31,0x32,0x31,0x37,0x31,0x33,
0x34,0x34,0x31,0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x31,0x32,0x33,0x34,0x35,0x35,0x36,0x36,0x37,0x37,0x38,0x38,0x30,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x00,0x00,0x00,0x00,0x00,0x59,0x30,0x50,0x36,0x36,0x30,0x39,0x33,0x30,0x30,0x64,
0x02,0x01,0x00,0x38,0x00,0x0e,0xfc,0x03,0xfc,0x03,0x00,0x0f,0x00,0x3c,0x00,0x00,0x00,0x00,0x00,
0x00,0x80,0x03,0xf8,0x0f,0x3c,0x1c,0x0e,0x30,0x0e,0x30,0x0e,0x38,0x3c,0x1e,0xf8,0x0f,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0x7f,0xfc,0x7f,0x00,0x67,0x00,0x63,0x00,0x63,0x00,0x67,0x00,
0x3e,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0x1f,0xfc,0x3f,0xde,0x71,0x86,0x73,0x86,0x73,
0x86,0x63,0xce,0x71,0xfc,0x30,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0x1f,0xfc,0x3f,0xde,
0x71,0x86,0x63,0x86,0xe3,0x87,0xe3,0xcf,0x61,0xfe,0x21,0xfc,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
0xf0,0x1f,0xfc,0x3f,0xfe,0x7f,0x0e,0x60,0x06,0xe0,0x06,0xe0,0x0e,0x70,0xfe,0x7f,0xfc,0x1f,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x0e,0x0c,0x3f,0x9c,0x7f,0xce,0x61,0xc6,0x61,0xce,0x61,0xce,0x71,
0xfc,0x3f,0xf8,0x1f,0x00,0x01,0x00,0x00,0x00,0x00,0x18,0x00,0x1c,0x38,0x1c,0x78,0x8e,0x73,0x8e,
0x73,0x8e,0x73,0xfc,0x3f,0xfc,0x3f,0x70,0x04,0x00,0x00,0x00,0x00,0xc0,0x03,0xf8,0x1f,0xfc,0x3f,
0x1c,0x78,0x0c,0x70,0x0c,0x70,0x3c,0x3f,0xf8,0x1f,0xe0,0x07,0x00,0x00,0x00,0x00,0x00,0x07,0xf8,
0x1f,0xf8,0x3f,0x1c,0x38,0x0c,0x30,0x1c,0x38,0xfc,0x1f,0xf0,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x42,0x38,0x34,0x41,0x32,0x37,0x38,0x31,
0x37,0x32,0x64,0x02,0x01,0xfc,0x3f,0xfc,0x3f,0x0c,0x31,0x0c,0x31,0x8c,0x33,0xf8,0x1f,0xf8,0x0e,
0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0xf8,0x1f,0xdc,0x3f,0x8e,0x33,0x86,0x31,0xce,0x3b,0xfc,
0x1f,0xf8,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0x00,0xf8,0x03,0xb8,0x07,0x38,0x1e,
0x3c,0x7e,0xfe,0x7f,0x3c,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0e,0x00,0xfe,0x00,0xfe,
0x0f,0x9c,0x7f,0x1c,0x7e,0xfc,0x3f,0xfc,0x03,0x3f,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x0f,0x3c,0x1f,0x3c,0x3f,0x70,0x77,0x60,0xe7,0xe0,0xc7,0x63,0x87,0x77,0x07,0x3f,0x07,0x1e,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x00,0x60,0x00,0x60,0xff,0x60,0xfe,0x63,0xc0,0x6f,0x00,0x7e,
0x00,0x78,0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x0c,0xfc,0x3f,0xce,0x3f,0x86,0x73,0x86,
0x63,0x8e,0x73,0xde,0x7f,0xfc,0x3f,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x18,
0x00,0x38,0xfe,0x7f,0xfe,0x7f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x30,0x00,0x30,0x00,0x30,0xfc,0x30,0xfc,0x33,0x80,0x3f,0x00,0x3c,0x00,0x38,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1c,0x1c,0x3e,0x3c,0x7e,0x38,0xee,0x38,0x8c,0x3b,0x0c,0x1f,0x0c,0x0e,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};



//================================================================================


uint32 count_char( const char *buf, char chr )
{
	char *ptr=(char *)buf;
	uint32 n=0;

    if((NULL==ptr) || (0==*ptr)) return 0;
    if(0==chr) return 0;

	for( ; ; )
	{
        if(chr == *ptr)
        {
            n++;
        }
        else if(0==*ptr)
        {
            break;
        }
        ptr++;
	}
	return(n);
}




int is_ok_ip( const char *ipstr )
{
    int n;
    int a1, a2, a3, a4;

    if( count_char( ipstr, '.' ) != 3 ) return 0;
    n=sscanf( ipstr, "%d.%d.%d.%d", &a1, &a2, &a3, &a4 );

    //DBG_DIS("in is_ok_ip 1:ip=%d.%d.%d.%d\r\n",a1,a2,a3,a4);
    if( n<4 || ( ( a1<0 || a1>255 ) || (a2<0 || a2>255 ) || (a3<0 || a3>255 ) || (a4<0 || a4>255 ) ) )
    {
		return 0;
    }
    else if((255==a1)&&(255==a2)&&(255==a3)&&(255==a4))
    {
        return 0;
    }
    else if((0==a1)&&(0==a2)&&(0==a3)&&(0==a4))
    {
        return 0;
    }
	else
	{
       return 1;
    }

}


int change_ip_str_to_val(char *ipstr,uint8 *val)
{
    if(NULL==ipstr) return -1;
    if(NULL==val)   return -2;

    if(!is_ok_ip(ipstr )) return -3;

    int a1, a2, a3, a4;
    sscanf( ipstr, "%d.%d.%d.%d", &a1, &a2, &a3, &a4 );

    val[0]=a1;
    val[1]=a2;
    val[2]=a3;
    val[3]=a4;

    return 0;
}



void get_save_para(void)
{
    int i;
    DBG_DIS("in get_save_para 0:\r\n");

    get_local_ip_info(g_local_ip,g_local_mk,g_local_gw,NULL,NULL,NULL);
    if(!is_ok_ip(g_local_ip))
    {
        DBG_DIS("in get_save_para 1:g_local_ip err [%s],use default ip\r\n",g_local_ip);
        sprintf(g_local_ip,"192.168.11.150");
        sprintf(g_local_mk,"255.255.255.0");
        sprintf(g_local_gw,"192.168.11.1");
    }

    DBG_DIS("get g_local_ip: %s\r\n",g_local_ip);
    DBG_DIS("get g_local_mk: %s\r\n",g_local_mk);
    DBG_DIS("get g_local_gw: %s\r\n",g_local_gw);


    g_local_port=get_local_port();
    DBG_DIS("get g_local_port: %d\r\n",g_local_port);


    get_server_ip(g_server_ip,NULL);
    if(!is_ok_ip(g_server_ip))
    {
        DBG_DIS("in get_save_para 2:server ip err [%s]\r\n",g_server_ip);
    }
    DBG_DIS("get g_server_ip: %s\r\n",g_server_ip);


    g_server_port=get_server_port();
    DBG_DIS("get g_server_port: %d\r\n",g_server_port);



    get_net_protocol_info(&g_prot_type, &g_prot_mode);
    DBG_DIS("get g_prot_type: %d\r\n",g_prot_type);
    DBG_DIS("get g_prot_mode: %d\r\n",g_prot_mode);


    get_net_switch(&g_net_send_switch);
    DBG_DIS("get g_net_send_switch: %d\r\n",g_net_send_switch);


    get_net_stop_time(g_not_send_time,10);
    for(i=0;i<10;++i)
    {
        DBG_DIS("get g_not_send_time[%d]:%02d:%02d-%02d:%02d\r\n",i,g_not_send_time[i].time_s.time_h,g_not_send_time[i].time_s.time_m,g_not_send_time[i].time_e.time_h,g_not_send_time[i].time_e.time_m);
    }


    //sd
    get_sd_res_alarm_thr(&g_sd_res_alarm_thr);
    DBG_DIS("get g_sd_res_alarm_thr: %d\r\n",g_sd_res_alarm_thr);










    DBG_DIS("in get_save_para 100:\r\n");


}






/****************************************************************************************************
**名称:void check_net_link(void)
**功能:检测网络连接
* 入口:无
* 出口:无
**auth:hxj, date: 2014-12-19 13:58
*****************************************************************************************************/
void check_net_link(void)
{
    if(lwIPLinkStatusGet(0, 1))
    {
        net_link_ok_flag=1;
    }
    else
    {
        net_link_ok_flag=0;
    }

}


/****************************************************************************************************
**名称:char is_ok_net_link(void)
**功能:判断网络连接正常吗
* 入口:无
* 出口:无
**auth:hxj, date: 2014-12-19 14:2
*****************************************************************************************************/
char is_ok_net_link(void)
{
    if(1==net_link_ok_flag)
    {
        return 1;
    }

    return 0;
}




void sys_ddms(void)
{
    // 1ms
    static int cnt=0;

    g_sys_ddms++;
    cnt++;
    if(cnt>=1000)
    {
        cnt=0;
        g_sys_dds++;
    }

}


extern INT32U  OSTimeGet (void);


uint32 get_sys_dds(void)
{
    return OSTimeGet()/1000;
}


uint32 get_sys_ddms(void)
{
    return OSTimeGet();
}





int get_flag_icbc_fsn_net()
{
	return flag_icbc_fsn_net_state;
}

int open_flag_icbc_fsn_net()
{
	int ret = 0;
	flag_icbc_fsn_net_state = 1;
	return ret;
}

int close_flag_icbc_fsn_net()
{
	int ret = 0;
	flag_icbc_fsn_net_state = 0;
	return ret;
}


int set_gbl_net_timeout(int second)
{
    int ret=0;
    gbl_timeout_sec = second;
    return ret;
}


int get_gbl_net_timeout(void)
{
  return gbl_timeout_sec;
}

uint16 get_server_port(void)
{
    uint16 port;
    port=8234;

#if DBG_SAVE_A8_PARA_USE_ADD_EN
    read_eeprom_2byte (ADS_M_PORT, (uint8 *)&port );
#else
    flash_para_opt_read(PARA_100A_ID_SERVER_PORT,(uint8 *)&port);
#endif

    if((0xffff==port) || (0==port))
    {
        port=8234;
    }

    return port;

}


uint16 get_local_port(void)
{
    uint16 port;
    port=8234;

#if DBG_SAVE_A8_PARA_USE_ADD_EN
    read_eeprom_2byte (ADS_LOCAL_PORT, (uint8 *)&port );
#else
    flash_para_opt_read(PARA_1008_ID_LOCAL_PORT,(uint8 *)&port);
#endif

    if((0xffff==port) || (0==port))
    {
        port=8234;
    }

    return port;

}




uint16 get_server_bit_img_port(void)
{
    uint16 port;
    port=7566;

    return port;

}



int save_server_port(uint16 port)
{
    write_eeprom_2byte (ADS_M_PORT,port);
    g_server_port=port;
    DBG_DIS("save g_server_port:%d\r\n",g_server_port);
    return 0;
}




int save_local_port(uint16 port)
{
    write_eeprom_2byte (ADS_LOCAL_PORT,port);
    g_local_port=port;

    DBG_DIS("save g_local_port:%d\r\n",g_local_port);

    return 0;
}





int get_net_protocol_info(uint8 *prot_type,uint8 *prot_mode)
{
    if(NULL != prot_type)
    {

#if DBG_SAVE_A8_PARA_USE_ADD_EN
        read_eeprom_1byte(ADS_NET_PROTOCOL_TYPE,prot_type);
#else
        flash_para_opt_read(PARA_100C_ID_NET_PROTOCOL_TYPE,prot_type);
#endif

    }


    if(NULL != prot_type)
    {

#if DBG_SAVE_A8_PARA_USE_ADD_EN
        read_eeprom_1byte(ADS_NET_PROTOCOL_MODE,prot_mode);
#else
        flash_para_opt_read(PARA_100D_ID_NET_PROTOCOL_MODE,prot_type);
#endif

    }

    return 0;
}


int save_net_protocol_info(uint8 prot_type,uint8 prot_mode)
{
    write_eeprom_1byte(ADS_NET_PROTOCOL_TYPE,prot_type);
    g_prot_type=prot_type;
    write_eeprom_1byte(ADS_NET_PROTOCOL_MODE,prot_mode);
    g_prot_mode=prot_mode;
    return 0;
}


int get_net_switch(uint8 *tmp_switch)
{

#if DBG_SAVE_A8_PARA_USE_ADD_EN
    read_eeprom_1byte(ADS_NET_SWITCH,tmp_switch);
#else
    flash_para_opt_read(PARA_100B_ID_NET_SWITCH,tmp_switch);
#endif

    return 0;
}


int save_net_switch(uint8 tmp_switch)
{
    write_eeprom_1byte(ADS_NET_SWITCH,tmp_switch);
    g_net_send_switch=tmp_switch;
    return 0;
}



int get_net_stop_time(NET_MASK_ONE_TIME *tmp_not_send_time,int num)
{

#if DBG_SAVE_A8_PARA_USE_ADD_EN
    int read_len=0;
    if(NULL == tmp_not_send_time) return 0;

    if(num>10) num=10;

    read_len=num*sizeof(NET_MASK_ONE_TIME);

#if DBG_NOT_SAVE_PARA_EN
    memset((char *)tmp_not_send_time,0,read_len);
#else
    read_eeprom_nbyte(ADS_NET_STOP_TIME,(uint8 *)tmp_not_send_time,read_len);
#endif

#else
    flash_para_opt_read(PARA_100E_ID_NET_STOP_TIME,(uint8 *)tmp_not_send_time);
#endif

    return 0;

}



int save_net_stop_time(NET_MASK_ONE_TIME *tmp_not_send_time,int num)
{
    int read_len=0;
    if(NULL == tmp_not_send_time) return 0;

    if(num>10) num=10;

    read_len=num*sizeof(NET_MASK_ONE_TIME);
    write_eeprom_nbyte(ADS_NET_STOP_TIME,(uint8 *)tmp_not_send_time,read_len);
    memcpy(g_not_send_time,(char *)tmp_not_send_time,read_len);
    return 0;

}





int get_server_ip(char *str_ip,uint8 *p_ip)
{
    unsigned char tmp_ip[4];

    #if DBG_NOT_SAVE_PARA_EN
        tmp_ip[0]=192;
        tmp_ip[1]=168;
        tmp_ip[2]=16;
        tmp_ip[3]=15;
    #else
        #if DBG_SAVE_A8_PARA_USE_ADD_EN
            read_eeprom_4byte(ADS_M_IP,tmp_ip);
        #else
            flash_para_opt_read(PARA_1009_ID_SERVER_IP,tmp_ip);
        #endif

    #endif

    if(NULL!=str_ip)
    {
        sprintf(str_ip,"%d.%d.%d.%d",tmp_ip[0],tmp_ip[1],tmp_ip[2],tmp_ip[3]);
    }

    if(NULL!=p_ip)
    {
        memcpy(p_ip,tmp_ip,4);
    }
    return 0;

}


int save_server_ip(uint8 *p_ip)
{
    if(NULL==p_ip) return -1;

    write_eeprom_4byte_p (ADS_M_IP,p_ip);
    sprintf(g_server_ip,"%d.%d.%d.%d",p_ip[0],p_ip[1],p_ip[2],p_ip[3]);

    DBG_DIS("save g_server_ip:%s\r\n",g_server_ip);
    return 0;
}


void get_local_mac(char *str_mac,uint8 *p_mac)
{
    if(NULL!=str_mac)
    {
        sprintf(str_mac,"%02X:%02X:%02X:%02X:%02X:%02X"
        ,g_mac[0]
        ,g_mac[1]
        ,g_mac[2]
        ,g_mac[3]
        ,g_mac[4]
        ,g_mac[5]);
    }

    if(NULL!=p_mac)
    {
        memcpy(p_mac,g_mac,6);
    }

}



int get_local_ip_info(char *str_ip,char *str_mk,char *str_gw,uint8 *p_ip,uint8 *p_mk,uint8 *p_gw)
{
    unsigned char tmp_ip[4];
    tmp_ip[0]=192;
    tmp_ip[1]=168;
    tmp_ip[2]=16;
    tmp_ip[3]=150;

    unsigned char tmp_mk[4];
    tmp_mk[0]=255;
    tmp_mk[1]=255;
    tmp_mk[2]=255;
    tmp_mk[3]=0;

    unsigned char tmp_gw[4];
    tmp_gw[0]=192;
    tmp_gw[1]=168;
    tmp_gw[2]=16;
    tmp_gw[3]=1;

    if((NULL!=str_ip) || (NULL!=p_ip))
    {
        #if DBG_NOT_SAVE_PARA_EN
            tmp_ip[0]=192;
            tmp_ip[1]=168;
            tmp_ip[2]=16;
            tmp_ip[3]=150;
        #else
            #if DBG_SAVE_A8_PARA_USE_ADD_EN
                read_eeprom_4byte (ADS_LOCAL_IP,tmp_ip);
            #else
                flash_para_opt_read(PARA_1004_ID_LOCAL_IP,tmp_ip);
            #endif

        #endif
    }

    if((NULL!=str_mk) || (NULL!=p_mk))
    {
        #if DBG_NOT_SAVE_PARA_EN
            tmp_mk[0]=255;
            tmp_mk[1]=255;
            tmp_mk[2]=255;
            tmp_mk[3]=0;
        #else
            #if DBG_SAVE_A8_PARA_USE_ADD_EN
                read_eeprom_4byte (ADS_LOCAL_MK,tmp_mk);
            #else
                flash_para_opt_read(PARA_1005_ID_LOCAL_MK,tmp_mk);
            #endif
        #endif
    }

    if((NULL!=str_gw) || (NULL!=p_gw))
    {
        #if DBG_NOT_SAVE_PARA_EN
            tmp_gw[0]=192;
            tmp_gw[1]=168;
            tmp_gw[2]=16;
            tmp_gw[3]=1;
        #else
            #if DBG_SAVE_A8_PARA_USE_ADD_EN
                read_eeprom_4byte (ADS_LOCAL_GW,tmp_gw);
            #else
                flash_para_opt_read(PARA_1006_ID_LOCAL_GW,tmp_gw);
            #endif
        #endif
    }


    if(NULL!=str_ip)
    {
        sprintf(str_ip,"%d.%d.%d.%d",tmp_ip[0],tmp_ip[1],tmp_ip[2],tmp_ip[3]);
    }

    if(NULL!=p_ip)
    {
        memcpy(p_ip,tmp_ip,4);
    }


    if(NULL!=str_mk)
    {
        sprintf(str_mk,"%d.%d.%d.%d",tmp_mk[0],tmp_mk[1],tmp_mk[2],tmp_mk[3]);
    }

    if(NULL!=p_mk)
    {
        memcpy(p_mk,tmp_mk,4);
    }

    if(NULL!=str_gw)
    {
        sprintf(str_gw,"%d.%d.%d.%d",tmp_gw[0],tmp_gw[1],tmp_gw[2],tmp_gw[3]);
    }

    if(NULL!=p_gw)
    {
        memcpy(p_gw,tmp_gw,4);
    }

    return 0;

}




int save_local_ip_info(uint8 *p_ip,uint8 *p_mk,uint8 *p_gw)
{

    if(NULL!=p_ip)
    {
        write_eeprom_4byte_p (ADS_LOCAL_IP,p_ip);
        sprintf(g_local_ip,"%d.%d.%d.%d",p_ip[0],p_ip[1],p_ip[2],p_ip[3]);

        DBG_DIS("save g_local_ip:%s\r\n",g_local_ip);
    }

    if(NULL!=p_mk)
    {
        write_eeprom_4byte_p (ADS_LOCAL_MK,p_mk);
        sprintf(g_local_mk,"%d.%d.%d.%d",p_mk[0],p_mk[1],p_mk[2],p_mk[3]);
        DBG_DIS("save g_local_mk:%s\r\n",g_local_mk);
    }

    if(NULL!=p_gw)
    {
        write_eeprom_4byte_p (ADS_LOCAL_GW,p_gw);
        sprintf(g_local_gw,"%d.%d.%d.%d",p_gw[0],p_gw[1],p_gw[2],p_gw[3]);
        DBG_DIS("save g_local_gw:%s\r\n",g_local_gw);
    }

    return 0;

}









void init_net_protocol_para(void)
{
    init_tcp_server_data();
}



int is_arm_busy(void)
{
    if(0!=arm_busy_flag)
    {
        return 1;
    }
    return 0;
}


int is_ok_def_fsn_str_size(void)
{
    int err_flag=0;

    if(32 != sizeof(STR_FSN_HEAD))
    {
        err_flag=-1;
    }

    if(128 != sizeof(STR_ONE_SN_IMAGE_DATA))
    {
        err_flag=-2;
    }

    if(1544 != sizeof(STR_FSN_BODY_BODY))
    {
        err_flag=-3;
    }

    if(100 != sizeof(STR_FSN_BODY_HEAD))
    {
        err_flag=-4;
    }

    if(1644 != sizeof(STR_FSN_BODY))
    {
        err_flag=-5;
    }


#if 0 /*hxj amend,date 2014-9-17 0:1*/
    if(328832 != sizeof(STR_FSN_FILE))
    {
        err_flag=-6;
    }
#endif


    if(0==err_flag)
    {
        DBG_NORMAL("test fsn str size OK\r\n");
        return 1;
    }
    else
    {
        DBG_ALARM("test fsn str size err\r\n");
    }

    return 0;

}



int get_day_by_year_and_month(int year,int month)
{
    int day=30;

    //一、三、五、七、八、十、腊(十二月)，是31天，
    //剩下的除过二月份，都是30天

    if((1==month)
        || (3==month)
        || (5==month)
        || (7==month)
        || (8==month)
        || (10==month)
        || (12==month)

    )
    {
        day=31;
    }
    else if(2==month)
    {
        //闰年的话有29天,不是闰年则是28天
        if( (year%400==0)||(((year%4)==0) && ((year%100)!=0)) )
        {
            //闰年
            day=29;
        }
        else
        {
            day=28;
        }
    }

    else
    {
        day=30;
    }


    return day;

}


/****************************************************************************************************
**名称:int set_os_sys_date(char *date)
**功能:date-(格式:20141230112541)
* 入口:无
* 出口:成功返回0
**auth:hxj, date: 2014-8-28 16:8
*****************************************************************************************************/
int set_os_sys_date(char *date)
{
    char tmp_date[15];
    DSP_DATE tmp_cur_rec_date;
    uint16 one_month_day;

    if(NULL == date) return -1;

    tmp_cur_rec_date.cur_s_cnt=GET_SYS_DD_S(NULL);

    memcpy(tmp_date,date,14);
    tmp_date[14]=0;
#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 0:%s\r\n",tmp_date);
#endif

    //20140822235959
    if('2' != tmp_date[0]) return -1;
    if('0' != tmp_date[1]) return -2;
    if(('1' != tmp_date[2]) && ('2' != tmp_date[2]) && ('3' != tmp_date[2])) return -3;
    if((tmp_date[3]<'0') || (tmp_date[3]>'9')) return -4;

    tmp_cur_rec_date.year=GET_4B_ASICC_10_VAL(&tmp_date[0]);

#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 1_1:year=%d\r\n",tmp_cur_rec_date.year);
#endif

    tmp_cur_rec_date.month=GET_2B_ASICC_10_VAL(&tmp_date[4]);

#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 1_1:month=%d\r\n",tmp_cur_rec_date.month);
#endif

    if(tmp_cur_rec_date.month>12) return -5;

    tmp_cur_rec_date.day=GET_2B_ASICC_10_VAL(&tmp_date[6]);

#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 1_1:day=%d\r\n",tmp_cur_rec_date.day);
#endif


    one_month_day=get_day_by_year_and_month(tmp_cur_rec_date.year,tmp_cur_rec_date.month);
    if(tmp_cur_rec_date.day>one_month_day) return -6;

    tmp_cur_rec_date.time_h=GET_2B_ASICC_10_VAL(&tmp_date[8]);
#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 1_1:time_h=%d\r\n",tmp_cur_rec_date.time_h);
#endif

    if(tmp_cur_rec_date.time_h>23) return -7;

    tmp_cur_rec_date.time_m=GET_2B_ASICC_10_VAL(&tmp_date[10]);
#if DISPLAY_SYS_TIME_ENABLE

    DBG_NORMAL("set time 1_1:time_m=%d\r\n",tmp_cur_rec_date.time_m);
#endif

    if(tmp_cur_rec_date.time_m>59) return -8;

    tmp_cur_rec_date.time_s=GET_2B_ASICC_10_VAL(&tmp_date[12]);
#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 1_1:time_s=%d\r\n",tmp_cur_rec_date.time_s);
#endif

    if(tmp_cur_rec_date.time_s>59) return -9;

    //时间格式正确
#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 2_1:lock start\r\n");
#endif

    //copy时间
#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 2_2:lock start\r\n");
#endif

    cur_rec_date.cur_s_cnt=tmp_cur_rec_date.cur_s_cnt;
    memcpy(cur_rec_date.date_str,tmp_date,15);
    cur_rec_date.year=tmp_cur_rec_date.year;
    cur_rec_date.month=tmp_cur_rec_date.month;
    cur_rec_date.day=tmp_cur_rec_date.day;
    cur_rec_date.time_h=tmp_cur_rec_date.time_h;
    cur_rec_date.time_m=tmp_cur_rec_date.time_m;
    cur_rec_date.time_s=tmp_cur_rec_date.time_s;

    cur_rec_date.valid_flag=1;
#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 2_3:lock start\r\n");
#endif


#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 2_4:lock over\r\n");
#endif

    return 0;

}






/****************************************************************************************************
**名称:int is_ok_date(char *date)
**功能:检测date格式是否正确,date-(格式:20141230112541)
* 入口:无
* 出口:成功返回1,出错返回0
**auth:hxj, date: 2014-12-31 14:21
*****************************************************************************************************/
int is_ok_date(char *date)
{
    char tmp_date[15];
    DSP_DATE tmp_cur_rec_date;
    uint16 one_month_day;
    int err_code_val=0;

    if(NULL==date)
    {
        err_code_val=-1;
        goto time_err;
    }

    tmp_cur_rec_date.cur_s_cnt=GET_SYS_DD_S(NULL);
    memcpy(tmp_date,date,14);
    tmp_date[14]=0;
#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 0:%s\r\n",tmp_date);
#endif

    //20140822235959
    if('2' != tmp_date[0])
    {
        err_code_val=-1;
        goto time_err;
    }
    if('0' != tmp_date[1])
    {
        err_code_val=-2;
        goto time_err;
    }

    if(('1' != tmp_date[2]) && ('2' != tmp_date[2]) && ('3' != tmp_date[2]))
    {
        err_code_val=-3;
        goto time_err;
    }
    if((tmp_date[3]<'0') || (tmp_date[3]>'9'))
    {
        err_code_val=-4;
        goto time_err;
    }


    tmp_cur_rec_date.year=GET_4B_ASICC_10_VAL(&tmp_date[0]);

#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 1_1:year=%d\r\n",tmp_cur_rec_date.year);
#endif

    tmp_cur_rec_date.month=GET_2B_ASICC_10_VAL(&tmp_date[4]);

#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 1_1:month=%d\r\n",tmp_cur_rec_date.month);
#endif

    if(tmp_cur_rec_date.month>12)
    {
        err_code_val=-5;
        goto time_err;
    }

    tmp_cur_rec_date.day=GET_2B_ASICC_10_VAL(&tmp_date[6]);
#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 1_1:day=%d\r\n",tmp_cur_rec_date.day);
#endif
    one_month_day=get_day_by_year_and_month(tmp_cur_rec_date.year,tmp_cur_rec_date.month);
    if(tmp_cur_rec_date.day>one_month_day)
    {
        err_code_val=-6;
        goto time_err;
    }

    tmp_cur_rec_date.time_h=GET_2B_ASICC_10_VAL(&tmp_date[8]);
#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 1_1:time_h=%d\r\n",tmp_cur_rec_date.time_h);
#endif

    if(tmp_cur_rec_date.time_h>23)
    {
        err_code_val=-7;
        goto time_err;
    }

    tmp_cur_rec_date.time_m=GET_2B_ASICC_10_VAL(&tmp_date[10]);
#if DISPLAY_SYS_TIME_ENABLE

    DBG_NORMAL("set time 1_1:time_m=%d\r\n",tmp_cur_rec_date.time_m);
#endif

    if(tmp_cur_rec_date.time_m>59)
    {
        err_code_val=-8;
        goto time_err;
    }

    tmp_cur_rec_date.time_s=GET_2B_ASICC_10_VAL(&tmp_date[12]);
#if DISPLAY_SYS_TIME_ENABLE
    DBG_NORMAL("set time 1_1:time_s=%d\r\n",tmp_cur_rec_date.time_s);
#endif

    if(tmp_cur_rec_date.time_s>59)
    {
        err_code_val=-9;
        goto time_err;
    }

    //时间格式正确
    return 1;

time_err:

    if(err_code_val<0)
    {
        //DBG_ALARM("in is_ok_date 100:err_code_val=%d\r\n",err_code_val);
        err_code_val=0;
    }

    return 0;

}




/****************************************************************************************************
**名称:int change_date_to_val_by_2014_year(char *date,int *ret_change_time)
**功能:转换时间到秒,以2014年作为参考
* 入口:无
* 出口:成功返回0,出错返回负数
**auth:hxj, date: 2014-9-9 9:15
*****************************************************************************************************/
int change_date_to_val_by_2014_year(char *date,int *ret_change_time)
{
    int val_year=35942400;
    int val_month=2764800;
    int val_day=86400;
    int val_t_h=3600;
    int val_t_m=60;
    int val_t_s=1;

    int calc_year_val=2014;
    int year_max=57;
    int year_min=-57;
    int ret_time_val=(year_min-1)*val_year;

    uint16 tmp_year=0;
    uint16 tmp_month=0;
    uint16 tmp_day=0;
    uint16 tmp_t_h=0;
    uint16 tmp_t_m=0;
    uint16 tmp_t_s=0;

    char tmp_date[15];
    uint16 one_month_day;
    int off_year=0;
    int tmp_4b;


    if(NULL==date)              return -1;
    if(NULL==ret_change_time)   return -2;

    memcpy(tmp_date,date,14);
    tmp_date[14]=0;

    //20140822235959
    if(!is_digital_str(tmp_date[0])) return -3;
    if(!is_digital_str(tmp_date[1])) return -4;
    if(!is_digital_str(tmp_date[2])) return -5;
    if(!is_digital_str(tmp_date[3])) return -6;

    tmp_year=GET_4B_ASICC_10_VAL(&tmp_date[0]);
    tmp_month=GET_2B_ASICC_10_VAL(&tmp_date[4]);
    if(tmp_month>12) return -7;

    tmp_day=GET_2B_ASICC_10_VAL(&tmp_date[6]);
    one_month_day=get_day_by_year_and_month(tmp_year,tmp_month);
    if(tmp_day>one_month_day) return -8;

    tmp_t_h=GET_2B_ASICC_10_VAL(&tmp_date[8]);
    if(tmp_t_h>23) return -9;

    tmp_t_m=GET_2B_ASICC_10_VAL(&tmp_date[10]);
    if(tmp_t_m>59) return -10;

    tmp_t_s=GET_2B_ASICC_10_VAL(&tmp_date[12]);
    if(tmp_t_s>59) return -11;

    //时间格式正确
    tmp_4b=tmp_year;
    off_year=tmp_4b-calc_year_val;
    if(off_year>=0)
    {
        if(off_year>year_max)
        {
            return -12;

        }
    }
    else
    {
        if(off_year<year_min)
        {
            return -13;
        }
    }

    ret_time_val=off_year*val_year;
    tmp_4b=tmp_month;
    ret_time_val+=tmp_4b*val_month;
    tmp_4b=val_day;
    ret_time_val+=tmp_4b*val_day;
    tmp_4b=tmp_t_h;
    ret_time_val+=tmp_4b*val_t_h;
    tmp_4b=tmp_t_m;
    ret_time_val+=tmp_4b*val_t_m;
    tmp_4b=tmp_t_s;
    ret_time_val+=tmp_4b*val_t_s;

    *ret_change_time=ret_time_val;

    return 0;

}





/****************************************************************************************************
**名称:int get_os_sys_date(char *date)
**功能:获得dsp的系统时间,date(date大小为15个,有14个字符,例如:20140822172930)
* 入口:无
* 出口:成功返回0,出错返回负数
**auth:hxj, date: 2014-8-22 16:36
*****************************************************************************************************/
int get_os_sys_date(char *date)
{
    uint32 all_val=0;
    uint32 add_val=0;
    uint32 one_month_day=0;
    DSP_DATE tmp_cur_rec_date;
    uint32 update_flag=0;

    if(NULL == date) return -1;

    if(0==cur_rec_date.valid_flag) return -2;


    memcpy(&tmp_cur_rec_date,&cur_rec_date,sizeof(DSP_DATE));


    add_val=(GET_SYS_DD_S(NULL)-tmp_cur_rec_date.cur_s_cnt); // 秒数
    // 秒数
    if(0!=add_val)
    {
        all_val=tmp_cur_rec_date.time_s+add_val;
        tmp_cur_rec_date.time_s=all_val%60;
        add_val=all_val/60;
    }

    //分钟
    if(0!=add_val)
    {
        all_val=tmp_cur_rec_date.time_m+add_val;
        tmp_cur_rec_date.time_m=all_val%60;
        add_val=all_val/60;
    }

    //小时
    if(0!=add_val)
    {
        all_val=tmp_cur_rec_date.time_h+add_val;
        tmp_cur_rec_date.time_h=all_val%24;
        add_val=all_val/24;
    }

    //天
    if(0!=add_val)
    {
        one_month_day=get_day_by_year_and_month(tmp_cur_rec_date.year,tmp_cur_rec_date.month);
        all_val=tmp_cur_rec_date.day+add_val;
        tmp_cur_rec_date.day=all_val%one_month_day;
        add_val=all_val/one_month_day;
    }

    //月
    if(0!=add_val)
    {
        update_flag=1;
        all_val=tmp_cur_rec_date.month+add_val;
        tmp_cur_rec_date.month=all_val%12;
        add_val=all_val/12;
    }

    //年
    if(0!=add_val)
    {
        all_val=tmp_cur_rec_date.year+add_val;
        tmp_cur_rec_date.year=all_val;
    }

    sprintf(tmp_cur_rec_date.date_str,"%04d%02d%02d%02d%02d%02d",
        tmp_cur_rec_date.year,
        tmp_cur_rec_date.month,
        tmp_cur_rec_date.day,
        tmp_cur_rec_date.time_h,
        tmp_cur_rec_date.time_m,
        tmp_cur_rec_date.time_s
    );

    if(!is_ok_date(tmp_cur_rec_date.date_str)) return -5;

    memcpy(date,tmp_cur_rec_date.date_str,15);
    if(update_flag)
    {
        DBG_NORMAL("get time 2: auto update sys time\r\n");

        memcpy(cur_rec_date.date_str,tmp_cur_rec_date.date_str,sizeof(tmp_cur_rec_date.date_str));
        cur_rec_date.cur_s_cnt=tmp_cur_rec_date.cur_s_cnt;
        cur_rec_date.year=tmp_cur_rec_date.year;
        cur_rec_date.month=tmp_cur_rec_date.month;
        cur_rec_date.day=tmp_cur_rec_date.day;
        cur_rec_date.time_h=tmp_cur_rec_date.time_h;
        cur_rec_date.time_m=tmp_cur_rec_date.time_m;
        cur_rec_date.time_s=tmp_cur_rec_date.time_s;

    }


    return 0;

}




void init_tcp_server_data(void)
{
    DBG_DIS("in init_tcp_server_data 0:\r\n");

    memset(&cur_arm_mac_info,0,sizeof(cur_arm_mac_info));

    //cur_rec_date.valid_flag=0;
    p_dat_file_buf_data=(DAT_FILE_BUF *)malloc(sizeof(DAT_FILE_BUF)*RECV_DAT_FILE_BUF_MAX);
    if(NULL==p_dat_file_buf_data)
    {
        STOP_RUN("malloc dat_file_buf_data err\r\n");
    }
    else
    {
        memset((char *)p_dat_file_buf_data,0,(sizeof(DAT_FILE_BUF)*RECV_DAT_FILE_BUF_MAX));
    }


    fsn_data_max_len=sizeof(STR_FSN_FILE);

    p_g_fsn_data=(char *)malloc(fsn_data_max_len);
    if(NULL==p_g_fsn_data)
    {
        STOP_RUN("malloc p_g_fsn_data err\r\n");
    }
    else
    {
        memset((char *)p_g_fsn_data,0,fsn_data_max_len);
    }


    busi_data_max_len=sizeof(STR_BUFI_FILE);
    p_g_busi_txt=(char *)malloc(busi_data_max_len);
    if(NULL==p_g_busi_txt)
    {
        STOP_RUN("malloc p_g_busi_txt err\r\n");
    }
    else
    {
        memset((char *)p_g_busi_txt,0,busi_data_max_len);
    }

    if(0==is_ok_def_fsn_str_size())
    {
        STOP_RUN("is_ok_def_fsn_str_size is err\r\n");
    }
    g_init_tcp_ip_data_buf_over_flag=1;


    DBG_DIS("in init_tcp_server_data 100:\r\n");

}



/****************************************************************************************************
**名称:int set_dat_data_to_dat_buf(int point,char *buf,int len,char *time,int ch_time)
**功能:
* 入口:无
* 出口:成功返回0
**auth:hxj, date: 2014-9-10 9:46
*****************************************************************************************************/
int set_dat_data_to_dat_buf(int point,char *buf,int len,char *time,int ch_time)
{
    if((point<0) || (point>=RECV_DAT_FILE_BUF_MAX)) return -1;
    if(NULL ==buf)  return -2;
    if(0 ==len)     return -3;
    if(NULL ==time) return -4;

    p_dat_file_buf_data[point].dat_len=len;
    p_dat_file_buf_data[point].change_time=ch_time;
    memcpy(p_dat_file_buf_data[point].time,time,14);
    p_dat_file_buf_data[point].time[14]=0;
    p_dat_file_buf_data[point].send_time_out_dds=0;

    memcpy((char *)&p_dat_file_buf_data[point].dat_buf,buf,len);

    p_dat_file_buf_data[point].money_cnt=GET_2B_SMALLSN_VAL(p_dat_file_buf_data[point].dat_buf.head_old.all_num);
    p_dat_file_buf_data[point].dat_valid_flag=DATF_HAVE_DATA;

    return 0;

}



int set_dat_file_time_out_val(int point,uint32 time_out)
{
    if(NULL==p_dat_file_buf_data) return -1;
    if((point<0) || (point>=RECV_DAT_FILE_BUF_MAX)) return -2;

    p_dat_file_buf_data[point].send_time_out_dds=time_out;

    return 0;
}





/****************************************************************************************************
**名称:int set_dat_file_send_time_out_val(int point)
**功能:设置发送dat文件超时间
* 入口:无
* 出口:成功返回0
**auth:hxj, date: 2014-8-28 15:7
*****************************************************************************************************/
int set_dat_file_send_time_out_val(int point)
{
    uint32 time_out=GET_SYS_DD_S(NULL)+2*60;
    return set_dat_file_time_out_val(point,time_out);
}



int get_change_time_val_by_index(int point)
{
    if(NULL==p_dat_file_buf_data) return -1;

    if((point<0) || (point>=RECV_DAT_FILE_BUF_MAX)) return -2;

    return p_dat_file_buf_data[point].change_time;
}


void check_dat_file_send_time_out(void)
{
    uint32 cur_time_dds;
    int i;

    cur_time_dds=GET_SYS_DD_S(NULL);

    for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
    {
        if(DATF_SEND_START==p_dat_file_buf_data[i].dat_valid_flag)
        {
            if(cur_time_dds>p_dat_file_buf_data[i].send_time_out_dds)
            {
                p_dat_file_buf_data[i].dat_valid_flag=DATF_HAVE_DATA;
            }
        }
    }

}




/****************************************************************************************************
**名称:int get_empty_dat_buf_sum_num(void)
**功能:获取空的dat文件缓冲数
* 入口:无
* 出口:沒有返回0
**auth:hxj, date: 2014-8-25 16:48
*****************************************************************************************************/
int get_empty_dat_buf_sum_num(void)
{
    int sum_empty_num=0;
    int i;

    if(NULL == p_dat_file_buf_data) return 0;

    for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
    {
        if(DATF_EMPTY==p_dat_file_buf_data[i].dat_valid_flag)
        {
            sum_empty_num++;
        }
    }

    return sum_empty_num;

}




/****************************************************************************************************
**名称:int get_need_send_dat_file_num(void)
**功能:获取需要发送的dat文件个数
* 入口:无
* 出口:无
**auth:hxj, date: 2014-8-25 16:54
*****************************************************************************************************/
int get_need_send_dat_file_num(void)
{
    int sum_send_num=0;
    int i;

    if(NULL == p_dat_file_buf_data) return 0;

    for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
    {
        if(DATF_HAVE_DATA==p_dat_file_buf_data[i].dat_valid_flag)
        {
            sum_send_num++;
        }
    }

    return sum_send_num;
}



/****************************************************************************************************
**名称:int check_save_same_dat_file_by_time(char *time)
**功能:检测要发送的文件是否已经发送过
* 入口:无
* 出口:如果文件已经发送过,则返回1,否则返回0
**auth:hxj, date: 2014-9-16 14:40
*****************************************************************************************************/
int check_save_same_dat_file_by_time(char *tmp_time)
{
    int i;

    if(NULL == p_dat_file_buf_data) return 0;
    if(NULL == tmp_time) return 0;


    for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
    {
        if( DATF_EMPTY != p_dat_file_buf_data[i].dat_valid_flag )
        {
            if(0 == strncmp( p_dat_file_buf_data[i].time,tmp_time,14) )
            {
               return 1;
            }
        }
    }


    return 0;
}




/****************************************************************************************************
**名称:int get_empty_dat_buf_point(void)
**功能:获取dat文件缓冲空的指針
* 入口:无
* 出口:成功返回index,无空的或不可用返回最大值
**auth:hxj, date: 2014-8-25 16:48
*****************************************************************************************************/
int get_empty_dat_buf_point(void)
{
    int point_empty=RECV_DAT_FILE_BUF_MAX;
    int i;
    static int j_point=0;

    if(NULL == p_dat_file_buf_data) return RECV_DAT_FILE_BUF_MAX;

    for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
    {
        if(j_point>=RECV_DAT_FILE_BUF_MAX) j_point=0;
        if(DATF_EMPTY==p_dat_file_buf_data[j_point].dat_valid_flag)
        {
            point_empty=j_point;
            break;
        }
        j_point++;
    }
    return point_empty;
}




/****************************************************************************************************
**名称:int set_dat_file_flag_by_time(char *time,char tmp_valid_flag)
**功能:设置dat文件状态标志,通过时间time
* 入口:无
* 出口:设置成功返回0,
**auth:hxj, date: 2015-1-4 11:17
*****************************************************************************************************/
int set_dat_file_flag_by_time(char *time,char tmp_valid_flag)
{
    int i;
    int time_len=14;
    int ch_time=0;

    if(NULL == time) return -1;
    if(NULL == p_dat_file_buf_data) return -2;

    if(0==change_date_to_val_by_2014_year(time,&ch_time))
    {
        //DBG_NORMAL("change_date_to_val_by_2014_year ok\r\n");
        for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
        {
            //DBG_NORMAL("111 i=%d,dat_valid_flag=%d,[%s]\r\n",i,p_dat_file_buf_data[i].dat_valid_flag,p_dat_file_buf_data[i].time);
            if(ch_time == p_dat_file_buf_data[i].change_time)
            {

#if DBG_NOT_SAVE_SD_AND_DIR_NET_SEND_EN /*hxj amend,date 2014-12-23 13:58*/
                p_dat_file_buf_data[i].dat_valid_flag=DATF_EMPTY;
#else
                p_dat_file_buf_data[i].dat_valid_flag=tmp_valid_flag;
#endif
                return 0;
            }
        }

    }
    else
    {
        for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
        {
            //DBG_NORMAL("i=%d,dat_valid_flag=%d\r\n",i,p_dat_file_buf_data[i].dat_valid_flag);
            //DBG_NORMAL("111 i=%d,dat_valid_flag=%d,[%s]\r\n",i,p_dat_file_buf_data[i].dat_valid_flag,p_dat_file_buf_data[i].time);
            if(0 == strncmp(p_dat_file_buf_data[i].time,time,time_len))
            {
#if DBG_NOT_SAVE_SD_AND_DIR_NET_SEND_EN /*hxj amend,date 2014-12-23 13:58*/
                    p_dat_file_buf_data[i].dat_valid_flag=DATF_EMPTY;
#else
                    p_dat_file_buf_data[i].dat_valid_flag=tmp_valid_flag;
#endif
                return 0;
            }
        }

    }

    return -10;

}




/****************************************************************************************************
**名称:int set_recv_dat_file_flag_by_index(int index,uint8 tmp_valid_flag)
**功能:设置dat文件状态标志,通过index
* 入口:无
* 出口:设置成功返回0,
**auth:hxj, date: 2015-1-4 13:47
*****************************************************************************************************/
int set_recv_dat_file_flag_by_index(int index,uint8 tmp_valid_flag)
{

#if DBG_NOT_SAVE_SD_AND_DIR_NET_SEND_EN /*hxj amend,date 2014-12-23 13:58*/
    tmp_valid_flag=DATF_EMPTY;
#endif

    return set_flag_dat_buf(index,tmp_valid_flag);

}



/****************************************************************************************************
**名称:void insert_sort(int p,int r,st_alm_map *almmap)
**功能:插入排序,从小到大
* 入口:p-指向开始序号(一般为0),r-指向结尾的指针(总个数-1)
* 出口:无
*****************************************************************************************************/
void insert_sort_by_change_time ( int p, int r, DAT_TIME_FIND *tmp_find_group )
{
	DAT_TIME_FIND temp;
	int i, j;

	if ( p >= r ) return;

	for ( i = p + 1; i <= r; ++i )
	{
        memcpy(&temp,&tmp_find_group[i],sizeof(DAT_TIME_FIND));
		j = i;
		while ( 1 )
		{
			j--;
			if ( temp.change_time< tmp_find_group[j].change_time )
			{
                memcpy(&tmp_find_group[j + 1],&tmp_find_group[j],sizeof(DAT_TIME_FIND));
				if ( j <= p ) break;
			}
			else
			{
				j++;
				break;
			}
		}

        memcpy(&tmp_find_group[j],&temp,sizeof(DAT_TIME_FIND));
	}

}



void display_dat_buf_info(char *des)
{
    int i;
    //uint32 cur_dds;

    if(NULL != des)
    {
        DBG_NORMAL("%s,==============start===============\r\n",des);
    }


    if(NULL == p_dat_file_buf_data)
    {
        DBG_NORMAL("%s,==============  end===============\r\n",des);
        return;
    }


    DBG_NORMAL("cur_dds=0x%08x,(%d)\r\n",GET_SYS_DD_S(NULL),GET_SYS_DD_S(NULL));

    for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
    {
        if(DATF_EMPTY != p_dat_file_buf_data[i].dat_valid_flag)
        {
            DBG_NORMAL("i=%02d,dat_valid_flag=%d,change_time=0x%08x ,time=[%s],send_time_out_dds=0x%08x,money_cnt=%d\r\n",
                i,p_dat_file_buf_data[i].dat_valid_flag,
                p_dat_file_buf_data[i].change_time,
                p_dat_file_buf_data[i].time,
                p_dat_file_buf_data[i].send_time_out_dds,
                p_dat_file_buf_data[i].money_cnt);
        }
    }

    if(NULL != des)
    {
        DBG_NORMAL("%s,==============  end===============\r\n",des);
    }

}




int display_dat_buf_info_to_buf(char *des,char *r_buf,int r_buf_size)
{
    int i;
    uint32 cur_dds;
    int all_str_len=0;

    if(NULL != des)
    {
        if( (r_buf_size-all_str_len) > 1)
        {
            snprintf(&r_buf[all_str_len],r_buf_size-all_str_len,"%s,==start==\r\n",des);
            all_str_len=strlen(r_buf);
        }
        else
        {
            return all_str_len;
        }
    }


    if( (r_buf_size-all_str_len) > 1)
    {
        snprintf(&r_buf[all_str_len],r_buf_size-all_str_len,"dat_file_to_fsn_change_err_cnt=%d\r\n",dat_file_to_fsn_change_err_cnt);
        all_str_len=strlen(r_buf);
    }
    else
    {
        return all_str_len;
    }


    if(NULL == p_dat_file_buf_data)
    {
        if( (r_buf_size-all_str_len) > 1)
        {
            snprintf(&r_buf[all_str_len],r_buf_size-all_str_len,"%s,==  end==\r\n",des);
            all_str_len=strlen(r_buf);
        }
        else
        {
            return all_str_len;
        }

        return all_str_len;
    }

    cur_dds=GET_SYS_DD_S(NULL);

    if( (r_buf_size-all_str_len) > 1)
    {
        snprintf(&r_buf[all_str_len],r_buf_size-all_str_len,"cur_dds=0x%08x,(%d)\r\n",cur_dds,cur_dds);
        all_str_len=strlen(r_buf);
    }
    else
    {
        return all_str_len;
    }

    for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
    {
        if(DATF_EMPTY != p_dat_file_buf_data[i].dat_valid_flag)
        {
            if( (r_buf_size-all_str_len) > 1)
            {
                snprintf(&r_buf[all_str_len],r_buf_size-all_str_len,"i=%02d,valid_flag=%d,change_time=0x%08x ,time=[%s],send_time_out_dds=0x%08x,money_cnt=%d\r\n",
                    i,p_dat_file_buf_data[i].dat_valid_flag,
                    p_dat_file_buf_data[i].change_time,
                    p_dat_file_buf_data[i].time,
                    p_dat_file_buf_data[i].send_time_out_dds,
                    p_dat_file_buf_data[i].money_cnt);

                all_str_len=strlen(r_buf);
            }
            else
            {
                return all_str_len;
            }
        }
    }

    if(NULL != des)
    {
        if( (r_buf_size-all_str_len) > 1)
        {
            snprintf(&r_buf[all_str_len],r_buf_size-all_str_len,"%s,==  end==\r\n",des);
            all_str_len=strlen(r_buf);
        }
        else
        {
            return all_str_len;
        }

    }

    return all_str_len;

}




/****************************************************************************************************
**名称:int get_valid_dat_buf_point(void)
**功能:获取dat文件缓冲非空的指針(需要发送的)
* 入口:无
* 出口:成功返回指针,全空的返回最大值
**auth:hxj, date: 2014-8-25 16:56
*****************************************************************************************************/
int get_valid_dat_buf_point(void)
{
    DAT_TIME_FIND tmp_find_group[RECV_DAT_FILE_BUF_MAX];
    int need_find_num=0;
    int i;
    //int temp_i_val;

    if(NULL == p_dat_file_buf_data) return RECV_DAT_FILE_BUF_MAX;

    #if DISPLAY_DAT_FILE_INFO_ENABLE
        display_dat_buf_info("in get_valid_dat_buf_point 1:");
    #endif

    need_find_num=0;
    for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
    {
        if(DATF_HAVE_DATA == p_dat_file_buf_data[i].dat_valid_flag)
        {
            tmp_find_group[need_find_num].i_val=i;
            tmp_find_group[need_find_num].change_time=p_dat_file_buf_data[i].change_time;
            need_find_num++;
        }
    }

    if(0==need_find_num)    return RECV_DAT_FILE_BUF_MAX;

    //从小到大排序 以 change_time
    insert_sort_by_change_time( 0, need_find_num - 1, tmp_find_group );

#if 0 /*hxj amend,date 2014-9-11 15:30*/
    DBG_NORMAL("in get_valid_dat_buf_point 2_1:--------------------------\r\n");
    DBG_NORMAL("in get_valid_dat_buf_point 2_2:need_find_num=%d\r\n",need_find_num);

    for(i=0;i<need_find_num;++i)
    {
        temp_i_val=tmp_find_group[i].i_val;
        DBG_NORMAL("i=%02d,i_val=%02d,change_time=0x%08x ,time=[%s],\r\n",i,temp_i_val,tmp_find_group[i].change_time,p_dat_file_buf_data[temp_i_val].time);
    }

    DBG_NORMAL("in get_valid_dat_buf_point 2_3:--------------------------\r\n");
    DBG_NORMAL("in get_valid_dat_buf_point 1:need_find_num=%d,valid_point=%d\r\n",need_find_num,tmp_find_group[0].i_val);
#endif

    return tmp_find_group[0].i_val;

}



DAT_FILE_FORMAT_C * get_dat_file_info(int point,int *dat_len)
{
    if( (point>=0) && (point<RECV_DAT_FILE_BUF_MAX))
    {
        if(NULL != dat_len)
        {
            *dat_len=p_dat_file_buf_data[point].dat_len;
        }
        return &p_dat_file_buf_data[point].dat_buf;
    }

    return NULL;
}



/****************************************************************************************************
**名称:int set_flag_dat_buf(int point,uint8 set_valid_flag)
**功能:设置dat文件缓冲标志
* 入口:无
* 出口:成功返回0
**auth:hxj, date: 2014-9-5 13:30
*****************************************************************************************************/
int set_flag_dat_buf(int point,uint8 set_valid_flag)
{
    int i;

    if(NULL==p_dat_file_buf_data) return -1;

    if( (point>=0) && (point<RECV_DAT_FILE_BUF_MAX))
    {
        if(DATF_EMPTY==set_valid_flag)
        {
            p_dat_file_buf_data[point].dat_len=0;
            p_dat_file_buf_data[point].money_cnt=0;
            p_dat_file_buf_data[point].change_time=0;
            p_dat_file_buf_data[point].send_time_out_dds=0;
            //memset((char *)p_dat_file_buf_data[point].time,0,15);
            for(i=0;i<15;++i)
            {
                p_dat_file_buf_data[point].time[i]=0;

            }

            p_dat_file_buf_data[point].dat_buf.head_old.all_num[0]=0;
            p_dat_file_buf_data[point].dat_buf.head_old.all_num[1]=0;
            p_dat_file_buf_data[point].dat_valid_flag=DATF_EMPTY;
        }
        else
        {
            p_dat_file_buf_data[point].dat_valid_flag=set_valid_flag;
        }

    }
    else
    {
        return -2;
    }

    return 0;
}



/****************************************************************************************************
**名称:void clear_dat_buf(int point)
**功能:清除dat文件缓冲标志
* 入口:无
* 出口:无
**auth:hxj, date: 2014-8-25 17:1
*****************************************************************************************************/
void clear_dat_buf(int point)
{
    set_flag_dat_buf(point,DATF_EMPTY);
}



void clear_dat_buf_by_change_time(int ch_time)
{
    int i;

    for(i=0;i<RECV_DAT_FILE_BUF_MAX;++i)
    {
        if((DATF_SEND_START==p_dat_file_buf_data[i].dat_valid_flag) && (ch_time==p_dat_file_buf_data[i].change_time))
        {
            set_flag_dat_buf(i,DATF_EMPTY);
            break;
        }
    }

}



/****************************************************************************************************
**名称:int creat_icbc_busi_file_by_fsn_file(char *fsn_buf,int fsn_len,char *busi_buf,int *ret_busi_len,DSP_upload_head *p_upload_head)
**功能:创建工行 busi 文件,从fsn 文件中
* 入口:无
* 出口:相同返回1,否则返回负数
**auth:hxj, date: 2014-9-28 13:9
*****************************************************************************************************/
int creat_icbc_busi_file_by_fsn_file(char *fsn_buf,int fsn_len,char *busi_buf,int *ret_busi_len,DSP_upload_head *p_upload_head)
{

    STR_FSN_HEAD *p_fsn_head=NULL;
    STR_FSN_BODY *p_fsn_info=NULL;
    STR_FSN_BODY *p_fsn_info_one=NULL;
    busi_head *p_busi_head=NULL;
    busi_info *p_busi_info=NULL;
    busi_info *p_busi_info_one=NULL;
    int mon_k=0;
    int j=0;
    int str_len=0;
    int money_all_cnt=0;
    int year;
    int month;
    int day;
    int t_h,t_m,t_s;
    char tmp_str[100]={0};
    char tmp_str_fsn[100]={0};
	int Ver_code=0;
    uint8 *p_c=NULL;

    if(NULL == fsn_buf)     	return -1;
    if(NULL == busi_buf)    	return -2;
	if(NULL == ret_busi_len)    return -3;
    if(NULL == p_upload_head)   return -4;
    if( fsn_len<=0)             return -5;



    p_fsn_head=(STR_FSN_HEAD *)fsn_buf;
    p_fsn_info=(STR_FSN_BODY *)&p_fsn_head[1];

    money_all_cnt=p_fsn_head->Counter;
    if( fsn_len != sizeof(STR_FSN_HEAD)+sizeof(STR_FSN_BODY)*money_all_cnt )
    {
        return -5;
    }

    p_busi_head=(busi_head *)busi_buf;
    p_busi_info=(busi_info *)&p_busi_head[1];

	//设置busi 头信息
    memset(tmp_str,0,sizeof(tmp_str));
    memset(tmp_str_fsn,0,sizeof(tmp_str_fsn));


#if 0 /*hxj amend,date 2014-10-8 14:49*/
    MY_LOGBUF(p_upload_head->local_sn,4,"local_sn=");
    MY_LOGBUF(p_upload_head->sub_dank_sn,4,"sub_dank_sn=");
    MY_LOGBUF(p_upload_head->net_sn,4,"net_sn=");
#endif


#if 0 /*hxj amend,date 2014-9-26 15:47*/
    //test
    memcpy(p_busi_head->local_sn,"1234",4);
    memcpy(p_busi_head->sub_dank_sn,"5678",4);
    memcpy(p_busi_head->net_sn,"9012",4);
#else
    p_c=p_upload_head->local_sn;
    if(!(is_digital_str(p_c[0]) && is_digital_str(p_c[1]) && is_digital_str(p_c[2]) && is_digital_str(p_c[3])))
    {
        memcpy(p_upload_head->local_sn,"1234",4);
    }

    p_c=p_upload_head->sub_dank_sn;
    if(!(is_digital_str(p_c[0]) && is_digital_str(p_c[1]) && is_digital_str(p_c[2]) && is_digital_str(p_c[3])))
    {
        memcpy(p_upload_head->sub_dank_sn,"5678",4);
    }

    p_c=p_upload_head->net_sn;
    if(!(is_digital_str(p_c[0]) && is_digital_str(p_c[1]) && is_digital_str(p_c[2]) && is_digital_str(p_c[3])))
    {
        memcpy(p_upload_head->net_sn,"9012",4);
    }

    memcpy(p_busi_head->local_sn,p_upload_head->local_sn,4);
    memcpy(p_busi_head->sub_dank_sn,p_upload_head->sub_dank_sn,4);
    memcpy(p_busi_head->net_sn,p_upload_head->net_sn,4);

#endif

    memcpy(p_busi_head->fact_sn,"10",2);
    memcpy(p_busi_head->mach_sn,&p_upload_head->mach_code_str[11],10);

    memcpy(p_busi_head->busi_type,"11",2);

#if defined(FZ2000) || defined(FZ1500) /*hxj amend,date 2014-11-26 15:8*/
    memcpy(p_busi_head->mach_type,"03",2);  //03-清分机
#else
    memcpy(p_busi_head->mach_type,"05",2);  //05-A类机
#endif

    memcpy(p_busi_head->card_sn,"                   ",19);      //卡号
    memcpy(p_busi_head->Transaction_code,"     ",5);            //交易代码
    memcpy(p_busi_head->mach_s_num,"      ",6);                 //端机流水号
    memcpy(p_busi_head->rh_busi_type,"GM",2);


    for(mon_k=0;mon_k<money_all_cnt;++mon_k)
    {
        p_fsn_info_one=&p_fsn_info[mon_k];
        p_busi_info_one=&p_busi_info[mon_k];

        memset(tmp_str,0,sizeof(tmp_str));
        memset(tmp_str_fsn,0,sizeof(tmp_str_fsn));

        //20140526092251
        year=(p_fsn_info_one->data_head.Date>>9) + 1980;
        month=(p_fsn_info_one->data_head.Date&0x1ff)>>5;
        day=(p_fsn_info_one->data_head.Date&0x1f);

        t_h=(p_fsn_info_one->data_head.Time)>>11;
        t_m=(p_fsn_info_one->data_head.Time&0x7ff)>>5;
        t_s=(p_fsn_info_one->data_head.Time&0x1f)<<1;

        sprintf(tmp_str,"%04d%02d%02d%02d%02d%02d",year,month,day,t_h,t_m,t_s);
        strcat(tmp_str_fsn,tmp_str);

        // UH54990236
        str_len=10;
        for(j=0;j<str_len;++j)
        {
            tmp_str[j]=p_fsn_info_one->data_head.SNo[j] ;
        }
        tmp_str[str_len]=0;
        strcat(tmp_str_fsn,tmp_str);

        // CNY
        str_len=3;
        for(j=0;j<str_len;++j)
        {
            tmp_str[j]=p_fsn_info_one->data_head.MoneyFlag[j];
        }
        tmp_str[str_len]=0;
        strcat(tmp_str_fsn,tmp_str);

        //050
        sprintf(tmp_str,"%03d",p_fsn_info_one->data_head.Valuta);
        strcat(tmp_str_fsn,tmp_str);

        //1999
        Ver_code=0;
        switch(p_fsn_info_one->data_head.Ver)
        {
                case 0:
                    Ver_code=1990;
                break;
                case 1:
                    Ver_code=1999;
                break;
                case 2:
                    Ver_code=2005;
                break;
                default:
#if 0 /*hxj amend,date 2014-7-22 17:38*/
                    //test
                    Ver_code=0;
#else
                    Ver_code=9999;
#endif
                break;
        }
        sprintf(tmp_str,"%04d",Ver_code);
        strcat(tmp_str_fsn,tmp_str);

        //真假
        sprintf(tmp_str,"%d",p_fsn_info_one->data_head.tfFlag);
        strcat(tmp_str_fsn,tmp_str);

        //成色
        sprintf(tmp_str,"00");
        strcat(tmp_str_fsn,tmp_str);

        //序号
        sprintf(tmp_str,"%05d",mon_k+1);
        strcat(tmp_str_fsn,tmp_str);
        //备用
        sprintf(tmp_str,"          ");
        strcat(tmp_str_fsn,tmp_str);

        //--------------------------------------
		memcpy(p_busi_info_one,tmp_str_fsn,(int)sizeof(busi_info));
    }


	*ret_busi_len=sizeof(busi_head)+sizeof(busi_info)*money_all_cnt;


    return 1;

}




/****************************************************************************************************
**名称:int is_may_net_send(void)
**功能:检测是否可以网发
* 入口:无
* 出口:可以网发,返回1
**auth:hxj, date: 2014-12-23 19:19
*****************************************************************************************************/
int is_may_net_send(void)
{
    return 1;
    
    if( (!is_ok_net_link()) || (0==g_app_net_up_flag) )
    {
        return 0;
    }

    //检测不网发时段

    int i;
    char date[15];
    uint32 cur_ds=0;
    for(i=0;i<GET_LIST_NUM(g_not_send_time);++i)
    {
        if ( (g_not_send_time[i].time_s.time_h>23) ||(g_not_send_time[i].time_s.time_m>59))  continue;

        g_not_send_time_ds[i].time_s_ds=g_not_send_time[i].time_s.time_h*60+g_not_send_time[i].time_s.time_m;
        g_not_send_time_ds[i].time_e_ds=g_not_send_time[i].time_e.time_h*60+g_not_send_time[i].time_e.time_m;

        if( g_not_send_time_ds[i].time_s_ds==g_not_send_time_ds[i].time_e_ds )
        {
            continue;
        }
        else
        {
            if(0!=get_os_sys_date(date))
            {
                return 1;
            }

            //2014 08 22 17 29 30
            cur_ds=(GET_2B_ASICC_10_VAL(&date[8]))*60+GET_2B_ASICC_10_VAL(&date[10]);
            if(g_not_send_time_ds[i].time_e_ds > g_not_send_time_ds[i].time_s_ds)
            {
                if((cur_ds >= g_not_send_time_ds[i].time_s_ds)&&(cur_ds < g_not_send_time_ds[i].time_e_ds))
                {
                    return 0;
                }
            }
            else
            {
                if((cur_ds >= g_not_send_time_ds[i].time_e_ds)&&(cur_ds < g_not_send_time_ds[i].time_s_ds))
                {
                    return 0;
                }
            }

        }

    }

    return 1;

}







/****************************************************************************************************
**名称:int is_may_net_send_big(void)
**功能:检测是否可以网发大图
* 入口:无
* 出口:可以网发,返回1
**auth:hxj, date: 2015-3-4 19:32
*****************************************************************************************************/
int is_may_net_send_big(void)
{
    if( (!is_ok_net_link()) || (0==g_app_net_up_flag) )
    {
        return 0;
    }

    return 1;

}













/****************************************************************************************************
**名称:int my_send(SK_TYPE sk,char *buf,int len,int flag)
**功能:
* 入口:无
* 出口:成功返回发送的长度,出错返回负数
**auth:hxj, date: 2015-1-8 9:36
*****************************************************************************************************/
int my_send(SK_TYPE sk,char *buf,int len,int flag)
{
	int one_send_len=1460;
	int cnt=0;
	int ys=0;
	int send_point=0;
	int i;
	int ret_send=0;
    int err_code=0;

#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in my_send 0:send_len=%d\r\n", len);
#endif

    if((NULL==buf) || (len<=0)) return 0;

    if(!is_may_net_send())
    {
        err_code=-300;
        goto send_err;
    }

	cnt=len/one_send_len;
	ys=len%one_send_len;

	for(i=0;i<cnt;++i)
	{
        if(!is_may_net_send())
        {
            err_code=-300;
            goto send_err;
        }

		ret_send=send(sk,&buf[send_point],one_send_len,flag);
		if(ret_send<0)
		{
            err_code=-1;
            goto send_err;
        }

		send_point+=one_send_len;

	}


	if(0!=ys)
	{
        if(!is_may_net_send())
        {
            err_code=-300;
            goto send_err;
        }
		ret_send=send(sk,&buf[send_point],ys,flag);
		if(ret_send<0)
		{
            err_code=-2;
            goto send_err;
        }
		send_point+=ys;
	}

#if DISPLAY_CONNECT_ENABLE
    if(len != send_point)
    {
        DBG_NORMAL("in my_send 1:send err1,len=%d,send_point=%d\r\n", len,send_point);
    }
    else
    {
        DBG_NORMAL("in my_send 2:send data ok,send_point=%d\r\n", send_point);
    }
#endif

	return send_point;

send_err:

    return err_code;

}





/****************************************************************************************************
**名称:int my_send_big_img(SK_TYPE sk,char *buf,int len,int flag)
**功能:
* 入口:无
* 出口:成功返回发送的长度,出错返回负数
**auth:hxj, date: 2015-3-4 19:29
*****************************************************************************************************/
int my_send_big_img(SK_TYPE sk,char *buf,int len,int flag)
{
	int one_send_len=1460;
	int cnt=0;
	int ys=0;
	int send_point=0;
	int i;
	int ret_send=0;
    int err_code=0;

#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in my_send_big_img 0:send_len=%d\r\n", len);
#endif

    if((NULL==buf) || (len<=0)) return 0;

    if(!is_may_net_send_big())
    {
        err_code=-300;
        goto send_err;
    }

	cnt=len/one_send_len;
	ys=len%one_send_len;

	for(i=0;i<cnt;++i)
	{
        if(!is_may_net_send_big())
        {
            err_code=-301;
            goto send_err;
        }

		ret_send=send(sk,&buf[send_point],one_send_len,flag);
		if(ret_send<0)
		{
            DBG_DIS("in my_send_big_img 1:send err,ret_send=%d\r\n", ret_send);
            err_code=-1;
            goto send_err;
        }

		send_point+=one_send_len;

	}


	if(0!=ys)
	{
        if(!is_may_net_send_big())
        {
            err_code=-302;
            goto send_err;
        }

		ret_send=send(sk,&buf[send_point],ys,flag);
		if(ret_send<0)
		{
            DBG_DIS("in my_send_big_img 2:send err,ret_send=%d\r\n", ret_send);
            err_code=-2;
            goto send_err;
        }
		send_point+=ys;
	}

#if DISPLAY_CONNECT_ENABLE
    if(len != send_point)
    {
        DBG_NORMAL("in my_send_big_img 10_1:send err1,len=%d,send_point=%d\r\n", len,send_point);
    }
    else
    {
        DBG_NORMAL("in my_send_big_img 10_2:send data ok,send_point=%d\r\n", send_point);
    }
#endif

	return send_point;

send_err:

    return err_code;

}



/****************************************************************************************************
**名称:int my_recv(SK_TYPE sk,char *buf,int len,int flag)
**功能:
* 入口:无
* 出口:无
**auth:hxj, date: 2015-3-4 19:30
*****************************************************************************************************/
int my_recv(SK_TYPE sk,char *buf,int len,int flag)
{
    if((NULL==buf) || (len<=0)) return -200;

    if(!is_may_net_send())
    {
        return -300;
    }

    return recv(sk,buf,len,flag);

}



/****************************************************************************************************
**名称:int my_recv_big_img(SK_TYPE sk,char *buf,int len,int flag)
**功能:
* 入口:无
* 出口:无
**auth:hxj, date: 2015-3-4 19:30
*****************************************************************************************************/
int my_recv_big_img(SK_TYPE sk,char *buf,int len,int flag)
{
    if((NULL==buf) || (len<=0)) return -200;

    if(!is_may_net_send_big())
    {
        return -300;
    }

    return recv(sk,buf,len,flag);

}









#define DBG_SYS_TIME_NOT_SYNC_ENABLE    0


/****************************************************************************************************
**名称:int net_send_icbc_fsn_by_fsn(void)
**功能:发送工行的fsn文件(在用的)
* 入口:无
* 出口:成功返回大小0,成功发送一个数据,返回10
**auth:hxj, date: 2014-9-26 13:44
*****************************************************************************************************/
int net_send_icbc_fsn_by_fsn(void)
{
	int ret = 0;
	struct sockaddr_in sa_server;
	int len_recv;
	int i_0;
	unsigned int u_i_0;
	char ch_0;
	char buf_snd[300];
	char buf_recv[1024];
	int len_to_send;
	unsigned int len_txt;
	unsigned int len_fsn;
	char *p_icbc_buf_fsn;
    int errcode=-4;
    int ok_data_len=14;
    char time[15]={0};
    char buf_err_code[5]={'4','4','4','4',0};
    uint16 err_code_val=0x4000;
    uint16 port;
    char ip[30]={0};

    int  dat_point=0;
    DAT_FILE_FORMAT_C * p_dat_file=NULL;
    int dat_file_len=0;
    int tmp_ret=0;
    int tmp_ret2=0;
    DSP_upload_head tmp_upload_head;
    //int delay_arm_time_sysc_ack_cnt=0;
    static int first_run_delay_flag=1;
    //static int one_delay_flag=1;
    uint16 money_all_cnt=0;
    int timeout_ms=0;


    if(!is_may_net_send())
    {
#if DISPLAY_CONNECT_ENABLE
        DBG_NORMAL("may not send,retur 0\r\n");
#endif
        return 0;
    }


    if(1==g_send_big_img_flag)
    {
#if DISPLAY_CONNECT_ENABLE
        //DBG_NORMAL("g_send_big_img_flag,retur 0\r\n");
#endif
        return 0;
    }

    port=g_server_port;
    memcpy(ip,g_server_ip,sizeof(g_server_ip));
    if(!is_ok_ip(ip))
    {
        DBG_DIS("server ip err [%s],retur 0\r\n",ip);
        MY_DELAY_X_S(10);
        return 0;
    }


    if(1==first_run_delay_flag)
    {
        first_run_delay_flag=0;
        DBG_NORMAL("first delay start,sleep %d\r\n",PARA_FIRST_DELAY_S_VAL);
        MY_DELAY_X_S(PARA_FIRST_DELAY_S_VAL);
        DBG_NORMAL("first delay end \r\n");
        //g_dsp_sync_time_delya_dds=GET_SYS_DD_S("4")+60;

    }

    if((0 == get_need_send_dat_file_num())&&(1==g_get_net_date_flag))
    {
        errcode=0;
        goto err;
    }


    if(0 == get_need_send_dat_file_num())
    {
        if((0==g_dsp_recv_arm_sync_time_ack_flag) && ( (0==g_dsp_sync_time_delya_dds) || (GET_SYS_DD_S("6")>=g_dsp_sync_time_delya_dds)))
        {
            ;
        }
        else
        {
            errcode=0;
            goto err;
        }
    }


    if(NULL == p_g_fsn_data)
    {
        errcode=-150;
		goto err;
    }


    if(NULL == p_g_busi_txt)
    {
        errcode=-151;
		goto err;
    }


    p_g_busi_txt_data=&p_g_busi_txt[21];

    p_icbc_buf_fsn =p_g_fsn_data;


    g_connect_wz_start_flag=0;
    g_connect_wz_timeout_s=0;
    if( INVALID_SOCKET != g_sk_wz)
    {
        DBG_NORMAL("alarm,close socket,g_sk_wz=0x%08x\r\n",(int)g_sk_wz);
        SK_COLSE(g_sk_wz);
        g_sk_wz=INVALID_SOCKET;
    }

	g_sk_wz = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_sk_wz == INVALID_SOCKET)
    {
        g_sk_wz =INVALID_SOCKET;
		errcode=-301;
		goto err;
	}

#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("creat,socket,g_sk_wz=0x%08x\r\n",(int)g_sk_wz);
#endif

#if 1 /*hxj amend,date 2014-12-17 15:44*/

#if 0 /*hxj amend,date 2014-12-23 16:31*/
	if(-1 == setsockopt(g_sk_wz, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) )
	{

        DBG_NORMAL("errno=%d\r\n",errno);
		errcode=-302;
        goto err;
    }
#endif

    timeout_ms=get_gbl_net_timeout()*1000;
    if( -1 == setsockopt(g_sk_wz, SOL_SOCKET, SO_RCVTIMEO, &timeout_ms, sizeof(timeout_ms)) )
    {
		errcode=-303;
        goto err;
    }
#endif



    #if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("get net time,connect to..., server ip=%s,port=%d\r\n",ip,port);
    #endif

    memset(&sa_server,0,sizeof(struct sockaddr_in));
	sa_server.sin_family = AF_INET;
	sa_server.sin_len = sizeof(sa_server);
    sa_server.sin_port = htons ( port );
    sa_server.sin_addr.s_addr = inet_addr ( ip );

    g_connect_wz_start_flag=0;
    g_connect_wz_timeout_s=GET_SYS_DD_S(NULL)+get_gbl_net_timeout()+60;
    g_connect_wz_start_flag=1;


//#if DISPLAY_CONNECT_ENABLE
#if 0
    //test
    DBG_NORMAL("\r\n connect sleep 10s start\r\n");
    MY_DELAY_X_S(10);
    DBG_NORMAL("connect sleep 10s   end\r\n");
#endif

	ret = connect(g_sk_wz, (struct sockaddr*)&sa_server, sizeof(sa_server));
	if(ret < 0)
    {
        g_connect_wz_start_flag=0;
        g_connect_wz_timeout_s=0;

        get_os_sys_date(time);

        DBG_NORMAL("connect err, server ip=%s,port=%d,time=%s,dds=%d\r\n",ip,port,time,GET_SYS_DDS(NULL));
        //DBG_NORMAL("connect err, delay  %d s \r\n",PARA_CONNECT_ERR_DELAY_S_VAL);
        MY_DELAY_X_S(PARA_CONNECT_ERR_DELAY_S_VAL);

		errcode=-5;
        goto err;
	}

#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("connect ok \r\n",ip,port);
#endif

    g_connect_wz_start_flag=0;
    g_connect_wz_timeout_s=0;

#if 0==DBG_SYS_TIME_NOT_SYNC_ENABLE

    if(0==g_get_net_date_flag)
    {

        //0. time sync
    	len_to_send = 4;
    	memcpy(buf_snd, "0005", len_to_send);

#if DISPLAY_CONNECT_ENABLE
        DBG_NORMAL("in net_send_icbc_fsn_by_fsn 10_1:send data\r\n");
#endif

    	ret = my_send(g_sk_wz, buf_snd, len_to_send, 0);
    	if (ret < 0)
    	{
            errcode=-101;
    		goto err;
    	}


#if DISPLAY_CONNECT_ENABLE
        DBG_NORMAL("in net_send_icbc_fsn_by_fsn 10_2:recv data\r\n");
#endif

    	// recv
    	len_recv = my_recv(g_sk_wz, buf_recv, sizeof(buf_recv), 0);
    	if (len_recv < 0)
    	{
            errcode=-102;
    		goto err;
    	}

    	if ( 22 != len_recv )
    	{
            errcode=-103;
    		goto err;
    	}

    	if (strncmp(&buf_recv[0], "0000", 4) != 0)
    	{
            errcode=-104;
    		goto err;
    	}

        //接收：30 30 30 30 0E 00 00 00 32 30 31 34 30 37 31 37 31 31 30 38 34 31
    	if( GET_4B_SMALLSN_VAL(&buf_recv[4]) != ok_data_len)
        {
            errcode=-105;
            goto err;
    	}

        if(0 != set_os_sys_date(&buf_recv[8]) )
        {
            errcode=-106;
            goto err;
        }
        else
        {
            g_get_net_date_flag=1;
            DBG_NORMAL("get net time ok\r\n");
        }

    }

    #if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("get net time ok\r\n");
    #endif

#if 0 /*hxj amend,date 2014-9-28 10:46*/
	get_os_sys_date(time);
    DBG_NORMAL("get net time=%s \r\n",time);
#endif


#if 0 /*hxj amend,date 2015-1-7 9:2*/
    if((0==g_dsp_recv_arm_sync_time_ack_flag) && ( (0==g_dsp_sync_time_delya_dds) || (GET_SYS_DD_S("6")>=g_dsp_sync_time_delya_dds)) )
    {
        //add time sync to arm
        g_dsp_sync_time_delya_dds=GET_SYS_DD_S("7")+60;

        memset(time,0,sizeof(time));
        if(0==get_os_sys_date(time))
        {
            if(!is_arm_busy())
            {
                delay_arm_time_sysc_ack_cnt=10;
                while(delay_arm_time_sysc_ack_cnt--)
                {
                    DELAY_X_MS(100);
                    if(1==g_dsp_recv_arm_sync_time_ack_flag)
                    {
                        DBG_NORMAL("arm sysc time ok,delay_arm_time_sysc_ack_cnt=%d\r\n",delay_arm_time_sysc_ack_cnt);
                        break;
                    }
                    else
                    {
                        DBG_NORMAL("wait arm sysc time ack,delay_arm_time_sysc_ack_cnt=%d\r\n",delay_arm_time_sysc_ack_cnt);
                    }
                }
            }
        }
    }
#else
    //DBG_NORMAL("not sync sys time \r\n");
#endif
#endif

    if(0 == get_need_send_dat_file_num())
    {
        errcode=0;
        goto err;
    }

find_next:

    if(1==g_send_big_img_flag)
    {
#if DISPLAY_CONNECT_ENABLE
        //DBG_NORMAL("g_send_big_img_flag,retur 0\r\n");
#endif
        errcode=0;
        goto err;
    }


    if(0 == get_need_send_dat_file_num())
    {
        errcode=-201;
		goto err;
    }

    dat_point=get_valid_dat_buf_point();
    if(dat_point>=RECV_DAT_FILE_BUF_MAX)
    {
        errcode=-202;
        goto err;

    }

    DBG_NORMAL("send index=%02d \r\n",dat_point);

    p_dat_file=get_dat_file_info(dat_point,&dat_file_len);
    money_all_cnt=GET_2B_SMALLSN_VAL(p_dat_file->head_old.all_num);

    //转换dat格式到fsn格式
    tmp_ret=creat_fsn_by_dat((STR_FSN_FILE *)p_g_fsn_data,fsn_data_max_len,(int *)&len_fsn,p_dat_file,dat_file_len);
    tmp_ret2=get_upload_head_info_by_dat_file(&tmp_upload_head,p_dat_file,dat_file_len);
    if( (0!=tmp_ret) || (0!=tmp_ret2))
    {
        DBG_ALARM("dat file format err,del file\r\n");
        set_recv_dat_file_flag_by_index(dat_point,DATF_STOP_SEND);
        goto find_next;
    }

    //创建工行 busi 文件,从fsn 文件中
    creat_icbc_busi_file_by_fsn_file((char *)p_g_fsn_data,len_fsn,p_g_busi_txt_data,(int *)&len_txt,&tmp_upload_head);
    //----------------------------------------

    //DBG_NORMAL("len_fsn=%d,len_txt=%d\r\n",len_fsn,len_txt);

    //=====================================================

	// 1. check server state
	len_to_send = 4;
	memcpy(buf_snd, "0001", len_to_send);


#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_1:send data,len_to_send=%d\r\n",len_to_send);
#endif
	ret = my_send(g_sk_wz, buf_snd, len_to_send, 0);
	if (ret < 0) {
		errcode=-6;
        goto err;
	}


#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_2:recv data\r\n");
#endif

	// recv
	len_recv = my_recv(g_sk_wz, buf_recv, sizeof(buf_recv), 0);
	if (len_recv > 3) {
		if (strncmp(buf_recv, "0000", 4) != 0)
        {
            //server busy
			memcpy(buf_err_code, buf_recv, 4);
			errcode=-23;
			goto err;
		}
	} else if (len_recv < 0){
		errcode=-7;
        goto err;
	}


	// 2. send machine id, etc, head
	memcpy(buf_snd, "0003", 4);
	i_0 = 32;
	memset(buf_snd + 4, 0, 4);
	memcpy(buf_snd + 4, (char *)&i_0, 4);

    //网发时间 &buf_snd[8]
    if( 0!= get_os_sys_date(time) )
    {
		errcode=-107;
        goto err;
    }
    memcpy(&buf_snd[8],time,8);

    //机具编号  &buf_snd[16] ,此处的 机具编号 应该为机构编号
    //24 位机具编号格式：
    //地区号4位，支行号4
    //位，网点号4位，厂家编
    //号2位，机器编号是10位，
    //如不足10位，前面补0，
    //厂家编号2位由于A 类点
    //钞机厂商仍未出台，暂全
    //部设置为00
    //注:就是busi文件中头24个字节！

    memcpy(&buf_snd[16],p_g_busi_txt_data,24);
	len_to_send = 40;

#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_3:send data,len_to_send=%d\r\n",len_to_send);
#endif
	ret = my_send(g_sk_wz, buf_snd, len_to_send, 0);
	if (ret < 0) {
		errcode=-8;
        goto err;
	}

	// recv
#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_4:recv data\r\n");
#endif
	len_recv = my_recv(g_sk_wz, buf_recv, sizeof(buf_recv), 0);
	if (len_recv > 3) {
		if (strncmp(buf_recv, "0000", 4) != 0) {
			memcpy(buf_err_code, buf_recv, 4);
			errcode=-33;
			goto err;
		}
	} else if (len_recv < 0){
		errcode=-9;
        goto err;
	}


	// 3. send img data
	memcpy(buf_snd, "0004", 4);
	memset(buf_snd + 4, 0, 4);


	u_i_0 = len_fsn + 13 + 1 ; //1078 + 116*16;
	memcpy(buf_snd + 4, (char *)&u_i_0, 4);
	ch_0 = 13;
	memcpy(buf_snd + 8, (char *)&ch_0, 1);
	memcpy(buf_snd + 9, "imageInfo.fsn", 13);


	len_to_send = 22;

#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_5:send data,len_to_send=%d,BY_LEN=%d,fsn_file=%d\r\n",len_to_send,(len_to_send+len_fsn-8),len_fsn);
#endif
	ret = my_send(g_sk_wz, buf_snd, len_to_send, 0);
	if (ret < 0) {
		errcode=-10;
        goto err;
	}

#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_51:send data,len_fsn=%d\r\n",len_fsn);
#endif
	ret = my_send(g_sk_wz, p_icbc_buf_fsn, len_fsn, 0);
	if (ret < 0) {
		errcode=-12;
		goto err;
	}

	// here, recv, 1152476=701 pcs, len_recv == -1
	// recv

#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_6:recv data,ddms=%d\r\n",get_sys_ddms());
#endif

	len_recv = my_recv(g_sk_wz, buf_recv, sizeof(buf_recv), 0);
	if (len_recv > 3) {
		if (strncmp(buf_recv, "0000", 4) != 0) {
			memcpy(buf_err_code, buf_recv, 4);
			errcode=-43;
			goto err;
		}
	}
    else if (len_recv < 0)
	{
        DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_61:len_recv=%d,ddms=%d\r\n",len_recv,get_sys_ddms());
		errcode=-13;
	    goto err;
	}


	// 4. a. send head text data
	memcpy(buf_snd, "0004", 4);
	memset(buf_snd + 4, 0, 4);
	u_i_0 = len_txt + 12 + 1;
	memcpy(buf_snd + 4, (char *)&u_i_0, 4);
	ch_0 = 12;
	memcpy(buf_snd + 8, (char *)&ch_0, 1);
	memcpy(buf_snd + 9, "busiInfo.txt", 12);

    memcpy(p_g_busi_txt,buf_snd,21);

    len_to_send = 21+len_txt;

#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_7:send data,send_len=%d,BY_LEN=%d,busi_file=%d\r\n",len_to_send,(len_to_send-8),len_txt);
#endif
    ret = my_send(g_sk_wz, p_g_busi_txt, len_to_send, 0);
	if (ret < 0) {
		errcode=-17;
        goto err;
	}

	// recv
#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_8:recv data,ddms=%d\r\n",get_sys_ddms());
#endif

	len_recv = my_recv(g_sk_wz, buf_recv, sizeof(buf_recv), 0);
	if (len_recv > 3)
    {
		if (strncmp(buf_recv, "0000", 4) != 0) {
			memcpy(buf_err_code, buf_recv, 4);
			errcode=-53;
			goto err;
		}
	}
    else if (len_recv < 0)
	{
        DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_81:len_recv=%d,ddms=%d\r\n",len_recv,get_sys_ddms());
		errcode=-14;
		goto err;
	}


	// 5. break connection
	len_to_send = 4;
	memcpy(buf_snd, "0000", len_to_send);

#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in net_send_icbc_fsn_by_fsn 11_9:send data\r\n");
#endif
	ret = my_send(g_sk_wz, buf_snd, len_to_send, 0);
	if (ret < 0) {
		errcode=-15;
		goto err;
	}

    set_recv_dat_file_flag_by_index(dat_point,DATF_SEND_OK);
	ret=10;

    get_os_sys_date(time);
    DBG_NORMAL("net_send_ok,money_all_cnt=%d,time=%s,dds=%d\r\n",money_all_cnt,time,GET_SYS_DDS(NULL));
	errcode=ret;
err:

    g_connect_wz_start_flag=0;
    g_connect_wz_timeout_s=0;

    if( INVALID_SOCKET != g_sk_wz)
    {
#if DISPLAY_CONNECT_ENABLE
        DBG_NORMAL("normal,close socket,g_sk_wz=0x%08x\r\n",(int)g_sk_wz);
#endif
        SK_COLSE(g_sk_wz);
        g_sk_wz=INVALID_SOCKET;

        if(-5 == errcode)
        {
            //DBG_NORMAL("connect err,close skt after delay  %d s\r\n",PARA_CONNECT_ERR_DELAY_S_VAL);
            MY_DELAY_X_S(PARA_CONNECT_ERR_DELAY_S_VAL);
        }

#if 0 /*hxj amend,date 2014-10-11 15:1*/
        if(1==one_delay_flag)
        {
            one_delay_flag=0;
            DBG_NORMAL("SK_COLSE delay %ds start\r\n",2);
            MY_DELAY_X_S(2);
            DBG_NORMAL("SK_COLSE delay %ds   end\r\n",2);
        }

#endif

    }

    if((0!=errcode) && (10!=errcode))
    {
        DBG_NORMAL("errcode=%d,%c%c%c%c\r\n",errcode,buf_err_code[0],buf_err_code[1],buf_err_code[2],buf_err_code[3]);

        err_code_val=GET_4B_ASICC_16_VAL(buf_err_code);
        switch(err_code_val)
        {
            case 0x0000:
            case 0x4000:
            break;
            case 0x1000:    //服务忙
            {
                //延时参数可以配置
                MY_DELAY_X_S(60);
            }
            break;
            case 0x1001:    //（接收）数据异常,不再上传
            case 0x1004:    //文件信息长度不符合60+52*n的长度要求,busi文件长度错,不再上传
            {
                set_recv_dat_file_flag_by_index(dat_point,DATF_STOP_SEND);
            }
            break;
            case 0x1002:    //创建文件夹失败
            {
                ;
            }
            break;
            case 0x1003:    //创建文件失败
            {
                ;
            }
            break;
            case 0x1005:    //发送文件内容与长度不符
            {
                ;
            }
            break;
            default:
            break;

        }

    }

    return errcode;

}






/****************************************************************************************************
**名称:int net_send_big_img(char *buf,int len,int recv_data_flag)
**功能:网络发送大图数据
* 入口:无
* 出口:成功返回10,出错返回0或负数
**auth:hxj, date: 2015-1-13 13:35
*****************************************************************************************************/
int net_send_big_img(char *buf,int len,int recv_data_flag)
{
	int ret = 0;
	struct sockaddr_in sa_server;
	int len_recv;
	char buf_recv[1024];
    int errcode=-4;
    char time[15]={0};
    char buf_err_code[5]={'4','4','4','4',0};
    uint16 port;
    char ip[30]={0};
    int timeout_ms=0;


    if(0==g_send_big_img_flag)
    {
        DBG_NORMAL("send big img beyond time,retur 0\r\n");
        return 0;
    }

    if( !is_may_net_send_big() )
    {
        DBG_NORMAL("big img may not send,retur 0\r\n");
        return 0;
    }

    if( INVALID_SOCKET != g_sk_big_img)
    {
        DBG_NORMAL("big img socket ok,g_sk_big_img=0x%08x\r\n",(int)g_sk_big_img);
    }
    else
    {
        port=get_server_bit_img_port();
        memcpy(ip,g_server_ip,sizeof(g_server_ip));
        if(!is_ok_ip(ip))
        {
            DBG_DIS("server ip err [%s],retur 0\r\n",ip);
            return 0;
        }

    	g_sk_big_img = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    	if (g_sk_big_img == INVALID_SOCKET)
        {
            g_sk_big_img =INVALID_SOCKET;
    		errcode=-301;
    		goto err;
    	}

#if DISPLAY_CONNECT_ENABLE
        DBG_NORMAL("creat,socket,g_sk_big_img=0x%08x\r\n",(int)g_sk_big_img);
#endif

#if 1 /*hxj amend,date 2014-12-17 15:44*/

#if 0 /*hxj amend,date 2014-12-23 16:31*/
    	if(-1 == setsockopt(g_sk_big_img, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) )
    	{

            DBG_NORMAL("errno=%d\r\n",errno);
    		errcode=-302;
            goto err;
        }
#endif

        timeout_ms=get_gbl_net_timeout()*1000;
        if( -1 == setsockopt(g_sk_big_img, SOL_SOCKET, SO_RCVTIMEO, &timeout_ms, sizeof(timeout_ms)) )
        {
    		errcode=-303;
            goto err;
        }
#endif

        #if DISPLAY_CONNECT_ENABLE
        DBG_NORMAL("get net time,connect to..., server ip=%s,port=%d\r\n",ip,port);
        #endif

        memset(&sa_server,0,sizeof(struct sockaddr_in));
    	sa_server.sin_family = AF_INET;
    	sa_server.sin_len = sizeof(sa_server);
        sa_server.sin_port = htons ( port );
        sa_server.sin_addr.s_addr = inet_addr ( ip );

        if(0==g_send_big_img_flag)
        {
            errcode=-50;
            goto err;
        }

    	ret = connect(g_sk_big_img, (struct sockaddr*)&sa_server, sizeof(sa_server));
    	if(ret < 0)
        {
            get_os_sys_date(time);
            DBG_NORMAL("big img connect err, server ip=%s,port=%d,time=%s,dds=%d\r\n",ip,port,time,GET_SYS_DDS(NULL));
            //DBG_NORMAL("connect err, delay  %d s \r\n",PARA_CONNECT_ERR_DELAY_S_VAL);
            MY_DELAY_X_S(PARA_CONNECT_ERR_DELAY_S_VAL);

    		errcode=-5;
            goto err;
    	}

#if DISPLAY_CONNECT_ENABLE
        DBG_NORMAL("connect ok \r\n",ip,port);
#endif

    }


    if(0==g_send_big_img_flag)
    {
        errcode=-52;
        goto err;
    }

    //=====================================================
	ret = my_send_big_img(g_sk_big_img, buf, len, 0);
	if (ret < 0)
    {
        DBG_DIS("in net_send_big_img 11_1:ret=%d\r\n",ret);
		errcode=-6;
        goto err;
	}


#if DISPLAY_CONNECT_ENABLE
    DBG_NORMAL("in net_send_big_img 11_2:recv data\r\n");
#endif

    if((1==recv_data_flag) || (2==recv_data_flag))
    {
        if(0==g_send_big_img_flag)
        {
            errcode=-71;
            goto err;
        }

    	// recv
    	len_recv = my_recv_big_img(g_sk_big_img, buf_recv, sizeof(buf_recv), 0);
    	if (len_recv > 0)
        {
            ;
    	}
        else if (len_recv < 0)
    	{
    		errcode=-7;
            goto err;
    	}
    }

	ret=10;
    get_os_sys_date(time);
    DBG_NORMAL("big img net_send_ok,time=%s,dds=%d\r\n",time,GET_SYS_DDS(NULL));
	errcode=ret;


    if(2==recv_data_flag)
    {
        if( INVALID_SOCKET != g_sk_big_img)
        {
#if DISPLAY_CONNECT_ENABLE
            DBG_NORMAL("normal,close socket,g_sk_big_img=0x%08x\r\n",(int)g_sk_big_img);
#endif
            SK_COLSE(g_sk_big_img);
            g_sk_big_img=INVALID_SOCKET;

        }
    }

    return errcode;

err:

    if( INVALID_SOCKET != g_sk_big_img)
    {
#if DISPLAY_CONNECT_ENABLE
        DBG_NORMAL("normal,close socket,g_sk_big_img=0x%08x\r\n",(int)g_sk_big_img);
#endif
        SK_COLSE(g_sk_big_img);
        g_sk_big_img=INVALID_SOCKET;

        if(-5 == errcode)
        {
            //DBG_NORMAL("connect err,close skt after delay  %d s\r\n",PARA_CONNECT_ERR_DELAY_S_VAL);
            MY_DELAY_X_S(PARA_CONNECT_ERR_DELAY_S_VAL);
        }
    }

    if((0!=errcode) && (10!=errcode))
    {
        DBG_NORMAL("big img errcode=%d,%c%c%c%c\r\n",errcode,buf_err_code[0],buf_err_code[1],buf_err_code[2],buf_err_code[3]);
    }

    return errcode;

}




void send_dat_buf(void)
{
    int ret_val=0;

    for(;;)
    {

        if(1==g_send_big_img_flag)
        {
            DELAY_X_S(2);    //单位为 2s
            continue;
        }

        ret_val=net_send_icbc_fsn_by_fsn();
        if(10==ret_val)
        {
            DELAY_X_MS(100);  //单位为 ms
            continue;
        }
        else
        {
            DELAY_X_S(2);    //单位为 2s
        }
    }

}



int set_socket_sendrecv_timeout ( SK_TYPE sockfd, int timeout_seconds_recv,int timeout_seconds_send )
{

	struct timeval timeo;
	int len = sizeof ( timeo );

	timeo.tv_sec = timeout_seconds_recv;
	timeo.tv_usec = 0;

	if ( -1==setsockopt ( sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeo, len ))
	{

        #if DISPLAY_CONNECT_ENABLE
		    DBG_ERROR_MY( "setsockopt recv time out error\n" );
        #endif
	}

	timeo.tv_sec = timeout_seconds_send;
	timeo.tv_usec = 0;

	if ( -1==setsockopt ( sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len ))
	{

        #if DISPLAY_CONNECT_ENABLE
		    DBG_ERROR_MY ( "setsockopt send time out error\n" );
        #endif
		return 0;
	}
	return 1;
}





/****************************************************************************************************
**名称:int select_recv_tcp(int sock, char *buf, int len, int sec, int usec)
**功能:读取tcp数据
* 入口:无
* 出口:小于0出错,
**auth:hxj, date: 2014-5-12 13:9
*****************************************************************************************************/
int select_recv_tcp(SK_TYPE sock, char *buf, int len, int sec, int usec)
{
	int num_recv;
	struct timeval waitTime;
	fd_set fdSocket;
	int nRet;

	waitTime.tv_sec = sec;
	waitTime.tv_usec = usec;

	FD_ZERO(&fdSocket);
	FD_SET(sock, &fdSocket);

	nRet = select (sock + 1, &fdSocket, NULL, NULL, &waitTime );   // 可读
	if (nRet < 0)
    {
		num_recv = -1;
	}
    else if (nRet == 0)
	{
        //timeout
		num_recv = -2;
	}
    else
	{
		if(FD_ISSET(sock, &fdSocket))
		{
			num_recv = recv(sock, buf, len, 0);
		}
	}

	return num_recv;
}



/****************************************************************************************************
**名称:int select_send_tcp ( SK_TYPE fd, char *buf, int len ,int flag, int sec, int usec)
**功能:对标准的send()函数进行容错处理
* 入口:fd-文件句柄,buf-数据首指针,len-长度
* 出口:返回写入的字节数,小于0出错
**auth:hxj, date: 2014-5-22 8:20
*****************************************************************************************************/
int select_send_tcp ( SK_TYPE fd, char *buf, int len ,int flag, int sec, int usec)
{
	int wlen = 0;
	int one_ok_wlen=0;
	struct timeval waitTime;
	fd_set fdSocket;
	int nRet;
    int one_send_cnt=0;
    int i;
    int one_wlen;

	if ( NULL == buf )  return 0;
	if (len <= 0 )     	return 0;


	waitTime.tv_sec = sec;
	waitTime.tv_usec = usec;

	FD_ZERO(&fdSocket);
	FD_SET(fd, &fdSocket);


	nRet = select (fd + 1, NULL,&fdSocket,  NULL, &waitTime );          // 可写
	if (nRet < 0)
    {
		return -1;
	}
    else if (nRet == 0)
	{
        //timeout
		return 0;
	}
    else
	{
#if 0 /*hxj amend,date 2014-5-27 11:3*/
        //可写
        one_ok_wlen = send (fd,buf,len,flag);
        if(one_ok_wlen>0)
        {
            wlen=one_ok_wlen;
        }
        else
        {
            return -2;
        }
#else
        one_send_cnt=ONE_SEND_LEN;
        for (i = 0; i < len;)
        {
            if (len - i < one_send_cnt)
            {
                one_wlen = len - i;
            }
            else
            {
                one_wlen = one_send_cnt;
            }

            if (0 == get_flag_icbc_fsn_net())
            {
                //return -2;
            }

            one_ok_wlen = send(fd, buf + i, one_wlen, flag);
            if (one_ok_wlen < 0)
            {
                return -1;
            }
            i += one_wlen;
        }

#endif

	}

    return wlen;

}






/****************************************************************************************************
**名称:int my_send_1( SK_TYPE fd, char *buf, int len ,int flag)
**功能:对标准的send()函数进行容错处理
* 入口:fd-文件句柄,buf-数据首指针,len-长度
* 出口:返回写入的字节数
**auth:hxj, date: 2014-5-22 8:7
*****************************************************************************************************/
int my_send_1( SK_TYPE fd, char *buf, int len ,int flag)
{
    int wlen = 0;
    int error_count = 0;
    int one_wlen=0;
    int one_ok_wlen=0;
    int temp_send_timeout=get_gbl_net_timeout();
    struct timeval waitTime;
    fd_set fdSocket;
    int nRet;


    if ( NULL == buf )  return 0;
    if (len <= 0 )      return 0;


    waitTime.tv_sec = temp_send_timeout;
    waitTime.tv_usec = 0;

    FD_ZERO(&fdSocket);
    FD_SET(fd, &fdSocket);

    nRet = select (fd + 1, NULL,&fdSocket,  NULL, &waitTime );   // 可读
    if (nRet < 0)
    {
        return wlen;
    }
    else if (nRet == 0)
    {
        //timeout
        return wlen;
    }
    else
    {
        //可写
    }


    for ( ;; )
    {
        one_wlen=len - wlen;
        one_ok_wlen = send ( fd, &buf[wlen],one_wlen ,flag);
        if ( one_ok_wlen >= one_wlen )
        {
            return len;
        }
        else if( one_ok_wlen <= 0 )
        {
            error_count++;
            if ( error_count > 10 ) return wlen;

            FD_ZERO(&fdSocket);
            FD_SET(fd, &fdSocket);

            nRet = select (fd + 1, NULL,&fdSocket,  NULL, &waitTime );   // 可读
            if (nRet < 0)
            {
                return wlen;
            }
            else if (nRet == 0)
            {
                //timeout
                return wlen;
            }
            else
            {
                //可写
            }

        }
        else
        {
            wlen+=one_ok_wlen;
        }
    }
#if ARM_USE

#else
    return wlen;
#endif
}




/****************************************************************************************************
**名称:int my_write ( int fd, char *buf, int len )
**功能:对标准的write()函数进行容错处理
* 入口:fd-文件句柄,buf-数据首指针,len-长度
* 出?返回写入的字节数
**auth:hxj, date: 2012-3-31 9:12
*****************************************************************************************************/
int my_write ( int fd, char *buf, int len )
{
	if ( fd < 0 ) 	    return 0;
	if ( NULL == buf )  return 0;
	if (len <= 0 )     	return 0;

	int wlen = 0;
	int error_count = 0;
	int one_wlen=0;
	int one_ok_wlen=0;

	for ( ;; )
	{
		one_wlen=len - wlen;
		one_ok_wlen = write ( fd, &buf[wlen],one_wlen );
		if ( one_ok_wlen >= one_wlen )
		{
			return len;
		}
		else if( one_ok_wlen <= 0 )
		{
			error_count++;
			if ( error_count > 3 )	return wlen;
		}
		else
		{
			wlen+=one_ok_wlen;
		}
	}

	//return wlen;

}



/****************************************************************************************************
**名称:int tcp_send_data(SK_TYPE fd,char *buf,int len)
**功能:tcp发送数据
* 入口:无
* 出口:成功返回0,出错返回负数
**auth:hxj, date: 2014-5-14 11:23
*****************************************************************************************************/
int tcp_send_data(SK_TYPE fd,char *buf,int len)
{

#if 0 /*hxj amend,date 2014-5-22 8:6*/

   // if(SK_INVALID_VAL == fd) return -1;
    //#if ARM_USE
    #if 1
    	if(len != my_send_1(fd, buf, len, 0))
        {
    		return -1;
    	}
        return 0;
    #else
        if(len != my_write ( fd, buf, len ))
        {
            return -1;
        }
        return 0;
    #endif
#else

    int i=0;
    int all_timeout_s=3*60;
    int one_timeout_ms=ONE_TIMEOUT_END_VAL;
    int cnt=0;
    int wlen = 0;
    int one_wlen=0;
    int one_ok_wlen=0;
    int one_send_cnt=0;

    if (0 == get_flag_icbc_fsn_net())
    {
        //return -2;
    }

    if( 0 != one_timeout_ms )
    {


        all_timeout_s=get_gbl_net_timeout();
        cnt=all_timeout_s/one_timeout_ms;
        if(all_timeout_s%one_timeout_ms)
        {
            cnt++;
        }

        if(cnt<=0)
        {
            cnt=1;
        }


        for(i=0;i<cnt;++i)
        {
            if (0 == get_flag_icbc_fsn_net())
            {
                //return -2;
            }

            one_wlen=len - wlen;
            one_ok_wlen = select_send_tcp ( fd, &buf[wlen],one_wlen ,0,one_timeout_ms,0);
            if(0==one_ok_wlen)
            {
                continue;
            }
            else if(one_ok_wlen<0)
            {
                return -1;
            }
            else if ( one_ok_wlen >= one_wlen )
            {
                return 0;
            }
            else
            {
                wlen+=one_ok_wlen;
            }

            if(wlen==len) return 0;

        }

    }
    else
    {

#if 0 /*hxj amend,date 2014-5-27 10:44*/
        one_wlen=len - wlen;
        one_ok_wlen = send ( fd, &buf[wlen],one_wlen,0);
        if(one_ok_wlen<0)
        {
            return -1;
        }

#else

        //one_send_cnt=800000;
        one_send_cnt=ONE_SEND_LEN;

        for (i = 0; i < len;)
        {
            if (len - i < one_send_cnt)
            {
                one_wlen = len - i;
            }
            else
            {
                one_wlen = one_send_cnt;
            }

            if (0 == get_flag_icbc_fsn_net())
            {
                //return -2;
            }

            one_ok_wlen = send(fd, buf + i, one_wlen, 0);
            if (one_ok_wlen < 0)
            {
                return -1;
            }
            i += one_wlen;
        }

#endif

        return 0;
    }

    if(wlen==len) return 0;
    return -1;

#endif

}




int fsn_init_head(STR_FSN_HEAD *p_file_head)
{
    if(NULL == p_file_head) return -1;

    p_file_head->HeadStart[0]=20;
    p_file_head->HeadStart[1]=10;
    p_file_head->HeadStart[2]=7;
    p_file_head->HeadStart[3]=26;


    p_file_head->HeadString[0]=0;
    p_file_head->HeadString[1]=1;

#if 1 /*hxj amend,date 2014-8-26 9:41*/
    //0x2E 表示 数据记录包含图像序列号，
    p_file_head->HeadString[2]=0x2E;
#else
    //0x2D表示 数据不包含图像序列号
    p_file_head->HeadString[2]=0x2D;
#endif

    p_file_head->HeadString[3]='S';
    p_file_head->HeadString[4]='N';
    p_file_head->HeadString[5]='o';

    p_file_head->Counter=0;

    p_file_head->HeadEnd[0]=0;
    p_file_head->HeadEnd[1]=1;
    p_file_head->HeadEnd[2]=2;
    p_file_head->HeadEnd[3]=3;

    return 0;
}


int time_change(char *time,uint16 *p_date,uint16 *p_time)
{
    int year_i;
	int month_i;
	int day_i;
	int hour_i;
	int minute_i;
	int second_i;

	year_i = 1000*(time[0] - '0') + 100*(time[1] - '0')+ 10*(time[2] - '0') + (time[3] - '0');
	month_i = 10*(time[4] - '0') + (time[5] - '0');
	day_i = 10*(time[6] - '0') + (time[7] - '0');
	*p_date = ((year_i - 1980) << 9) + (month_i<<5) + day_i;
	hour_i = 10*(time[8] - '0') + (time[9] - '0');
	minute_i = 10*(time[10] - '0') + (time[11] - '0');
	second_i = 10*(time[12] - '0') + (time[13] - '0');
	*p_time = (hour_i<<11) + (minute_i<<5) + (second_i>>1);

    return 0;
}



int change_pic_data(STR_ONE_SN_IMAGE_DATA *SNo,uint8 *raw_pic)
{
    int i;
    int j;
    int k;
    char *dg_p_c;
    unsigned short u_s_0=0;
    Uint32 u_i_0=0;

    if(NULL== SNo)      return -1;
    if(NULL== raw_pic)  return -2;

	dg_p_c =(char *) raw_pic;
    for (i = 0; i < 8; i++)
    {
		for (j = 0; j < 16; j++)
        {
			if((i==7) && (j==4))
			{
				break;
			}
			else
			{
				memcpy((char *)&u_s_0, (dg_p_c + i*16*2 + j*2), 2);
				//u_s_0 = (Uint16)*(p_pic_bit + i*16 + j);
				u_i_0 = 0;
				for (k = 0; k < 16; k++)
                {
					if ((u_s_0 >> k) & 0x01 == 1)
                    {
						u_i_0 = u_i_0 | (0x01 << (2*k));
						u_i_0 = u_i_0 | (0x01 << (2*k + 1));
					}
				}
				SNo[i].Data[j*2] = u_i_0;
				SNo[i].Data[j*2 + 1] = u_i_0;
			}
		}
	}

    return 0;
}







/****************************************************************************************************
**名称:int change_pic_data_two(STR_ONE_SN_IMAGE_DATA *SNo,uint8 *raw_pic)
**功能:转换压缩的图像,压缩后为180有效字节
* 入口:无
* 出口:无
**auth:hxj, date: 2014-12-30 12:53
*****************************************************************************************************/
int change_pic_data_two(STR_ONE_SN_IMAGE_DATA *SNo,uint8 *raw_pic)
{
    int i;
    int j;
    int k;
    uint16 *dg_p_c;
    uint16 u_s_0=0;
    uint32 u_i_0=0;
	int snx = 9;

    if(NULL== SNo)      return -1;
    if(NULL== raw_pic)  return -2;

	dg_p_c =(uint16 *) raw_pic;
    for (i = 0; i < 10; i++)
    {
        for (j = 0; j < snx; j++)
        {
			u_s_0 = (Uint16)*(dg_p_c + i*snx + j);
			u_i_0 = 0;
			for (k = 0; k < 16; k++)
			{
			    if ((u_s_0 >> k) & 0x01 == 1)
				{
					u_i_0 = u_i_0 | (0x01 << (k+8));
				}
		    }
	        SNo[i].Data[j+12] = u_i_0;
		}
	}

    return 0;


}




/****************************************************************************************************
**名称:int creat_fsn_by_dat(STR_FSN_FILE *p_fsn_file,int fsn_buf_len,int *p_ret_fsn_len,DAT_FILE_FORMAT_C *p_dat_file,int dat_file_len)
**功能:转换dat格式到fsn格式
* 入口:无
* 出口:无
**auth:hxj, date: 2014-8-26 14:23
*****************************************************************************************************/
int creat_fsn_by_dat(STR_FSN_FILE *p_fsn_file,int fsn_buf_len,int *p_ret_fsn_len,DAT_FILE_FORMAT_C *p_dat_file,int dat_file_len)
{
    uint32 money_all_cnt=0;
    int i_money;
    int i;
    STR_FSN_BODY *p_one_fsn_body=NULL;
    DAT_HEAD_OLD_C *p_dat_head_old=NULL;
    DAT_DATA_INFO_C *p_one_dat_info=NULL;
    STR_FSN_BODY_HEAD tmp_fsn_body_head;
    int year,year_ys;

    if(NULL== p_fsn_file)       return -1;
    if(NULL== p_ret_fsn_len)    return -2;
    if(NULL== p_dat_file)       return -3;

    //长度检测
    money_all_cnt=GET_2B_SMALLSN_VAL(p_dat_file->head_old.all_num);

    if(0==money_all_cnt)  return -4;
    if(money_all_cnt>PARA_ONE_TIME_MONEY_MAX_NUM) return -5;

    if( dat_file_len!=(sizeof(DAT_HEAD_OLD_C)+sizeof(DAT_HEAD_ADD_C)+money_all_cnt*sizeof(DAT_DATA_INFO_C)) )
    {
        return -6;
    }

    *p_ret_fsn_len=sizeof(STR_FSN_HEAD)+money_all_cnt*sizeof(STR_FSN_BODY);
    if(*p_ret_fsn_len>fsn_buf_len) return -7;


    p_dat_head_old=&p_dat_file->head_old;


    //头初始化
    fsn_init_head(&p_fsn_file->fsn_head);

    //设置时间
    uint16 temp_Date;            //验钞启动日期
    uint16 temp_Time;            //验钞启动时间
    time_change((char *)p_dat_head_old->time_setup,&temp_Date,&temp_Time);
    tmp_fsn_body_head.Date=temp_Date;
    tmp_fsn_body_head.Time=temp_Time;

    //机具编号
    for(i=0;i<24;++i)
    {
        tmp_fsn_body_head.MachineSNo[i]=cur_arm_mac_info.mach_code[i];
    }


    //重置年份
    #if CUR_PROTOCOL!=PROTOCOL_ZX
        tmp_fsn_body_head.MachineSNo[3]=p_dat_head_old->time_setup[2];
        tmp_fsn_body_head.MachineSNo[4]=p_dat_head_old->time_setup[3];
    #endif


    for(i_money=0;i_money<money_all_cnt;i_money++)
    {
        p_one_fsn_body=&p_fsn_file->fsn_body[i_money];
        p_one_dat_info=&p_dat_file->data_info[i_money];

        #if DISPLAY_DAT_FILE_INFO_ENABLE
            MY_LOGBUF(p_one_dat_info->res_1,sizeof(DAT_DATA_INFO_C),"dis DAT info:");
        #endif

        //--------------------------------------------------------------
        //验钞启动日期
        p_one_fsn_body->data_head.Date=tmp_fsn_body_head.Date;
        //验钞启动时间
        p_one_fsn_body->data_head.Time=tmp_fsn_body_head.Time;

        //真、假、残和旧币标志
        p_one_fsn_body->data_head.tfFlag=p_one_dat_info->money_flag;
        //错误码(3个)
        if(1==p_one_fsn_body->data_head.tfFlag)
        {
            p_one_fsn_body->data_head.ErrorCode[0]=0;
            p_one_fsn_body->data_head.ErrorCode[1]=0;
            p_one_fsn_body->data_head.ErrorCode[2]=0;
        }
        else
        {
            p_one_fsn_body->data_head.ErrorCode[0]=5;
            p_one_fsn_body->data_head.ErrorCode[1]=0;
            p_one_fsn_body->data_head.ErrorCode[2]=0;
        }


        //货币标志
        p_one_fsn_body->data_head.MoneyFlag[0] = 'C';
        p_one_fsn_body->data_head.MoneyFlag[1] = 'N';
        p_one_fsn_body->data_head.MoneyFlag[2] = 'Y';
        p_one_fsn_body->data_head.MoneyFlag[3] = 0;

        //版本号
        switch(p_one_dat_info->money_ver)
        {
            case 0:
            case 1:
            case 2:
                p_one_fsn_body->data_head.Ver=p_one_dat_info->money_ver;
            break;
            default:
                p_one_fsn_body->data_head.Ver=9999;
            break;
        }
        //币值
        p_one_fsn_body->data_head.Valuta=p_one_dat_info->money_val;
        //冠字号码字符数
        p_one_fsn_body->data_head.CharNUM=10;

        //冠字号码字符数
    	for (i=0; i<10; i++)
        {
    		p_one_fsn_body->data_head.SNo[i] = p_one_dat_info->money_sn[i];
    	}
    	p_one_fsn_body->data_head.SNo[10] = 0;
    	p_one_fsn_body->data_head.SNo[11] = 0;

        //保留填 0
        p_one_fsn_body->data_head.Reserve1=0;

        //机具编号
#if 0 /*hxj amend,date 2014-12-17 10:57*/
        memcpy(p_one_fsn_body->data_head.MachineSNo,tmp_fsn_body_head.MachineSNo,sizeof(tmp_fsn_body_head.MachineSNo));
#else
        for(i=0;i<24;++i)
        {
            p_one_fsn_body->data_head.MachineSNo[i]=tmp_fsn_body_head.MachineSNo[i];
        }

#endif




        p_one_fsn_body->data_head.MachineSNo[0]=BOC_STR_VAL_1;
        p_one_fsn_body->data_head.MachineSNo[1]=BOC_STR_VAL_2;
        p_one_fsn_body->data_head.MachineSNo[2]=BOC_STR_VAL_3;


        year=(tmp_fsn_body_head.Date>>9) + 1980;
        year_ys=year%100;

        p_one_fsn_body->data_head.MachineSNo[3]=year_ys/10+0x30;
        p_one_fsn_body->data_head.MachineSNo[4]=year_ys%10+0x30;

        //--------------------------------------------------------------
        //字符数
        p_one_fsn_body->data_body.Num=10;

        //每个图像字符高度
        p_one_fsn_body->data_body.height=32;

        //每个图像字符宽度
        p_one_fsn_body->data_body.width=32;

        //保留填 0
        p_one_fsn_body->data_body.Reserve2=0;

        //设置图像
        memset(p_one_fsn_body->data_body.SNo,0,sizeof(p_one_fsn_body->data_body.SNo));
#if 0 /*hxj amend,date 2014-12-30 13:8*/
        change_pic_data(p_one_fsn_body->data_body.SNo,p_one_dat_info->money_sn_pic);
#else
        change_pic_data_two(p_one_fsn_body->data_body.SNo,p_one_dat_info->money_sn_pic);
#endif

    }

    p_fsn_file->fsn_head.Counter=money_all_cnt;


    return 0;

}


int is_need_change_not_normal_char_and_space(char temp_c)
{

    if(((temp_c)>='0')&&((temp_c)<='9'))
    {
         return 0;
    }
    else if(((temp_c)>='a')&&((temp_c)<='z'))
    {
        return 0;
    }
    else if(((temp_c)>='A')&&((temp_c)<='Z'))
    {
        return 0;
    }
    else if((temp_c)=='_' )
    {
        return 0;
    }
    else if((temp_c)=='+' )
    {
        return 0;
    }
    else if((temp_c)=='-' )
    {
        return 0;
    }
    else if((temp_c)=='~' )
    {
        return 0;
    }
    else if((temp_c)=='(' )
    {
        return 0;
    }
    else if((temp_c)==')' )
    {
        return 0;
    }
    else if((temp_c)=='.' )
    {
        return 0;
    }
    else if((temp_c)=='&' )
    {
        return 0;
    }
    else if((temp_c)=='%' )
    {
        return 0;
    }
    else if((temp_c)=='$' )
    {
        return 0;
    }
    else if((temp_c)=='@' )
    {
        return 0;
    }
    else if((temp_c)==';' )
    {
        return 0;
    }
    else if((temp_c)==0x27 )  //'
    {
        return 0;
    }
    else if((temp_c)=='^' )
    {
        return 0;
    }
    else if((temp_c)=='`' )
    {
        return 0;
    }
    else if((temp_c)=='{' )
    {
        return 0;
    }
    else if((temp_c)=='}' )
    {
        return 0;
    }
    else if((temp_c)=='[' )
    {
        return 0;
    }
    else if((temp_c)==']' )
    {
        return 0;
    }
    else if((temp_c)=='=' )
    {
        return 0;
    }

    return 1;

}






/****************************************************************************************************
**名称:int get_upload_head_info_by_dat_file(STR_FSN_FILE *p_fsn_file,DAT_FILE_FORMAT_C *p_dat_file,int dat_file_len)
**功能:从dat文件中获取上传头信息
* 入口:无
* 出口:无
**auth:hxj, date: 2014-8-26 14:59
*****************************************************************************************************/
int get_upload_head_info_by_dat_file(DSP_upload_head *p_ret_upload_head,DAT_FILE_FORMAT_C *p_dat_file,int dat_file_len)
{
    uint32 money_all_cnt=0;
    DAT_HEAD_OLD_C *p_dat_head_old=NULL;
    DAT_HEAD_ADD_C *p_dat_head_add=NULL;
    int i;
    char temp_c;


    if(NULL== p_ret_upload_head)    return -1;
    if(NULL== p_dat_file)           return -2;

    //长度检测
    money_all_cnt=GET_2B_SMALLSN_VAL(p_dat_file->head_old.all_num);

    if(0==money_all_cnt)  return -3;
    if(money_all_cnt>PARA_ONE_TIME_MONEY_MAX_NUM) return -4;

    if( dat_file_len!=(sizeof(DAT_HEAD_OLD_C)+sizeof(DAT_HEAD_ADD_C)+money_all_cnt*sizeof(DAT_DATA_INFO_C)) )
    {
        return -5;
    }

    p_dat_head_old=&p_dat_file->head_old;
    p_dat_head_add=&p_dat_file->head_add;

    memset(p_ret_upload_head->dat_file_time_str,0,sizeof(p_ret_upload_head->dat_file_time_str));
    memcpy(p_ret_upload_head->dat_file_time_str,p_dat_head_old->time_setup,sizeof(p_dat_head_old->time_setup));

    memset(p_ret_upload_head->mach_code_str,0,sizeof(p_ret_upload_head->mach_code_str));
    //memcpy(p_ret_upload_head->mach_code_str,p_dat_head_old->mach_code,sizeof(p_dat_head_old->mach_code));
    memcpy(p_ret_upload_head->mach_code_str,cur_arm_mac_info.mach_code,sizeof(p_dat_head_old->mach_code));

    //add
    memset(p_ret_upload_head->opt_sn_str,0,sizeof(p_ret_upload_head->opt_sn_str));
    memcpy(p_ret_upload_head->opt_sn_str,p_dat_head_add->opt_sn,sizeof(p_dat_head_add->opt_sn));

    //12345678
    for(i=0;i<8;++i)
    {
        temp_c=p_ret_upload_head->opt_sn_str[8-i-1];
        if(is_need_change_not_normal_char_and_space(temp_c))
        {
            p_ret_upload_head->opt_sn_str[8-i-1]=0;
        }
    }

    memset(p_ret_upload_head->atm_transation_str,0,sizeof(p_ret_upload_head->atm_transation_str));
    memcpy(p_ret_upload_head->atm_transation_str,p_dat_head_add->atm_transation,sizeof(p_dat_head_add->atm_transation));

    //ATM_1234567
    for(i=0;i<20;++i)
    {
        temp_c=p_ret_upload_head->atm_transation_str[20-i-1];
        if(is_need_change_not_normal_char_and_space(temp_c))
        {
            p_ret_upload_head->atm_transation_str[20-i-1]=0;
        }

    }

    //add
    memcpy(p_ret_upload_head->local_sn,p_dat_head_add->local_sn,4);
    memcpy(p_ret_upload_head->sub_dank_sn,p_dat_head_add->sub_dank_sn,4);
    memcpy(p_ret_upload_head->net_sn,p_dat_head_add->net_sn,4);

    return 0;

}





int net_interface_config(void)
{
    char local_ip[20]={0};
    char local_mk[20]={0};
    char local_gw[20]={0};

    DBG_NORMAL("in net_interface_config 0:\r\n");
    get_local_ip_info(local_ip,local_mk,local_gw,NULL,NULL,NULL);
    if(!is_ok_ip(local_ip))
    {
        DBG_DIS("local_ip err [%s],use default ip\r\n",local_ip);
        sprintf(local_ip,"192.168.11.150");
        sprintf(local_mk,"255.255.255.0");
        sprintf(local_gw,"192.168.11.1");
    }

    DBG_DIS("local_ip:%s\r\n",local_ip);
    DBG_DIS("local_mk:%s\r\n",local_mk);
    DBG_DIS("local_gw:%s\r\n",local_gw);


    LWIP_IF lwipIfPort1;

    /* Get the MAC address */
    EVMMACAddrGet(0, lwipIfPort1.macArray);
    

    //B8 D8 12 E0 xx xx
    DBG_DIS("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n"
    ,lwipIfPort1.macArray[5]
    ,lwipIfPort1.macArray[4]
    ,lwipIfPort1.macArray[3]
    ,lwipIfPort1.macArray[2]
    ,lwipIfPort1.macArray[1]
    ,lwipIfPort1.macArray[0]);

    int i;
    for(i=0;i<6;++i)
    {
        g_mac[i]=lwipIfPort1.macArray[5-i];
    }


    lwipIfPort1.slvPortNum = 1;
    lwipIfPort1.instNum = 0;
#if 0 /*hxj amend,date 2014-12-15 16:37*/
    lwipIfPort1.ipAddr =  0xC0A80B96;   //192.168.11.150
    lwipIfPort1.netMask = 0xFFFFFF00;
    lwipIfPort1.gwAddr =  0xC0A80B01;
#else
    lwipIfPort1.ipAddr =  htonl(inet_addr(local_ip));
    lwipIfPort1.netMask = htonl(inet_addr(local_mk));
    lwipIfPort1.gwAddr =  htonl(inet_addr(local_gw));
#endif

    lwipIfPort1.ipMode = IPADDR_USE_STATIC; //使用静态分配IP
    int ret_code=0;

    my_lwIPInit(&lwipIfPort1,&ret_code);

    if(3==ret_code)
    {
        g_auto_negotiation_oked_flag=1;
    }

    DBG_NORMAL("in net_interface_config 100:\r\n");
    g_net_first_init_over_flag=1;

    return 0;

}


/****************************************************************************************************
**名称:int net_interface_reconfig(void)
**功能:
* 入口:无
* 出口:成功返回1
**auth:hxj, date: 2014-12-23 12:54
*****************************************************************************************************/
int net_interface_reconfig(void)
{
    int ret=0;
    DBG_NORMAL("in net_interface_reconfig 0:\r\n");

    ret=lwIPInit_re();
    DBG_NORMAL("in net_interface_reconfig 100:\r\n");
    return ret;

}



/****************************************************************************************************
**名称:int save_dat_file_to_send_buf(char *sub_data_buf,uint32 sub_data_len)
**功能:
* 入口:无
* 出口:成功返回0
**auth:hxj, date: 2014-12-18 16:35
*****************************************************************************************************/
int save_dat_file_to_send_buf(char *sub_data_buf,uint32 sub_data_len)
{
    uint32 off_val;
    DAT_FILE_FORMAT_C * p_dat_file=NULL;
    int change_time;
    char time[15];
    int dat_buf_point=0;

    if(NULL == sub_data_buf) return -1;
    if(NULL == p_dat_file_buf_data) return -11;

    if(sub_data_len <(sizeof(DAT_HEAD_OLD_C)+sizeof(DAT_HEAD_ADD_C)+sizeof(DAT_DATA_INFO_C)) )
    {
        DBG_ALARM("alarm :dat len err1\r\n");
        return -2;
    }

    off_val=sub_data_len-sizeof(DAT_HEAD_OLD_C)-sizeof(DAT_HEAD_ADD_C);
    if( (0==off_val)
        || ( 0!= (off_val % sizeof(DAT_DATA_INFO_C)))
        || (( off_val / sizeof(DAT_DATA_INFO_C) ) > PARA_ONE_TIME_MONEY_MAX_NUM)
        )
    {

        DBG_ALARM("alarm :dat len err2\r\n");
        return -3;
    }

    //设置值
    p_dat_file=(DAT_FILE_FORMAT_C *)sub_data_buf;
    if(0!=change_date_to_val_by_2014_year((char *)p_dat_file->head_old.time_setup,&change_time))
    {
        char err_time[15]={0};
        memcpy(err_time,(char *)p_dat_file->head_old.time_setup,14);
        DBG_ALARM("alarm :change_date_to_val_by_2014_year err\r\n",err_time);
        return -4;
    }

    if(DISPLAY_RECV_DAT_INFO_EN)
    {
        DBG_NORMAL("save dat to send buf start\r\n");
    }

    memset(time,0,sizeof(time));
    memcpy(time,(char *)p_dat_file->head_old.time_setup,14);

    if(check_save_same_dat_file_by_time(time))
    {
        // file saved
        DBG_ALARM("alarm :save dat same time=[%s]\r\n",time);
        return -5;
    }

    //放入到dat缓冲中
    dat_buf_point=get_empty_dat_buf_point();
    if(dat_buf_point>=RECV_DAT_FILE_BUF_MAX)
    {
        DBG_ALARM("alarm :no empty buf,time=[%s]\r\n",time);
        return -6;
    }

    if(0!=set_dat_data_to_dat_buf(dat_buf_point,sub_data_buf,sub_data_len,(char *)p_dat_file->head_old.time_setup,change_time))
    {
        DBG_ALARM("alarm :save date err,time=[%s]\r\n",time);
        return -7;
    }


    if(DISPLAY_RECV_DAT_INFO_EN)
    {
        DBG_NORMAL("save dat to send buf ok\r\n");
    }

    return 0;

}



void test_init_dat_buf(void)
{


#if TEST_NET_PC_SNED_DAT_EN

    int ret=0;
    if(1==g_init_tcp_ip_data_buf_over_flag)
    {
        //test use
        ret=save_dat_file_to_send_buf(dat_file_buf_1,sizeof(dat_file_buf_1));
        //DBG_NORMAL("in test_init_dat_buf 1:ret=%d\r\n",ret);

        ret=save_dat_file_to_send_buf(dat_file_buf_2,sizeof(dat_file_buf_2));
        //DBG_NORMAL("in test_init_dat_buf 2:ret=%d\r\n",ret);
    }

#endif



}





/****************************************************************************************************
**名称:void task_net_server(void *pdata)
**功能:线程任务
* 入口:无
* 出口:无
**auth:hxj, date: 2014-12-17 15:6
*****************************************************************************************************/
void task_net_server(void *pdata)
{
    DBG_DIS("in task_net_server 0:\r\n");
    MY_DELAY_X_S(1);

    init_net_protocol_para();
    net_interface_config();
    test_init_dat_buf();
    send_dat_buf();
}



/****************************************************************************************************
**名称:void task_net_server(void *pdata)
**功能:线程任务
* 入口:无
* 出口:无
**auth:hxj, date: 2014-12-17 15:6
*****************************************************************************************************/
void task_net_server_time(void *pdata)
{

    uint32 t_cnt=0;
    static char link_status_bef=1;
    static char link_status=1;
    static uint32  re_config_phy_delay=PARA_RE_CONFIG_PHY_DELAY;
    static char re_config_phy_flag=0;
    static uint32  check_net_line_delay=1;


    DBG_DIS("in task_net_server_time 0:\r\n");
    MY_DELAY_X_S(1);

    for(;;)
    {
        // 1s

        MY_DELAY_X_S(1);
        t_cnt++;
        //DBG_DIS("in task_net_server_time 1:t_cnt=%d,sysdds=%d,sys_ddms=%d\r\n",t_cnt,GET_SYS_DDS(NULL),GET_SYS_DDMS(NULL));

        if(0==(t_cnt%2))
        {
            if(1==g_init_tcp_ip_data_buf_over_flag)
            {
                test_init_dat_buf();
            }
        }

        //DBG_DIS("in task_net_server_time 1:t_cnt=%d\r\n",t_cnt);

        if( ((1==g_net_first_init_over_flag)&&(0==(t_cnt%check_net_line_delay)))
            || (t_cnt<=3) )
        {
            check_net_link();
            if(is_ok_net_link())
            {
                link_status=1;
            }
            else
            {
                link_status=0;
            }

        }

        if( (0==link_status_bef) && (1==link_status) )
        {
            DBG_NET("in task_net_server_time 2:t_cnt=%d,net is up \r\n",t_cnt);
            g_app_net_up_flag=0;
            g_app_net_up_delay=PARA_G_APP_NET_UP_DELAY;
            if(0==g_auto_negotiation_oked_flag)
            {
                re_config_phy_flag=1;
                g_app_net_up_delay=PARA_G_APP_NET_UP_DELAY+PARA_RE_CONFIG_PHY_DELAY;
            }

            check_net_line_delay=10*60;

        }

        if( (1==link_status_bef) && (0==link_status) )
        {
            DBG_NET("in task_net_server_time 4:t_cnt=%d,net is down\r\n",t_cnt);
            g_app_net_up_flag=0;
            check_net_line_delay=10;
        }


        if(1==link_status)
        {
            if(1!=g_app_net_up_flag)
            {
                if(0 != g_app_net_up_delay)
                {
                    g_app_net_up_delay--;
                }
                else
                {
                    g_app_net_up_delay=PARA_G_APP_NET_UP_DELAY;
                    g_app_net_up_flag=1;
                }
            }
        }


        if(0!=re_config_phy_flag)
        {
            if(0!=re_config_phy_delay)
            {
                re_config_phy_delay--;
                DBG_NET("in task_net_server_time 3:t_cnt=%d,re_config_phy_delay=%d\r\n",t_cnt,re_config_phy_delay);
            }
            else
            {
                re_config_phy_delay=PARA_RE_CONFIG_PHY_DELAY;
                re_config_phy_flag=0;
                if(1==net_interface_reconfig())
                {
                    g_auto_negotiation_oked_flag=1;
                }
            }
        }

        link_status_bef=link_status;


    }

}











/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

