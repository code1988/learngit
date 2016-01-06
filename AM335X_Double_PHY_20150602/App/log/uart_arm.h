/***************文件信息********************************************************************************
**文   件   名: uart_arm.h
**说        明:
**创   建   人: hxj
**创   建 日期: 2014-8-15 13:40
*******************************************************************************************************/
#ifndef	__INC_UART_ARM_H
#define	__INC_UART_ARM_H

#include "def_config.h"





extern int  init_uart0(int rate);
extern void loginfo ( const char *fmt, ... );
extern void logbuf ( unsigned char *buf, int buflen, char *fmt , ... );
extern void logbuf_two ( unsigned char *buf, int buflen, char *des_buf);




#endif
/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

