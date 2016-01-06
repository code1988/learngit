/***************文件信息********************************************************************************
**文   件   名: in_comm_protocol.c
**说        明:
**创   建   人: hxj
**创   建 日期: 2014-12-24 14:42
*******************************************************************************************************/
#include "in_comm_protocol.h"
#include "def_config.h"






/****************************************************************************************************
**名称:void recv_data_to_cyc_buf_n ( strrecCycBufN *recCycBufN, uint8 *buf, uint32 len )
**功能:读数据到环型缓冲中
* 入口:recCycBufN-缓冲指针,buf-数据首指针,len-长度
* 出口:无
**auth:hxj, date: 2014-12-24 15:35
*****************************************************************************************************/
void recv_data_to_cyc_buf_n ( strRecvCycBuf_N *recCycBufN, uint8 *buf, uint32 len )
{
	//优化的
	strRecvCycBuf_N *recCycBuf;
	uint32 free_len;
	uint32 need_write_len;
	uint32 after_write_len;
	uint32 before_write_len;
	uint32 temp_read_point;
	uint32 temp_write_point;
	uint32 temp_4b;
	uint8 *temp_dst_point;
	uint8 *temp_src_point;


	if ( NULL == recCycBufN ) return;
	if ( ( NULL == buf ) || ( 0 == len ) ) return;

	recCycBuf = recCycBufN;

#if DISPLAY_READ_DATA_TO_CYCBUF_ENABLE
    logbuf ( buf, len, "in tcp_recv_data_to_tcp_buf 1:" );
#endif

	if ( 1 == recCycBuf->full_flag )
    {
        #if DISPLAY_RECV_CYC_BUF_FULL_EN
        DBG_NORMAL("in tcp_recv_data_to_tcp_buf 2:full_flag=1\n");
        #endif
        return;
    }

	temp_read_point = recCycBuf->read_point;
	temp_write_point = recCycBuf->write_point;

	if ( temp_write_point == temp_read_point )
	{
		free_len = recCycBuf->rec_buf_size;
	}
	else if ( temp_write_point > temp_read_point )
	{
		free_len = recCycBuf->rec_buf_size + temp_read_point - temp_write_point;
	}
	else
	{
		free_len = temp_read_point - temp_write_point;
	}

	if ( len > free_len )
	{
		need_write_len = free_len;
	}
	else
	{
		need_write_len = len;
	}
	temp_4b = temp_write_point + need_write_len;
	temp_dst_point = &recCycBuf->rec_buf[temp_write_point];
	temp_src_point = &buf[0];
	if ( temp_4b <= recCycBuf->rec_buf_size - 1 )
	{
        memcpy(temp_dst_point,temp_src_point,need_write_len);
		recCycBuf->write_point += need_write_len;
	}
	else
	{
		after_write_len = recCycBuf->rec_buf_size - temp_write_point;
		before_write_len = need_write_len - after_write_len;
        memcpy(temp_dst_point,temp_src_point,after_write_len);
		temp_dst_point = &recCycBuf->rec_buf[0];
		temp_src_point = &buf[after_write_len];
        memcpy(temp_dst_point,temp_src_point,before_write_len);
		recCycBuf->write_point = before_write_len;

	}

	if ( recCycBuf->write_point == recCycBuf->read_point )  recCycBuf->full_flag = 1;


}




int get_index_empty_frame_by_frameN(strOneFrameData_N *frameData,int frame_size)
{
	int i;
	int index=frame_size;

	if(NULL==frameData)	 	return frame_size;
	if(NULL==frame_size)	return frame_size;

	for(i=0;i<frame_size;++i)
	{
		if(0==frameData->frame_flag)
		{
            index=i;
            break;
        }
	}

    return index;

}



uint8 calc_fota_frame_sum(uint8 *buf,int len)
{
    uint8 sum=0;
    int i;
    if( (NULL==buf) || (len<=0) ) return sum;

    for(i=0;i<len;++i)
    {
        sum+=buf[i];
    }

    return sum;

}



uint32 get_prot_fota_head(void)
{
    //倒序
    return PARA_FOTA_FRAME_HEAD;
}



void init_cycbuf_n(strRecvCycBuf_N *recCycBuf)
{
    if(NULL==recCycBuf) return;

    recCycBuf->write_point = 0;
    recCycBuf->read_point = 0;
    recCycBuf->pre_read_point = 0;
    recCycBuf->full_flag = 0;

    recCycBuf->frameData.frame_flag=0;
    recCycBuf->frameData.frame_point=0;
    recCycBuf->frameData.frame_len=0;

}


strRecvCycBuf_N *malloc_cycbuf_n(uint32 cyc_buf_size,uint32 one_frame_size)
{
    strRecvCycBuf_N *recCycBuf=NULL;


    recCycBuf=malloc(sizeof(strRecvCycBuf_N));
    if(NULL==recCycBuf)
    {
        DBG_MAL("malloc recCycBuf err\r\n");
        goto malloc_err;
    }
    else
    {
        memset((char *)recCycBuf,0,sizeof(strRecvCycBuf_N));
    }

    recCycBuf->rec_buf_size=cyc_buf_size;
    recCycBuf->rec_buf=malloc(recCycBuf->rec_buf_size);
    if(NULL==recCycBuf->rec_buf)
    {
        DBG_MAL("malloc rec_buf err\r\n");
        goto malloc_err;
    }
    else
    {
        memset((char *)recCycBuf->rec_buf,0,recCycBuf->rec_buf_size);
    }

    recCycBuf->frameData.frame_data_size=one_frame_size;
    recCycBuf->frameData.frame_data=malloc(recCycBuf->frameData.frame_data_size);

    if(NULL==recCycBuf->frameData.frame_data)
    {
        DBG_MAL("malloc frame_data err\r\n");
        goto malloc_err;
    }
    else
    {
        memset((char *)recCycBuf->frameData.frame_data,0,recCycBuf->frameData.frame_data_size);
    }

    init_cycbuf_n(recCycBuf);
    return recCycBuf;
malloc_err:

    STOP_RUN("malloc err\r\n");


}






/****************************************************************************************************
**名称:void find_frame_by_prot_fota ( strRecvCycBuf_N *recCycBufN ,uint8 index_rec_frame)
**功能:从数据缓冲中查找帧
* 入口:recCycBufN-缓冲指针
* 出口:无
**auth:hxj, date: 2015-3-2 14:41
*****************************************************************************************************/
void find_frame_by_prot_fota ( strRecvCycBuf_N *recCycBufN ,uint8 index_rec_frame)
{
	strRecvCycBuf_N *recCycBuf=recCycBufN;
	uint8 temp_receive;
	uint32  temp_counter = 0;
	uint32  end_flag = 0;
    uint32 msg_length=0;
    strRecvFotaFrame *p_buf_c=NULL;
    uint8 sum;
    static int rec_frame_data_flag[4]={0};

    if(index_rec_frame>=GET_LIST_NUM(rec_frame_data_flag)) return;

	if ( NULL == recCycBuf ) 	return;
    if (( NULL == recCycBuf->rec_buf) || (0==recCycBuf->rec_buf_size))   return;
    if (( NULL == recCycBuf->frameData.frame_data) || (0==recCycBuf->frameData.frame_data_size))   return;

	if ( 0 == recCycBuf->frameData.frame_flag )
	{
        recCycBuf->pre_read_point=recCycBuf->read_point;

		while ( ( recCycBuf->pre_read_point != recCycBuf->write_point ) || ( 1 == recCycBuf->full_flag ) )
		{
			temp_receive = recCycBuf->rec_buf[recCycBuf->pre_read_point];
			temp_counter++;

            if(0==rec_frame_data_flag[index_rec_frame])
            {
    			switch ( recCycBuf->frameData.frame_point )
    			{
    				case 0:
    				{
    					if ( ((get_prot_fota_head()>>0) & 0xff) == temp_receive )
    					{
    						recCycBuf->frameData.frame_data[recCycBuf->frameData.frame_point] = temp_receive;
    						recCycBuf->frameData.frame_point++;
                            recCycBuf->frameData.frame_len=0;
    					}
    				}
    				break;
    				case 1:
    				{
    					if ( ((get_prot_fota_head()>>8) & 0xff) == temp_receive )
    					{
    						recCycBuf->frameData.frame_data[recCycBuf->frameData.frame_point] = temp_receive;
    						recCycBuf->frameData.frame_point++;
    					}
                        else
                        {
							recCycBuf->pre_read_point=recCycBuf->read_point;
                            recCycBuf->read_point=(recCycBuf->read_point+1)%recCycBuf->rec_buf_size;
                            recCycBuf->frameData.frame_point=0;
                        }
    				}
                    break;
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
    				{
						recCycBuf->frameData.frame_data[recCycBuf->frameData.frame_point] = temp_receive;
						recCycBuf->frameData.frame_point++;
    				}
                    break;
    				case 16:
    				{
						recCycBuf->frameData.frame_data[recCycBuf->frameData.frame_point] = temp_receive;
						recCycBuf->frameData.frame_point++;

                        //判断返回帧长度是否正确
                        p_buf_c=(strRecvFotaFrame *)recCycBuf->frameData.frame_data;
                        msg_length=sizeof(strRecvFotaFrameHead)+p_buf_c->head_data.h_data_len+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG;
                        sum=calc_fota_frame_sum((uint8 *)p_buf_c,sizeof(strRecvFotaFrameHead)-FOTA_LEN_HEAD_SUM);
                        if(sum != p_buf_c->head_data.h_sum)
                        {
                            DBG_PRO("head sum err,calc sum=%d,recv_sum=%d \r\n",sum,p_buf_c->head_data.h_sum);
							recCycBuf->pre_read_point=recCycBuf->read_point;
							recCycBuf->read_point=(recCycBuf->read_point+1)%recCycBuf->rec_buf_size;
							recCycBuf->frameData.frame_point=0;
                        }
                        else
                        {
                            recCycBuf->frameData.frame_len=msg_length;
#if 0 /*hxj amend,date 2015-1-16 12:58*/
                            DBG_PRO("msg_length=%d,s=%d,h_l=%d,frame_data_size=%d\r\n",msg_length,sizeof(strRecvFotaFrameHead),p_buf_c->head_data.h_data_len,recCycBuf->frameData.frame_data_size);
#endif
                        }

    				}
                    break;
    				default:
    				{
    					if ( recCycBuf->frameData.frame_point >= recCycBuf->frameData.frame_data_size )
    					{
							recCycBuf->pre_read_point=recCycBuf->read_point;
							recCycBuf->read_point=(recCycBuf->read_point+1)%recCycBuf->rec_buf_size;
							recCycBuf->frameData.frame_point=0;
    					}
    					else
    					{
    						recCycBuf->frameData.frame_data[recCycBuf->frameData.frame_point] = temp_receive;
    						recCycBuf->frameData.frame_point++;
							rec_frame_data_flag[index_rec_frame]=1;

    					}
    				}
    				break;

    			}
            }
            else
            {
				if ( recCycBuf->frameData.frame_point >= recCycBuf->frameData.frame_data_size )
				{
                    DBG_PRO("recv frame err\r\n");
					recCycBuf->pre_read_point=recCycBuf->read_point;
					recCycBuf->read_point=(recCycBuf->read_point+1)%recCycBuf->rec_buf_size;
					recCycBuf->frameData.frame_point = 0;
					rec_frame_data_flag[index_rec_frame]=0;
				}
				else
				{
					recCycBuf->frameData.frame_data[recCycBuf->frameData.frame_point] = temp_receive;
					recCycBuf->frameData.frame_point++;
					if(recCycBuf->frameData.frame_point>=recCycBuf->frameData.frame_len)
					{
						//接收完一帧数据
						rec_frame_data_flag[index_rec_frame]=0;
						end_flag=1;
					}
				}
            }

			if ( 1 == end_flag )
			{
                end_flag=0;
				recCycBuf->frameData.frame_len = recCycBuf->frameData.frame_point;
                recCycBuf->frameData.frame_flag = 1;
                recCycBuf->frameData.frame_point = 0;
                //接收完一帧数据
                //DBG_PRO("recv one frame\r\n");
			}

			//读指针到尾时,则指向头
			recCycBuf->pre_read_point=(recCycBuf->pre_read_point+1)%recCycBuf->rec_buf_size;
			if ( ( recCycBuf->pre_read_point == recCycBuf->write_point ) || ( 1 == recCycBuf->frameData.frame_flag ) || ( temp_counter > recCycBuf->rec_buf_size ) )	break;	//读完一串数据或者读完接收环形缓冲区,则退出
		}

        recCycBuf->read_point=recCycBuf->pre_read_point;
		if ( 1 == recCycBuf->full_flag )
		{
			recCycBuf->write_point = 0;
			recCycBuf->read_point = 0;
            recCycBuf->pre_read_point = 0;
			recCycBuf->full_flag = 0;
            DBG_PRO("full_flag=1 \r\n");
		}

	}

}


void add_frame_fota_all_sum_and_end(strOneFrameData_N *p_frameData)
{
    if(NULL == p_frameData) return;

    strRecvFotaFrame *p_buf_c=(strRecvFotaFrame *)p_frameData->frame_data;
    p_frameData->frame_len=sizeof(strRecvFotaFrameHead)+p_buf_c->head_data.h_data_len+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG;

    p_buf_c->head_data.h_sum=calc_fota_frame_sum((uint8 *)p_buf_c,sizeof(strRecvFotaFrameHead)-FOTA_LEN_HEAD_SUM);
    uint8 sum=calc_fota_frame_sum((uint8 *)p_buf_c,p_frameData->frame_len-FOTA_LEN_END_SUM-FOTA_LEN_END_FLAG);
    SET_1B_OFF_VAL(p_buf_c,p_frameData->frame_len-FOTA_LEN_END_SUM-FOTA_LEN_END_FLAG,sum,1);
    SET_1B_OFF_VAL(p_buf_c,p_frameData->frame_len-FOTA_LEN_END_FLAG,PARA_FOTA_FRAME_END,1);
}



void create_ack_frame_by_err_ack(strOneFrameData_N *recv_frameData,strOneFrameData_N *ret_frameData,uint8 ack)
{
    strRecvFotaFrame *p_buf_c=NULL;

    if(NULL==recv_frameData) return;
    if(NULL==ret_frameData)  return;

    memcpy(ret_frameData->frame_data,recv_frameData->frame_data,recv_frameData->frame_len);
    ret_frameData->frame_len=recv_frameData->frame_len;

    p_buf_c=(strRecvFotaFrame *)ret_frameData->frame_data;
    p_buf_c->head_data.h_fun=(p_buf_c->head_data.h_fun & PARA_FUN_MASK) | PARA_DSP_TO_ARM;
    p_buf_c->head_data.h_ack=ack;

    if(recv_frameData->frame_len > (sizeof(strRecvFotaFrameHead)+0+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG) )
    {
        p_buf_c->head_data.h_data_len=FOTA_LEN_DTAT_ID;
    }
    else
    {
        p_buf_c->head_data.h_data_len=0;
    }

    add_frame_fota_all_sum_and_end(ret_frameData);

}







void create_ack_frame_by_read(strOneFrameData_N *recv_frameData,strOneFrameData_N *ret_frameData)
{
    strRecvFotaFrame *p_buf_c_send=NULL;
    strRecvFotaFrame *p_buf_c_recv=NULL;
    uint32      bak_h_data_len=0;

    if(NULL==recv_frameData) return;
    if(NULL==ret_frameData)  return;
    p_buf_c_recv=(strRecvFotaFrame *)recv_frameData->frame_data;
    p_buf_c_send=(strRecvFotaFrame *)ret_frameData->frame_data;

    bak_h_data_len=p_buf_c_send->head_data.h_data_len;
    memcpy(&p_buf_c_send->head_data,&p_buf_c_recv->head_data,sizeof(strRecvFotaFrameHead));
    p_buf_c_send->head_data.h_data_len=bak_h_data_len;
    p_buf_c_send->frame_id=p_buf_c_recv->frame_id;

    p_buf_c_send->head_data.h_fun=(p_buf_c_send->head_data.h_fun & PARA_FUN_MASK) | PARA_DSP_TO_ARM;
    p_buf_c_send->head_data.h_ack=ACK_OK;
    add_frame_fota_all_sum_and_end(ret_frameData);

}









void display_id_list(strCmdIdList *p_id_wr_list,char *des)
{
    if(NULL == p_id_wr_list) return;

    DBG_DIS("in display_id_list 0:================start================\r\n");
    if(NULL != des)
    {
        DBG_DIS("in display_id_list 1:%s:\r\n",des);
    }

    int i;
    for(i=0;NULL != p_id_wr_list[i].id_des;++i)
    {
        DBG_DIS("in display_id_list 2:id=0x%04X,id_len=%3d,id_wlen=%3d,check_len_en=%d,id_ret_len=%3d,check_ret_len_en=%d,des=%s\r\n",p_id_wr_list[i].id_data,p_id_wr_list[i].id_len,p_id_wr_list[i].id_wlen,p_id_wr_list[i].check_len_en,p_id_wr_list[i].id_ret_len,p_id_wr_list[i].check_ret_len_en,p_id_wr_list[i].id_des);
    }
    DBG_DIS("in display_id_list 3:================  end================\r\n");

}





/****************************************************************************************************
**名称:int fota_frame_pre_manage(strOneFrameData_N *pframeData,strCmdIdList *p_tmp_id_wr_list,p_fun_send send_fun,uint16 self_h_index_min,uint16 self_h_index_max)
**功能:fota帧预处理
* 入口:无
* 出口:正常帧返回1,出错返回0或负数
**auth:hxj, date: 2014-12-29 10:33
*****************************************************************************************************/
int fota_frame_pre_manage(strOneFrameData_N *pframeData,strCmdIdList *p_tmp_id_wr_list,P_FUN_SEND send_fun,uint16 self_h_index_min,uint16 self_h_index_max)
{
    if(NULL == pframeData)          return -1;
    if(NULL == p_tmp_id_wr_list)    return -2;
    if(NULL == send_fun)            return -3;


    strRecvFotaFrame *p_buf_c=NULL;
    uint32 msg_length=0;
    uint32 check_msg_length=0;
    uint8 sum=0;
    int recv_data_len=0;
    int recv_self_send_frame_ack_flag=0;

    strOneFrameData_N tmp_one_frame;

    static uint8 *tmp_one_frame_frame_data=NULL;
    if(NULL==tmp_one_frame_frame_data)
    {
        tmp_one_frame_frame_data=(uint8 *)malloc(PARA_SIZE_DSP_SEND_ONE_FRAME);
        if(NULL==tmp_one_frame_frame_data)
        {
            DBG_ALARM("malloc err\r\n");
            return 0;
        }
    }

    tmp_one_frame.frame_data_size=PARA_SIZE_DSP_SEND_ONE_FRAME;
    tmp_one_frame.frame_data=tmp_one_frame_frame_data;

    p_buf_c=(strRecvFotaFrame *)pframeData->frame_data;
    msg_length=pframeData->frame_len;
    recv_data_len=msg_length-(sizeof(strRecvFotaFrameHead)+FOTA_LEN_DTAT_ID+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG);


    if(1==pframeData->frame_flag)
    {
#if 0 /*hxj amend,date 2015-1-13 15:22*/
        if( ID_DSP_BIG_IMG_DATA != p_buf_c->frame_id )
#else
        if(1)
#endif
        {
            sum=calc_fota_frame_sum((uint8 *)p_buf_c,msg_length-2);
            if(sum != GET_1B_OFF_VAL(p_buf_c, msg_length-2))
            {
                DBG_PRO("in fota_frame_pre_manage 1:all sum err,calc sum=%d,recv sum=%d,id=0x%04x,msg_length=%d,recv_data_len=%d\r\n",sum,GET_1B_OFF_VAL(p_buf_c, msg_length-2),p_buf_c->frame_id,msg_length,recv_data_len);
                DBG_BUF_PRO((uint8 *)p_buf_c,24,"head buf");
                DBG_BUF_PRO((uint8 *)p_buf_c+msg_length-2-10,2+10,"end buf");
                create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_ALL_SUM);
                send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                goto ret_send;
            }
        }

        recv_self_send_frame_ack_flag=0;
        if((p_buf_c->head_data.h_index >= self_h_index_min)&&(p_buf_c->head_data.h_index<= self_h_index_max))
        {
            recv_self_send_frame_ack_flag=1;
            if(ACK_OK != p_buf_c->head_data.h_ack)
            {
                DBG_PRO("in fota_frame_pre_manage 2:recv ack frame have err(self send frame)\r\n");
                DBG_BUF_PRO((uint8 *)p_buf_c,msg_length,"in fota_frame_pre_manage 2_1:err frame:");
                goto ret_send;
            }
        }
        else
        {

            if( ( ACK_NEED_RET != p_buf_c->head_data.h_ack)
                && (ACK_NOT_RET != p_buf_c->head_data.h_ack))
            {
                DBG_PRO("in fota_frame_pre_manage 3:ack err,recv ack=0x%02x\r\n",p_buf_c->head_data.h_ack);
                DBG_BUF_PRO((uint8 *)p_buf_c,msg_length,"in fota_frame_pre_manage 3_1:err frame:");
                create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_ACK);
                send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                goto ret_send;

            }
        }


        if(PARA_FOTA_FRAME_END != GET_1B_OFF_VAL(p_buf_c, msg_length-1))
        {
            DBG_PRO("in fota_frame_pre_manage 4:end flag err,recv end flag=0x%02x\r\n",GET_1B_OFF_VAL(p_buf_c, msg_length-1));
            DBG_BUF_PRO((uint8 *)p_buf_c,msg_length,"in fota_frame_pre_manage 4_1:err frame:");
            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_END_FLAG);
            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
            goto ret_send;
        }

        if(PARA_FCODE_NET != p_buf_c->head_data.h_code)
        {
            DBG_PRO("in fota_frame_pre_manage 5:FCODE err,h_code=%d\r\n",p_buf_c->head_data.h_code);
            DBG_BUF_PRO((uint8 *)p_buf_c,msg_length,"in fota_frame_pre_manage 5_1:err frame:");
            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_FCODE);
            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
            goto ret_send;
        }

        if( PARA_DSP_TO_ARM != (p_buf_c->head_data.h_fun & PARA_DIR_MASK) )
        {
            DBG_PRO("in fota_frame_pre_manage 6:dir err,h_code=%d\r\n",(p_buf_c->head_data.h_fun & PARA_DIR_MASK));
            DBG_BUF_PRO((uint8 *)p_buf_c,msg_length,"in fota_frame_pre_manage 6_1:err frame:");
            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_DIR);
            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
            goto ret_send;

        }

        uint8 fun_code=0;
        fun_code=p_buf_c->head_data.h_fun & PARA_FUN_MASK;
        if(PARA_FUN_RD == fun_code)
        {
            ;
        }
        else if(PARA_FUN_WT== fun_code)
        {
            ;
        }
        else if(PARA_FUN_CH== fun_code)
        {
            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_OK);
            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
            goto ret_send;
        }
        else
        {
            DBG_PRO("in fota_frame_pre_manage 7:fun code err,h_code=%d\r\n",fun_code);
            DBG_BUF_PRO((uint8 *)p_buf_c,msg_length,"in fota_frame_pre_manage 7_1:err frame:");
            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_FUN_CODE);
            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
            goto ret_send;
        }


#if 0 /*hxj amend,date 2014-12-25 16:19*/
        //test
        static uint16 recv_index_bef=0;
        uint16 recv_index;
        uint16 index_off=0;

        recv_index=p_buf_c->head_data.h_index;

        if(recv_index > recv_index_bef)
        {
            index_off=recv_index-recv_index_bef;
        }
        else
        {
            index_off=recv_index_bef-recv_index;
        }

        if(index_off >1 )
        {
            DBG_PRO("1_index_off=%d,recv_index_bef=%d,recv_index=%d\r\n",index_off,recv_index_bef,recv_index);
        }
        else
        {
            //DBG_PRO("2_index_off=%d,recv_index_bef=%d,recv_index=%d\r\n",index_off,recv_index_bef,recv_index);
        }


        if(0==recv_index)
        {
            DBG_PRO("3_index_off=%d,recv_index_bef=%d,recv_index=%d\r\n",index_off,recv_index_bef,recv_index);
        }

        if( 0== (recv_index%100) )
        {
            DBG_PRO("4_index_off=%d,recv_index_bef=%d,recv_index=%d\r\n",index_off,recv_index_bef,recv_index);
        }

        recv_index_bef=recv_index;

        pframeData->frame_flag=0;

        return;

#endif

        int i;
        for(i=0;NULL != p_tmp_id_wr_list[i].id_des;++i)
        {
            if( p_buf_c->frame_id == p_tmp_id_wr_list[i].id_data )
            {

                if(0==recv_self_send_frame_ack_flag)
                {
                    //对方的读写判断
                    if(CK_ON==p_tmp_id_wr_list[i].check_len_en)
                    {
                        if(PARA_FUN_RD == fun_code)
                        {
                            check_msg_length=sizeof(strRecvFotaFrameHead)+(FOTA_LEN_DTAT_ID+p_tmp_id_wr_list[i].id_len)+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG;
                        }
                        else
                        {
                            check_msg_length=sizeof(strRecvFotaFrameHead)+(FOTA_LEN_DTAT_ID+p_tmp_id_wr_list[i].id_wlen)+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG;
                        }

                        if(check_msg_length != msg_length)
                        {
                            if(PARA_FUN_RD == fun_code)
                            {
                                DBG_PRO("in fota_frame_pre_manage 8_0:read id data len err,id=0x%04x,ok_len=%d,recv_data_len=%d\r\n",p_tmp_id_wr_list[i].id_data,p_tmp_id_wr_list[i].id_len,recv_data_len);
                            }
                            else
                            {
                                DBG_PRO("in fota_frame_pre_manage 8_1:write id data len err,id=0x%04x,ok_len=%d,recv_data_len=%d\r\n",p_tmp_id_wr_list[i].id_data,p_tmp_id_wr_list[i].id_len,recv_data_len);
                            }
                            DBG_BUF_PRO((uint8 *)p_buf_c, msg_length,"in fota_frame_pre_manage 8_2:");
                            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_ID_DATA);
                            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                            goto ret_send;
                        }
                    }

                    if(PARA_FUN_RD == fun_code)
                    {
                        if(ACK_NEED_RET != p_buf_c->head_data.h_ack)
                        {
                            DBG_PRO("in fota_frame_pre_manage 9_0:read ack err,recv ack=0x%02x,id=0x%04x\r\n",p_buf_c->head_data.h_ack,p_tmp_id_wr_list[i].id_data);
                            DBG_BUF_PRO((uint8 *)p_buf_c, msg_length,"in fota_frame_pre_manage 9_1:");
                            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_ACK);
                            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                            goto ret_send;
                        }

                        if(NULL ==  p_tmp_id_wr_list[i].get_param)
                        {
                            DBG_PRO("in fota_frame_pre_manage 9_2:not id support ,id(0x%04X) \r\n",p_buf_c->frame_id);
                            DBG_BUF_PRO((uint8 *)p_buf_c,msg_length,"in fota_frame_pre_manage 9_3: frame:");
                            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_ID_OPT);
                            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                            goto ret_send;
                        }
                    }
                    else if(PARA_FUN_WT== fun_code)
                    {
                        if(NULL == p_tmp_id_wr_list[i].set_param )
                        {
                            DBG_PRO("in fota_frame_pre_manage 10:not id support ,id(0x%04X) \r\n",p_buf_c->frame_id);
                            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_ID_OPT);
                            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                            goto ret_send;
                        }
                    }

                }
                else
                {
                    //自己的读写返回帧处理
                    if( (CK_ON==p_tmp_id_wr_list[i].check_read_ret_len_en)&&(PARA_FUN_RD == fun_code))
                    {
                        check_msg_length=sizeof(strRecvFotaFrameHead)+(FOTA_LEN_DTAT_ID+p_tmp_id_wr_list[i].id_read_ret_len)+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG;
                        if(check_msg_length != msg_length)
                        {
                            DBG_PRO("in fota_frame_pre_manage 11_0:read id data len err,id=0x%04x,ok_len=%d,recv_data_len=%d\r\n",p_tmp_id_wr_list[i].id_data,p_tmp_id_wr_list[i].id_len,recv_data_len);
                            DBG_BUF_PRO((uint8 *)p_buf_c, msg_length,"in fota_frame_pre_manage 11_2:");
                            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_ID_DATA);
                            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                            goto ret_send;
                        }
                    }
                    else if((CK_ON==p_tmp_id_wr_list[i].check_write_ret_len_en)&&(PARA_FUN_WT == fun_code))
                    {
                        check_msg_length=sizeof(strRecvFotaFrameHead)+(FOTA_LEN_DTAT_ID+p_tmp_id_wr_list[i].id_write_ret_len)+FOTA_LEN_END_SUM+FOTA_LEN_END_FLAG;
                        if(check_msg_length != msg_length)
                        {
                            DBG_PRO("in fota_frame_pre_manage 12_1:write id data len err,id=0x%04x,ok_len=%d,recv_data_len=%d\r\n",p_tmp_id_wr_list[i].id_data,p_tmp_id_wr_list[i].id_len,recv_data_len);
                            DBG_BUF_PRO((uint8 *)p_buf_c, msg_length,"in fota_frame_pre_manage 12_2:");
                            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_ID_DATA);
                            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
                            goto ret_send;
                        }
                    }

                    if(PARA_FUN_RD== fun_code)
                    {
                        if(NULL == p_tmp_id_wr_list[i].self_get_param )
                        {
                            DBG_PRO("in fota_frame_pre_manage 13:not read return manage fun,id(0x%04X) \r\n",p_buf_c->frame_id);
                            goto ret_send;
                        }
                    }
                    else if(PARA_FUN_WT== fun_code)
                    {
                        if(NULL == p_tmp_id_wr_list[i].self_set_param )
                        {
                            DBG_PRO("in fota_frame_pre_manage 14:not write return manage fun,id(0x%04X) \r\n",p_buf_c->frame_id);
                            goto ret_send;
                        }
                    }

                }

                break;
            }

        }


        if(NULL == p_tmp_id_wr_list[i].id_des)
        {
            //not def id
            DBG_PRO("in fota_frame_pre_manage 40:not def id(0x%04X) \r\n",p_buf_c->frame_id);
            create_ack_frame_by_err_ack(pframeData,&tmp_one_frame,ACK_ERR_INVALID_ID);
            send_fun(tmp_one_frame.frame_data,tmp_one_frame.frame_len);
            goto ret_send;

        }

    }

    return 1;

ret_send:

    return 0;


}









/*******************************************************************************************************
**                            End Of File
*******************************************************************************************************/

