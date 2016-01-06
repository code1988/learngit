/***************文件信息********************************************************************************
**文   件   名: def_config.h
**说		明: 包含标准的头文件,全局的宏定义,结构体定义
**创   建   人: 胡小建
**创   建 日期: 2010年11月03日
----------------历史信息--------------------------------------------------------------------------------
**修   改 内容: XXXXXX
**修   改   人: XXX
**修   改 日期: XXXX年XX月XX日
********************************************************************************************************/
#ifndef  __DEF_CONFIG_H
#define  __DEF_CONFIG_H


#define ARM_USE          1
#define DBG_GBG          0
#define lin_USE          0



//---------------------------------------------------------------------------------
//                      头文件定义
//---------------------------------------------------------------------------------

#if ARM_USE
//#include "net_thrd.h"
/* runtime include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>      //包含 isspace()
#include <stdarg.h>     //包含 vsprintf(),vsnprintf()

#if 0 /*hxj amend,date 2014-12-19 16:4*/
//arm socket
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#endif




#else
//linux
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/wait.h>


#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <math.h>



#include <linux/types.h>
#include <linux/fs.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>	//已经包含了	<time.h>
#include <dirent.h>     //目录处理
#include <ctype.h>      //包含 isspace()
#include <stdarg.h>     //包含 vsprintf(),vsnprintf()

#endif



#define INVALID_SOCKET      -1
#define SK_TYPE             int
#define SK_INVALID_VAL      INVALID_SOCKET
#define SK_COLSE            closesocket
#define INVALID_HANDLE      0xFFFFFFFF




#if 0 /*hxj amend,date 2014-5-27 10:32*/
#define ONE_TIMEOUT_SEND_VAL            20          //s
#define ONE_TIMEOUT_RECV_VAL            20          //s

#else
#define ONE_TIMEOUT_SEND_VAL            0          //s
#define ONE_TIMEOUT_RECV_VAL            0          //s
#endif

#define ONE_SEND_LEN                    1460




//-------------------------------------------------------------------------------------------------------------
//支持的协议定义
#define PROTOCOL_GH                                     0                           //0-工行协议
#define PROTOCOL_GR                                     1                           //1-光荣协议
#define PROTOCOL_WR                                     2                           //2-维融协议
#define PROTOCOL_ZX                                     3                           //3-中信协议

//当前用的协议定义
#define CUR_PROTOCOL                                    PROTOCOL_GH                 //当前协议


#if CUR_PROTOCOL>PROTOCOL_ZX
#error "CUR_PROTOCOL defile err"
#endif



//-------------------------------------------------------------------------------------------------------------
//DSP程序的版本号
#if CUR_PROTOCOL==PROTOCOL_GH
    //工行
    #define DSP_APP_VER  "$m%FOTA^*+*@032.144.T2"
#elif CUR_PROTOCOL==PROTOCOL_GR
    //光荣
    #define DSP_APP_VER  "$m%FOTA^*+*@032.143.G2"
#elif CUR_PROTOCOL==PROTOCOL_WR
    //维融
    #define DSP_APP_VER  "$m%FOTA^*+*@032.143.W1"
#elif CUR_PROTOCOL==PROTOCOL_ZX
    //中信
    #define DSP_APP_VER  "$m%FOTA^*+*@032.144.Z9"
#else
    #error "need define ver"
#endif

//-------------------------------------------------------------------------------------------------------------
//para
#define BOC_STR_VAL_1                                   'B'                         //BOC字符第1个值
#define BOC_STR_VAL_2                                   'O'                         //BOC字符第2个值
#define BOC_STR_VAL_3                                   'C'                         //BOC字符第3个值


#define PARA_FIRST_DELAY_S_VAL                          1                           //开始延时处理等待秒数
#define PARA_CONNECT_ERR_DELAY_S_VAL                    5                           //连接出错延时处理等待秒数
#define PARA_G_APP_NET_UP_DELAY						    5	                        //应用层网络up延时参数
#define PARA_RE_CONFIG_PHY_DELAY					    10	                        //重配置PHY延时参数


#define PARA_SIZE_DSP_RECV_CBUF                         (4*1024*1024)                       //接收DSP数据的环形缓冲的大小
#define PARA_SIZE_DSP_RECV_ONE_FRAME                    (2*1024*1024)                       //接收DSP数据一帧的大小
#define PARA_SIZE_DSP_SEND_ONE_FRAME                    PARA_SIZE_DSP_RECV_ONE_FRAME        //发送DSP数据一帧的大小


#define PARA_SIZE_ARM_UART_RECV_CBUF                    (1024*1024*1)                       //接收ARM串口数据的环形缓冲的大小
#define PARA_SIZE_ARM_UART_SEND_CBUF                    (1024*1024*1)                       //接收ARM串口数据的环形缓冲的大小

#define PARA_SIZE_ARM_UART_RECV_ONE_FRAME               (1024)                              //接收ARM串口数据一帧的大小
#define PARA_SIZE_ARM_UART_SEND_ONE_FRAME               PARA_SIZE_ARM_UART_RECV_ONE_FRAME   //发送ARM串口数据一帧的大小



//-------------------------------------------------------------------------------------------------------------
//test
#define TEST_NET_PC_SNED_DAT_EN			   	            0		                    //测试网络发送DAT使能

//mask fun
#define NET_PC_FUN_EN			   	                    1		                    //网络发送功能使能

#define DBG_NOT_SAVE_SD_AND_DIR_NET_SEND_EN             0                           //不保存SD卡,直接网发使能

#define DBG_NOT_SAVE_PARA_EN                            1                           //不保存参数使能
#define DBG_MASK_DSPMutex_EN                            1                           //屏幕DSP锁使能



#if FZ2000
#define DBG_SAVE_A8_PARA_USE_ADD_EN                     1                           //调试保存A8参数用地址方式使能

#define DBG_DSP_RECV_TWO_EN                             1                           //DSP的2路接收使能   in FZ2000
#define DBG_MASK_DSP_NET1_EN                            0                           //屏幕DSP的网络1使能 in FZ2000
#define DBG_MASK_DSP_NET2_EN                            0                           //屏幕DSP的网络2使能 in FZ2000
#else
#define DBG_SAVE_A8_PARA_USE_ADD_EN                     0                           //调试保存A8参数用地址方式使能

#define DBG_DSP_RECV_TWO_EN                             0                           //DSP的2路接收使能   in FZ1500
#define DBG_MASK_DSP_NET1_EN                            0                           //屏幕DSP的网络1使能 in FZ1500
#define DBG_MASK_DSP_NET2_EN                            0                           //屏幕DSP的网络2使能 in FZ1500
#endif



//dat文件操作标志
#define DATF_EMPTY                                      0                           //空
#define DATF_HAVE_DATA                                  1                           //填好数据还没有发送
#define DATF_SEND_START                                 2                           //填好的数据正在发送
#define DATF_SEND_OK                                    3                           //填好的数据发送成功
#define DATF_STOP_SEND                                  4                           //填好的数据不再发送



//================================================================================
//                      显示宏定义
//--------------------------------------------------------------------------------
//最后为0
#define DISPLAY_RECV_DAT_INFO_EN                        1       //显示接收DAT信息显示使能
#define DISPLAY_DAT_FILE_INFO_ENABLE                    0      	//显示DAT文件信息使能
#define DISPLAY_SYS_TIME_ENABLE                  	    0       //显示系统时间使能
#define DISPLAY_CONNECT_ENABLE                  	    1       //显示连接使能

#define DISPLAY_READ_DATA_TO_CYCBUF_ENABLE              0       //显示读数据到缓冲区使能
#define DISPLAY_RECV_CYC_BUF_FULL_EN                    1       //显示接收缓冲区满使能

#define DISPLAY_DSP_SEND_DATA_EN                        0       //显示发送dsp数据使能
#define DISPLAY_DSP_RECV_DATA_EN                        1       //显示接收dsp数据使能
#define DISPLAY_DSP_ID_INFO_EN                          1       //显示dsp通信ID信息使能
#define DISPLAY_DSP_ADD_MONEY_INFO_EN                   1       //显示dsp增加一张钞信息使能





//--------------------------------------------------------------------------------
//配置用(最后为1)



//--------------------------------------------------------------------------------



//--------------------------------------------------------------------------------
//数据类型定义

#if 0 /*hxj amend,date 2014-12-17 15:20*/
//这一段无需改动
#ifndef TRUE
#define TRUE  1
#endif

#ifndef true
#define true  1
#endif


#ifndef FALSE
#define FALSE 0
#endif

#ifndef false
#define false 0
#endif


#ifndef bool
#define bool char
#endif
#endif

#define VAL_VOL     volatile        //定义不让优化的变量

//--------------------------------------------------------------------------------
#ifndef _UINT8_32_
#define _UINT8_32_
//linux中

#define	uint8 	unsigned	char 	        //  8-bit	unsigned	value(0~255)
#define	int8  	signed  	char 	        //  8-bit	signed  	value(-128~127)
#define	uint16	unsigned	short	        // 16-bit	unsigned	value(0~65535)
#define	int16 	signed  	short	        // 16-bit	signed  	value(-32768~32767)
#define	uint32	unsigned	int	 	        // 32-bit	unsigned	value(0~4294967295)
#define	int32 	signed  	int	 	        // 32-bit	signed  	value(-2147483648~2147483647)
#define	uint64	unsigned    long long	 	// 64-bit	unsigned	value(0~18446744073709551615)
#define	int64 	signed  	long long	    // 64-bit	signed  	value(-9223372036854775808~9223372036854775807)
#define	f32   	            float   	    // 32-bit	    		value(-3.402823E-38～+3.402823E+38)   (表示绝对值非常小1.17E-38～3.40E+38)  //注:float占4字节
#define	f64   	            double  	    // 64-bit	    		value(-1.7E-308～+1.7E+308)           (表示绝对值非常小2.22E-308～1.7E+308) //注:double占8字节


#define	Uint8 	uint8
#define	Uint16 	uint16
#define	Uint32 	uint32

#define	Int8 	int8
#define	Int16 	int16
#define	Int32 	int32
#endif

//================================================================================
//                                  延时
//--------------------------------------------------------------------------------
//定时器
//基于1ms
#define  	Delay_10ms 			    10
#define  	Delay_30ms				30
#define  	Delay_50ms 			    50
#define  	Delay_70ms 			    70
#define  	Delay_90ms 			    90
#define  	Delay_100ms 			100
#define  	Delay_200ms 			200
#define  	Delay_300ms 			300
#define  	Delay_500ms 			500
#define  	Delay_700ms 			700
#define  	Delay_900ms 			900
#define 	Delay_1000ms 			1000
#define	    Delay_1s 				1000
#define	    Delay_2s 				2000
#define	    Delay_3s 				3000
#define	    Delay_4s 				4000
#define	    Delay_5s 				5000
#define	    Delay_6s 				6000
#define	    Delay_7s 				7000
#define	    Delay_8s 				8000
#define	    Delay_9s 				9000
#define	    Delay_10s 				10000
#define  	Delay_1min  			60000

extern uint32 get_sys_dds(void);
extern uint32 get_sys_ddms(void);

#define  	MY_DELAY_X_MS(ms)       OSTimeDlyHMSM (0,0,0,(ms))          //延时 ms 毫秒
#define  	MY_DELAY_X_S(s)         OSTimeDlyHMSM (0,0,(s),0)           //延时  s 秒
#define  	DELAY_X_MS(ms)          OSTimeDlyHMSM (0,0,0,(ms))          //延时 ms 毫秒
#define  	DELAY_X_S(s)            OSTimeDlyHMSM (0,0,(s),0)           //延时  s 秒
#define  	GET_SYS_DD_S(p)         get_sys_dds()                       //获取系统嘀嗒秒数
#define  	GET_SYS_DDS(p)          get_sys_dds()                       //获取系统嘀嗒秒数
#define  	GET_SYS_DD_MS(p)        get_sys_ddms()                      //获取系统嘀嗒秒数
#define  	GET_SYS_DDMS(p)         get_sys_ddms()                      //获取系统嘀嗒秒数



//---------------------------------------------------------------------------
//                      位结构定义
//---------------------------------------------------------------------------


#if ARM_USE

#else

//位定义(8位)
typedef	struct	_strBit8_
{
	uint8   b0: 1;
	uint8   b1: 1;
	uint8   b2: 1;
	uint8   b3: 1;
	uint8   b4: 1;
	uint8   b5: 1;
	uint8   b6: 1;
	uint8   b7: 1;
} strBit8;

typedef	union	_uniBit8_
{
	strBit8  myb8;
	uint8    z8;
} uniBit8;

//位定义(16位)
typedef	struct	_strBit16_
{
	uint8   b0: 1;
	uint8   b1: 1;
	uint8   b2: 1;
	uint8   b3: 1;
	uint8   b4: 1;
	uint8   b5: 1;
	uint8   b6: 1;
	uint8   b7: 1;
	uint8   b8: 1;
	uint8   b9: 1;
	uint8   b10: 1;
	uint8   b11: 1;
	uint8   b12: 1;
	uint8   b13: 1;
	uint8   b14: 1;
	uint8   b15: 1;
} strBit16;

typedef	union	_uniBit16_
{
	strBit16  myb16;
	uint16    z16;
} uniBit16;

//位定义(32位)
typedef	struct	_strBit32_
{
	uint8   b0: 1;
	uint8   b1: 1;
	uint8   b2: 1;
	uint8   b3: 1;
	uint8   b4: 1;
	uint8   b5: 1;
	uint8   b6: 1;
	uint8   b7: 1;
	uint8   b8: 1;
	uint8   b9: 1;
	uint8   b10: 1;
	uint8   b11: 1;
	uint8   b12: 1;
	uint8   b13: 1;
	uint8   b14: 1;
	uint8   b15: 1;
	uint8   b16: 1;
	uint8   b17: 1;
	uint8   bi8: 1;
	uint8   b19: 1;
	uint8   b20: 1;
	uint8   b21: 1;
	uint8   b22: 1;
	uint8   b23: 1;
	uint8   b24: 1;
	uint8   b25: 1;
	uint8   b26: 1;
	uint8   b27: 1;
	uint8   b28: 1;
	uint8   b29: 1;
	uint8   b30: 1;
	uint8   b31: 1;
} strBit32;

typedef	union	_uniBit32_
{
	strBit32  myb32;
	uint32    z32;
} uniBit32;


#endif






//---------------------------------------------------------------------------
//                              位操作
//---------------------------------------------------------------------------
#define     bit8_set(data,num)              data |= (((uint8)1)<<(num))             //把data的第num(0~ 7)位置位
#define     bit8_clr(data,num)              data &= (~(((uint8)1)<<(num)))          //把data的第num(0~ 7)位清零
#define     bit16_set(data,num)             data |= (((uint16)1)<<(num))            //把data的第num(0~15)位置位
#define     bit16_clr(data,num)             data &= (~(((uint16)1)<<(num)))         //把data的第num(0~15)位清零
#define     bit32_set(data,num)             data |= (((uint32)1)<<(num))            //把data的第num(0~31)位置位
#define     bit32_clr(data,num)             data &= (~(((uint32)1)<<(num)))         //把data的第num(0~31)位清零
#define     bit64_set(data,num)             data |= ((1LL)<<(num))                  //把data的第num(0~63)位置位
#define     bit64_clr(data,num)             data &= (~((1LL)<<(num)))               //把data的第num(0~63)位清零
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#define     bit8_is(data,num)               (data & (((uint8)1)<<(num)))            //判断data的第num(0~ 7)位,如为0,则值为0,否则不为0
#define     bit16_is(data,num)              (data & (((uint16)1)<<(num)))           //判断data的第num(0~15)位,如为0,则值为0,否则不为0
#define     bit32_is(data,num)              (data & (((uint32)1)<<(num)))           //判断data的第num(0~31)位,如为0,则值为0,否则不为0
#define     bit64_is(data,num)              (data & ((1LL)<<(num)))                 //判断data的第num(0~63)位,如为0,则值为0,否则不为0
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#define     val8_set(num)                   (((uint8)1)<<(num))                     //把0的第num(0~ 7)位置位
#define     val16_set(num)                  (((uint16)1)<<(num))                    //把0的第num(0~15)位置位
#define     val32_set(num)                  (((uint32)1)<<(num))                    //把0的第num(0~31)位置位
#define     val64_set(num)                  ((1LL)<<(num))                          //把0的第num(0~63)位置位
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#define     bit8_1(data,num)                bit8_set(data,num)                      //把data的第num(0~ 7)位置位
#define     bit8_0(data,num)                bit8_clr(data,num)                      //把data的第num(0~ 7)位清零
#define     bit16_1(data,num)               bit16_set(data,num)                     //把data的第num(0~15)位置位
#define     bit16_0(data,num)               bit16_clr(data,num)                     //把data的第num(0~15)位清零
#define     bit32_1(data,num)               bit32_set(data,num)                     //把data的第num(0~31)位置位
#define     bit32_0(data,num)               bit32_clr(data,num)                     //把data的第num(0~31)位清零
#define     bit64_1(data,num)               bit64_set(data,num)                     //把data的第num(0~63)位置位
#define     bit64_0(data,num)               bit64_clr(data,num)                     //把data的第num(0~63)位清零
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//位判断 1
#define     BIT_X_IS(all_bits,data,num)         bit##all_bits##_is(data,num)        //判断data的第num(从0开始)位是否为1,all_bits指data总位数(8,16,32,64)flag(0-清零,1-置位)
#define     BIT_M_IS(         data,num)         bit32_is(data,num)
//位操作
#define     BIT_X_OPT(all_bits,flag,data,num)   bit##all_bits##_##flag(data,num)    //把data的第num(从0开始)位清零或置位,all_bits指data总位数(8,16,32,64)flag(0-清零,1-置位)
#define     BIT_M_OPT(        flag,data,num)    bit32_##flag(data,num)              //把data的第num(从0开始)位清零或置位,data总位数32,flag(0-清零,1-置位)
//位值
#define     BIT_X_VAL(all_bits,num)             val##all_bits##_set(num)
#define     BIT_M_VAL(        num)              val32_set(num)
//================================================================================
//                      数值判断
//--------------------------------------------------------------------------------
#define    is_digital(a)  (((a)>=0)&&((a)<=9))
#define    is_digital_str(a)    (((a)>='0')&&((a)<='9'))

//================================================================================
//                      字节处理
//--------------------------------------------------------------------------------

#define    GET_4B_NETSN_VAL(add)   ((((uint32)(*(((uint8 *)(add))+0)))<<24) | (((uint32)(*(((uint8 *)(add))+1)))<<16) | (((uint32)(*(((uint8 *)(add))+2)))<<8)  | (((uint32)(*(((uint8 *)(add))+3)))<<0))
#define    SET_4B_NETSN_VAL(add,val,cp_len)   (*(((uint8 *)(add))+0))=(val)>>24;(*(((uint8 *)(add))+1))=(val)>>16;(*(((uint8 *)(add))+2))=(val)>>8;(*(((uint8 *)(add))+3))=(val)>>0

#define    GET_2B_NETSN_VAL(add)   ((((uint16)(*(((uint8 *)(add))+0)))<<8)  | (((uint16)(*(((uint8 *)(add))+1)))<<0))
#define    SET_2B_NETSN_VAL(add,val,cp_len)   (*(((uint8 *)(add))+0))=(val)>>8;(*(((uint8 *)(add))+1))=(val)>>0


#define    GET_1B_VAL(add)              (*(((uint8 *)(add))+0))
#define    SET_1B_VAL(add,val,cp_len)   (*(((uint8 *)(add))+0))=(val)


#define    SET_4B_SMALLSN_VAL(add,val,cp_len)   (*(((uint8 *)(add))+0))=(val)>>0;(*(((uint8 *)(add))+1))=(val)>>8;(*(((uint8 *)(add))+2))=(val)>>16;(*(((uint8 *)(add))+3))=(val)>>24
#define    GET_4B_SMALLSN_VAL(add)   ((((uint32)(*(((uint8 *)(add))+0)))<<0) | (((uint32)(*(((uint8 *)(add))+1)))<<8) | (((uint32)(*(((uint8 *)(add))+2)))<<16)  | (((uint32)(*(((uint8 *)(add))+3)))<<24))
#define    GET_2B_SMALLSN_VAL(add)   ((((uint16)(*(((uint8 *)(add))+0)))<<0)  | (((uint16)(*(((uint8 *)(add))+1)))<<8))
#define    SET_2B_SMALLSN_VAL(add,val,cp_len)   (*(((uint8 *)(add))+0))=(val)>>0;(*(((uint8 *)(add))+1))=(val)>>8


#define    GET_1B_OFF_VAL(add,off)              (*(((uint8 *)(add))+(off)))
#define    SET_1B_OFF_VAL(add,off,val,cp_len)   (*(((uint8 *)(add))+(off)))=(val)


#define    GET_2B_OFF_SMALLSN_VAL(add,off)              ((((uint16)(*(((uint8 *)(add))+(off)+0)))<<0)  | (((uint16)(*(((uint8 *)(add))+(off)+1)))<<8))
#define    SET_2B_OFF_SMALLSN_VAL(add,off,val,cp_len)   (*(((uint8 *)(add))+(off)+0))=(val)>>0;(*(((uint8 *)(add))+(off)+1))=(val)>>8









//================================================================================
//                      获取字符值处理
//--------------------------------------------------------------------------------

// "1"--->1,"a"--->10,"A"--->10
#define GET_1B_ASCII_VAL(add,off)	 										        \
    ((((*(((uint8 *)(add))+(off)))<='9')&&((*(((uint8 *)(add))+(off)))>='0'))? 	\
    ((*(((uint8 *)(add))+(off)))-'0'+ 0) 											\
    :((((*(((uint8 *)(add))+(off)))<='F')&&((*(((uint8 *)(add))+(off)))>='A'))? 	\
    ((*(((uint8 *)(add))+(off)))-'A'+10) 											\
    :((((*(((uint8 *)(add))+(off)))<='f')&&((*(((uint8 *)(add))+(off)))>='a'))? 	\
    ((*(((uint8 *)(add))+(off)))-'a'+10)   										\
    :0)) 																			    \
    )

//"100F" -->0x100F
#define    GET_4B_ASICC_16_VAL(add)  	    \
    ( ((uint16)GET_1B_ASCII_VAL(add,0)<<12)     \
    | ((uint16)GET_1B_ASCII_VAL(add,1)<<8)      \
    | ((uint16)GET_1B_ASCII_VAL(add,2)<<4)      \
    | ((uint16)GET_1B_ASCII_VAL(add,3)<<0) 		\
    )


//"123af10F" -->0x123af10F
#define    GET_8B_ASICC_16_VAL(add)  	    \
    ( ((uint16)GET_1B_ASCII_VAL(add,0)<<28)     \
    | ((uint16)GET_1B_ASCII_VAL(add,1)<<24)     \
    | ((uint16)GET_1B_ASCII_VAL(add,2)<<20)     \
    | ((uint16)GET_1B_ASCII_VAL(add,3)<<16)     \
    | ((uint16)GET_1B_ASCII_VAL(add,4)<<12)     \
    | ((uint16)GET_1B_ASCII_VAL(add,5)<<8) 		\
    | ((uint16)GET_1B_ASCII_VAL(add,6)<<4) 		\
    | ((uint16)GET_1B_ASCII_VAL(add,7)<<0) 		\
    )


// "15"  -->15
#define    GET_2B_ASICC_10_VAL(add)  		    \
     ( ((uint16)(GET_1B_ASCII_VAL(add,0)*10))   	\
     + ((uint16)(GET_1B_ASCII_VAL(add,1)*1))  	    \
     )


//"2014" -->2014
#define    GET_4B_ASICC_10_VAL(add)  		    \
     ( ((uint32)(GET_1B_ASCII_VAL(add,0)*1000))   	\
     + ((uint32)(GET_1B_ASCII_VAL(add,1)*100))  	\
     + ((uint32)(GET_1B_ASCII_VAL(add,2)*10))      \
     + ((uint32)(GET_1B_ASCII_VAL(add,3)*1))  	    \
     )


//================================================================================
//                  字节获取
//--------------------------------------------------------------------------------
#define     my_ip4_addr1(ipaddr)            (((uint8*)(ipaddr))[0])
#define     my_ip4_addr2(ipaddr)            (((uint8*)(ipaddr))[1])
#define     my_ip4_addr3(ipaddr)            (((uint8*)(ipaddr))[2])
#define     my_ip4_addr4(ipaddr)            (((uint8*)(ipaddr))[3])

#define     my_ip4_addr1_16(ipaddr)         ((uint16)my_ip4_addr1(ipaddr))
#define     my_ip4_addr2_16(ipaddr)         ((uint16)my_ip4_addr2(ipaddr))
#define     my_ip4_addr3_16(ipaddr)         ((uint16)my_ip4_addr3(ipaddr))
#define     my_ip4_addr4_16(ipaddr)         ((uint16)my_ip4_addr4(ipaddr))

//================================================================================






//--------------------------------------------------------------------------------
//计算宏
#define     GET_LIST_NUM(L)                 (sizeof(L)/sizeof(L[0]))                //获取列表成员个数








extern void  loginfo ( const char *fmt, ... );
extern void  logbuf ( unsigned char *buf, int buflen, char *fmt , ... );



//--------------------------------------------------------------------------------
//普通打印
#if 1
#define 	DBG_NORMAL(fmt, args...)		        loginfo(fmt, ## args)
#else
#define	    DBG_NORMAL(fmt, args...)
#endif

#if 1
#define 	DBG_MAL(fmt, args...)		            loginfo(fmt, ## args)
#else
#define	    DBG_MAL(fmt, args...)
#endif


#if 1
#define 	DBG_DIS(fmt, args...)		            loginfo(fmt, ## args)
#else
#define	    DBG_DIS(fmt, args...)
#endif


#if 1
#define 	DBG_OS(fmt, args...)		            loginfo(fmt, ## args)
#else
#define	    DBG_OS(fmt, args...)
#endif


#if 0
#define 	DBG_RECV(fmt, args...)		            loginfo(fmt, ## args)
#else
#define	    DBG_RECV(fmt, args...)
#endif



#if 1
#define 	DBG_PRO(fmt, args...)		            loginfo(fmt, ## args)
#else
#define	    DBG_PRO(fmt, args...)
#endif


#if 1
#define 	DBG_ID(fmt, args...)		            loginfo(fmt, ## args)
#else
#define	    DBG_ID(fmt, args...)
#endif




#if 1
#define 	DBG_BUF_PRO(buf,buflen,fmt, args...)    logbuf((buf),(buflen),fmt, ## args)
#else
#define	    DBG_BUF_PRO(fmt, args...)
#endif




#if 0
#define 	DBG_ARP(fmt, args...)		            loginfo(fmt, ## args)
#else
#define	    DBG_ARP(fmt, args...)
#endif


#if 0
#define 	DBG_BUF_ARP(buf,buflen,fmt, args...)    logbuf((buf),(buflen),fmt, ## args)
#else
#define	    DBG_BUF_ARP(fmt, args...)
#endif



#if 0
#define 	DBG_CON(fmt, args...)		            loginfo(fmt, ## args)
#else
#define	    DBG_CON(fmt, args...)
#endif


#if 0
#define 	DBG_BUF_CON(buf,buflen,fmt, args...)    logbuf((buf),(buflen),fmt, ## args)
#else
#define	    DBG_BUF_CON(fmt, args...)
#endif




#if 0
#define 	DBG_NET(fmt, args...)		            loginfo(fmt, ## args)
#else
#define	    DBG_NET(fmt, args...)
#endif

#if 0
#define 	DBG_BUF_NET(buf,buflen,fmt, args...)    logbuf((buf),(buflen),fmt, ## args)
#else
#define	    DBG_BUF_NET(fmt, args...)
#endif


#if 1
#define 	DBG_BUF_NOR(buf,buflen,fmt, args...)    logbuf((buf),(buflen),fmt, ## args)
#else
#define	    DBG_BUF_NOR(fmt, args...)
#endif


#if 1
#define 	MY_LOGBUF(buf,buflen,fmt, args...)      logbuf((buf),(buflen),fmt, ## args)
#else
#define	    MY_LOGBUF(fmt, args...)
#endif



//告警打印
#if 1
#define 	DBG_ALARM(fmt, args...)		            loginfo("\r\nALARM:[File: %s Line: %d] Function: %s ,",__FILE__,__LINE__,__FUNCTION__);loginfo(fmt, ## args)
#else
#define 	DBG_ALARM(fmt, args...)
#endif


#if 1
#define 	DBG_STOP(fmt, args...)		            loginfo("\r\nSTOP:[File: %s Line: %d] Function: %s ,",__FILE__,__LINE__,__FUNCTION__);loginfo(fmt, ## args)
#else
#define 	DBG_STOP(fmt, args...)
#endif




//测试
#if 1
#define 	DBG_TEST(fmt, args...)			        loginfo(fmt, ## args)
#else
#define 	DBG_TEST(fmt, args...)
#endif


//--------------------------------------------------------------------------------
//打印:文件\函数\行
#if 1
#define 	DBG_FILE_INFO()				            loginfo("\n[File: %s Line: %d] Function: %s\n",__FILE__,__LINE__,__FUNCTION__)
#else
#define 	DBG_FILE_INFO()
#endif



//错误
#if 1
#define 	DBG_ERROR_FILE_INFO()	                loginfo("\nerror:[File: %s Line: %d] Function: %s , ",__FILE__,__LINE__,__FUNCTION__)

#else
#define 	DBG_ERROR_FILE_INFO()
#endif




//错误
#if 1
#define 	DBG_ERROR_MY(fmt, args...)	            perror ( "error info:\n" );DBG_ERROR_FILE_INFO();loginfo(fmt, ## args)
#else
#define 	DBG_ERROR_MY(fmt, args...)
#endif



//系统错误
#if 1
#define 	DBG_SYS_ERROR(fmt, args...)	            perror ( "error info:\n" );DBG_ERROR_FILE_INFO();loginfo(fmt, ## args)
#else
#define 	DBG_SYS_ERROR(fmt, args...)
#endif


//其他错误打印输出
#if 1
#define 	DBG_OTHER(fmt, args...)		            loginfo(fmt, ## args)
#else
#define 	DBG_OTHER(fmt, args...)
#endif



#if 0
#define     my_ip_addr_debug_print(debug, ipaddr) \
                    loginfo("%02X.%02X.%02X.%02X\r\n",             \
                        ipaddr != NULL ? my_ip4_addr1_16(ipaddr) : 0,       \
                        ipaddr != NULL ? my_ip4_addr2_16(ipaddr) : 0,       \
                        ipaddr != NULL ? my_ip4_addr3_16(ipaddr) : 0,       \
                        ipaddr != NULL ? my_ip4_addr4_16(ipaddr) : 0)
#else
#define     my_ip_addr_debug_print(debug, ipaddr)
#endif




//显示宏函数
#define     STOP_RUN(des)               DBG_STOP(des)
#define 	NEED_ADD_CODE()		        DBG_ALARM("need add code\n")
#define     DIS_HELP_NOT_SUPPORT()  	DBG_FILE_INFO();DBG_NORMAL( "Current version not support!\n" )
#define     DIS_ERR_NOT_SUPPORT()       DBG_ERROR_MY ( "not support !!! \n" );
#define     DIS_ERR_NEED_ADD_CODE()	    DBG_ERROR_MY("need to add code at here\n");





#if ARM_USE
//UCOS
#define 	ATT_BYTE_NO_ALIGNED
#define 	ATT_BYTE_NO_ALIGNED_SUPPORT_pragma_pack(n)      1                   //用IAR支持的不对齐,紧凑分布

#else
//LINUX
//--------------------------------------------------------------------------------
//属性定义
//字节不对齐设置
#define 	ATT_BYTE_NO_ALIGNED   	    __attribute__((packed))                 //设置字节为不对齐,紧凑分布
#define 	ATT_BYTE_ALIGNED   	        __attribute__((aligned))                //字节对齐(默认)
//参数格式定义
#define     ATT_FORMAT_DEF_1_2          __attribute__((format(printf,1,2)))    //检测参数
#define     ATT_FORMAT_DEF_2_3          __attribute__((format(printf,2,3)))    //检测参数
#define     ATT_FORMAT_DEF_3_4          __attribute__((format(printf,3,4)))    //检测参数

#endif
//--------------------------------------------------------------------------------










//--------------------------------------------------------------------------------
//宏判断






//---------------------------------------------------------------------------------
/*
//测试字体对齐
12345678901234567890123456
abcdefghijklmnopqrstuvwxyz
ABCDEFGHIJKLMNOPQRSTUVWXYZ
11111111111111111111111111
llllllllllllllllllllllllll
LLLLLLLLLLLLLLLLLLLLLLLLLL
ZZZZZZZZZZZZZZZZZZZZZZZZZZ
结结结结结结结结结结结结结
义义义义义义义义义义义义义
*/

//---------------------------------------------------------------------------------
//                      结构体定义
//---------------------------------------------------------------------------------


#if ARM_USE

#else

//定义浮点计算结构体
//不对齐
typedef	union	_uniFloatCountN_
{
	float   f_num;      //浮点数
	uint32  b4_dat;     // 4字节
	uint8   b_dat[4];   //浮点字节      ,b_dat[3]的最高位为符号位,1-负值,0-正值
} ATT_BYTE_NO_ALIGNED
uniFloatCountN;


//对齐
typedef	union	_uniFloatCount_
{
	float   f_num;      //浮点数
	uint32  b4_dat;     // 4字节
	uint8   b_dat[4];   //浮点字节      ,b_dat[3]的最高位为符号位,1-负值,0-正值
} uniFloatCount;



//不对齐
typedef	union	_uni4bCountN_
{
	uint32  b4_dat;
	int32   b4_dat_s;
	uint8   b_dat[4];
} ATT_BYTE_NO_ALIGNED
uni4bCountN;


//对齐
typedef	union	_uni4bCount_
{
	uint32  b4_dat;
	int32   b4_dat_s;
	uint8   b_dat[4];
} uni4bCount;


//不对齐
typedef	union	_uni2bCountN_
{
	uint16  b2_dat;
	int16   b2_dat_s;
	uint8   b_dat[2];
} ATT_BYTE_NO_ALIGNED
uni2bCountN;


//对齐
typedef	union	_uni2bCount_
{
	uint16  b2_dat;
	int16   b2_dat_s;
	uint8   b_dat[2];
} uni2bCount;

#endif




#pragma   pack(1)
//======================================
//不对齐测试,测试IAR是ok的长度为5
typedef	struct	_strByte1_
{
    uint8   b_dat;
	uint32  b2_dat;
} ATT_BYTE_NO_ALIGNED
strByte1;
#pragma   pack()




#endif
/****************************************************************************************************
**                            End Of File
*****************************************************************************************************/





