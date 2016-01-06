/***************文件信息********************************************************************************
**文   件   名: uart_arm.c
**说        明: ARM 串口
**创   建   人: hxj
**创   建 日期: 2014-8-15 13:40
*******************************************************************************************************/
#include "def_config.h"
#include "uart_arm.h"
#include "bsp_debug.h"
#include <stdarg.h>     //包含 vsprintf(),vsnprintf()


static char sprintf_buf[5120];
static char log_buf_str[4096];


/****************************************************************************************************
**名称:int init_uart0(int rate)
**功能:初始化串口
* 入口:无
* 出口:无
**auth:hxj, date: 2014-12-16 14:47
*****************************************************************************************************/
int init_uart0(int rate)
{
	int ret_val=0;
    BSP_DebugUARTInit_two(rate);
    return ret_val;

}


void  loginfo ( const char *fmt, ... )
{
    int n =0;
    char info_ts[200]={0};
    va_list args;
    int sb_size=sizeof(sprintf_buf);

    if(NULL==fmt) return;
    sprintf_buf[0]=0;

    va_start ( args, fmt );
    n = vsnprintf ( sprintf_buf,sizeof(sprintf_buf), fmt, args );
    va_end ( args );

    if( n > (sb_size-1) )
    {
        n=sb_size;
        sprintf(info_ts,"in loginfo 1:printf str too long,need len=%d,curr len=%d\r\n",n,(sb_size-1));
        uart0_send(info_ts,strlen(info_ts));

        if(sb_size > 5)
        {
            sprintf_buf[sb_size-5]='.';
            sprintf_buf[sb_size-4]='.';
            sprintf_buf[sb_size-3]='.';
            sprintf_buf[sb_size-2]='\n';
            sprintf_buf[sb_size-1]=0;
        }

    }
    else if(n<0)
    {
        sprintf(info_ts,"in loginfo 2:call vsnprintf err\r\n");
        uart0_send(info_ts,strlen(info_ts));
        return;
    }

    uart0_send(sprintf_buf,n);

}




/****************************************************************************************************
**名称:void logbuf ( unsigned char *buf, int buflen, char *fmt2 , ... )
**功能:此函数用在DSP中有点问题,fmt2参数无效,有点奇怪
* 入口:无
* 出口:无
**auth:hxj, date: 2014-9-1 10:16
*****************************************************************************************************/
void logbuf ( unsigned char *buf, int buflen, char *fmt , ... )
{
    int i = 0;
    int s_len=0;
    int p_len=0;
    int sb_size=sizeof(log_buf_str);
    int n=0;
    va_list args;

    if(NULL!=fmt)
    {
        va_start ( args, fmt );
        n = vsnprintf ( log_buf_str,sizeof(log_buf_str), fmt, args );
        va_end ( args );

        if( n > (sb_size-1) )
        {
            loginfo("in logbuf 1:printf str too long,need len=%d,curr len=%d\r\n",n,(sb_size-1));
            if(sb_size > 5)
            {
                log_buf_str[sb_size-5]='.';
                log_buf_str[sb_size-4]='.';
                log_buf_str[sb_size-3]='.';
                log_buf_str[sb_size-2]='\n';
                log_buf_str[sb_size-1]=0;

            }
        }
        else if(n<0)
        {
            loginfo("in logbuf 2:call vsnprintf err\r\n");
            return;
        }
        loginfo("%s,",log_buf_str);
    }

    //=======================
    memset ( log_buf_str, 0, sizeof ( log_buf_str ) );
    s_len=(sizeof(log_buf_str)-2)/2;

    if(buflen>s_len)
    {
        loginfo("in logbuf 3:input buflen too loog\r\n");
        p_len=s_len;
    }
    else
    {
        p_len=buflen;
    }

    for ( i = 0; i < p_len; i++ )
    {
        sprintf ( &log_buf_str[i * 2], "%02X", ( unsigned char ) buf[i] );
    }

    loginfo("len=%d,[%s]\r\n",buflen,log_buf_str);

}




void logbuf_two ( unsigned char *buf, int buflen, char *des_buf)
{
    int i = 0;
    int s_len=0;
    int p_len=0;

    if(NULL!=des_buf)
    {
        loginfo("%s,",des_buf);
    }

    //=======================
    memset ( log_buf_str, 0, sizeof ( log_buf_str ) );
    s_len=(sizeof(log_buf_str)-2)/2;

    if(buflen>s_len)
    {
        loginfo("in logbuf 3:input buflen too loog\r\n");
        p_len=s_len;
    }
    else
    {
        p_len=buflen;
    }

    for ( i = 0; i < p_len; i++ )
    {
        sprintf ( &log_buf_str[i * 2], "%02X", ( unsigned char ) buf[i] );
    }

    loginfo("len=%d,[%s]\r\n",buflen,log_buf_str);

}



/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

