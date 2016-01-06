/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: dsp_comm.c
* Author			: CODE
* Date First Issued	: 141030
* Version			: V
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
* History			:
*                     20141123 - 协议格式更新
                      20150212 - W5500改用中断触发方式 by code
                      20150305 - 添加超时功能
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "bsp.h"
#include "def_config.h"
#include "dsp_net.h"
#include "app.h"



/* Private define-----------------------------------------------------------------------------*/
#if DBG_DSP_RECV_TWO_EN
OS_EVENT *W5500AEvent;          // W5500A事件控制块
void *W5500OAOSQ[4];             // W5500A消息队列
#endif

OS_EVENT *W5500BEvent;          // W5500B事件控制块
void *W5500OBOSQ[4];             // W5500B消息队列

INT16U FramNum;                         // 帧序号
static INT8U S_Data_Buffer[2][4096];    // socket收发缓冲

INT32U TickVal;                         // 用于计时

uint8 dsp1_net_init_ok_flag=0;
uint8 dsp2_net_init_ok_flag=0;


/* Private functions--------------------------------------------------------------------------*/
#if DBG_DSP_RECV_TWO_EN
void TaskComWithDSP1(void *pdata)
{   
    // 屏幕相关变量定义
    static INT8U    NetFlag = 0;
    static          _BSP_MESSAGE NetMessage;
    
    // W5500相关变量定义
    INT8U               perr;
    _BSP_MESSAGE        *pW5500Msg;
    _BSPW5500_CONFIG    W5500_InitStructure;
    _BSPGPIO_CONFIG     GPIO_InitStructure;
    
	INT8U Detination_IP[4]	={192,168,11,131};					// 目的IP
	INT8U Source_IP[4]		={192,168,11,35};					// 源IP(服务器)
	INT8U Gateway_IP[4]		={192,168,11,1};					// 网关
	INT8U Subnet_Mask[4]	={255,255,255,0};					// 子网掩码
	INT8U MAC_Addr[6]		={0x48,0x53,0x00,0x57,0x55,0x00};	// MAC

    dsp_net_init_buf();

    // 中断线初始化
    GPIO_InitStructure.PortNum = PORT1;
    GPIO_InitStructure.PinNum = 20;
    GPIO_InitStructure.Dir = GPIO_DIR_I;
    GPIO_InitStructure.IntLine = GPIO_INTA;
    GPIO_InitStructure.IntType = GPIO_INT_TYPE_L;
    GPIO_InitStructure.pEvent = W5500AEvent;
    BSP_GPIOConfig(&GPIO_InitStructure);
    
    // W5500初始化
    memcpy(W5500_InitStructure.GW_IP,Gateway_IP,4);
	memcpy(W5500_InitStructure.MAC,MAC_Addr,6);
	memcpy(W5500_InitStructure.S_IP,Source_IP,4);
	memcpy(W5500_InitStructure.Sub_Mask,Subnet_Mask,4);
	memcpy(W5500_InitStructure.D_IP,Detination_IP,4);
	W5500_InitStructure.D_Port = DESPORT_NUM;
	W5500_InitStructure.S_Port = 7566;
	W5500_InitStructure.s_num = SOCKET0;

	DBG_PRO("in TaskComWithDSP1 5_1:call BSP_W5500Config start\r\n");

	BSP_W5500Config(W5500A,&W5500_InitStructure);
    BSP_W5500Socket(W5500A,MR_TCP|ND_MC_MMB);
    BSP_SocketListen(W5500A);

    DBG_PRO("in TaskComWithDSP1 5_2:call BSP_W5500Config   end\r\n");

    DBG_PRO("in TaskComWithDSP1 8:start while\r\n");

	dsp1_net_init_ok_flag=1;
    TEST_SET_ID_DSP_BASIC_INFO();
    
#ifdef FZ1500    
    set_os_sys_date("20150313091000");
#endif //FZ1500
	while(1)
	{
        pW5500Msg = OSQPend(W5500AEvent,SYS_DELAY_250ms,&perr);
        if(OS_ERR_NONE == perr)
        {
            switch(pW5500Msg->MsgID & MAIN_ID_MASK)
            {
                case IR_CON:        // 连接成功
                    // 获取DSP版本号
                    NetFlag = 1;
                    NetMessage.DivNum = 0;
                    NetMessage.MsgID = APP_DISP_COMFROM_DSP;
                    NetMessage.pData = &NetFlag;
                    NetMessage.DataLen = 1;
                    SYSPost(DispTaskEvent,&NetMessage);
                    MY_DELAY_X_S(1);

                    DBG_DIS("send version cmd to dsp \r\n");
                    SndDSP_VersionGet(S_Data_Buffer[W5500A],&NetMessage);
                    break;
                case IR_DISCON:     // 连接断开
                case IR_TIMEOUT:    // 超时
                    // 重新开始监听
        			Disconnect(W5500A);
                    BSP_W5500Socket(W5500A,MR_TCP|ND_MC_MMB);
                    BSP_SocketListen(W5500A);
                    break;
                case IR_RECV:       // 有数据需要接收
                case SEND_REQ:      // 有数据需要发送
                    Deal_DSPData(W5500A,pW5500Msg);
                    break;
                default:
                    break;
            }
        }
	}
}
#endif

void TaskComWithDSP2(void *pdata)
{
    // 屏幕相关变量定义
    static INT8U    NetFlag = 0;
    static          _BSP_MESSAGE NetMessage;
    
    INT8U perr;
    _BSP_MESSAGE  *pW5500Msg;
    _BSPW5500_CONFIG W5500_InitStructure;
    _BSPGPIO_CONFIG GPIO_InitStructure;

	INT8U Detination_IP[4]	={192,168,16,131};					// 目的IP
	INT8U Source_IP[4]		={192,168,16,35};					// 源IP(服务器)
	INT8U Gateway_IP[4]		={192,168,16,1};					// 网关
	INT8U Subnet_Mask[4]	={255,255,255,0};					// 子网掩码
	INT8U MAC_Addr[6]		={0x48,0x53,0x00,0x57,0x55,0x00};	// MAC
     
    // 中断线初始化
    GPIO_InitStructure.PortNum = PORT1;
    GPIO_InitStructure.PinNum = 22;
    GPIO_InitStructure.Dir = GPIO_DIR_I;
    GPIO_InitStructure.IntLine = GPIO_INTB;
    GPIO_InitStructure.IntType = GPIO_INT_TYPE_L;
    GPIO_InitStructure.pEvent = W5500BEvent;
    BSP_GPIOConfig(&GPIO_InitStructure);
    
    // W5500初始化
	memcpy(W5500_InitStructure.GW_IP,Gateway_IP,4);
	memcpy(W5500_InitStructure.MAC,MAC_Addr,6);
	memcpy(W5500_InitStructure.S_IP,Source_IP,4);
	memcpy(W5500_InitStructure.Sub_Mask,Subnet_Mask,4);
	memcpy(W5500_InitStructure.D_IP,Detination_IP,4);
	W5500_InitStructure.D_Port = DESPORT_NUM;
	W5500_InitStructure.S_Port = 7566;
	W5500_InitStructure.s_num = SOCKET0;

    BSP_W5500Config(W5500B,&W5500_InitStructure);
    BSP_W5500Socket(W5500B,MR_TCP|ND_MC_MMB);
    BSP_SocketListen(W5500B);

    while(1)
	{

        pW5500Msg = OSQPend(W5500BEvent,SYS_DELAY_250ms,&perr);
        if(OS_ERR_NONE == perr)
        {
            switch(pW5500Msg->MsgID)
            {
                case IR_CON:        // 连接成功
                    // 获取DSP版本号
                    NetFlag = 1;
                    NetMessage.DivNum = 1;
                    NetMessage.MsgID = APP_DISP_COMFROM_DSP;
                    NetMessage.pData = &NetFlag;
                    NetMessage.DataLen = 1;
                    SYSPost(DispTaskEvent,&NetMessage);
                    MY_DELAY_X_S(1);
                    SndDSP_VersionGet(S_Data_Buffer[W5500B],&NetMessage);
                    break;
                case IR_DISCON:     // 连接断开
                case IR_TIMEOUT:    // 超时
                    // 重新开始监听
        			Disconnect(W5500B);
                    BSP_W5500Socket(W5500B,MR_TCP|ND_MC_MMB);
                    BSP_SocketListen(W5500B);
                    break;
                case IR_RECV:       // 有数据需要接收
                case SEND_REQ:      // 有数据需要发送
                    Deal_DSPData(W5500B,pW5500Msg);
                    break;
                default:
                    break;
            }
        }

	}
}

void Task_PHY_DSP(void *pdata)
{
#if 1
    LWIP_IF lwipIfPort2;
    int i;
    char local_ip[20]="192.168.11.35";
    char local_mk[20]="255.255.255.0";
    char local_gw[20]="192.168.11.1";
    int ret_code=0;
    SK_TYPE dsp_socket=INVALID_SOCKET,c_socket;
    struct sockaddr_in sa_server,sa_client;
    uint16 port = 7566;
    int ret = 0;
    INT32U lens;

    EVMMACAddrGet(1, lwipIfPort2.macArray);
    for(i=0;i<6;++i)
    {
        g_mac[i]=lwipIfPort2.macArray[5-i];
    }

    lwipIfPort2.slvPortNum = 2;
    lwipIfPort2.instNum = 0;
    lwipIfPort2.ipAddr =  htonl(inet_addr(local_ip));
    lwipIfPort2.netMask = htonl(inet_addr(local_mk));
    lwipIfPort2.gwAddr =  htonl(inet_addr(local_gw));
    lwipIfPort2.ipMode = IPADDR_USE_STATIC; //使用静态分配IP


    my_lwIPInit(&lwipIfPort2,&ret_code);

    //port=7566;
    //memcpy(ip,g_server_ip,sizeof(g_server_ip));
    
    dsp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&sa_server,0,sizeof(struct sockaddr_in));
	sa_server.sin_family = AF_INET;
    sa_server.sin_port = htons ( port );
    sa_server.sin_addr.s_addr = htonl ( INADDR_ANY );
    ret = bind(dsp_socket, (struct sockaddr*)&sa_server, sizeof(sa_server));
    ret = listen(dsp_socket,TCP_DEFAULT_LISTEN_BACKLOG);
    OSTimeDlyHMSM(0,0,1,0);	
    
    c_socket = lwip_accept(dsp_socket, (struct sockaddr*)&sa_client, &lens);
    for(;;)
    {
        ret = recv(c_socket,S_Data_Buffer[1],4096,0);
        if(ret >0)
            OSTimeDlyHMSM(0,0,1,0);
    }
        
    #endif

}



/***********************************************************************************************
* Function Name	: Deal_DSPData
* Description	: ê?·￠êy?Yμ?′|àíoˉêy?ó?ú
* Input			: DevNum    éè±?o?
                  *pMsg     ???￠
* Return		:
* Note(s)		:
* Contributor	: 150112	code
***********************************************************************************************/
INT8U Deal_DSPData(INT8U DevNum,_BSP_MESSAGE *pTxMsg)
{
	INT16U          Rx_len;

    if((pTxMsg->MsgID & MAIN_ID_MASK) == SEND_REQ)             // 如果是发送请求
    {
        switch(pTxMsg->MsgID & SUB_ID_MASK)
        {
            case SUB_ID_UPDATE:            // 程序升级命令
                SndDSP_UpdtCMD(S_Data_Buffer[DevNum],pTxMsg);
                break;
            case SUB_ID_ADJ:            // CS校验
                SndDSP_CheckCSCMD(S_Data_Buffer[DevNum],pTxMsg);
                break;
            case SUB_ID_BIG_IMAGE:        // 大图查看
                SndDSP_BIGIMAGECMD(S_Data_Buffer[DevNum],pTxMsg);
                break;
            default:
                break;
        }
    }

    else if((pTxMsg->MsgID & MAIN_ID_MASK) == IR_RECV)     // 如果是接收请求
    {
        Rx_len = BSP_W5500Recv(DevNum,S_Data_Buffer[DevNum]);
        if(Rx_len > 0)
        {
            if(W5500B==DevNum)
            {
                DBG_DIS("in Deal_DSPData 1:Rx_len=%d\r\n",Rx_len);

                dsp_net_recv_data_to_cyc_buf_n (S_Data_Buffer[DevNum],Rx_len);
                Rx_len=0;
            }
        }
        else
            return FALSE;
    }
    else
        return FALSE;  

    return TRUE;
}

/***********************************************************************************************
* Function Name	: Judge_Ack
* Description	: 判断协议帧类别
* Input			: *pFrame   协议帧指针
* Return		: _ASWR_FRAME      应答帧
                  _NORM_FREAM     普通帧
* Note(s)		: 应答帧的处理程序暂未添加
* Contributor	: 141106	code
***********************************************************************************************/

_Frame_Cat Judge_Ack(INT8U *pFrame)
{
    _Frame_Cat  frame_category;

    switch(*(pFrame+FRAME_ACK_OFFSET))
    {
        case 0x00:      // 应答帧
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
            frame_category = _ASWR_FRAME;
            // 这里添加应答帧处理程序
            break;
        case 0xff:      // 普通帧，要求应答
            frame_category = _NOR_FRAME_ACK;
            break;
        case 0xbb:      // 普通帧，不要求应答
            frame_category = _NOR_FRAM_NACK;
            break;
        default:
            break;
    }

    return frame_category;
}

/***********************************************************************************************
* Function Name	: RecDSP_UpdtRT
* Description	:  ARM读图像板升级结果
* Input			: *pFrame   协议帧指针
* Return		: _OP_ID_ERR
                  _ID_DATA_ERR
                  _NO_ERR
* Note(s)		:
* Contributor	: 141106	code
***********************************************************************************************/
void RecDSP_UpdtRT(INT8U *pFrame)
{
    switch(*(INT16U *)(pFrame))

    {
        case 0x0001:    // DSP升级失败
			PraperLevelUpFail();
            break;
        case 0x0002:    // FPGA升级失败
			PraperLevelUpFail();
            break;
        case 0x0003:    // 人民币模板升级失败
			PraperLevelUpFail();
            break;
        case 0x0101:    // DSP升级成功
            PraperLevelUpED();
            break;
        case 0x0102:    // FPGA升级成功
            PraperLevelUpED();
            break;
        case 0x0103:    // 人民币模板升级成功
            PraperLevelUpED();
            break;
        case 0x0201:    // DSP不支持的升级标志
            break;
        case 0x0202:    // FPGA不支持的升级标志
            break;
        case 0x0203:    // 人民币模板不支持的升级标志
            break;
        default:        // ID_DATA内容不对
            break;
    }

}

/***********************************************************************************************
* Function Name	: SndDSP_UpdtCMD
* Description	:  发送升级命令
* Input			: *pFrame   协议帧指针
                  *BootMsg  update消息结构体
* Return		:
* Note(s)		: pData指针指向的第一个数据是升级标志
* Contributor	: 141106	code
***********************************************************************************************/
void SndDSP_UpdtCMD(INT8U *pFrame,_BSP_MESSAGE *UpdtMsg)
{
    INT32U TotlSegNum;
    INT32U CurSeg=1;
    INT16U Tx_len;
    INT16U Rx_lens;
    INT16U cout;
    INT8U cs=0;
    INT8U RecvBuff[23];
    _Frame_Cat fCatgry;
    _UpdtFixFmt *UpdtFixFmt = (_UpdtFixFmt *)(pFrame+FRAME_DATA_OFFSET);

    TotlSegNum = UpdtMsg->DataLen/4000 + ((UpdtMsg->DataLen%4000)?1:0);     // 总段数

    UpdtFixFmt->Targt = *(UpdtMsg->pData)++;
    UpdtFixFmt->TotlSeg = TotlSegNum;
    UpdtFixFmt->TotlLen = UpdtMsg->DataLen;

    FRAME_FIX_INFO(pFrame);                          // 固定部分填充
    *(pFrame+FRAME_ACK_OFFSET) = REQ_ACK;            // 要求应答
    *(pFrame+FRAME_ATTRIBUTE_OFFSET) = ARM_WR_DSP;       // 写操作

    TickVal = OSTimeGet();
    Operat_Socket_INT_Indiv(W5500B,IMR_CON | IMR_DISCON | IMR_TIMEOUT);     // 暂时关闭接收中断
    do
    {
        *(INT16U *)(pFrame+FRAME_INDEX_OFFSET) = ((FramNum++) | 0x8000);    // 帧序号

        // 数据域长度
        if(UpdtMsg->DataLen > 4000)
        {
            *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET) = 4000+2+UPDATEHEADLEN;
            UpdtMsg->DataLen -= 4000;
        }
        else
        {
            *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET) = UpdtMsg->DataLen+2+UPDATEHEADLEN;
            UpdtMsg->DataLen = 0;
        }

        // 帧头校验和
        for(cout=0;cout<FRAME_HEADSUM_OFFSET;cout++)
            cs += *(pFrame+cout);
        *(pFrame+FRAME_HEADSUM_OFFSET) = cs;

        // ID
        *(INT16U *)(pFrame+FRAME_CMD_ID_OFFSET) = ID_DSP_DSP_LEVEL_UP;

        // ID_DATA
        UpdtFixFmt->CurSeg = CurSeg++;
        UpdtFixFmt->CurLen = *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)-2-UPDATEHEADLEN;
        memcpy(pFrame+FRAME_DATA_OFFSET+UPDATEHEADLEN,UpdtMsg->pData,UpdtFixFmt->CurLen);

        UpdtMsg->pData += UpdtFixFmt->CurLen;

        // 整帧校验和
        for(;cout<(*(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)+FRAMEHEADLEN);cout++)
            cs += *(pFrame+cout);
        *(pFrame+cout++) = cs;
        cs = 0;

        // 帧尾
        *(pFrame+cout++) = 0x16;

        // 帧长计算
        Tx_len = cout;

        BSP_W5500Send(UpdtMsg->DivNum,pFrame,Tx_len);
        
        // 每发一包数据都需要等待DSP的应答，确保数据已被收到
        for(;;)
        {
            Rx_lens = BSP_W5500Recv(UpdtMsg->DivNum,RecvBuff);
            if(Rx_lens > 0)
            {
                // 帧类别判断
                fCatgry = Judge_Ack(RecvBuff);

                // 判断是否是应答帧
                if(fCatgry == _ASWR_FRAME)
                    break;
            }
        }
    }while(UpdtMsg->DataLen);

    Operat_Socket_INT_Indiv(W5500B,IMR_CON | IMR_DISCON | IMR_RECV | IMR_TIMEOUT); // 重新打开接收中断
    TickVal = OSTimeGet() - TickVal;
}

/***********************************************************************************************
* Function Name	: SndDSP_VersionGet
* Description	:  DSP版本取得
* Input			: *pFrame   协议帧指针
                  *BootMsg  update消息结构体
* Return		:
* Note(s)		: pData指针指向的第一个数据是升级标志
* Contributor	: 141106	code
***********************************************************************************************/
void SndDSP_VersionGet(INT8U *pFrame,_BSP_MESSAGE *AdjuMsg)
{
    INT16U Tx_len;
    INT16U cout;
    INT8U cs=0;

    FRAME_FIX_INFO(pFrame);      // 固定部分填充
    *(pFrame+FRAME_ACK_OFFSET) = REQ_ACK;            // 要求应答
    *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET) = 0x00000002;
    *(pFrame+FRAME_ATTRIBUTE_OFFSET) = ARM_RD_DSP;       // 读操作
    *(INT16U *)(pFrame+FRAME_INDEX_OFFSET) = ((FramNum++) | 0x8000);   // 帧序号

    // 帧头校验和
    for(cout=0;cout<FRAME_HEADSUM_OFFSET;cout++)
        cs += *(pFrame+cout);
        *(pFrame+FRAME_HEADSUM_OFFSET) = cs;

    // ID
    *(INT16U *)(pFrame+FRAME_CMD_ID_OFFSET) = ID_DSP_VER;

    // 整帧校验和
    Tx_len = 21;
    cs = 0;
    for(cout=0;cout<20;cout++)
    cs += *(pFrame+cout);
    pFrame[19]= cs;
    pFrame[20]= 0x16;

//    for(;cout<(*(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)+FRAMEHEADLEN);cout++)
//        cs += *(pFrame+cout);
//        *(pFrame+cout++) = cs;
//
//    // 帧尾
//    *(pFrame+cout++) = 0x16;

    // 帧长计算
    //Tx_len = cout;

    DBG_BUF_PRO(pFrame, Tx_len,"send ver cmd:");
    BSP_W5500Send(AdjuMsg->DivNum,pFrame,Tx_len);

}

/***********************************************************************************************
* Function Name	: SndDSP_CheckCSCMD
* Description	: 发送CS校验命令
* Input			: *pFrame   协议帧指针
                  *BootMsg  version消息结构体
* Return		:
* Note(s)		:
* Contributor	: 141106	code
***********************************************************************************************/
void SndDSP_CheckCSCMD(INT8U *pFrame,_BSP_MESSAGE *VerMsg)
{
    INT16U Tx_len;
    INT16U cout;
    INT8U cs=0;

    FRAME_FIX_INFO(pFrame);      // 固定部分填充
    *(pFrame+FRAME_ACK_OFFSET) = REQ_ACK;            // 要求应答
    *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET) = 0x00000002;
    *(pFrame+FRAME_ATTRIBUTE_OFFSET) = ARM_WR_DSP;       // 读操作
    *(INT16U *)(pFrame+FRAME_INDEX_OFFSET) = ((FramNum++) | 0x8000);   // 帧序号

    // 帧头校验和
    for(cout=0;cout<FRAME_HEADSUM_OFFSET;cout++)
        cs += *(pFrame+cout);
        *(pFrame+FRAME_HEADSUM_OFFSET) = cs;

    // ID
    *(INT16U *)(pFrame+FRAME_CMD_ID_OFFSET) = ID_DSP_CS_CHECK;

    // 整帧校验和
    for(;cout<(*(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)+FRAMEHEADLEN);cout++)
        cs += *(pFrame+cout);
        *(pFrame+cout++) = cs;

    // 帧尾
    *(pFrame+cout++) = 0x16;

    // 帧长计算
    Tx_len = cout;
    BSP_W5500Send(VerMsg->DivNum,pFrame,Tx_len);
}

/***********************************************************************************************
* Function Name	: SndDSP_BIGIMAGECMD
* Description	: 查看大图指令
* Input			: *pFrame   协议帧指针
                  *BootMsg  version消息结构体
* Return		:
* Note(s)		:
* Contributor	: 150311	songchao
***********************************************************************************************/
void SndDSP_BIGIMAGECMD(INT8U *pFrame,_BSP_MESSAGE *VerMsg)
{
    INT16U Tx_len;
    INT16U cout;
    INT8U cs=0;
    FRAME_FIX_INFO(pFrame);      // 固定部分填充
    *(pFrame+FRAME_ACK_OFFSET) = NREQ_ACK;            // 不要求应答
    *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET) = 0x00000002;
    *(pFrame+FRAME_ATTRIBUTE_OFFSET) = ARM_WR_DSP;       // 读操作
    *(INT16U *)(pFrame+FRAME_INDEX_OFFSET) = ((FramNum++) | 0x8000);   // 帧序号

    // 帧头校验和
    for(cout=0;cout<FRAME_HEADSUM_OFFSET;cout++)
        cs += *(pFrame+cout);
        *(pFrame+FRAME_HEADSUM_OFFSET) = cs;

    // ID
    *(INT16U *)(pFrame+FRAME_CMD_ID_OFFSET) = ID_DSP_BIG_IMG_START;

    // 整帧校验和
    for(;cout<(*(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)+FRAMEHEADLEN);cout++)
        cs += *(pFrame+cout);
        *(pFrame+cout++) = cs;

    // 帧尾
    *(pFrame+cout++) = 0x16;

    // 帧长计算
    Tx_len = cout;
    BSP_W5500Send(VerMsg->DivNum,pFrame,Tx_len);
}

int send_dsp1_data(uint8 *buf,uint32 len)
{
    if(0==dsp1_net_init_ok_flag) return 0;

    DBG_PRO("dsp1_send:len=%d \r\n",len);
    
    BSP_W5500Send(DSP1,buf,len);

    return len;
}


int send_dsp2_data(uint8 *buf,uint32 len)
{
    if(0==dsp2_net_init_ok_flag) return 0;

    DBG_PRO("dsp2_send:len=%d \r\n",len);

    BSP_W5500Send(DSP2,buf,len);

    return len;
}



/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/

