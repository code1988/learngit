/***************�ļ���Ϣ********************************************************************************
**��   ��   ��: uart_arm.h
**˵        ��:
**��   ��   ��: hxj
**��   �� ����: 2014-8-15 13:40
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

