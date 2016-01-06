/***************文件信息********************************************************************************
**文   件   名: arm_com.h
**说        明:
**创   建   人: hxj
**创   建 日期: 2014-12-26 16:41
*******************************************************************************************************/
#ifndef	__INC_ARM_COMTT_H
#define	__INC_ARM_COMTT_H

#include "def_config.h"
#include "in_comm_protocol.h"



//






extern strRecvCycBuf_N *g_p_arm_uart_recv_cbuf;
extern strRecvCycBuf_N *g_p_arm_uart_send_cbuf;



extern void task_uart_server(void *pdata);
extern void arm_uart_recv_data_to_cyc_buf_n (uint8 *buf, uint32 len );
extern int arm_uart_send_data(uint8 *buf,uint32 len);










#endif
/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

