/***************文件信息********************************************************************************
**文   件   名: arm_com.c
**说        明:
**创   建   人: hxj
**创   建 日期: 2014-12-26 16:41
*******************************************************************************************************/
#include "def_config.h"
#include "bsp.h"
#include "arm_com.h"
#include "app_cfg.h"
#include "in_comm_protocol.h"
#include "dsp_net.h"



strRecvCycBuf_N *g_p_arm_uart_recv_cbuf=NULL;
strRecvCycBuf_N *g_p_arm_uart_send_cbuf=NULL;
OS_EVENT *g_p_arm_uart_send_Mutex=NULL;     // 串口发送互斥体




strCmdIdList arm_uart_id_wr_list[] =
{
    //抠图序号(2B)+抠图(180B)+面值(1B)+套别(1B)+真假(1B)

    //test id
 //   {INIT_DATA_ID(0x9999,0,15,0,0,0,NULL,NULL)},         //0x9999          //test

    //end flag
    {INIT_END_DATA_ID()},

};






void init_uart_data(void)
{
    g_p_arm_uart_recv_cbuf=malloc_cycbuf_n(PARA_SIZE_ARM_UART_RECV_CBUF,PARA_SIZE_ARM_UART_RECV_ONE_FRAME);
    if(NULL == g_p_arm_uart_recv_cbuf)
    {
        DBG_SYS_ERROR("in init_uart_data 1_1:malloc g_p_arm_uart_recv_cbuf err\r\n");
    }
    else
    {
        DBG_PRO("in init_uart_data 1_2:malloc init_uart_data ok\r\n");
    }


    g_p_arm_uart_send_cbuf=malloc_cycbuf_n(PARA_SIZE_ARM_UART_SEND_CBUF,PARA_SIZE_ARM_UART_SEND_ONE_FRAME);
    if(NULL == g_p_arm_uart_send_cbuf)
    {
        DBG_SYS_ERROR("in init_uart_data 2_1:malloc g_p_arm_uart_send_cbuf err\r\n");
    }
    else
    {
        DBG_PRO("in init_uart_data 2_2:malloc arm_send_uart_cbuf ok\r\n");
    }

    uint8  perr;
    g_p_arm_uart_send_Mutex=OSMutexCreate(PRI_ARM_UART_SEND,&perr);
    if(NULL == g_p_arm_uart_send_Mutex)
    {
        DBG_SYS_ERROR("in init_uart_data 3_2:OSMutexCreate g_p_arm_uart_send_Mutex err,perr=%d \r\n",perr);
    }

}




void arm_uart_recv_data_to_cyc_buf_n (uint8 *buf, uint32 len )
{
    recv_data_to_cyc_buf_n (g_p_arm_uart_recv_cbuf,buf,len);
}



void arm_uart_find_frame(void)
{
    if(NULL == g_p_arm_uart_recv_cbuf) return;
    find_frame_by_prot_fota ( g_p_arm_uart_recv_cbuf,0);
}


uint16 get_index_arm_uart_frame_num(void)
{
    static uint16 index=0x7fff;

    index++;
    if(index<0x8000) index=0x8000;
    return index;
}


int arm_uart_send_data(uint8 *buf,uint32 len)
{
    if(NULL== buf) return -1;
    if(0== len)    return 0;
    uint8  perr;


    if(NULL != g_p_arm_uart_send_Mutex)
    {
        OSMutexPend(g_p_arm_uart_send_Mutex,0,&perr);                  // 获取互斥锁
    }

    recv_data_to_cyc_buf_n (g_p_arm_uart_send_cbuf,buf,len);
    if(1==g_p_arm_uart_send_cbuf->frameData.frame_flag)
    {
        uart_tx_int_enable(UART2);
        g_p_arm_uart_send_cbuf->frameData.frame_flag=0;
    }

    if(NULL != g_p_arm_uart_send_Mutex)
    {
        OSMutexPost(g_p_arm_uart_send_Mutex);                          // 释放互斥锁
    }


    return NULL;


}



void arm_uart_frame_manage(void)
{
    if(NULL == g_p_arm_uart_recv_cbuf) return;

    strOneFrameData_N *pframeData=&g_p_arm_uart_recv_cbuf->frameData;
    strCmdIdList *p_tmp_id_wr_list=arm_uart_id_wr_list;
    P_FUN_SEND send_fun=arm_uart_send_data;
    strRecvFotaFrame *p_buf_c=NULL;
    int ret_pre=0;
    int i;
    int call_ret;
    uint8 fun_code=0;


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
    //uint32 msg_length=pframeData->frame_len;

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
                    if(PARA_FUN_RD == fun_code)
                    {
                        if(NULL !=  p_tmp_id_wr_list[i].get_param)
                        {
                            call_ret=p_tmp_id_wr_list[i].get_param(pframeData,&tmp_one_frame,p_buf_c->frame_id,p_tmp_id_wr_list[i].id_ret_len);
                            if(0==call_ret)
                            {
                                if(ACK_NOT_RET != p_buf_c->head_data.h_ack)
                                {
                                    create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_OK);
                                    send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                                }
                                goto ret_send;
                            }
                            else if(1==call_ret)
                            {
                                //not return ack
                            }
                            else
                            {
                                DBG_PRO("in dsp_net_frame_manage 2:get id(0x%04X),call_ret=%d \r\n",p_buf_c->frame_id,call_ret);
                            }
                        }
                    }
                    else if(PARA_FUN_WT== fun_code)
                    {
                        if(NULL != p_tmp_id_wr_list[i].set_param )
                        {
                            call_ret=p_tmp_id_wr_list[i].set_param(pframeData,&tmp_one_frame,p_buf_c->frame_id,p_tmp_id_wr_list[i].id_wlen);
                            if(0==call_ret)
                            {
                                if(ACK_NOT_RET != p_buf_c->head_data.h_ack)
                                {
                                    create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_OK);
                                    send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                                }
                                goto ret_send;
                            }
                            else if(1==call_ret)
                            {
                                //not return ack
                            }
                            else
                            {
                                DBG_PRO("in dsp_net_frame_manage 3:set id(0x%04X),call_ret=%d \r\n",p_buf_c->frame_id,call_ret);
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

ret_send:

    pframeData->frame_flag=0;

    return;



}









/****************************************************************************************************
**名称:void task_uart_server(void *pdata)
**功能:线程任务
* 入口:无
* 出口:无
**auth:hxj, date: 2014-12-29 11:29
*****************************************************************************************************/
void task_uart_server(void *pdata)
{

    init_uart_data();
    display_id_list(arm_uart_id_wr_list,"arm_uart_id_wr_list");

    //init uart
	_BSPUART_CONFIG UART_InitStructure;                         // 该变量名做了修改，因为跟Bsp_UART.c文件中变量同名，影响阅读判断
	void *uartOSQ[4];					                        // 消息队列
	OS_EVENT *uartEvent;				                        // 使用的事件

	uartEvent = OSQCreate(uartOSQ,4);
	UART_InitStructure.Baudrate = 115200;		                // 波特率
	UART_InitStructure.Parity = BSPUART_PARITY_NO;			    // 校验位
	UART_InitStructure.StopBits = BSPUART_STOPBITS_1;		    // 停止位
	UART_InitStructure.WordLength = BSPUART_WORDLENGTH_8D;	    // 数据位数
	UART_InitStructure.Work = BSPUART_WORK_FULLDUPLEX;		    // 工作模式
	UART_InitStructure.TxRespond = BSPUART_RESPOND_INT;	        // 中断模式
	UART_InitStructure.pEvent = uartEvent;	                    // 消息事件
	UART_InitStructure.MaxTxBuffer = 0;				            // 发送缓冲容量
	UART_InitStructure.MaxRxBuffer = 0;				            // 接收缓冲容量
	UART_InitStructure.pTxBuffer = NULL;					    // 发送缓冲指针
	UART_InitStructure.pRxBuffer = NULL;					    // 接收缓冲指针
	UART_InitStructure.TxSpacetime = 0;							// 发送帧间隔
	UART_InitStructure.RxOvertime = 10;			    			// 接收帧间隔

	BSP_UARTConfig(UART2,&UART_InitStructure);


    for(;;)
    {
        MY_DELAY_X_MS(5);
        //处理数据
        arm_uart_find_frame();
        arm_uart_frame_manage();
    }


}









/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

