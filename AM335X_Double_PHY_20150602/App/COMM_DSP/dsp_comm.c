/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: dsp_comm.c
* Author			: CODE
* Date First Issued	: 141030
* Version			: V
* Description		:
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
*                     20141123 - Э���ʽ����
                      20150212 - W5500�����жϴ�����ʽ by code
                      20150305 - ��ӳ�ʱ����
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
OS_EVENT *W5500AEvent;          // W5500A�¼����ƿ�
void *W5500OAOSQ[4];             // W5500A��Ϣ����
#endif

OS_EVENT *W5500BEvent;          // W5500B�¼����ƿ�
void *W5500OBOSQ[4];             // W5500B��Ϣ����

INT16U FramNum;                         // ֡���
static INT8U S_Data_Buffer[2][4096];    // socket�շ�����

INT32U TickVal;                         // ���ڼ�ʱ

uint8 dsp1_net_init_ok_flag=0;
uint8 dsp2_net_init_ok_flag=0;


/* Private functions--------------------------------------------------------------------------*/
#if DBG_DSP_RECV_TWO_EN
void TaskComWithDSP1(void *pdata)
{   
    // ��Ļ��ر�������
    static INT8U    NetFlag = 0;
    static          _BSP_MESSAGE NetMessage;
    
    // W5500��ر�������
    INT8U               perr;
    _BSP_MESSAGE        *pW5500Msg;
    _BSPW5500_CONFIG    W5500_InitStructure;
    _BSPGPIO_CONFIG     GPIO_InitStructure;
    
	INT8U Detination_IP[4]	={192,168,11,131};					// Ŀ��IP
	INT8U Source_IP[4]		={192,168,11,35};					// ԴIP(������)
	INT8U Gateway_IP[4]		={192,168,11,1};					// ����
	INT8U Subnet_Mask[4]	={255,255,255,0};					// ��������
	INT8U MAC_Addr[6]		={0x48,0x53,0x00,0x57,0x55,0x00};	// MAC

    dsp_net_init_buf();

    // �ж��߳�ʼ��
    GPIO_InitStructure.PortNum = PORT1;
    GPIO_InitStructure.PinNum = 20;
    GPIO_InitStructure.Dir = GPIO_DIR_I;
    GPIO_InitStructure.IntLine = GPIO_INTA;
    GPIO_InitStructure.IntType = GPIO_INT_TYPE_L;
    GPIO_InitStructure.pEvent = W5500AEvent;
    BSP_GPIOConfig(&GPIO_InitStructure);
    
    // W5500��ʼ��
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
                case IR_CON:        // ���ӳɹ�
                    // ��ȡDSP�汾��
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
                case IR_DISCON:     // ���ӶϿ�
                case IR_TIMEOUT:    // ��ʱ
                    // ���¿�ʼ����
        			Disconnect(W5500A);
                    BSP_W5500Socket(W5500A,MR_TCP|ND_MC_MMB);
                    BSP_SocketListen(W5500A);
                    break;
                case IR_RECV:       // ��������Ҫ����
                case SEND_REQ:      // ��������Ҫ����
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
    // ��Ļ��ر�������
    static INT8U    NetFlag = 0;
    static          _BSP_MESSAGE NetMessage;
    
    INT8U perr;
    _BSP_MESSAGE  *pW5500Msg;
    _BSPW5500_CONFIG W5500_InitStructure;
    _BSPGPIO_CONFIG GPIO_InitStructure;

	INT8U Detination_IP[4]	={192,168,16,131};					// Ŀ��IP
	INT8U Source_IP[4]		={192,168,16,35};					// ԴIP(������)
	INT8U Gateway_IP[4]		={192,168,16,1};					// ����
	INT8U Subnet_Mask[4]	={255,255,255,0};					// ��������
	INT8U MAC_Addr[6]		={0x48,0x53,0x00,0x57,0x55,0x00};	// MAC
     
    // �ж��߳�ʼ��
    GPIO_InitStructure.PortNum = PORT1;
    GPIO_InitStructure.PinNum = 22;
    GPIO_InitStructure.Dir = GPIO_DIR_I;
    GPIO_InitStructure.IntLine = GPIO_INTB;
    GPIO_InitStructure.IntType = GPIO_INT_TYPE_L;
    GPIO_InitStructure.pEvent = W5500BEvent;
    BSP_GPIOConfig(&GPIO_InitStructure);
    
    // W5500��ʼ��
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
                case IR_CON:        // ���ӳɹ�
                    // ��ȡDSP�汾��
                    NetFlag = 1;
                    NetMessage.DivNum = 1;
                    NetMessage.MsgID = APP_DISP_COMFROM_DSP;
                    NetMessage.pData = &NetFlag;
                    NetMessage.DataLen = 1;
                    SYSPost(DispTaskEvent,&NetMessage);
                    MY_DELAY_X_S(1);
                    SndDSP_VersionGet(S_Data_Buffer[W5500B],&NetMessage);
                    break;
                case IR_DISCON:     // ���ӶϿ�
                case IR_TIMEOUT:    // ��ʱ
                    // ���¿�ʼ����
        			Disconnect(W5500B);
                    BSP_W5500Socket(W5500B,MR_TCP|ND_MC_MMB);
                    BSP_SocketListen(W5500B);
                    break;
                case IR_RECV:       // ��������Ҫ����
                case SEND_REQ:      // ��������Ҫ����
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
    lwipIfPort2.ipMode = IPADDR_USE_STATIC; //ʹ�þ�̬����IP


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
* Description	: ��?���騺y?Y��?��|����o����y?��?��
* Input			: DevNum    ������?o?
                  *pMsg     ???��
* Return		:
* Note(s)		:
* Contributor	: 150112	code
***********************************************************************************************/
INT8U Deal_DSPData(INT8U DevNum,_BSP_MESSAGE *pTxMsg)
{
	INT16U          Rx_len;

    if((pTxMsg->MsgID & MAIN_ID_MASK) == SEND_REQ)             // ����Ƿ�������
    {
        switch(pTxMsg->MsgID & SUB_ID_MASK)
        {
            case SUB_ID_UPDATE:            // ������������
                SndDSP_UpdtCMD(S_Data_Buffer[DevNum],pTxMsg);
                break;
            case SUB_ID_ADJ:            // CSУ��
                SndDSP_CheckCSCMD(S_Data_Buffer[DevNum],pTxMsg);
                break;
            case SUB_ID_BIG_IMAGE:        // ��ͼ�鿴
                SndDSP_BIGIMAGECMD(S_Data_Buffer[DevNum],pTxMsg);
                break;
            default:
                break;
        }
    }

    else if((pTxMsg->MsgID & MAIN_ID_MASK) == IR_RECV)     // ����ǽ�������
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
* Description	: �ж�Э��֡���
* Input			: *pFrame   Э��ָ֡��
* Return		: _ASWR_FRAME      Ӧ��֡
                  _NORM_FREAM     ��ͨ֡
* Note(s)		: Ӧ��֡�Ĵ��������δ���
* Contributor	: 141106	code
***********************************************************************************************/

_Frame_Cat Judge_Ack(INT8U *pFrame)
{
    _Frame_Cat  frame_category;

    switch(*(pFrame+FRAME_ACK_OFFSET))
    {
        case 0x00:      // Ӧ��֡
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
            frame_category = _ASWR_FRAME;
            // �������Ӧ��֡�������
            break;
        case 0xff:      // ��ͨ֡��Ҫ��Ӧ��
            frame_category = _NOR_FRAME_ACK;
            break;
        case 0xbb:      // ��ͨ֡����Ҫ��Ӧ��
            frame_category = _NOR_FRAM_NACK;
            break;
        default:
            break;
    }

    return frame_category;
}

/***********************************************************************************************
* Function Name	: RecDSP_UpdtRT
* Description	:  ARM��ͼ����������
* Input			: *pFrame   Э��ָ֡��
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
        case 0x0001:    // DSP����ʧ��
			PraperLevelUpFail();
            break;
        case 0x0002:    // FPGA����ʧ��
			PraperLevelUpFail();
            break;
        case 0x0003:    // �����ģ������ʧ��
			PraperLevelUpFail();
            break;
        case 0x0101:    // DSP�����ɹ�
            PraperLevelUpED();
            break;
        case 0x0102:    // FPGA�����ɹ�
            PraperLevelUpED();
            break;
        case 0x0103:    // �����ģ�������ɹ�
            PraperLevelUpED();
            break;
        case 0x0201:    // DSP��֧�ֵ�������־
            break;
        case 0x0202:    // FPGA��֧�ֵ�������־
            break;
        case 0x0203:    // �����ģ�岻֧�ֵ�������־
            break;
        default:        // ID_DATA���ݲ���
            break;
    }

}

/***********************************************************************************************
* Function Name	: SndDSP_UpdtCMD
* Description	:  ������������
* Input			: *pFrame   Э��ָ֡��
                  *BootMsg  update��Ϣ�ṹ��
* Return		:
* Note(s)		: pDataָ��ָ��ĵ�һ��������������־
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

    TotlSegNum = UpdtMsg->DataLen/4000 + ((UpdtMsg->DataLen%4000)?1:0);     // �ܶ���

    UpdtFixFmt->Targt = *(UpdtMsg->pData)++;
    UpdtFixFmt->TotlSeg = TotlSegNum;
    UpdtFixFmt->TotlLen = UpdtMsg->DataLen;

    FRAME_FIX_INFO(pFrame);                          // �̶��������
    *(pFrame+FRAME_ACK_OFFSET) = REQ_ACK;            // Ҫ��Ӧ��
    *(pFrame+FRAME_ATTRIBUTE_OFFSET) = ARM_WR_DSP;       // д����

    TickVal = OSTimeGet();
    Operat_Socket_INT_Indiv(W5500B,IMR_CON | IMR_DISCON | IMR_TIMEOUT);     // ��ʱ�رս����ж�
    do
    {
        *(INT16U *)(pFrame+FRAME_INDEX_OFFSET) = ((FramNum++) | 0x8000);    // ֡���

        // �����򳤶�
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

        // ֡ͷУ���
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

        // ��֡У���
        for(;cout<(*(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)+FRAMEHEADLEN);cout++)
            cs += *(pFrame+cout);
        *(pFrame+cout++) = cs;
        cs = 0;

        // ֡β
        *(pFrame+cout++) = 0x16;

        // ֡������
        Tx_len = cout;

        BSP_W5500Send(UpdtMsg->DivNum,pFrame,Tx_len);
        
        // ÿ��һ�����ݶ���Ҫ�ȴ�DSP��Ӧ��ȷ�������ѱ��յ�
        for(;;)
        {
            Rx_lens = BSP_W5500Recv(UpdtMsg->DivNum,RecvBuff);
            if(Rx_lens > 0)
            {
                // ֡����ж�
                fCatgry = Judge_Ack(RecvBuff);

                // �ж��Ƿ���Ӧ��֡
                if(fCatgry == _ASWR_FRAME)
                    break;
            }
        }
    }while(UpdtMsg->DataLen);

    Operat_Socket_INT_Indiv(W5500B,IMR_CON | IMR_DISCON | IMR_RECV | IMR_TIMEOUT); // ���´򿪽����ж�
    TickVal = OSTimeGet() - TickVal;
}

/***********************************************************************************************
* Function Name	: SndDSP_VersionGet
* Description	:  DSP�汾ȡ��
* Input			: *pFrame   Э��ָ֡��
                  *BootMsg  update��Ϣ�ṹ��
* Return		:
* Note(s)		: pDataָ��ָ��ĵ�һ��������������־
* Contributor	: 141106	code
***********************************************************************************************/
void SndDSP_VersionGet(INT8U *pFrame,_BSP_MESSAGE *AdjuMsg)
{
    INT16U Tx_len;
    INT16U cout;
    INT8U cs=0;

    FRAME_FIX_INFO(pFrame);      // �̶��������
    *(pFrame+FRAME_ACK_OFFSET) = REQ_ACK;            // Ҫ��Ӧ��
    *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET) = 0x00000002;
    *(pFrame+FRAME_ATTRIBUTE_OFFSET) = ARM_RD_DSP;       // ������
    *(INT16U *)(pFrame+FRAME_INDEX_OFFSET) = ((FramNum++) | 0x8000);   // ֡���

    // ֡ͷУ���
    for(cout=0;cout<FRAME_HEADSUM_OFFSET;cout++)
        cs += *(pFrame+cout);
        *(pFrame+FRAME_HEADSUM_OFFSET) = cs;

    // ID
    *(INT16U *)(pFrame+FRAME_CMD_ID_OFFSET) = ID_DSP_VER;

    // ��֡У���
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
//    // ֡β
//    *(pFrame+cout++) = 0x16;

    // ֡������
    //Tx_len = cout;

    DBG_BUF_PRO(pFrame, Tx_len,"send ver cmd:");
    BSP_W5500Send(AdjuMsg->DivNum,pFrame,Tx_len);

}

/***********************************************************************************************
* Function Name	: SndDSP_CheckCSCMD
* Description	: ����CSУ������
* Input			: *pFrame   Э��ָ֡��
                  *BootMsg  version��Ϣ�ṹ��
* Return		:
* Note(s)		:
* Contributor	: 141106	code
***********************************************************************************************/
void SndDSP_CheckCSCMD(INT8U *pFrame,_BSP_MESSAGE *VerMsg)
{
    INT16U Tx_len;
    INT16U cout;
    INT8U cs=0;

    FRAME_FIX_INFO(pFrame);      // �̶��������
    *(pFrame+FRAME_ACK_OFFSET) = REQ_ACK;            // Ҫ��Ӧ��
    *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET) = 0x00000002;
    *(pFrame+FRAME_ATTRIBUTE_OFFSET) = ARM_WR_DSP;       // ������
    *(INT16U *)(pFrame+FRAME_INDEX_OFFSET) = ((FramNum++) | 0x8000);   // ֡���

    // ֡ͷУ���
    for(cout=0;cout<FRAME_HEADSUM_OFFSET;cout++)
        cs += *(pFrame+cout);
        *(pFrame+FRAME_HEADSUM_OFFSET) = cs;

    // ID
    *(INT16U *)(pFrame+FRAME_CMD_ID_OFFSET) = ID_DSP_CS_CHECK;

    // ��֡У���
    for(;cout<(*(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)+FRAMEHEADLEN);cout++)
        cs += *(pFrame+cout);
        *(pFrame+cout++) = cs;

    // ֡β
    *(pFrame+cout++) = 0x16;

    // ֡������
    Tx_len = cout;
    BSP_W5500Send(VerMsg->DivNum,pFrame,Tx_len);
}

/***********************************************************************************************
* Function Name	: SndDSP_BIGIMAGECMD
* Description	: �鿴��ͼָ��
* Input			: *pFrame   Э��ָ֡��
                  *BootMsg  version��Ϣ�ṹ��
* Return		:
* Note(s)		:
* Contributor	: 150311	songchao
***********************************************************************************************/
void SndDSP_BIGIMAGECMD(INT8U *pFrame,_BSP_MESSAGE *VerMsg)
{
    INT16U Tx_len;
    INT16U cout;
    INT8U cs=0;
    FRAME_FIX_INFO(pFrame);      // �̶��������
    *(pFrame+FRAME_ACK_OFFSET) = NREQ_ACK;            // ��Ҫ��Ӧ��
    *(INT32U *)(pFrame+FRAME_DATALEN_OFFSET) = 0x00000002;
    *(pFrame+FRAME_ATTRIBUTE_OFFSET) = ARM_WR_DSP;       // ������
    *(INT16U *)(pFrame+FRAME_INDEX_OFFSET) = ((FramNum++) | 0x8000);   // ֡���

    // ֡ͷУ���
    for(cout=0;cout<FRAME_HEADSUM_OFFSET;cout++)
        cs += *(pFrame+cout);
        *(pFrame+FRAME_HEADSUM_OFFSET) = cs;

    // ID
    *(INT16U *)(pFrame+FRAME_CMD_ID_OFFSET) = ID_DSP_BIG_IMG_START;

    // ��֡У���
    for(;cout<(*(INT32U *)(pFrame+FRAME_DATALEN_OFFSET)+FRAMEHEADLEN);cout++)
        cs += *(pFrame+cout);
        *(pFrame+cout++) = cs;

    // ֡β
    *(pFrame+cout++) = 0x16;

    // ֡������
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



/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/

