/*****************************************Copyright(C)******************************************
*******************************************�㽭��̩*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_W5500.c
**Ӳ          ��: am335x
**��    ��    ��: WB & code
**��  ��  ��  ��: 140930
**��  ��  ��  ��: V0.1
**��          ��: w5500����TCP/IPЭ��ջ��10/100M MAC, PHY.
                  ����ʵ����TCP��������TCP�ͻ��ˡ�UDP��3��ģʽ
                  8������socket����ͨѶ
                  spiģʽ0��3
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��:
**��          ��:
**��          ��:
**��          ��:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "bsp.h"

/* Private define-----------------------------------------------------------------------------*/

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/

_BSPW5500_CONFIG W5500_Config[2];

/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function Name	: Close_Common_INT
* Description	: �ر���ͨ�ж�
* Input			: DevNum		2·W5500ѡ��
                  IntNum        �ж�ID:     IM_IR7
                                            IM_IR6
                                            IM_IR5
                                            IM_IR4
* Return		:
* Note(s)		:
* Contributor	: 141115	CODE
***********************************************************************************************/
void Close_Common_INT(INT8U DevNum,INT8U IntNum)
{
    INT8U IntVal;
    IntVal = (~IntNum) & 0xF0;
    Write_1_Byte(DevNum,IMR,IntVal);
}

/***********************************************************************************************
* Function Name	: Open_Common_INT
* Description	: ����ͨ�ж�
* Input			: DevNum		2·W5500ѡ��
                  IntNum        �ж�ID:     IM_IR7
                                            IM_IR6
                                            IM_IR5
                                            IM_IR4
* Return		:
* Note(s)		: 
* Contributor	: 141115	CODE
***********************************************************************************************/
void Open_Common_INT(INT8U DevNum,INT8U IntNum)
{
    INT8U IntVal;
    IntVal = IntNum & 0xF0;
    Write_1_Byte(DevNum,IMR,IntVal);
}

/***********************************************************************************************
* Function Name	: Open_Socket_INT
* Description	: ��ָ��socket�ж�
* Input			: DevNum		2·W5500ѡ��
                  S             socket��
* Return		:
* Note(s)		:
* Contributor	: 141115	CODE
***********************************************************************************************/
void Open_Socket_INT(INT8U DevNum,SOCKET s)
{
    INT8U sID;

    sID = 0x01<<s;

    Write_1_Byte(DevNum,SIMR,sID);
}

/***********************************************************************************************
* Function Name	: Operat_Socket_INT_Indiv
* Description	: ����ָ��socket�ж�
* Input			: DevNum		2·W5500ѡ��
                  IntNum        �жϺ�
* Return		:
* Note(s)		: �򿪹رղ��������⣬1�򿪣�0�رգ���5λ��Ч
* Contributor	: 141115	CODE
***********************************************************************************************/
void Operat_Socket_INT_Indiv(INT8U DevNum,INT8U IntNum)
{
    INT8U IntVal;

    IntVal = IntNum & 0x1F;
    Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_IMR,IntVal);
}

/***********************************************************************************************
* Function Name	: Set_INT_IntervalTime
* Description	: 
* Input			: 
* Return		:
* Note(s)		: 
* Contributor	: 141115	CODE
***********************************************************************************************/
void Set_INT_IntervalTime(INT8U DevNum,INT16U intvtime)
{
    Write_2_Byte(DevNum,INTLEVEL,intvtime);
}

/***********************************************************************************************
* Function Name	: Set_KeepAlive
* Description	: 
* Input			: 
* Return		: 
* Note(s)		:
* Contributor	: 150305	code
***********************************************************************************************/
void Set_KeepAlive(INT8U DevNum,INT8U alivetime)
{
    Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_KPALVTR,alivetime/5);		
}

/***********************************************************************************************
* Function Name	: Set_ReTran_OutTime
* Description	: 
* Input			: 
* Return		: 
* Note(s)		:
* Contributor	: 150305	code
***********************************************************************************************/
void Set_ReTran_OutTime(INT8U DevNum,INT8U outtime)
{
    Write_1_Byte(DevNum,RTR,outtime);		
}

/***********************************************************************************************
* Function Name	: Set_ReTran_Num
* Description	: 
* Input			: 
* Return		: 
* Note(s)		:
* Contributor	: 150305	code
***********************************************************************************************/
void Set_ReTran_Num(INT8U DevNum,INT8U nums)
{
    Write_1_Byte(DevNum,RCR,nums);		
}

/***********************************************************************************************
* Function Name	: Get_Socket_INTNum
* Description	: ��ȡSocket�жϺţ����ڼ���Socket�ж���
* Input			: DevNum		2·W5500ѡ��
* Return		: S7_INT		Socket7
                  S6_INT		Socket6
                  S5_INT		Socket5
                  S4_INT		Socket4
                  S3_INT		Socket3
                  S2_INT		Socket2
                  S1_INT		Socket1
                  S0_INT		Socket0
* Note(s)		:
* Contributor	: 140930	WB
***********************************************************************************************/
INT8U Get_Socket_INTNum(INT8U DevNum)
{
  return Read_1_Byte(DevNum,SIR);

}
/***********************************************************************************************
* Function Name	: Get_Socket_INTType
* Description	: ��ȡSocket�ж�����
* Input			: DevNum		2·W5500ѡ��
* Return		: IR_SEND_OK	�������
                  IR_TIMEOUT	��ʱ
                  IR_RECV		���յ�����
                  IR_DISCON		���ܵ�FIN/ACK�ظ���
                  IR_CON		�ɹ���������
* Note(s)		:
* Contributor	: 140930	WB
***********************************************************************************************/
INT8U Get_Socket_INTType(INT8U DevNum)
{
    INT8U rev;
    rev=Read_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_IR);
    Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_IR,rev);		/* Clear IR flag */
    return rev;
}

/***********************************************************************************************
* Function Name	: Get_Socket_Status
* Description	: ��ȡSocket״̬
* Input			: DevNum			2·W5500ѡ��
				  s					socketѡ��(0~7)
* Return		: SOCK_CLOSED		�ر�״̬
                  SOCK_INIT			�˿ڴ򿪲�����TCP����ģʽ
                  SOCK_LISTEN		������ģʽ�ȴ���������
                  SOCK_ESTABLISHED	����״̬
                  SOCK_CLOSE_WAIT	�յ��Ͽ���������
                  SOCK_UDP			UDPģʽ��
                  SOCK_MACRAW		MACRAWģʽ��
* Note(s)		:
* Contributor	: 140930	WB
***********************************************************************************************/
INT8U Get_Socket_Status(INT8U DevNum)
{
    return Read_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_SR);
}

/***********************************************************************************************
* Function Name	: Disconnect
* Description	: ���ͶϿ���������
* Input			: DevNum			2·W5500ѡ��
				  s					socketѡ��(0~7)
* Return		:
* Note(s)		:
* Contributor	: 141017	code
***********************************************************************************************/
void Disconnect(INT8U DevNum)
{
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_CR,DISCON);
	W5500_Config[DevNum].State = 0;
}

/***********************************************************************************************
* Function Name : Close
* Description   : �ر�socket
* Input		  : DevNum			  2·W5500ѡ��
* Return		  :
* Note(s) 	  :
* Contributor   : 141017	  code
***********************************************************************************************/
void Close(INT8U DevNum)
{
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_CR,CLOSE);		// �ر�ָ����socket
	W5500_Config[DevNum].State = 0;
}

  /***********************************************************************************************
* Function Name	: Socket_Connect
* Description	: ����socket����TCP�ͻ���ģʽ
* Input			: DevNum			2·W5500ѡ��
* Return		: FALSE/TRUE		ʧ��/�ɹ�
* Note(s)		:
* Contributor	: 140930	WB
***********************************************************************************************/
INT32U BSP_SocketConnect(INT8U DevNum)
{
	// ����Ŀ��IP
	Write_SOCK_4_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_DIPR, W5500_Config[DevNum].D_IP);

	// ����Ŀ�Ķ˿�
	Write_SOCK_2_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_DPORTR, W5500_Config[DevNum].D_Port);

	// ����socketΪTCP�ͻ���
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_CR,CONNECT);
    OSTimeDlyHMSM(0,0,0,5);	/* Wait for a moment */

	return TRUE;
}
/***********************************************************************************************
* Function Name	: Socket_Listen
* Description	: ����socket����TCP������ģʽ
* Input			: DevNum			2·W5500ѡ��
* Return		: FALSE/TRUE		ʧ��/�ɹ�
* Note(s)		:
* Contributor	: 140930	WB
***********************************************************************************************/
INT32U BSP_SocketListen(INT8U DevNum)
{
	// ����socketΪTCP������
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_CR,LISTEN);

	// ��socket״̬�Ĵ������ж�socket�Ƿ��ڼ���״̬
	while(Read_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_SR)!=SOCK_LISTEN);

	return TRUE;
}

/***********************************************************************************************
* Function Name	: BSP_W5500Config
* Description	: W5500 spi�ӿڳ�ʼ���Լ���������
* Input			: DevNum		2·W5500ѡ��
				  *Initstruct	������Ϣ�ṹ��ָ��
* Return		: none
* Note(s)		: �����ʱĿ����ȷ��MAC��ʵд��Ĵ���
* Contributor	: 140930	WB
***********************************************************************************************/
void BSP_W5500Config(INT8U DevNum,_BSPW5500_CONFIG *Initstruct)
{
	memcpy(&W5500_Config[DevNum],Initstruct,sizeof(_BSPW5500_CONFIG));

	// spi��ʼ��
	Init_W5500_SPI(DevNum);
	OSTimeDlyHMSM(0,0,0,5);

	// Ӳ����λ
	while((Read_1_Byte(DevNum,PHYCFGR)&LINK)==0)
        OSTimeDlyHMSM(0,0,1,0);
    
	// �����λ
	Write_1_Byte(DevNum,MR, RST);
	OSTimeDlyHMSM(0,0,0,50);
   
    Set_INT_IntervalTime(DevNum,0x1000);

    Set_ReTran_Num(DevNum,2);

	// ��������
	Write_Bytes(DevNum,GAR, Initstruct->GW_IP, 4);
	OSTimeDlyHMSM(0,0,0,50);

	// ������������
	Write_Bytes(DevNum,SUBR, Initstruct->Sub_Mask, 4);
	OSTimeDlyHMSM(0,0,0,50);

	// MAC����
	Write_Bytes(DevNum,SHAR, Initstruct->MAC, 6);
	OSTimeDlyHMSM(0,0,0,50);

	// IP����
	Write_Bytes(DevNum,SIPR, Initstruct->S_IP, 4);
	OSTimeDlyHMSM(0,0,0,50);
}


/***********************************************************************************************
* Function Name	: Process_IR
* Description	: �жϴ���
* Input			: DevNum	2·W5500ѡ��
				  s 		socket��(0~7)
* Return		:
* Note(s)		:
* Contributor	: 140930	WB
***********************************************************************************************/
INT8U BSP_W5500GetIR(INT8U DevNum)
{
	INT8U flag;

	// ��ȡ�����жϵ�socket��
	flag=Get_Socket_INTNum(DevNum);

	// �ж�ָ����socket�Ƿ�����ж�
	if(flag&(0x01<<W5500_Config[DevNum].s_num))
	{
		// ��ȡsocket�ж�����
		flag=Get_Socket_INTType(DevNum);

		if(flag&IR_CON)		                                // socket�ɹ���������
		    return IR_CON;
		else if(flag&IR_DISCON)		                        // socket���ӶϿ�
		    return IR_DISCON;
        else if(flag&IR_RECV)                               // �����ж�
    	    return IR_RECV;
		else if(flag&IR_TIMEOUT)							// ��ʱ�ж�
		    return IR_TIMEOUT;
		else if(flag&IR_SEND_OK)							// �������
		    return IR_SEND_OK;        
	}
    
    return 0;
}

INT8U BSP_W5500Process(INT8U DevNum,INT8U EventNum)
{
    static INT8U S_Data_Buffer[4096]; 							// socket�շ�����
    INT16U      Rx_len;
    
    switch(EventNum)
    {
        case IR_CON:        // ���ӳɹ�
            break;
        case IR_DISCON:     // ���ӶϿ�
            // ���¿�ʼ��ʼ����
			Disconnect(DevNum);
            BSP_W5500Socket(DevNum,MR_TCP|ND_MC_MMB);
            BSP_SocketListen(DevNum);
            break;
        case IR_RECV:       // ��������Ҫ����
            Rx_len = BSP_W5500Recv(DevNum,S_Data_Buffer);
            if(Rx_len == 0)
                return 1;
            BSP_W5500Send(DevNum,S_Data_Buffer,Rx_len);
            break;
        case SEND_REQ:      // ��������Ҫ����
            BSP_W5500Send(DevNum,S_Data_Buffer,1);
            break;
        case IR_TIMEOUT:    // ��ʱ
            return 1;
        case IR_SEND_OK:    // �������
            break;
        default:
            return 1;
            
    }
    return 0;
}

/***********************************************************************************************
* Function Name	: BSP_W5500Recv
* Description	: �����ջ���������
* Input			: DevNum	2·W5500ѡ��
				  *R_Buf 	���ݴ�ŵ�ַָ��
* Return		: len		��ȡ�����ݳ���
* Note(s)		:
* Contributor	: 141029	code
***********************************************************************************************/
INT16U BSP_W5500Recv(INT8U DevNum,INT8U *R_Buf)
{
	INT16U len;

	len = Read_SOCK_Data_Buffer(DevNum,W5500_Config[DevNum].s_num,R_Buf);

    // �����UDPЭ��
	if(W5500_Config[DevNum].Prtl & MR_UDP)
	{
		if(len > 0)
		{
			// �ӽ�����������ȡ�Է�IP��ַ
			memcpy(W5500_Config[DevNum].D_IP,R_Buf,4);
			// ����ȡ�ĶԷ�IP��ַд��socket��Ŀ��IP�Ĵ���
			Write_SOCK_4_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_DIPR, W5500_Config[DevNum].D_IP);

			// �ӽ�����������ȡ�Է��˿ں�
			W5500_Config[DevNum].D_Port= R_Buf[4]*0x100+R_Buf[5];
			//����ȡ�ĶԷ��˿ں�д��socket��Ŀ�Ķ˿ڼĴ���
			Write_SOCK_2_Byte(DevNum,W5500_Config[DevNum].s_num, Sn_DPORTR, W5500_Config[DevNum].D_Port);
		}
	}

	return len;
}

/***********************************************************************************************
* Function Name	: BSP_W5500Send
* Description	: д���ݵ����ͻ�����
* Input			: DevNum	2·W5500ѡ��
				  *T_Buf	���ݴ�ŵ�ַָ��
				  len		д������ݳ���
* Return		:
* Note(s)		:
* Contributor	: 141029	code
***********************************************************************************************/
void BSP_W5500Send(INT8U DevNum,INT8U *T_Buf,INT16U len)
{
	Write_SOCK_Data_Buffer(DevNum,W5500_Config[DevNum].s_num, T_Buf, len);
}

/***********************************************************************************************
* Function Name	: w5500_socket
* Description	: socket����
* Input			: DevNum	2·W5500ѡ��
				  protocol 	socketЭ��:TCP/UDP/MACRAW(δʵ��)
* Return		:
* Note(s)		:
* Contributor	: 141029	code
***********************************************************************************************/
void BSP_W5500Socket(INT8U DevNum,INT8U protocol)
{
	// ����Դ�˿ں�
	Write_SOCK_2_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_PORT, W5500_Config[DevNum].S_Port);

    // �ر�������ͨ�ж�
    Close_Common_INT(DevNum,IM_IR4 | IM_IR5 | IM_IR6 | IM_IR7);
    // ��������ͨ�ж�
    //Open_Common_INT(DevNum,IM_IR4 | IM_IR5 | IM_IR6 | IM_IR7);

    // ��ָ��socket�ж�,Ŀǰֻʹ�õ��˿ڹ���ģʽ
	Open_Socket_INT(DevNum,W5500_Config[DevNum].s_num);

    // ��ָ��socket��ָ���ж�,�������IMR_SENDOK��ȫ����
    Operat_Socket_INT_Indiv(DevNum,IMR_CON | IMR_DISCON | IMR_RECV | IMR_TIMEOUT);
    
	// ��������䵥Ԫ
	Write_SOCK_2_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_MSSR, 1460);

	// ����RX ����ߴ�Ϊ 2K
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_RXBUF_SIZE, 0x04);

	// ����TX ����ߴ�Ϊ 2K
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_TXBUF_SIZE, 0x04);

	// ����socketЭ��
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_MR,protocol);

	// ��socket
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_CR,OPEN);

	// ��socket״̬�Ĵ������ж�socket�Ƿ���ָ��Э��ģʽ
	if(protocol & MR_TCP)
		while(Read_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_SR)!=SOCK_INIT);
	else if(protocol & MR_UDP)
		while(Read_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_SR)!=SOCK_UDP);

    Set_KeepAlive(DevNum,5);
    
	W5500_Config[DevNum].State = 1;
	W5500_Config[DevNum].Prtl  = protocol;
}

/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/
