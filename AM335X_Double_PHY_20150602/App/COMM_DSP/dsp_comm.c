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
#include "dsp_comm.h"
#include "def_config.h"
#include "dsp_net.h"
#include "app.h"
#include "flashtaskappinterface.h"
#include "CoodinateInit.h"
#include "mmcsdmain.h"
#include "DisplayMain.h"
#include "DispkeyFunction.h"
#include "DispDSP.h"
/* Private define-----------------------------------------------------------------------------*/
//#if USE_PHY
#define TASK_STK_SIZE_PHY_DSP_SND       0x3000
OS_STK Task_PHY_DSP_SNDStk[TASK_STK_SIZE_PHY_DSP_SND];       // A8_PHY_DSP通讯任务堆栈
OS_EVENT *PHYEvent;
void *PHYOSQ[4];

static INT8U Comm_Lwip_Init(INT8U portNum);
static INT8U Comm_Server_Init(INT16U s_port);
static INT8U Comm_Server_Establish_Connection(void);
static INT8U Comm_Server_Dealwith_Data(fd_set *rfd,fd_set *wfd);
static void * Comm_Socket_Manager_Creat(SK_TYPE c_sock);
static INT8U Comm_Frame_Creat(FramePara_S *para);
static INT8U Comm_Socket_Manager_Free(Socket_S *pnode);
static INT8U SndDSP_Lwip_VersionGet(void);
static INT8U SndDSP_Lwip_UpdtCMD(_BSP_MESSAGE *pmsg);
static INT8U SndDSP_Lwip_CheckCSCMD(void);
static INT8U SndDSP_Lwip_BIGIMAGECMD(_BSP_MESSAGE *pmsg);
static INT8U SndDSP_Lwip_BlackListCMD(_BSP_MESSAGE *pmsg);

//#else

static INT8U    dsp_Data_Buffer[16*1024-1];    // socket收发缓冲
OS_EVENT *DSPSndEvent;
void *DSPSndOSQ[10];

static void SndDSP_BlackListCMD(INT8U *pFrame,_BSP_MESSAGE *blacklist);
static void SndDSP_BlackListPowerOnCMD(INT8U * pFrame, INT8U * pData, INT32U DataLen);
void SndDSP_UpdtCMD(INT8U *pFrame,_BSP_MESSAGE *UpdtMsg);
void SndDSP_VersionGet(INT8U *pFrame,INT8U divnum);
void SndDSP_CheckCSCMD(INT8U *pFrame,_BSP_MESSAGE *VerMsg);
void SndDSP_BIGIMAGECMD(INT8U *pFrame,_BSP_MESSAGE *picmsg);
void Deal_DSPData(INT8U DevNum);
INT8U* APP_BlackListPowerOnSendDsp(INT8U num, INT8U * pdata, INT8U len);
//#endif

//#if !USE_PHY //双网卡暂时注释
static INT8U tmp_frame[1024]={0}; //临时变量
//#endif

static INT16U FramNum = 0;          // 帧序号
INT8U Pic_Md;                       // 大图模式
INT32U TickVal;

uint8 dsp1_net_init_ok_flag=0;
uint8 dsp2_net_init_ok_flag=0;

//使用的网络芯片，此变量需要在操作网络相关前赋值
_BSP_NET_TYPE NetChipUse = NET_TYPE_8720;


extern INT8U big_img_flag; //发大图标志 1:正在发大图
extern _APP_BLACKLIST APP_BLACKLIST[2];
extern INT8U BlackLists[2][201][10];

/* Private functions--------------------------------------------------------------------------*/
//#if USE_PHY
static Typ_Config_Lwip LwipManager;

INT8U APP_Inside_Net_Status_Get(void)
{
    return (LwipManager.s_num?1:0);
}

int APP_Socket_Send(INT8U *buf,INT32U len)
{
    INT32U realens;

    realens = send(LwipManager.pnode->socket,buf,len,0);
    if(realens != len)
    {
        DBG_ERR("SOCKET send realens = %d, len = %d\r\n",realens,len);
        return 0;
    }

    return len;
}

void Task_PHY_DSP(void *pdata)
{
    uint16 port = 7566;
    int ret = 0;

    dsp_net_init_buf();

    ret = Comm_Lwip_Init(CPSW_PORT1);
    if(ret == FALSE)
    {
        DBG_ERR("Comm_Lwip_Init FALSE,Delete self task...\r\n");
        OSTaskDel(OS_PRIO_SELF);
    }
    DBG_INFO("Comm_Lwip_Init OK!\r\n");
    DBG_INFO("Comm_DSP server port:%d\r\n",port);
	while(1)
    {
        ret = Comm_Server_Init(port);
        if(ret == FALSE)
        {
            DBG_ERR("Comm_Server_Init FALSE!\r\n");
            OSTimeDlyHMSM(0,0,2,0);
            continue;
        }
        DBG_INFO("Comm_Server_Init OK!\r\n");

        ret = Comm_Server_Establish_Connection();
        if(ret == FALSE)
        {
            DBG_ERR("Comm_Server_Establish_Connection FALSE!\r\n");
            OSTimeDlyHMSM(0,0,2,0);
        }
    }
}

void Task_PHY_DSP_SND(void *pdata)
{
    _BSP_MESSAGE *pMsg;						// 消息指针
    INT8U err;

    // 获取DSP版本号
    if(SndDSP_Lwip_VersionGet() == TRUE)
    {
        DBG_INFO("SndDSP_VersionGet OK!\r\n");
    }
    else
    {
        DBG_INFO("SndDSP_VersionGet FALSE!\r\n");
    }

    for(;;)
    {
        pMsg = OSQPend(PHYEvent,SYS_DELAY_100ms,&err);
        if(err == OS_NO_ERR)				    // 收到消息
        {
            switch(pMsg->MsgID)
            {
                case SUB_ID_VER:
                    SndDSP_Lwip_VersionGet();
                    break;
                case SUB_ID_BIG_IMAGE:
                    SndDSP_Lwip_BIGIMAGECMD(pMsg);
                    break;
                case SUB_ID_ADJ:
                    SndDSP_Lwip_CheckCSCMD();
                    break;
                case SUB_ID_BKLST:
                    SndDSP_Lwip_BlackListCMD(pMsg);
                    break;
                case SUB_ID_UPDATE:
                    SndDSP_Lwip_UpdtCMD(pMsg);
                    break;
                default:
                    break;

            }
        }
    }
}

/***********************************************************************************************
* Function Name	: Comm_Lwip_Init
* Description	: 网卡硬件初始化
* Input			: portNum   -   CPSW PORT
* Return		:
* Note(s)		:
* Contributor	: 150603	code
***********************************************************************************************/
static INT8U Comm_Lwip_Init(INT8U portNum)
{
    INT32U  ret;
    int     ret_code=0;

    char    local_ip[20]="4.4.4.4";
    char    local_mk[20]="255.255.255.0";
    char    local_gw[20]="4.4.4.1";


    static LWIP_IF lwip;

    LwipManager.t_lwip = &lwip;

    EVMMACAddrGet(portNum, LwipManager.t_lwip->macArray);

    LwipManager.t_lwip->instNum = 0;
    LwipManager.t_lwip->slvPortNum = portNum+1;
    LwipManager.t_lwip->ipAddr =  htonl(inet_addr(local_ip));
    LwipManager.t_lwip->netMask = htonl(inet_addr(local_mk));
    LwipManager.t_lwip->gwAddr =  htonl(inet_addr(local_gw));
    LwipManager.t_lwip->ipMode = IPADDR_USE_STATIC; //使用静态分配IP

    DBG_NORMAL("Comm_DSP server ip:%s\r\n",local_ip);
    ret = my_lwIPInit(LwipManager.t_lwip,&ret_code);
    if(ret == 0)
        return FALSE;

    return TRUE;
}

/***********************************************************************************************
* Function Name	: Comm_Server_Init
* Description	: 服务端初始化
* Input			: s_port   -   server port
* Return		:
* Note(s)		:
* Contributor	: 150603	code
***********************************************************************************************/
static INT8U Comm_Server_Init(INT16U s_port)
{
    struct sockaddr_in sa_server;
    int alive,idle,intv,cnt;

    LwipManager.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(LwipManager.socket < 0)
        return FALSE;
    alive = 1;
    if(setsockopt(LwipManager.socket,SOL_SOCKET, SO_KEEPALIVE,&alive,sizeof(alive)))
    {
        DBG_ERR("Open keepalive fail!\r\n");
        close(LwipManager.socket);
        return FALSE;
    }
    idle = 5;
    if(setsockopt(LwipManager.socket,IPPROTO_TCP, TCP_KEEPIDLE,&idle,sizeof(idle)))
    {
        DBG_ERR("Set keepalive idle time fail!\r\n");
        close(LwipManager.socket);
        return FALSE;
    }
    intv = 4;
    if(setsockopt(LwipManager.socket,IPPROTO_TCP, TCP_KEEPINTVL,&intv,sizeof(intv)))
    {
        DBG_ERR("Set keepalive intervel time fail!\r\n");
        close(LwipManager.socket);
        return FALSE;
    }
    cnt = 3;
    if(setsockopt(LwipManager.socket,IPPROTO_TCP, TCP_KEEPCNT,&cnt,sizeof(cnt)))
    {
        DBG_ERR("Set keepalive counter fail!\r\n");
        close(LwipManager.socket);
        return FALSE;
    }

    memset(&sa_server,0,sizeof(struct sockaddr_in));
	sa_server.sin_family = AF_INET;
    sa_server.sin_port = htons ( s_port );
    sa_server.sin_addr.s_addr = htonl ( INADDR_ANY );
    if(bind(LwipManager.socket, (struct sockaddr*)&sa_server, sizeof(sa_server)) == -1)
    {
        close(LwipManager.socket);
        return FALSE;
    }

    if(listen(LwipManager.socket,TCP_DEFAULT_LISTEN_BACKLOG) == -1)
    {
        close(LwipManager.socket);
        return FALSE;
    }

    return TRUE;
}

/***********************************************************************************************
* Function Name	: Comm_Server_Establish_Connection
* Description	: 服务端建立连接
* Input			:
* Return		:
* Note(s)		:
* Contributor	: 150603	code
***********************************************************************************************/
static INT8U Comm_Server_Establish_Connection(void)
{
    struct timeval waitTime = {0,500000};
    fd_set rfd,wfd;
    SK_TYPE c_socket = INVALID_SOCKET;
    struct sockaddr_in sa_client;
    INT32U sin_size;
    int ret = 0;

    sin_size = sizeof(struct sockaddr_in);
    memset(&sa_client,0,sizeof(struct sockaddr_in));
    LwipManager.s_num = 0;      // socket连接数清0

    for(;;)
    {
        FD_ZERO(&rfd);
        FD_SET(LwipManager.socket,&rfd);
        ret = select(LwipManager.socket + 1,&rfd,NULL,NULL,&waitTime);
        if(ret == -1)
        {
            close(LwipManager.socket);
            return FALSE;
        }
        else if(ret == 0)
        {
            continue;
        }
        else
        {
            if(FD_ISSET(LwipManager.socket,&rfd))
            {
                c_socket = accept(LwipManager.socket, (struct sockaddr*)&sa_client, &sin_size);
                DBG_INFO("Comm_Server_Establish_Connection OK!\r\n");

                Poweron_control.DSPInitFlag = TRUE; //跟DSP通讯成功
                LwipManager.s_num++;
                LwipManager.pnode = Comm_Socket_Manager_Creat(c_socket);

                // 创建发送线程
                OSTaskCreateExt(Task_PHY_DSP_SND,
                    NULL,
                    &Task_PHY_DSP_SNDStk[TASK_STK_SIZE_PHY_DSP_SND-1],
					PRI_Task_PHY_DSP_SND,
					1,
					&Task_PHY_DSP_SNDStk[0],
					TASK_STK_SIZE_PHY_DSP_SND,
                    NULL,
                    OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

                for(;;)
                {
                    FD_ZERO(&rfd);
                    FD_ZERO(&wfd);

                    FD_SET(c_socket,&rfd);

                    INT32U size;
                    xBuffer_Occupied_Size(LwipManager.pnode->xbuf,&size);
                    if(size)
                        FD_SET(c_socket,&wfd);

                    ret = select(c_socket + 1,&rfd,&wfd,NULL,&waitTime);
                    if(ret == -1)
                    {
                        DBG_ERR("SOCKET ERROR\r\n");
                        Comm_Socket_Manager_Free(LwipManager.pnode);
                        OSTaskDel(PRI_Task_PHY_DSP_SND);
                        break;
                    }
                    else if(ret == 0)
                    {
                        continue;
                    }
                    else
                    {
                        if(Comm_Server_Dealwith_Data(&rfd,&wfd)!= TRUE)
                        {
                            DBG_ERR("SOCKET ERROR\r\n");
                            Comm_Socket_Manager_Free(LwipManager.pnode);
                            OSTaskDel(PRI_Task_PHY_DSP_SND);
                            break;
                        }
                    }
                }
            }
        }
    }
}

/***********************************************************************************************
* Function Name	: Comm_Server_Dealwith_Data
* Description	: 处理收发数据
* Input			: *rfd   -   读集合
* Return		:
* Note(s)		:
* Contributor	: 150616	code
***********************************************************************************************/
static INT8U Comm_Server_Dealwith_Data(fd_set *rfd,fd_set *wfd)
{
    int     recvLens = 0;
    Socket_S    *pnode;

    pnode = LwipManager.pnode;
    if(FD_ISSET(pnode->socket,rfd))
    {
        recvLens = recv(pnode->socket,pnode->rpbuf,RX_BUF_SIZE,0);
        if(recvLens <= 0)
        {
            return FALSE;
        }
        else
        {  
            dsp_net_recv_data_to_cyc_buf_n2(pnode->rpbuf,recvLens);
            return TRUE;
        }
    }
    if(FD_ISSET(pnode->socket,wfd))
    {
        INT32U size;
        xBuffer_Occupied_Size(pnode->xbuf,&size);

        if(size > 1400)
            size = 1400;

        if(xBuffer_Read(pnode->xbuf,pnode->tpbuf,size) != TRUE)
            return FALSE;

        int ret;
        ret= APP_Socket_Send(pnode->tpbuf,size);
        if(!ret)
            return FALSE;

        return TRUE;
    }

    return FALSE;
}

static void * Comm_Socket_Manager_Creat(SK_TYPE c_sock)
{
    Socket_S *header;

    if(c_sock < INVALID_SOCKET)
        return NULL;

    header = (Socket_S *)malloc(sizeof(Socket_S));
    header->socket = c_sock;
    header->tpbuf = malloc(TX_BUF_SIZE);
    header->rpbuf = malloc(RX_BUF_SIZE);
    header->next = NULL;
    header->prev = NULL;
    header->xbuf = xBuffer_Create(XBUFFER_SIZE,PRI_DSP_MUTEX);

    return header;
}

static INT8U Comm_Socket_Manager_Free(Socket_S *pnode)
{
    if(pnode->socket == INVALID_SOCKET)
        return FALSE;

    close(pnode->socket);
    pnode->socket = INVALID_SOCKET;

    free(pnode->tpbuf);
    free(pnode->rpbuf);

    if(xBuffer_Free(pnode->xbuf) != TRUE)
        return FALSE;

    free(pnode);

    LwipManager.s_num--;
    LwipManager.pnode = NULL;

    return TRUE;
}

static INT8U Comm_Frame_Creat(FramePara_S *para)
{
    INT8U buf[4096];
    FrameHeader_S *h = (FrameHeader_S *)buf;
    INT32U i,lens;
    INT8U cs = 0;

    h->fheader = 0x02A0;
    h->fattr = para->wr;
    h->fcode = 0x03;
    h->findex = ((FramNum++) | 0x8000);
    h->pver = 0x01;
    h->ack = para->ack;
    h->datalens = para->datalens+ 2;

    for(i=0;i<FRAME_HEADSUM_OFFSET;i++)
        cs += buf[i];
    h->fhCheckSum = cs;

    h->fcmdID = para->cmdID;

    if(para->datalens)
        memcpy(buf + FRAME_DATA_OFFSET,para->pbuf,para->datalens);

    lens = FRAME_DATA_OFFSET + para->datalens;
    for(;i<lens;i++)
        cs += buf[i];

    buf[lens] = cs;
    lens++;

    buf[lens] = 0x16;
    lens++;

    return xBuffer_Write(LwipManager.pnode->xbuf,buf,lens);
}

static INT8U SndDSP_Lwip_VersionGet(void)
{
    FramePara_S f_para = {NULL,0,REQ_ACK,ARM_RD_DSP,ID_DSP_VER};

    return Comm_Frame_Creat(&f_para);
}

static INT8U SndDSP_Lwip_UpdtCMD(_BSP_MESSAGE *pmsg)
{
    INT8U t_buf[4096];
    _UpdtFixFmt *up_header = (_UpdtFixFmt *)t_buf;

    FramePara_S f_para = {NULL,0,REQ_ACK,ARM_WR_DSP,ID_DSP_DSP_LEVEL_UP};

    up_header->TotlSeg   = pmsg->DataLen/4000 + ((pmsg->DataLen%4000)?1:0);
    up_header->TotlLen   = pmsg->DataLen;
    up_header->Targt     = *(pmsg->pData)++;
    //up_header->pdata     = pmsg->pData;

    DBG_INFO("This is upgrage start...");
    for(up_header->CurSeg = 1;up_header->CurSeg <= up_header->TotlSeg;up_header->CurSeg++)
    {
        up_header->CurLen = (up_header->CurSeg < up_header->TotlSeg)?4000:(up_header->TotlLen%4000);

        memcpy(t_buf+UPDATEHEADLEN,pmsg->pData,up_header->CurLen);
        f_para.pbuf = t_buf;
        f_para.datalens = UPDATEHEADLEN + up_header->CurLen;
        if(Comm_Frame_Creat(&f_para) != TRUE)
        {
            DBG_ERR("Comm_Frame_Creat error!\r\n");
            return FALSE;
        }

        DBG_INFO("Upgrage %dst packet\r\n",up_header->CurSeg);
        pmsg->pData += up_header->CurLen;
    }

    Print_UpdataProgress(2);

    return TRUE;
}

static INT8U SndDSP_Lwip_CheckCSCMD(void)
{
    FramePara_S f_para = {NULL,0,REQ_ACK,ARM_WR_DSP,ID_DSP_CS_CHECK};

    return Comm_Frame_Creat(&f_para);
}

static INT8U SndDSP_Lwip_BIGIMAGECMD(_BSP_MESSAGE *pmsg)
{
    INT8U buf[1024];
    
    FramePara_S f_para = {NULL,0,REQ_ACK,ARM_WR_DSP,ID_A8_TRIGGER_DSP_SEND_BIG_IMG};

    if(pmsg->DataLen > MAX_PACKET_NUM)
        return FALSE;

    Pic_Md = buf[0] = pmsg->DivNum;
    buf[1] = pmsg->DataLen;
    memcpy(buf + 2,pmsg->pData,pmsg->DataLen);
    
    f_para.pbuf = buf;
    f_para.datalens = pmsg->DataLen + 3;
 
    return Comm_Frame_Creat(&f_para);
}

static INT8U SndDSP_Lwip_BlackListCMD(_BSP_MESSAGE *pmsg)
{
    FramePara_S f_para = {NULL,0,REQ_ACK,ARM_WR_DSP,ID_DSP_BLACK_LIST};

    f_para.pbuf = pmsg->pData;
    f_para.datalens = pmsg->DataLen;

    return Comm_Frame_Creat(&f_para);
}

//#else
#if DBG_DSP_RECV_TWO_EN
void TaskComWithDSP(void *pdata)
{
	INT8U Detination_IP[4]	={192,168,11,20};					// 目的IP
	INT8U Source_IP[4]		={192,168,11,35};					// 源IP(服务器)
	INT8U Gateway_IP[4]		={192,168,11,1};					// 网关
	INT8U Subnet_Mask[4]	={255,255,255,0};					// 子网掩码
	INT8U MAC_Addr[6]		={0x48,0x53,0x00,0x57,0x55,0x00};	// MAC
    static uint8 first_connect_ok_1=0;
    static uint8 first_connect_ok_2=0;
    uint16 server_port=7566;
    int delay_flag=0;

    dsp_net_init_buf();


	_BSPW5500_CONFIG W5500_InitStructure;

	memcpy(W5500_InitStructure.GW_IP,Gateway_IP,4);
	memcpy(W5500_InitStructure.MAC,MAC_Addr,6);
	memcpy(W5500_InitStructure.S_IP,Source_IP,4);
	memcpy(W5500_InitStructure.Sub_Mask,Subnet_Mask,4);
	memcpy(W5500_InitStructure.D_IP,Detination_IP,4);
	W5500_InitStructure.D_Port = DESPORT_NUM;
	W5500_InitStructure.S_Port = server_port;
	W5500_InitStructure.s_num = SOCKET0;

	// W5500初始化
	DBG_PRO("in TaskComWithDSP 1_1:call BSP_W5500Config start\r\n");
	BSP_W5500Config(W5500A,&W5500_InitStructure);
    DBG_PRO("in TaskComWithDSP 1_2:call BSP_W5500Config   end\r\n");

	// W5500初始化
	DBG_PRO("in TaskComWithDSP 2_1:call BSP_W5500Config start\r\n");
    BSP_W5500Config(W5500B,&W5500_InitStructure);
    DBG_PRO("in TaskComWithDSP 2_2:call BSP_W5500Config   end\r\n");


    // 创建收发消息队列
    DSPSndEvent = OSQCreate(DSPSndOSQ,10);

    dsp1_net_init_ok_flag=1;
	dsp2_net_init_ok_flag=1;


	while(1)
	{
        delay_flag=0;
		switch(Get_Socket_Status(W5500A))
		{
			case SOCK_ESTABLISHED:								// 如果连接成功
                if(0==first_connect_ok_1)
                {
                    first_connect_ok_1=1;
                    DBG_DIS("in TaskComWithDSP1 6:have cliect connect ok\r\n");
                }

				Deal_DSPData(W5500A);
				break;
			case SOCK_CLOSE_WAIT:								// 如果收到对方发来的断开连接请求
				// 回复断开
				Disconnect(W5500A);
				break;
			case SOCK_CLOSED:									// 如果socket处于关闭状态
				// 配置socket为TCP server模式，打开socket
				BSP_W5500Socket(W5500A,MR_TCP|ND_MC_MMB);
				break;
			case SOCK_INIT:										// 如果socket处于打开状态
				// 开始监听
				BSP_SocketListen(W5500A);
                break;
			default:
                delay_flag=1;
				break;
		}


		switch(Get_Socket_Status(W5500B))
        {
            case SOCK_ESTABLISHED:                              // 如果连接成功
                if(0==first_connect_ok_2)
                {
                    first_connect_ok_2=1;
                    DBG_DIS("in TaskComWithDSP2 6:have cliect connect ok\r\n");
                }
                Deal_DSPData(W5500B);
                break;
            case SOCK_CLOSE_WAIT:                               // 如果收到对方发来的断开连接请求
                // 回复断开
                Disconnect(W5500B);
                break;
            case SOCK_CLOSED:                                   // 如果socket处于关闭状态
                // 配置socket为TCP server模式，打开socket
                BSP_W5500Socket(W5500B,MR_TCP|ND_MC_MMB);
                break;
            case SOCK_INIT:                                     // 如果socket处于打开状态
                // 开始监听
                BSP_SocketListen(W5500B);
                break;
            default:
                delay_flag=1;
                break;
		}


        if(1==delay_flag)
        {
            delay_flag=0;
            MY_DELAY_X_MS(1);
        }

	}
}



#else


/* Private functions--------------------------------------------------------------------------*/
void TaskComWithDSP1(void *pdata)
{
    //INT8U perr;
    INT8U NetCount = 0;
    INT8U NetDefaultCount = 0;
    INT8U NetFlag = 0;
    static _BSP_MESSAGE NetMessage;
	INT8U Detination_IP[4]	={192,168,11,20};					// 目的IP
	INT8U Source_IP[4]		={192,168,11,35};					// 源IP(服务器)
	INT8U Gateway_IP[4]		={192,168,11,1};					// 网关
	INT8U Subnet_Mask[4]	={255,255,255,0};					// 子网掩码
	INT8U MAC_Addr[6]		={0x48,0x53,0x00,0x57,0x55,0x00};	// MAC

#if FZ1500
	INT8U       S_Data_Buffer[4096];    // socket收发缓冲
#endif

    static int client_connected_ok_flag=0;



    dsp_net_init_buf();

	_BSPW5500_CONFIG W5500_InitStructure;

	memcpy(W5500_InitStructure.GW_IP,Gateway_IP,4);
	memcpy(W5500_InitStructure.MAC,MAC_Addr,6);
	memcpy(W5500_InitStructure.S_IP,Source_IP,4);
	memcpy(W5500_InitStructure.Sub_Mask,Subnet_Mask,4);
	memcpy(W5500_InitStructure.D_IP,Detination_IP,4);
	W5500_InitStructure.D_Port = DESPORT_NUM;
	W5500_InitStructure.S_Port = 7566;
	W5500_InitStructure.s_num = SOCKET0;

    OSTimeDlyHMSM(0,0,0,500);


	// W5500初始化
	DBG_PRO("in TaskComWithDSP1 5_1:call BSP_W5500Config start\r\n");

	BSP_W5500Config(W5500A,&W5500_InitStructure);

    DBG_PRO("in TaskComWithDSP1 5_2:call BSP_W5500Config   end\r\n");

    // 创建收发消息队列
    DSPSndEvent = OSQCreate(DSPSndOSQ,10);

    dsp1_net_init_ok_flag=1;
    int delay_flag=0;


    DBG_PRO("in TaskComWithDSP1 8:start while\r\n");
	while(1)
	{
        delay_flag=0;
		switch(Get_Socket_Status(W5500A))
		{
			case SOCK_ESTABLISHED:								// 如果连接成功

                if(0==client_connected_ok_flag)
                {
                    DBG_DIS("client connected ok\r\n");
                    client_connected_ok_flag=1;
                }

				Deal_DSPData(W5500A);
#if FZ1500
                if(NetCount==0)
                {
                    NetFlag = 1;
                    NetCount++;
                    NetDefaultCount = 0;
                    NetMessage.MsgID = APP_DISP_COMFROM_DSP;
                    NetMessage.pData = &NetFlag;
                    NetMessage.DataLen = 1;
                    SYSPost(DispTaskEvent,&NetMessage);
                    MY_DELAY_X_S(2);

                    DBG_DIS("send version cmd to dsp \r\n");
                    SndDSP_VersionGet(S_Data_Buffer,DSP1);

                }
                else if(NetCount == 255)
                {
                    NetCount = 1;
                }else
                {
                    NetCount++;
                }
#endif
				break;
			case SOCK_CLOSE_WAIT:								// 如果收到对方发来的断开连接请求
				// 回复断开
				Disconnect(W5500A);
				break;
			case SOCK_CLOSED:									// 如果socket处于关闭状态
				// 配置socket为TCP server模式，打开socket
				BSP_W5500Socket(W5500A,MR_TCP|ND_MC_MMB);
				break;
			case SOCK_INIT:										// 如果socket处于打开状态
				// 开始监听
				BSP_SocketListen(W5500A);
                break;
			default:
#if FZ1500
                if(NetDefaultCount==0)
                {
                      NetFlag = 0;
                      NetCount = 0;
                      NetDefaultCount++;
                      NetMessage.MsgID = APP_DISP_COMFROM_DSP;
                      NetMessage.pData = &NetFlag;
                      NetMessage.DataLen = 1;
                      SYSPost(DispTaskEvent,&NetMessage);
                }
                else if(NetDefaultCount == 255)
                {
                    NetDefaultCount = 1;
                }else
                {
                    NetDefaultCount++;
                }
#endif
                delay_flag=1;
				break;
		}


        if(1==delay_flag)
        {
            delay_flag=0;
            MY_DELAY_X_MS(5);
        }

	}
}

void TaskComWithDSP2(void *pdata)
{
//    INT8U ret = 0;
    static INT8U    NetFlag = 0;
    static          _BSP_MESSAGE NetMessage;
    static INT8U    get_ver_ok_Flag = 0;
	INT8U *pBuf = NULL;

    INT8U Detination_IP[4]  ={192,168,11,131};                   // 目的IP
    INT8U Source_IP[4]      ={192,168,11,35};                   // 源IP(服务器)
    INT8U Gateway_IP[4]     ={192,168,11,1};                    // 网关
    INT8U Subnet_Mask[4]    ={255,255,255,0};                   // 子网掩码
    INT8U MAC_Addr[6]       ={0x48,0x53,0x00,0x57,0x55,0x00};   // MAC

    static uint8 first_connect_ok_2=0;
    uint16 server_port=7566;
//    int delay_flag=0;

    dsp_net_init_buf();

    _BSPW5500_CONFIG W5500_InitStructure;

    memcpy(W5500_InitStructure.GW_IP,Gateway_IP,4);
    memcpy(W5500_InitStructure.MAC,MAC_Addr,6);
    memcpy(W5500_InitStructure.S_IP,Source_IP,4);
    memcpy(W5500_InitStructure.Sub_Mask,Subnet_Mask,4);
    memcpy(W5500_InitStructure.D_IP,Detination_IP,4);
    W5500_InitStructure.D_Port = DESPORT_NUM;
    W5500_InitStructure.S_Port = server_port;
    W5500_InitStructure.s_num = SOCKET0;


    // W5500初始化
    DBG_INFO("call BSP_W5500Config start,dds=%d\r\n",GET_SYS_DDS("NULL"));
    BSP_W5500Config(W5500B,&W5500_InitStructure);
    DBG_INFO("call BSP_W5500Config   end,dds=%d\r\n",GET_SYS_DDS("NULL"));


#if 0 /*hxj amend,date 2015-5-12 19:27*/
    // 创建收发消息队列
    DSPRecEvent = OSQCreate(DSPRecOSQ,10);
    DSPSndEvent = OSQCreate(DSPSndOSQ,10);
#endif

    Poweron_control.DSPInitFlag = FALSE; //初始化
    dsp2_net_init_ok_flag=1;
    OSTimeDlyHMSM(0,0,0,500);

    while(1)
    {
//        delay_flag=0;
        switch(Get_Socket_Status(W5500B))
        {
            case SOCK_ESTABLISHED:                              // 如果连接成功
                if(0==first_connect_ok_2)
                {
                    first_connect_ok_2 = 1;
					Poweron_control.DSPInitFlag = TRUE; //跟DSP通讯成功
                    DBG_INFO("have cliect connect ok,dds=%d\r\n",GET_SYS_DDS("NULL"));

                    NetFlag = 1;

                    if(0==get_ver_ok_Flag)
                    {
                        get_ver_ok_Flag=1;
                        MY_DELAY_X_S(1);
                        SndDSP_VersionGet(NULL,DSP2);
                    }
					OSTimeDlyHMSM(0, 0, 0, 10);
					//发送黑名单给DSP
					if (Poweron_control.BlackInitFlag == TRUE)
					{
						Poweron_control.BlackInitFlag = FALSE;
						//发送用户黑名单
						pBuf = APP_BlackListPowerOnSendDsp(BLACK_SHOW_BLOCK, (INT8U*)BlackLists[BLACK_SHOW_BLOCK], APP_BLACKLIST[BLACK_SHOW_BLOCK].SaveNum);
                        SndDSP_BlackListPowerOnCMD(dsp_Data_Buffer, pBuf, APP_BLACKLIST[BLACK_SHOW_BLOCK].SaveNum * BLACK_PER_NUM + 4);

						//发送非用户黑名
						pBuf = APP_BlackListPowerOnSendDsp(BLACK_DISSHOW_BLOCK, (INT8U*)BlackLists[BLACK_DISSHOW_BLOCK], APP_BLACKLIST[BLACK_DISSHOW_BLOCK].SaveNum);
						SndDSP_BlackListPowerOnCMD(dsp_Data_Buffer, pBuf, APP_BLACKLIST[BLACK_DISSHOW_BLOCK].SaveNum * BLACK_PER_NUM + 4);

						DBG_INFO("BlackList Send to DSP OK\r\n");
					}

                }
                Deal_DSPData(W5500B);
                break;
            case SOCK_CLOSE_WAIT:                               // 如果收到对方发来的断开连接请求
                // 回复断开
                Disconnect(W5500B);
                if(1==NetFlag)
                {
                    NetFlag = 0;
                    NetMessage.DivNum = DSP2;
                    NetMessage.MsgID = APP_DISP_COMFROM_DSP;
                    NetMessage.pData = &NetFlag;
                    NetMessage.DataLen = 1;
                    SYSPost(DispTaskEvent,&NetMessage);
                }
                MY_DELAY_X_MS(5);
                break;
            case SOCK_CLOSED:                                   // 如果socket处于关闭状态
                // 配置socket为TCP server模式，打开socket
                BSP_W5500Socket(W5500B,MR_TCP|ND_MC_MMB);
                MY_DELAY_X_MS(5);
                break;
            case SOCK_INIT:                                     // 如果socket处于打开状态
                // 开始监听
                BSP_SocketListen(W5500B);
                MY_DELAY_X_MS(5);
                break;
            default:
//                delay_flag=1;
                MY_DELAY_X_MS(10);
                break;
        }

    }
}

#endif



/***********************************************************************************************
* Function Name	: Deal_DSPData
* Description	: 收发数据的处理函数接口
* Input			: DevNum  设备号
* Return		:
* Note(s)		:
* Contributor	: 141106	code
***********************************************************************************************/
void Deal_DSPData(INT8U DevNum)
{
    INT8U           perr;
    _BSP_MESSAGE *  pTxMsg;
	int             Rx_len=0;
	static uint32 BeforeDds = 0;

#if DISPLAY_DSP_RECV_DATA_EN
    static uint32   recv_all_len=0;
    static uint32   recv_all_len_M=0;
    static uint32   recv_all_len_M_bef=0;
#endif

    //轮询发送
    pTxMsg = (_BSP_MESSAGE *)OSQAccept(DSPSndEvent,&perr);
    if(pTxMsg != NULL)
    {
        switch(pTxMsg->MsgID)
        {
            case SUB_ID_VER://读版本
                SndDSP_VersionGet(NULL,pTxMsg->DivNum);
                break;

            case SUB_ID_UPDATE:            // 程序升级命令
                SndDSP_UpdtCMD(dsp_Data_Buffer,pTxMsg);
                break;
            case SUB_ID_ADJ:            // CS校验
                SndDSP_CheckCSCMD(dsp_Data_Buffer,pTxMsg);
                break;
            case SUB_ID_BIG_IMAGE:        // 大图查看

				if(0==BeforeDds)
				{
					BeforeDds = GET_SYS_DDS(NULL);
				}
				else
				{
					if((GET_SYS_DDS(NULL) - BeforeDds) <= 2)
					{
						break;
					}
				}
				BeforeDds = GET_SYS_DDS(NULL);
                SndDSP_BIGIMAGECMD(dsp_Data_Buffer,pTxMsg);
                break;
            case SUB_ID_BKLST:          // 黑名单
                SndDSP_BlackListCMD(dsp_Data_Buffer,pTxMsg);
                break;
            default:
                break;
        }
    }

    //轮询接收
	//读socket接收缓冲
	Rx_len = BSP_W5500Recv(DevNum,dsp_Data_Buffer,sizeof(dsp_Data_Buffer));
    if(Rx_len > 0)
    {
		//DBG_INFO("DSP Receicve the real time data length == %d\r\n",Rx_len);
		//for(INT16U i=0;i<Rx_len;i++ )
		//{
		//	DBG_DIS("%2x ",dsp_Data_Buffer[i]);
		//}
		//DBG_INFO(" end \r\n");
        if(W5500A==DevNum)
        {
#if DISPLAY_DSP_RECV_DATA_EN
            DBG_DIS("RL1=%d\r\n",Rx_len);
#endif
            dsp_net_recv_data_to_cyc_buf_n (dsp_Data_Buffer,Rx_len);
            Rx_len=0;
        }
        else if(W5500B==DevNum)
        {
#if DISPLAY_DSP_RECV_DATA_EN
            //DBG_DIS("RL2=%d\r\n",Rx_len);
            recv_all_len+=Rx_len;
            recv_all_len_M=recv_all_len/(1*1024*1024);

            if(recv_all_len_M_bef!=recv_all_len_M)
            {
                recv_all_len_M_bef=recv_all_len_M;
                DBG_DIS("M=%d,RL2=%d\r\n",recv_all_len_M,Rx_len);
            }
#endif
            dsp_net_recv_data_to_cyc_buf_n2 (dsp_Data_Buffer,Rx_len);
            Rx_len=0;
        }
    }
//    else
//    {
//        MY_DELAY_X_MS(10);
//    }
    if (big_img_flag)
    {
        MY_DELAY_X_MS(10);
    }
    else
    {
        MY_DELAY_X_MS(5);
    }



}

/***********************************************************************************************
* Function Name	: Analyse_DSPProtocol_Frame
* Description	: 解析DSP发来的协议帧
* Input			: *pFrame   协议帧指针
* Return		: _ABANDON
                  _FCODE_ERR
                  _SUM_CHK_ERR
                  _TAIL_ERR
                  _NO_ERR
* Note(s)		: 帧起始符、帧头校验未通过的直接丢弃
* Contributor	: 141106	code
***********************************************************************************************/
_err_no Analyse_DSPProtocol_Frame(INT8U *pFrame)
{
    INT16U cout;
    INT8U cs=0;

    // 帧起始符校验
    if( *(INT16U *)(pFrame+FRAME_HEAD_OFFSET) != 0x02A0 )
        return _ABANDON;

    // 帧头校验
    for(cout=0;cout<FRAME_HEADSUM_OFFSET;cout++)
        cs += *(pFrame+cout);
    if(cs != *(pFrame+FRAME_HEADSUM_OFFSET))
        return _ABANDON;

    // 功能码校验
    if((*(pFrame+FRAME_ATTRIBUTE_OFFSET) & 0x1F) > 3)
        return _FCODE_ERR;

    // 数据域校验
    switch(*(INT16U *)(pFrame+FRAME_CMD_ID_OFFSET) )
    {
        case 0x000B:            // DSP的boot初始化结果，目前该命令做在马达ARM里
            switch(*(pFrame + FRAME_DATA_OFFSET))
            {
                case 0:
                case 1:
                case 2:
                    break;
                default:
                    return _ID_DATA_ERR;
            }
        case 0x3102:            // DSP升级结果
            switch(*(INT16U *)(pFrame + FRAME_DATA_OFFSET))
            {
                case 0x0001:
                case 0x0002:
                case 0x0003:
                case 0x0101:
                case 0x0102:
                case 0x0103:
                case 0x0201:
                case 0x0202:
                case 0x0203:
                    break;
                default:
                    return _ID_DATA_ERR;
            }
        case 0x3201:
            break;
        case 0x0002:
        case 0x0003:
        case 0x0010:
        case 0x00E0:
        case 0x00E1:
        case 0x0001:
        case 0x000A:
            if((*(pFrame+FRAME_ATTRIBUTE_OFFSET) & 0x1F) != 2)
                return _OP_ID_ERR;
            break;
        default:
            return _ID_ERR;
    }

    // 整帧校验
    for(;cout<(*(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)+FRAMEHEADLEN);cout++)
        cs += *(pFrame+cout);
    if(cs != *(pFrame+cout++))
        return _SUM_CHK_ERR;

    // 帧尾校验
    if(*(pFrame+cout) != 0x16)
        return _TAIL_ERR;

    return _NO_ERR;
}

/***********************************************************************************************
* Function Name	: Mk_Ack
* Description	: 生成应答帧
* Input			: *pFrame   协议帧指针
                  ack_flag  应答标志:   _NO_ERR
                                        _SUM_CHK_ERR
                                        _TAIL_ERR
                                        _ID_ERR
                                        _ID_DATA_ERR
                                        _FCODE_ERR
                                        _OP_ID_ERR
* Return		:
* Note(s)		:
* Contributor	: 141106	code
***********************************************************************************************/
void Mk_Ack(INT8U DevNum,INT8U *pFrame,_err_no ack_flag)
{
    INT8U FrameBuf[21]={0};
    INT16U cout;
    INT8U cs=0;

    FRAME_FIX_INFO(FrameBuf);      // 固定部分填充
    *(FrameBuf+FRAME_ACK_OFFSET) = ack_flag;
    *(INT32U *)(FrameBuf+FRAME_DATALEN_OFFSET) = 0x00000002;
    *(INT16U *)(FrameBuf+FRAME_INDEX_OFFSET) = *(INT16U *)(pFrame+FRAME_INDEX_OFFSET);

    // 帧头校验和
    for(cout=0;cout<FRAME_HEADSUM_OFFSET;cout++)
        cs += FrameBuf[cout];
    FrameBuf[FRAME_HEADSUM_OFFSET] = cs;

    // ID
    *(INT16U *)(FrameBuf+FRAME_CMD_ID_OFFSET) = *(INT16U *)(pFrame+FRAME_CMD_ID_OFFSET);

    // 整帧校验和
    for(;cout<(*(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)+FRAMEHEADLEN);cout++)
        cs += FrameBuf[cout];
    FrameBuf[cout++] = cs;

    // 帧尾
    FrameBuf[20] = 0x16;

    // 发送应答帧
    BSP_W5500Send(DevNum,FrameBuf,21);
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
    _UpdtFixFmt *UpdtFixFmt = (_UpdtFixFmt *)(pFrame+FRAME_DATA_OFFSET);

    TotlSegNum = UpdtMsg->DataLen/16000 + ((UpdtMsg->DataLen%16000)?1:0);     // 总段数

    UpdtFixFmt->Targt = *(UpdtMsg->pData)++;
    UpdtFixFmt->TotlSeg = TotlSegNum;
    UpdtFixFmt->TotlLen = UpdtMsg->DataLen;

    FRAME_FIX_INFO(pFrame);                          // 固定部分填充
    *(pFrame+FRAME_ACK_OFFSET) = REQ_ACK;            // 要求应答
    *(pFrame+FRAME_ATTRIBUTE_OFFSET) = ARM_WR_DSP;       // 写操作

    TickVal = OSTimeGet();


    DBG_INFO("in SndDSP_UpdtCMD 1:update dsp ,send data start,totalseg = %d\r\n",TotlSegNum);

    do
    {
    	DBG_INFO("This is %dst segment!\r\n",CurSeg);
        *(INT16U *)(pFrame+FRAME_INDEX_OFFSET) = ((FramNum++) | 0x8000);   // 帧序号

        // 数据域长度
        if(UpdtMsg->DataLen > 16000)
        {
            *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET) = 16000+2+UPDATEHEADLEN;
            UpdtMsg->DataLen -= 16000;
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

        unsigned char i=0;
        // 每发一包数据都需要等待DSP的应答，确保数据已被收到
        for(;;)
        {
            Rx_lens = BSP_W5500Recv(UpdtMsg->DivNum,RecvBuff,sizeof(RecvBuff));
            if(Rx_lens > 0)
            {
                if(RecvBuff[FRAME_ACK_OFFSET] != 0)
                {
                	DBG_ERR("Get error ACK,errno is %d \r\n",RecvBuff[FRAME_ACK_OFFSET]);
                    if(i >= 3)  // 重发3次不成功返回升级失败
                    {
                        PraperLevelUpFail();
                        return;
                    }
                    i++;
                    BSP_W5500Send(UpdtMsg->DivNum,pFrame,Tx_len);
                }
                else
                    break;
            }
            else if(0==Rx_lens)
            {
                ;
            }
            else
            {
                if(i >= 3)  // 重发3次不成功返回升级失败
                {
                    PraperLevelUpFail();
                    return;
                }
                i++;
                BSP_W5500Send(UpdtMsg->DivNum,pFrame,Tx_len);
            }
        }

    }
    while(UpdtMsg->DataLen);

    TickVal = OSTimeGet() - TickVal;

    DBG_INFO("in SndDSP_UpdtCMD 100:update dsp ,send data    end\r\n");
    Print_UpdataProgress(2);

}






/***********************************************************************************************
* Function Name	: SndDSP_VersionGet
* Description	:  DSP版本取得
* Input			: *tmp_pFrame   协议帧指针
                  *BootMsg  update消息结构体
* Return		:
* Note(s)		: pData指针指向的第一个数据是升级标志
* Contributor	: 141106	code
***********************************************************************************************/
void SndDSP_VersionGet(INT8U *pFrame,INT8U divnum)
{
    int tx_len;
    
    memset(tmp_frame, 0 ,sizeof(tmp_frame));

    DBG_DIS("in SndDSP_VersionGet 0:\r\n");

    tx_len=create_one_net_send_frame(tmp_frame,sizeof(tmp_frame),ID_DSP_VER,PARA_FUN_RD,ACK_NEED_RET,NULL,0);
    if(tx_len>0)
    {
        send_dsp_data_by_dev(divnum,tmp_frame,tx_len);
        DBG_DIS("in SndDSP_VersionGet 1:id=0x%04x\r\n",ID_DSP_VER);
    }
    else
    {
        DBG_DIS("in SndDSP_VersionGet 2:\r\n");
    }

    DBG_DIS("in SndDSP_VersionGet 100:\r\n");

}




/***********************************************************************************************
* Function Name	: SndDSP_CheckCSCMD
* Description	:  发送校正命令
* Input			: *pFrame   协议帧指针
                  *BootMsg  update消息结构体
* Return		:
* Note(s)		:
* Contributor	: 141106	code
***********************************************************************************************/
void SndDSP_CheckCSCMD(INT8U *pFrame,_BSP_MESSAGE *AdjuMsg)
{
    int tx_len;
	memset(tmp_frame, 0 ,sizeof(tmp_frame));

    tx_len=create_one_net_send_frame(tmp_frame,sizeof(tmp_frame),ID_DSP_CS_CHECK,PARA_FUN_WT,ACK_NOT_RET,NULL,0);
    if(tx_len>0)
    {
        send_dsp_data_by_dev(AdjuMsg->DivNum,tmp_frame,tx_len);
        DBG_INFO("in SndDSP_AdjustCMD 1:id=0x%04x\r\n",ID_DSP_CS_CHECK);
    }

}




/****************************************************************************************************
**名称:void SndDSP_BIGIMAGECMD(INT8U *pFrame,_BSP_MESSAGE *VerMsg)
**功能:查看大图指令
* 入口:无
* 出口:无
**auth:hxj, date: 2015-5-13 10:24
*****************************************************************************************************/
void SndDSP_BIGIMAGECMD(INT8U *pFrame,_BSP_MESSAGE *picmsg)
{
    int tx_len;
    
    if(picmsg->DataLen > MAX_PACKET_NUM)
        return;
    
    memset(tmp_frame, 0 ,sizeof(tmp_frame));
        
    // 显示终端
    Pic_Md = *pFrame = picmsg->DivNum;

    // 总张
    *(pFrame + 1) = picmsg->DataLen;

    // 每张标志
    memcpy(pFrame + 2,picmsg->pData,picmsg->DataLen);
    
    tx_len=create_one_net_send_frame(tmp_frame,sizeof(tmp_frame),ID_A8_TRIGGER_DSP_SEND_BIG_IMG,PARA_FUN_WT,ACK_NOT_RET,(char *)pFrame,picmsg->DataLen + 2);
    if(tx_len>0)
    {
        send_dsp_data_by_dev(DSP2,tmp_frame,tx_len);
        DBG_INFO("in Deal_DSPData 1:send big trigger cmd,id=0x%04x\r\n",ID_A8_TRIGGER_DSP_SEND_BIG_IMG);
    }


#if 0 /*hxj amend,date 2015-5-12 19:5*/
    Operat_Socket_INT_Indiv(VerMsg->DivNum,IMR_CON | IMR_DISCON | IMR_TIMEOUT);     // 暂时关闭接收中断
    img_len=0;
    g_dsp_send_big_img_over_flag=0;
    for(;;)
    {
        Rx_lens = BSP_W5500Recv(VerMsg->DivNum,S_Data_Buffer[VerMsg->DivNum]);
        if(Rx_lens > 0)
        {
            if(W5500B==VerMsg->DivNum)
            {
                #if DEF_DBG_RECV_DSP_DATA_EN
                    DBG_INFO("in Deal_DSPData 2:Rx_len=%d",Rx_lens);
                #endif

                img_len+=Rx_lens;
                img_m_len=img_len/(2*1024*1024);
                if(img_m_len_bef != img_m_len)
                {
                    DBG_DIS("img_len=%d,img_m_len=%d,dds=%d\r\n",img_len,img_m_len,GET_SYS_DDS(NULL));
                    img_m_len_bef=img_m_len;
                }

                dsp_net_recv_data_to_cyc_buf_n2 (S_Data_Buffer[VerMsg->DivNum],Rx_lens);
                if(*(INT16U *)(S_Data_Buffer[VerMsg->DivNum]+Rx_lens-22) == 0x3508)
                {
                    DBG_DIS("img_len=%d,img_m_len=%d,dds=%d,over\r\n",img_len,img_m_len,GET_SYS_DDS(NULL));
                    g_dsp_send_big_img_over_flag=1;
                    break;
                }

                Rx_lens=0;
            }
        }
        else
        {
            MY_DELAY_X_MS(1);
        }
    }
	Get_Socket_INTType(W5500B);
    Operat_Socket_INT_Indiv(W5500B,IMR_CON | IMR_DISCON | IMR_RECV | IMR_TIMEOUT); // 重新打开接收中断


#endif



}

static void SndDSP_BlackListCMD(INT8U *pFrame,_BSP_MESSAGE *blacklist)
{
    INT16U Tx_len;
    INT16U cout;
    INT8U cs=0;

    FRAME_FIX_INFO(pFrame);      // 固定部分填充
    *(pFrame+FRAME_ACK_OFFSET) = REQ_ACK;            // 要求应答
    *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET) = blacklist->DataLen + 2;
    *(pFrame+FRAME_ATTRIBUTE_OFFSET) = ARM_WR_DSP;       // 读操作
    *(INT16U *)(pFrame+FRAME_INDEX_OFFSET) = ((FramNum++) | 0x8000);   // 帧序号

    // 帧头校验和
    for(cout=0;cout<FRAME_HEADSUM_OFFSET;cout++)
        cs += *(pFrame+cout);
        *(pFrame+FRAME_HEADSUM_OFFSET) = cs;

    // ID
    *(INT16U *)(pFrame+FRAME_CMD_ID_OFFSET) = 0x3801;

    // 数据域
    memcpy(pFrame+FRAME_DATA_OFFSET,blacklist->pData,blacklist->DataLen);

    // 整帧校验和
    for(;cout<(*(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)+FRAMEHEADLEN);cout++)
        cs += *(pFrame+cout);
        *(pFrame+cout++) = cs;

    // 帧尾
    *(pFrame+cout++) = 0x16;

    // 帧长计算
    Tx_len = cout;

    BSP_W5500Send(DSP2,pFrame,Tx_len);
}


static void SndDSP_BlackListPowerOnCMD(INT8U *pFrame, INT8U *pData,INT32U DataLen)
{
    INT16U Tx_len;
    INT16U cout;
    INT8U cs=0;

    FRAME_FIX_INFO(pFrame);      // 固定部分填充
    *(pFrame+FRAME_ACK_OFFSET) = REQ_ACK;            // 要求应答
    *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET) = DataLen + 2;
    *(pFrame+FRAME_ATTRIBUTE_OFFSET) = ARM_WR_DSP;       // 读操作
    *(INT16U *)(pFrame+FRAME_INDEX_OFFSET) = ((FramNum++) | 0x8000);   // 帧序号

    // 帧头校验和
    for(cout=0;cout<FRAME_HEADSUM_OFFSET;cout++)
        cs += *(pFrame+cout);
        *(pFrame+FRAME_HEADSUM_OFFSET) = cs;

    // ID
    *(INT16U *)(pFrame+FRAME_CMD_ID_OFFSET) = 0x3801;

    // 数据域
    memcpy(pFrame+FRAME_DATA_OFFSET,pData,DataLen);

    // 整帧校验和
    for(;cout<(*(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)+FRAMEHEADLEN);cout++)
        cs += *(pFrame+cout);
        *(pFrame+cout++) = cs;

    // 帧尾
    *(pFrame+cout++) = 0x16;

    // 帧长计算
    Tx_len = cout;

    BSP_W5500Send(DSP2,pFrame,Tx_len);
}

int send_dsp1_data(uint8 *buf,uint32 len)
{
    int ret=0;
    if(0==dsp1_net_init_ok_flag) return 0;


    ret=BSP_W5500Send(DSP1,buf,len);
    if(ret !=len)
    {
        DBG_PRO("d1err=%d,ret=%d\r\n",len,ret);
    }
    else
    {
        DBG_PRO("d1ok=%d\r\n",len);
    }


    return len;
}


int send_dsp2_data(uint8 *buf,uint32 len)
{
    int ret=0;
    if(0==dsp2_net_init_ok_flag) return 0;


    ret=BSP_W5500Send(DSP2,buf,len);
    if(ret !=len)
    {
        DBG_PRO("d2err=%d,ret=%d\r\n",len,ret);
    }
    else
    {
        //DBG_PRO("d2ok=%d\r\n",len);
    }



    return len;
}


int send_dsp_data_by_dev(uint8 devnum,uint8 *buf,uint32 len)
{

    if(devnum==DSP1)
    {
        return send_dsp1_data(buf,len);
    }
    else if(devnum==DSP2)
    {
        return send_dsp2_data(buf,len);
    }

    return 0;

}

//#endif


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
            DBG_INFO("Update DSP fail!\r\n");
            PraperLevelUpFail();
            break;
        case 0x0002:    // FPGA升级失败
            DBG_INFO("Update FPGA fail!\r\n");
            PraperLevelUpFail();
            break;
        case 0x0003:    // 人民币模板升级失败
            DBG_INFO("Update CNY fail!\r\n");
            PraperLevelUpFail();
            break;
        case 0x0101:    // DSP擦除成功
            Print_UpdataProgress(3);
            break;
        case 0x0201:    // DSP写入成功
            Print_UpdataProgress(4);
            break;
        case 0x0301:    // DSP读取校验成功，即升级成功
        case 0x0401:
            DBG_INFO("Update DSP ok!\r\n");
            PraperLevelUpED();
            break;
        case 0x0102:    // FPGA升级成功
        case 0x0402:
            DBG_INFO("Update FPGA ok!\r\n");
            PraperLevelUpED();
            break;
        case 0x0103:        // 模板版号写入成功
            Print_UpdataProgress(3);
            break;
        case 0x0203:        // 模板擦除成功
            Print_UpdataProgress(4);
            break;
        case 0x0303:        // 模板写入成功
            Print_UpdataProgress(5);
            break;
        case 0x0403:        // 人民币模板读取校验成功，即升级成功
            DBG_INFO("Update CNY ok!\r\n");
            PraperLevelUpED();
            break;
        default:        // ID_DATA内容不对
            DBG_INFO("Update fail!\r\n");
            PraperLevelUpFail();
            break;
    }

}

/*****************************************************************************
 函 数 名  : GetComWithDSPState
 功能描述  : 获取A8与DSP通信状态
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年8月10日
    作    者   : Xieyb
    修改内容   : 新生成函数

*****************************************************************************/
INT8U GetComWithDSPState(void)
{
	return Poweron_control.DSPInitFlag;
}

XBUFFER_S *xBuffer_Create(INT32U size,INT8U prio)
{
    XBUFFER_S *p;

    p = (XBUFFER_S *)malloc(sizeof(XBUFFER_S));

    p->ptr = (INT8U *)malloc(size);

    INT8U err;
    p->mutex = OSMutexCreate(prio,&err);

    p->p_r = p->p_w = 0;
    p->tot_len = size;
    p->len = 0;

    return p;
}

INT8U xBuffer_Free(XBUFFER_S *p)
{
    if(p == NULL)
        return FALSE;

    INT8U err;
    if(OSMutexDel(p->mutex,OS_DEL_NO_PEND,&err) != NULL)
        return FALSE;

    free(p->ptr);
    free(p);

    return TRUE;
}

INT8U xBuffer_Write(XBUFFER_S *p, INT8U *ptr, INT32U size)
{
    if(p == NULL)
        return FALSE;

    INT8U err;
    OSMutexPend(p->mutex,0,&err);

    // 可写空间检测
    if((p->tot_len - p->len) < size)
    {
        OSMutexPost(p->mutex);
        return FALSE;
    }

    // 数据导入缓冲区
    if((p->tot_len - p->p_w) >= size)
    {
        memcpy(p->ptr + p->p_w,ptr,size);
        p->p_w += size;
        if(p->p_w == p->tot_len)
            p->p_w = 0;
    }
    else
    {
        memcpy(p->ptr + p->p_w,ptr,p->tot_len - p->p_w);
        memcpy(p->ptr,ptr + p->tot_len - p->p_w,size + p->p_w - p->tot_len);

        p->p_w = size + p->p_w - p->tot_len;
    }

    p->len += size;

    OSMutexPost(p->mutex);

    return TRUE;
}

INT8U xBuffer_Read(XBUFFER_S *p, INT8U *ptr, INT32U size)
{
    if(p == NULL)
        return FALSE;

    INT8U err;
    OSMutexPend(p->mutex,0,&err);

    // 可读长度检测
    if(size > p->len)
    {
        OSMutexPost(p->mutex);
        return FALSE;
    }

    // 数据从缓冲区导出
    if((p->tot_len - p->p_r) >= size)
    {
        memcpy(ptr,p->ptr + p->p_r,size);
        p->p_r += size;
        if(p->p_r == p->tot_len)
            p->p_r = 0;
    }
    else
    {
        memcpy(ptr,p->ptr + p->p_r,p->tot_len - p->p_r);
        memcpy(ptr + p->tot_len - p->p_r,p->ptr,size + p->p_r - p->tot_len);
        p->p_r = size + p->p_r - p->tot_len;
    }

    p->len -= size;

    OSMutexPost(p->mutex);

    return TRUE;
}

INT8U xBuffer_Occupied_Size(XBUFFER_S *p, INT32U *size)
{
    if(p == NULL)
        return FALSE;

    INT8U err;
    OSMutexPend(p->mutex,0,&err);

    *size = p->len;

    OSMutexPost(p->mutex);

    return TRUE;
}








/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/

