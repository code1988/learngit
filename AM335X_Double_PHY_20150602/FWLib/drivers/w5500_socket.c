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
#include "w5500_socket.h"

#define S_RX_SIZE	4096
#define S_TX_SIZE	4096

static INT8U W5500A_TxBuff[S_TX_SIZE] = {0}; 	// W5500 ���ͻ���
static INT8U W5500A_RxBuff[S_RX_SIZE] = {0}; 	// W5500 ���ջ���
static INT8U W5500B_TxBuff[S_TX_SIZE] = {0}; 	// W5500 ���ͻ���
static INT8U W5500B_RxBuff[S_RX_SIZE] = {0}; 	// W5500 ���ջ���
static _BSPSPI_CONFIG W5500_InitStructure[2];
/***********************************************************************************************
* Function Name	: Init_W5500_SPI
* Description	: SPI�ӿڳ�ʼ��
* Input			: num 	2·SPIѡ��
* Return		: 
* Note(s)		: 2·SPI����ʹ��1��Ƭѡ
* Contributor	: 140930	WB
***********************************************************************************************/
void Init_W5500_SPI(INT8U DevNum)
{      
    W5500_InitStructure[DevNum].num			= DevNum+1;                     // ӳ�䵽SPIʱ�����1
    W5500_InitStructure[DevNum].pEvent 		= NULL;
    W5500_InitStructure[DevNum].TxRespond 	= BSPSPI_RESPOND_NORMAL;	    // ��ѯģʽ
    W5500_InitStructure[DevNum].channel     = BSPSPI_CHN1;			        // 1ͨ��
    if(DevNum == W5500A)
    {
        W5500_InitStructure[DevNum].pTxBuffer 	= W5500A_TxBuff;			
        W5500_InitStructure[DevNum].pRxBuffer 	= W5500A_RxBuff;
        W5500_InitStructure[DevNum].MaxTxBuffer	= sizeof(W5500A_TxBuff);
        W5500_InitStructure[DevNum].MaxRxBuffer = sizeof(W5500A_RxBuff);
    }
    else if(DevNum == W5500B)
    {
        W5500_InitStructure[DevNum].pTxBuffer 	= W5500B_TxBuff;			
        W5500_InitStructure[DevNum].pRxBuffer 	= W5500B_RxBuff;
        W5500_InitStructure[DevNum].MaxTxBuffer	= sizeof(W5500B_TxBuff);
        W5500_InitStructure[DevNum].MaxRxBuffer = sizeof(W5500B_RxBuff);
    }
    W5500_InitStructure[DevNum].Mode        = BSPSPI_WORKMODE_MASTER;       // ��ģʽ
    W5500_InitStructure[DevNum].RxOvertime 	= SYS_DELAY_10ms;
    BSP_SPIConfig(&W5500_InitStructure[DevNum]);
}

/******************************* W5500 Write Operation *******************************/
/***********************************************************************************************
* Function Name	: Write_1_Byte
* Description	: ͨ�üĴ�����д1�ֽ�
* Input			: DevNum	2·W5500ѡ��
				  reg		ƫ�Ƶ�ַ
				  Dat		1�ֽ�����
* Return		: none
* Note(s)		: 
* Contributor	: 140930	WB
***********************************************************************************************/
void Write_1_Byte(INT8U DevNum,INT16U reg, INT8U Dat)
{

    /* Write Address */
    W5500_InitStructure[DevNum].pTxBuffer[0]=reg/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=reg;

    /* Write Control Byte */
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_WRITE|COMMON_R);
    /* Write 1 byte */
    W5500_InitStructure[DevNum].pTxBuffer[3]=Dat;

    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,4);
    BSP_SPICSDisEnable(DevNum+1);
}
/***********************************************************************************************
* Function Name	: Write_2_Byte
* Description	: ͨ�üĴ�����д2�ֽ�
* Input			: DevNum	2·W5500ѡ��
				  reg		ƫ�Ƶ�ַ
				  Dat		2�ֽ�����
* Return		: 
* Note(s)		: 
* Contributor	: 140930	WB
***********************************************************************************************/
void Write_2_Byte(INT8U DevNum,INT16U reg, INT16U Dat)
{
	
    /* Write Address */
    W5500_InitStructure[DevNum].pTxBuffer[0]=reg/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=reg;

    /* Write Control Byte */
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_WRITE|COMMON_R);
    /* Write 2 bytes */
    W5500_InitStructure[DevNum].pTxBuffer[3]=Dat/256;
    W5500_InitStructure[DevNum].pTxBuffer[4]=Dat;
    
    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,5);
    BSP_SPICSDisEnable(DevNum+1);
}
/***********************************************************************************************
* Function Name	: Write_Bytes
* Description	: ͨ�üĴ�����дsize�ֽ�
* Input			: DevNum	2·W5500ѡ��
				  reg		ƫ�Ƶ�ַ
				  *dat_ptr	����ָ��
				  size		���ݳ���
* Return		: none
* Note(s)		: 
* Contributor	: 140930	WB
***********************************************************************************************/
void Write_Bytes(INT8U DevNum,INT16U reg, INT8U *dat_ptr, INT16U size)
{
    INT16U i;

    /* Write Address */
    W5500_InitStructure[DevNum].pTxBuffer[0]=reg/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=reg;

    /* Write Control Byte */
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_WRITE|COMMON_R);

    /* Write n bytes */
    for(i=0;i<size;i++)
    {
        W5500_InitStructure[DevNum].pTxBuffer[3+i]=*dat_ptr;
        dat_ptr++;
    }
    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,size+3);
    BSP_SPICSDisEnable(DevNum+1);
}
/***********************************************************************************************
* Function Name	: Write_SOCK_1_Byte
* Description	: socket�Ĵ�����д1�ֽ�
* Input			: DevNum	2·W5500ѡ��
				  s			socketѡ��(0~7)
				  reg		ƫ�Ƶ�ַ
				  dat		1�ֽ�����
* Return		: 
* Note(s)		: 
* Contributor	: 140930	WB
***********************************************************************************************/
void Write_SOCK_1_Byte(INT8U DevNum,SOCKET s,  INT16U reg, INT8U dat)
{
    /* Write Address */
    W5500_InitStructure[DevNum].pTxBuffer[0]=reg/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=reg;
    /* Write Control Byte */
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_WRITE|(s*0x20+0x08));
    /* Write 1 byte */
    W5500_InitStructure[DevNum].pTxBuffer[3]=dat;
    
    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,4);
    BSP_SPICSDisEnable(DevNum+1);
}
/***********************************************************************************************
* Function Name	: Write_SOCK_2_Byte
* Description	: socket�Ĵ�����д2�ֽ�
* Input			: DevNum	2·W5500ѡ��
				  s			socketѡ��(0~7)
				  reg		ƫ�Ƶ�ַ
				  dat		2�ֽ�����
* Return		: 
* Note(s)		: 
* Contributor	: 140930	WB
***********************************************************************************************/
void Write_SOCK_2_Byte(INT8U DevNum,SOCKET s,INT16U reg, INT16U dat)
{
    /* Write Address */
    W5500_InitStructure[DevNum].pTxBuffer[0]=reg/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=reg;

    /* Write Control Byte */
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_WRITE|(s*0x20+0x08));

    /* Write 2 byte */
    W5500_InitStructure[DevNum].pTxBuffer[3]=dat/256;
    W5500_InitStructure[DevNum].pTxBuffer[4]=dat;
    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,5);
    BSP_SPICSDisEnable(DevNum+1);
}
/***********************************************************************************************
* Function Name	: Write_SOCK_4_Byte
* Description	: socket�Ĵ�����д4�ֽ�
* Input			: DevNum	2·W5500ѡ��
				  s			socketѡ��(0~7)
				  reg		ƫ�Ƶ�ַ
				  *dat_ptr	����ָ��
* Return		: 
* Note(s)		: 
* Contributor	: 140930	WB
***********************************************************************************************/
void Write_SOCK_4_Byte(INT8U DevNum,SOCKET s, INT16U reg, INT8U *dat_ptr)
{
    INT8U i;

    /* Write Address */
    W5500_InitStructure[DevNum].pTxBuffer[0]=reg/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=reg;

    /* Write Control Byte */
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_WRITE|(s*0x20+0x08));
    /* Write 4 bytes */
    for(i=0;i<4;i++)
    {
        W5500_InitStructure[DevNum].pTxBuffer[i+3]=*(dat_ptr+i);
    }
    
    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,7);
    BSP_SPICSDisEnable(DevNum+1);
}

/******************************* W5500 Read Operation *******************************/

/***********************************************************************************************
* Function Name	: Read_1_Byte
* Description	: ͨ�üĴ�������1�ֽ� 
* Input			: DevNum	2·W5500ѡ��
				  reg		ƫ�Ƶ�ַ
* Return		: rev		1�ֽ�����
* Note(s)		: 
* Contributor	: 140930	WB
***********************************************************************************************/
INT8U Read_1_Byte(INT8U DevNum,INT16U reg)
{
    INT8U     rev;

    /* Write Address */
    W5500_InitStructure[DevNum].pTxBuffer[0]=reg/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=reg;
    /* Write Control Byte */
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_READ|COMMON_R);

    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,3);
    BSP_SPIRead(DevNum+1,W5500_InitStructure[DevNum].pRxBuffer,1);
    BSP_SPICSDisEnable(DevNum+1);

    rev=W5500_InitStructure[DevNum].pRxBuffer[0];

    return rev;
}
/***********************************************************************************************
* Function Name	: Read_2_Byte
* Description	: ͨ�üĴ�������2�ֽ� 
* Input			: DevNum	2·W5500ѡ��
				  reg		ƫ�Ƶ�ַ
* Return		: rev		2�ֽ�����
* Note(s)		: 
* Contributor	: 140930	WB
***********************************************************************************************/
INT16U Read_2_Byte(INT8U DevNum,INT16U reg)
{
    INT16U     rev;

    /* Write Address */
    W5500_InitStructure[DevNum].pTxBuffer[0]=reg/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=reg;
    /* Write Control Byte */
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_READ|COMMON_R);

    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,3);
    BSP_SPIRead(DevNum+1,W5500_InitStructure[DevNum].pRxBuffer,2);
    BSP_SPICSDisEnable(DevNum+1);

    rev = (W5500_InitStructure[DevNum].pRxBuffer[0]<<8)|W5500_InitStructure[DevNum].pRxBuffer[1];

    return rev;
}

/***********************************************************************************************
* Function Name	: Read_SOCK_1_Byte
* Description	: socket�Ĵ�������1�ֽ�
* Input			: DevNum	2·W5500ѡ��
				  s			socketѡ��(0~7)
				  reg		ƫ�Ƶ�ַ
* Return		: rev		1�ֽ�����
* Note(s)		: 
* Contributor	: 140930	WB
***********************************************************************************************/
INT8U Read_SOCK_1_Byte(INT8U DevNum,SOCKET s, INT16U reg)
{
    INT8U     rev;

    /* Write Address */
    W5500_InitStructure[DevNum].pTxBuffer[0]=reg/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=reg;

    /* Write Control Byte */
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_READ|(s*0x20+0x08));

    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,3);
    BSP_SPIRead(DevNum+1,W5500_InitStructure[DevNum].pRxBuffer,1);
    BSP_SPICSDisEnable(DevNum+1);

    rev=W5500_InitStructure[DevNum].pRxBuffer[0];

	return rev;
}
/***********************************************************************************************
* Function Name	: Read_SOCK_2_Byte
* Description	: socket�Ĵ�������2�ֽ�
* Input			: DevNum	2·W5500ѡ��
				  s			socketѡ��(0~7)
				  reg		ƫ�Ƶ�ַ
* Return		: rev		2�ֽ�����
* Note(s)		: 
* Contributor	: 140930	WB
***********************************************************************************************/
INT16U Read_SOCK_2_Byte(INT8U DevNum,SOCKET s, INT16U reg)
{
    INT16U     rev;//,RxTemp

    /* Write Address */
    W5500_InitStructure[DevNum].pTxBuffer[0]=reg/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=reg;

    /* Write Control Byte */
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_READ|(s*0x20+0x08));

    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,3);
    BSP_SPIRead(DevNum+1,W5500_InitStructure[DevNum].pRxBuffer,2);
    BSP_SPICSDisEnable(DevNum+1);

    rev=(W5500_InitStructure[DevNum].pRxBuffer[0]<<8)|W5500_InitStructure[DevNum].pRxBuffer[1];

	return rev;
}

/***********************************************************************************************
* Function Name	: Read_SOCK_Data_Buffer
* Description	: ��socket���ջ�����
* Input			: DevNum	2·W5500ѡ��
				  s			socketѡ��(0~7)
				  *dat_ptr	������ָ��
* Return		: rx_size	���ݳ���
* Note(s)		: UDPҲ�ܵ��øú�����������ý�1460��ΪUDP��Ӧ��1472
* Contributor	: 140930	WB
***********************************************************************************************/
INT16U Read_SOCK_Data_Buffer(INT8U DevNum,SOCKET s, INT8U *dat_ptr)
{
	INT16U rx_size;
	INT16U offset, offset1;

	// �������յ������ݳ���
	rx_size=Read_SOCK_2_Byte(DevNum,s,Sn_RX_RSR);
	if(rx_size==0)		
		return 0;

	// �������ն�ָ���ַ
	offset=Read_SOCK_2_Byte(DevNum,s,Sn_RX_RD);
	offset1=offset;
	offset&=(S_RX_SIZE-1);		
    
    W5500_InitStructure[DevNum].pTxBuffer[0]=offset/256;	
    W5500_InitStructure[DevNum].pTxBuffer[1]=offset;
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_READ|(s*0x20+0x18));

    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,3);
	
    if((offset+rx_size)<S_RX_SIZE)		// �����������δ�������ջ���������ַ
    {
    	// һ����ɽ���
        memset(W5500_InitStructure[DevNum].pTxBuffer,0x00,rx_size);
        BSP_SPIRead(DevNum+1,W5500_InitStructure[DevNum].pRxBuffer,rx_size);
        BSP_SPICSDisEnable(DevNum+1);
        memcpy(dat_ptr,W5500_InitStructure[DevNum].pRxBuffer,rx_size);
        dat_ptr=dat_ptr+rx_size;
    }
	else								// ����������ݳ������ջ���������ַ
	{
		// �ֶ��ν���
		// ��һ�ν���
		offset=S_RX_SIZE-offset;
		memset(W5500_InitStructure[DevNum].pTxBuffer,0x00,offset);
        BSP_SPIRead(DevNum+1,W5500_InitStructure[DevNum].pRxBuffer,offset);
        BSP_SPICSDisEnable(DevNum+1);
        memcpy(dat_ptr,W5500_InitStructure[DevNum].pRxBuffer,offset);
        dat_ptr=dat_ptr+offset;

		// �ڶ��ν���
		W5500_InitStructure[DevNum].pTxBuffer[0]=0x00;
		W5500_InitStructure[DevNum].pTxBuffer[1]=0x00;
        W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_READ|(s*0x20+0x18));

        BSP_SPICSEnable(DevNum+1);
        BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,3);

        memset(W5500_InitStructure[DevNum].pTxBuffer,0x00,rx_size-offset);
        BSP_SPIRead(DevNum+1,W5500_InitStructure[DevNum].pRxBuffer,rx_size-offset);
        BSP_SPICSDisEnable(DevNum+1);
        memcpy(dat_ptr,W5500_InitStructure[DevNum].pRxBuffer,rx_size-offset);
	}

	// ���½��ն�ָ���ַ������
	offset1+=rx_size;
	Write_SOCK_2_Byte(DevNum,s,Sn_RX_RD, offset1);
	Write_SOCK_1_Byte(DevNum,s,Sn_CR, RECV);
	
	return rx_size;
}
/***********************************************************************************************
* Function Name	: Write_SOCK_Data_Buffer
* Description	: д���ͻ�����
* Input			: DevNum	2·W5500ѡ��
				  s			socketѡ��(0~7)
				  *dat_ptr	������ָ��
				  size		���ݳ���
* Return		: 
* Note(s)		: ���ݳ��Ȳ��ܳ��� S_TX_SIZE
* Contributor	: 140930	WB
***********************************************************************************************/
void Write_SOCK_Data_Buffer(INT8U DevNum,SOCKET s,INT8U *dat_ptr, INT16U size)
{
	INT16U offset,offset1;

	// ��������дָ���ַ
	offset=Read_SOCK_2_Byte(DevNum,s,Sn_TX_WR);
	offset1=offset;
	offset&=(S_TX_SIZE-1);		

    W5500_InitStructure[DevNum].pTxBuffer[0]=offset/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=offset;
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_WRITE|(s*0x20+0x10));
	
    if((offset+size)<S_TX_SIZE)		// ���д������δ�������ͻ���������ַ
    {
    	// һ����ɷ���
        memcpy(&W5500_InitStructure[DevNum].pTxBuffer[3],dat_ptr,size);
        BSP_SPICSEnable(DevNum+1);
        BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,size+3);
        BSP_SPICSDisEnable(DevNum+1);
    }
	else							// ���д�����ݳ������ͻ���������ַ
	{
		// �����η���
		// ��һ�η���
        offset=S_TX_SIZE-offset;
        memcpy(&W5500_InitStructure[DevNum].pTxBuffer[3],dat_ptr,offset);
        BSP_SPICSEnable(DevNum+1);
        BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,offset+3);
        BSP_SPICSDisEnable(DevNum+1);

        // �ڶ��η���
        W5500_InitStructure[DevNum].pTxBuffer[0]=0x00;
        W5500_InitStructure[DevNum].pTxBuffer[1]=0x00;  
        W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_WRITE|(s*0x20+0x10));                 
        memcpy(&W5500_InitStructure[DevNum].pTxBuffer[3],dat_ptr+offset,size-offset);
        BSP_SPICSEnable(DevNum+1); 
        BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,size-offset+3);
        BSP_SPICSDisEnable(DevNum+1);
	}


	// ���·���дָ���ַ������
	offset1+=size;
	Write_SOCK_2_Byte(DevNum,s,Sn_TX_WR, offset1);

	// ͨ��SEND����ͱ����ڷ��ͻ����е�����
	Write_SOCK_1_Byte(DevNum,s, Sn_CR, SEND);					
}

/************************(C)COPYRIGHT 2013 �㽭��̩*****END OF FILE****************************/