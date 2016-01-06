/***************�ļ���Ϣ********************************************************************************
**��   ��   ��: def_config.h
**˵		��: ������׼��ͷ�ļ�,ȫ�ֵĺ궨��,�ṹ�嶨��
**��   ��   ��: ��С��
**��   �� ����: 2010��11��03��
----------------��ʷ��Ϣ--------------------------------------------------------------------------------
**��   �� ����: XXXXXX
**��   ��   ��: XXX
**��   �� ����: XXXX��XX��XX��
********************************************************************************************************/
#ifndef  __DEF_CONFIG_H
#define  __DEF_CONFIG_H


#define ARM_USE          1
#define DBG_GBG          0
#define lin_USE          0



//---------------------------------------------------------------------------------
//                      ͷ�ļ�����
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
#include <ctype.h>      //���� isspace()
#include <stdarg.h>     //���� vsprintf(),vsnprintf()

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
#include <sys/time.h>	//�Ѿ�������	<time.h>
#include <dirent.h>     //Ŀ¼����
#include <ctype.h>      //���� isspace()
#include <stdarg.h>     //���� vsprintf(),vsnprintf()

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
//֧�ֵ�Э�鶨��
#define PROTOCOL_GH                                     0                           //0-����Э��
#define PROTOCOL_GR                                     1                           //1-����Э��
#define PROTOCOL_WR                                     2                           //2-ά��Э��
#define PROTOCOL_ZX                                     3                           //3-����Э��

//��ǰ�õ�Э�鶨��
#define CUR_PROTOCOL                                    PROTOCOL_GH                 //��ǰЭ��


#if CUR_PROTOCOL>PROTOCOL_ZX
#error "CUR_PROTOCOL defile err"
#endif



//-------------------------------------------------------------------------------------------------------------
//DSP����İ汾��
#if CUR_PROTOCOL==PROTOCOL_GH
    //����
    #define DSP_APP_VER  "$m%FOTA^*+*@032.144.T2"
#elif CUR_PROTOCOL==PROTOCOL_GR
    //����
    #define DSP_APP_VER  "$m%FOTA^*+*@032.143.G2"
#elif CUR_PROTOCOL==PROTOCOL_WR
    //ά��
    #define DSP_APP_VER  "$m%FOTA^*+*@032.143.W1"
#elif CUR_PROTOCOL==PROTOCOL_ZX
    //����
    #define DSP_APP_VER  "$m%FOTA^*+*@032.144.Z9"
#else
    #error "need define ver"
#endif

//-------------------------------------------------------------------------------------------------------------
//para
#define BOC_STR_VAL_1                                   'B'                         //BOC�ַ���1��ֵ
#define BOC_STR_VAL_2                                   'O'                         //BOC�ַ���2��ֵ
#define BOC_STR_VAL_3                                   'C'                         //BOC�ַ���3��ֵ


#define PARA_FIRST_DELAY_S_VAL                          1                           //��ʼ��ʱ����ȴ�����
#define PARA_CONNECT_ERR_DELAY_S_VAL                    5                           //���ӳ�����ʱ����ȴ�����
#define PARA_G_APP_NET_UP_DELAY						    5	                        //Ӧ�ò�����up��ʱ����
#define PARA_RE_CONFIG_PHY_DELAY					    10	                        //������PHY��ʱ����


#define PARA_SIZE_DSP_RECV_CBUF                         (4*1024*1024)                       //����DSP���ݵĻ��λ���Ĵ�С
#define PARA_SIZE_DSP_RECV_ONE_FRAME                    (2*1024*1024)                       //����DSP����һ֡�Ĵ�С
#define PARA_SIZE_DSP_SEND_ONE_FRAME                    PARA_SIZE_DSP_RECV_ONE_FRAME        //����DSP����һ֡�Ĵ�С


#define PARA_SIZE_ARM_UART_RECV_CBUF                    (1024*1024*1)                       //����ARM�������ݵĻ��λ���Ĵ�С
#define PARA_SIZE_ARM_UART_SEND_CBUF                    (1024*1024*1)                       //����ARM�������ݵĻ��λ���Ĵ�С

#define PARA_SIZE_ARM_UART_RECV_ONE_FRAME               (1024)                              //����ARM��������һ֡�Ĵ�С
#define PARA_SIZE_ARM_UART_SEND_ONE_FRAME               PARA_SIZE_ARM_UART_RECV_ONE_FRAME   //����ARM��������һ֡�Ĵ�С



//-------------------------------------------------------------------------------------------------------------
//test
#define TEST_NET_PC_SNED_DAT_EN			   	            0		                    //�������緢��DATʹ��

//mask fun
#define NET_PC_FUN_EN			   	                    1		                    //���緢�͹���ʹ��

#define DBG_NOT_SAVE_SD_AND_DIR_NET_SEND_EN             0                           //������SD��,ֱ������ʹ��

#define DBG_NOT_SAVE_PARA_EN                            1                           //���������ʹ��
#define DBG_MASK_DSPMutex_EN                            1                           //��ĻDSP��ʹ��



#if FZ2000
#define DBG_SAVE_A8_PARA_USE_ADD_EN                     1                           //���Ա���A8�����õ�ַ��ʽʹ��

#define DBG_DSP_RECV_TWO_EN                             1                           //DSP��2·����ʹ��   in FZ2000
#define DBG_MASK_DSP_NET1_EN                            0                           //��ĻDSP������1ʹ�� in FZ2000
#define DBG_MASK_DSP_NET2_EN                            0                           //��ĻDSP������2ʹ�� in FZ2000
#else
#define DBG_SAVE_A8_PARA_USE_ADD_EN                     0                           //���Ա���A8�����õ�ַ��ʽʹ��

#define DBG_DSP_RECV_TWO_EN                             0                           //DSP��2·����ʹ��   in FZ1500
#define DBG_MASK_DSP_NET1_EN                            0                           //��ĻDSP������1ʹ�� in FZ1500
#define DBG_MASK_DSP_NET2_EN                            0                           //��ĻDSP������2ʹ�� in FZ1500
#endif



//dat�ļ�������־
#define DATF_EMPTY                                      0                           //��
#define DATF_HAVE_DATA                                  1                           //������ݻ�û�з���
#define DATF_SEND_START                                 2                           //��õ��������ڷ���
#define DATF_SEND_OK                                    3                           //��õ����ݷ��ͳɹ�
#define DATF_STOP_SEND                                  4                           //��õ����ݲ��ٷ���



//================================================================================
//                      ��ʾ�궨��
//--------------------------------------------------------------------------------
//���Ϊ0
#define DISPLAY_RECV_DAT_INFO_EN                        1       //��ʾ����DAT��Ϣ��ʾʹ��
#define DISPLAY_DAT_FILE_INFO_ENABLE                    0      	//��ʾDAT�ļ���Ϣʹ��
#define DISPLAY_SYS_TIME_ENABLE                  	    0       //��ʾϵͳʱ��ʹ��
#define DISPLAY_CONNECT_ENABLE                  	    1       //��ʾ����ʹ��

#define DISPLAY_READ_DATA_TO_CYCBUF_ENABLE              0       //��ʾ�����ݵ�������ʹ��
#define DISPLAY_RECV_CYC_BUF_FULL_EN                    1       //��ʾ���ջ�������ʹ��

#define DISPLAY_DSP_SEND_DATA_EN                        0       //��ʾ����dsp����ʹ��
#define DISPLAY_DSP_RECV_DATA_EN                        1       //��ʾ����dsp����ʹ��
#define DISPLAY_DSP_ID_INFO_EN                          1       //��ʾdspͨ��ID��Ϣʹ��
#define DISPLAY_DSP_ADD_MONEY_INFO_EN                   1       //��ʾdsp����һ�ų���Ϣʹ��





//--------------------------------------------------------------------------------
//������(���Ϊ1)



//--------------------------------------------------------------------------------



//--------------------------------------------------------------------------------
//�������Ͷ���

#if 0 /*hxj amend,date 2014-12-17 15:20*/
//��һ������Ķ�
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

#define VAL_VOL     volatile        //���岻���Ż��ı���

//--------------------------------------------------------------------------------
#ifndef _UINT8_32_
#define _UINT8_32_
//linux��

#define	uint8 	unsigned	char 	        //  8-bit	unsigned	value(0~255)
#define	int8  	signed  	char 	        //  8-bit	signed  	value(-128~127)
#define	uint16	unsigned	short	        // 16-bit	unsigned	value(0~65535)
#define	int16 	signed  	short	        // 16-bit	signed  	value(-32768~32767)
#define	uint32	unsigned	int	 	        // 32-bit	unsigned	value(0~4294967295)
#define	int32 	signed  	int	 	        // 32-bit	signed  	value(-2147483648~2147483647)
#define	uint64	unsigned    long long	 	// 64-bit	unsigned	value(0~18446744073709551615)
#define	int64 	signed  	long long	    // 64-bit	signed  	value(-9223372036854775808~9223372036854775807)
#define	f32   	            float   	    // 32-bit	    		value(-3.402823E-38��+3.402823E+38)   (��ʾ����ֵ�ǳ�С1.17E-38��3.40E+38)  //ע:floatռ4�ֽ�
#define	f64   	            double  	    // 64-bit	    		value(-1.7E-308��+1.7E+308)           (��ʾ����ֵ�ǳ�С2.22E-308��1.7E+308) //ע:doubleռ8�ֽ�


#define	Uint8 	uint8
#define	Uint16 	uint16
#define	Uint32 	uint32

#define	Int8 	int8
#define	Int16 	int16
#define	Int32 	int32
#endif

//================================================================================
//                                  ��ʱ
//--------------------------------------------------------------------------------
//��ʱ��
//����1ms
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

#define  	MY_DELAY_X_MS(ms)       OSTimeDlyHMSM (0,0,0,(ms))          //��ʱ ms ����
#define  	MY_DELAY_X_S(s)         OSTimeDlyHMSM (0,0,(s),0)           //��ʱ  s ��
#define  	DELAY_X_MS(ms)          OSTimeDlyHMSM (0,0,0,(ms))          //��ʱ ms ����
#define  	DELAY_X_S(s)            OSTimeDlyHMSM (0,0,(s),0)           //��ʱ  s ��
#define  	GET_SYS_DD_S(p)         get_sys_dds()                       //��ȡϵͳ�������
#define  	GET_SYS_DDS(p)          get_sys_dds()                       //��ȡϵͳ�������
#define  	GET_SYS_DD_MS(p)        get_sys_ddms()                      //��ȡϵͳ�������
#define  	GET_SYS_DDMS(p)         get_sys_ddms()                      //��ȡϵͳ�������



//---------------------------------------------------------------------------
//                      λ�ṹ����
//---------------------------------------------------------------------------


#if ARM_USE

#else

//λ����(8λ)
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

//λ����(16λ)
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

//λ����(32λ)
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
//                              λ����
//---------------------------------------------------------------------------
#define     bit8_set(data,num)              data |= (((uint8)1)<<(num))             //��data�ĵ�num(0~ 7)λ��λ
#define     bit8_clr(data,num)              data &= (~(((uint8)1)<<(num)))          //��data�ĵ�num(0~ 7)λ����
#define     bit16_set(data,num)             data |= (((uint16)1)<<(num))            //��data�ĵ�num(0~15)λ��λ
#define     bit16_clr(data,num)             data &= (~(((uint16)1)<<(num)))         //��data�ĵ�num(0~15)λ����
#define     bit32_set(data,num)             data |= (((uint32)1)<<(num))            //��data�ĵ�num(0~31)λ��λ
#define     bit32_clr(data,num)             data &= (~(((uint32)1)<<(num)))         //��data�ĵ�num(0~31)λ����
#define     bit64_set(data,num)             data |= ((1LL)<<(num))                  //��data�ĵ�num(0~63)λ��λ
#define     bit64_clr(data,num)             data &= (~((1LL)<<(num)))               //��data�ĵ�num(0~63)λ����
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#define     bit8_is(data,num)               (data & (((uint8)1)<<(num)))            //�ж�data�ĵ�num(0~ 7)λ,��Ϊ0,��ֵΪ0,����Ϊ0
#define     bit16_is(data,num)              (data & (((uint16)1)<<(num)))           //�ж�data�ĵ�num(0~15)λ,��Ϊ0,��ֵΪ0,����Ϊ0
#define     bit32_is(data,num)              (data & (((uint32)1)<<(num)))           //�ж�data�ĵ�num(0~31)λ,��Ϊ0,��ֵΪ0,����Ϊ0
#define     bit64_is(data,num)              (data & ((1LL)<<(num)))                 //�ж�data�ĵ�num(0~63)λ,��Ϊ0,��ֵΪ0,����Ϊ0
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#define     val8_set(num)                   (((uint8)1)<<(num))                     //��0�ĵ�num(0~ 7)λ��λ
#define     val16_set(num)                  (((uint16)1)<<(num))                    //��0�ĵ�num(0~15)λ��λ
#define     val32_set(num)                  (((uint32)1)<<(num))                    //��0�ĵ�num(0~31)λ��λ
#define     val64_set(num)                  ((1LL)<<(num))                          //��0�ĵ�num(0~63)λ��λ
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
#define     bit8_1(data,num)                bit8_set(data,num)                      //��data�ĵ�num(0~ 7)λ��λ
#define     bit8_0(data,num)                bit8_clr(data,num)                      //��data�ĵ�num(0~ 7)λ����
#define     bit16_1(data,num)               bit16_set(data,num)                     //��data�ĵ�num(0~15)λ��λ
#define     bit16_0(data,num)               bit16_clr(data,num)                     //��data�ĵ�num(0~15)λ����
#define     bit32_1(data,num)               bit32_set(data,num)                     //��data�ĵ�num(0~31)λ��λ
#define     bit32_0(data,num)               bit32_clr(data,num)                     //��data�ĵ�num(0~31)λ����
#define     bit64_1(data,num)               bit64_set(data,num)                     //��data�ĵ�num(0~63)λ��λ
#define     bit64_0(data,num)               bit64_clr(data,num)                     //��data�ĵ�num(0~63)λ����
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//λ�ж� 1
#define     BIT_X_IS(all_bits,data,num)         bit##all_bits##_is(data,num)        //�ж�data�ĵ�num(��0��ʼ)λ�Ƿ�Ϊ1,all_bitsָdata��λ��(8,16,32,64)flag(0-����,1-��λ)
#define     BIT_M_IS(         data,num)         bit32_is(data,num)
//λ����
#define     BIT_X_OPT(all_bits,flag,data,num)   bit##all_bits##_##flag(data,num)    //��data�ĵ�num(��0��ʼ)λ�������λ,all_bitsָdata��λ��(8,16,32,64)flag(0-����,1-��λ)
#define     BIT_M_OPT(        flag,data,num)    bit32_##flag(data,num)              //��data�ĵ�num(��0��ʼ)λ�������λ,data��λ��32,flag(0-����,1-��λ)
//λֵ
#define     BIT_X_VAL(all_bits,num)             val##all_bits##_set(num)
#define     BIT_M_VAL(        num)              val32_set(num)
//================================================================================
//                      ��ֵ�ж�
//--------------------------------------------------------------------------------
#define    is_digital(a)  (((a)>=0)&&((a)<=9))
#define    is_digital_str(a)    (((a)>='0')&&((a)<='9'))

//================================================================================
//                      �ֽڴ���
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
//                      ��ȡ�ַ�ֵ����
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
//                  �ֽڻ�ȡ
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
//�����
#define     GET_LIST_NUM(L)                 (sizeof(L)/sizeof(L[0]))                //��ȡ�б��Ա����








extern void  loginfo ( const char *fmt, ... );
extern void  logbuf ( unsigned char *buf, int buflen, char *fmt , ... );



//--------------------------------------------------------------------------------
//��ͨ��ӡ
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



//�澯��ӡ
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




//����
#if 1
#define 	DBG_TEST(fmt, args...)			        loginfo(fmt, ## args)
#else
#define 	DBG_TEST(fmt, args...)
#endif


//--------------------------------------------------------------------------------
//��ӡ:�ļ�\����\��
#if 1
#define 	DBG_FILE_INFO()				            loginfo("\n[File: %s Line: %d] Function: %s\n",__FILE__,__LINE__,__FUNCTION__)
#else
#define 	DBG_FILE_INFO()
#endif



//����
#if 1
#define 	DBG_ERROR_FILE_INFO()	                loginfo("\nerror:[File: %s Line: %d] Function: %s , ",__FILE__,__LINE__,__FUNCTION__)

#else
#define 	DBG_ERROR_FILE_INFO()
#endif




//����
#if 1
#define 	DBG_ERROR_MY(fmt, args...)	            perror ( "error info:\n" );DBG_ERROR_FILE_INFO();loginfo(fmt, ## args)
#else
#define 	DBG_ERROR_MY(fmt, args...)
#endif



//ϵͳ����
#if 1
#define 	DBG_SYS_ERROR(fmt, args...)	            perror ( "error info:\n" );DBG_ERROR_FILE_INFO();loginfo(fmt, ## args)
#else
#define 	DBG_SYS_ERROR(fmt, args...)
#endif


//���������ӡ���
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




//��ʾ�꺯��
#define     STOP_RUN(des)               DBG_STOP(des)
#define 	NEED_ADD_CODE()		        DBG_ALARM("need add code\n")
#define     DIS_HELP_NOT_SUPPORT()  	DBG_FILE_INFO();DBG_NORMAL( "Current version not support!\n" )
#define     DIS_ERR_NOT_SUPPORT()       DBG_ERROR_MY ( "not support !!! \n" );
#define     DIS_ERR_NEED_ADD_CODE()	    DBG_ERROR_MY("need to add code at here\n");





#if ARM_USE
//UCOS
#define 	ATT_BYTE_NO_ALIGNED
#define 	ATT_BYTE_NO_ALIGNED_SUPPORT_pragma_pack(n)      1                   //��IAR֧�ֵĲ�����,���շֲ�

#else
//LINUX
//--------------------------------------------------------------------------------
//���Զ���
//�ֽڲ���������
#define 	ATT_BYTE_NO_ALIGNED   	    __attribute__((packed))                 //�����ֽ�Ϊ������,���շֲ�
#define 	ATT_BYTE_ALIGNED   	        __attribute__((aligned))                //�ֽڶ���(Ĭ��)
//������ʽ����
#define     ATT_FORMAT_DEF_1_2          __attribute__((format(printf,1,2)))    //������
#define     ATT_FORMAT_DEF_2_3          __attribute__((format(printf,2,3)))    //������
#define     ATT_FORMAT_DEF_3_4          __attribute__((format(printf,3,4)))    //������

#endif
//--------------------------------------------------------------------------------










//--------------------------------------------------------------------------------
//���ж�






//---------------------------------------------------------------------------------
/*
//�����������
12345678901234567890123456
abcdefghijklmnopqrstuvwxyz
ABCDEFGHIJKLMNOPQRSTUVWXYZ
11111111111111111111111111
llllllllllllllllllllllllll
LLLLLLLLLLLLLLLLLLLLLLLLLL
ZZZZZZZZZZZZZZZZZZZZZZZZZZ
��������������
��������������������������
*/

//---------------------------------------------------------------------------------
//                      �ṹ�嶨��
//---------------------------------------------------------------------------------


#if ARM_USE

#else

//���帡�����ṹ��
//������
typedef	union	_uniFloatCountN_
{
	float   f_num;      //������
	uint32  b4_dat;     // 4�ֽ�
	uint8   b_dat[4];   //�����ֽ�      ,b_dat[3]�����λΪ����λ,1-��ֵ,0-��ֵ
} ATT_BYTE_NO_ALIGNED
uniFloatCountN;


//����
typedef	union	_uniFloatCount_
{
	float   f_num;      //������
	uint32  b4_dat;     // 4�ֽ�
	uint8   b_dat[4];   //�����ֽ�      ,b_dat[3]�����λΪ����λ,1-��ֵ,0-��ֵ
} uniFloatCount;



//������
typedef	union	_uni4bCountN_
{
	uint32  b4_dat;
	int32   b4_dat_s;
	uint8   b_dat[4];
} ATT_BYTE_NO_ALIGNED
uni4bCountN;


//����
typedef	union	_uni4bCount_
{
	uint32  b4_dat;
	int32   b4_dat_s;
	uint8   b_dat[4];
} uni4bCount;


//������
typedef	union	_uni2bCountN_
{
	uint16  b2_dat;
	int16   b2_dat_s;
	uint8   b_dat[2];
} ATT_BYTE_NO_ALIGNED
uni2bCountN;


//����
typedef	union	_uni2bCount_
{
	uint16  b2_dat;
	int16   b2_dat_s;
	uint8   b_dat[2];
} uni2bCount;

#endif




#pragma   pack(1)
//======================================
//���������,����IAR��ok�ĳ���Ϊ5
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





