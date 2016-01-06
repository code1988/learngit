/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_W5500.c
**硬          件: am335x
**创    建    人: WB & code
**创  建  日  期: 140930
**最  新  版  本: V0.1
**描          述: w5500集成TCP/IP协议栈，10/100M MAC, PHY.
                  驱动实现了TCP服务器、TCP客户端、UDP共3种模式
                  8个独立socket独立通讯
                  spi模式0，3
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "bsp.h"
#include "w5500_socket.h"

#define S_RX_SIZE	4096
#define S_TX_SIZE	4096

static INT8U W5500A_TxBuff[S_TX_SIZE] = {0}; 	// W5500 发送缓冲
static INT8U W5500A_RxBuff[S_RX_SIZE] = {0}; 	// W5500 接收缓冲
static INT8U W5500B_TxBuff[S_TX_SIZE] = {0}; 	// W5500 发送缓冲
static INT8U W5500B_RxBuff[S_RX_SIZE] = {0}; 	// W5500 接收缓冲
static _BSPSPI_CONFIG W5500_InitStructure[2];
/***********************************************************************************************
* Function Name	: Init_W5500_SPI
* Description	: SPI接口初始化
* Input			: num 	2路SPI选择
* Return		: 
* Note(s)		: 2路SPI都是使用1号片选
* Contributor	: 140930	WB
***********************************************************************************************/
void Init_W5500_SPI(INT8U DevNum)
{      
    W5500_InitStructure[DevNum].num			= DevNum+1;                     // 映射到SPI时必须加1
    W5500_InitStructure[DevNum].pEvent 		= NULL;
    W5500_InitStructure[DevNum].TxRespond 	= BSPSPI_RESPOND_NORMAL;	    // 轮询模式
    W5500_InitStructure[DevNum].channel     = BSPSPI_CHN1;			        // 1通道
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
    W5500_InitStructure[DevNum].Mode        = BSPSPI_WORKMODE_MASTER;       // 主模式
    W5500_InitStructure[DevNum].RxOvertime 	= SYS_DELAY_10ms;
    BSP_SPIConfig(&W5500_InitStructure[DevNum]);
}

/******************************* W5500 Write Operation *******************************/
/***********************************************************************************************
* Function Name	: Write_1_Byte
* Description	: 通用寄存器区写1字节
* Input			: DevNum	2路W5500选择
				  reg		偏移地址
				  Dat		1字节数据
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
* Description	: 通用寄存器区写2字节
* Input			: DevNum	2路W5500选择
				  reg		偏移地址
				  Dat		2字节数据
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
* Description	: 通用寄存器区写size字节
* Input			: DevNum	2路W5500选择
				  reg		偏移地址
				  *dat_ptr	数据指针
				  size		数据长度
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
* Description	: socket寄存器区写1字节
* Input			: DevNum	2路W5500选择
				  s			socket选择(0~7)
				  reg		偏移地址
				  dat		1字节数据
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
* Description	: socket寄存器区写2字节
* Input			: DevNum	2路W5500选择
				  s			socket选择(0~7)
				  reg		偏移地址
				  dat		2字节数据
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
* Description	: socket寄存器区写4字节
* Input			: DevNum	2路W5500选择
				  s			socket选择(0~7)
				  reg		偏移地址
				  *dat_ptr	数据指针
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
* Description	: 通用寄存器区读1字节 
* Input			: DevNum	2路W5500选择
				  reg		偏移地址
* Return		: rev		1字节数据
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
* Description	: 通用寄存器区读2字节 
* Input			: DevNum	2路W5500选择
				  reg		偏移地址
* Return		: rev		2字节数据
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
* Description	: socket寄存器区读1字节
* Input			: DevNum	2路W5500选择
				  s			socket选择(0~7)
				  reg		偏移地址
* Return		: rev		1字节数据
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
* Description	: socket寄存器区读2字节
* Input			: DevNum	2路W5500选择
				  s			socket选择(0~7)
				  reg		偏移地址
* Return		: rev		2字节数据
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
* Description	: 读socket接收缓冲区
* Input			: DevNum	2路W5500选择
				  s			socket选择(0~7)
				  *dat_ptr	缓冲区指针
* Return		: rx_size	数据长度
* Note(s)		: UDP也能调用该函数，不过最好将1460改为UDP对应的1472
* Contributor	: 140930	WB
***********************************************************************************************/
INT16U Read_SOCK_Data_Buffer(INT8U DevNum,SOCKET s, INT8U *dat_ptr)
{
	INT16U rx_size;
	INT16U offset, offset1;

	// 读出接收到的数据长度
	rx_size=Read_SOCK_2_Byte(DevNum,s,Sn_RX_RSR);
	if(rx_size==0)		
		return 0;

	// 读出接收读指针地址
	offset=Read_SOCK_2_Byte(DevNum,s,Sn_RX_RD);
	offset1=offset;
	offset&=(S_RX_SIZE-1);		
    
    W5500_InitStructure[DevNum].pTxBuffer[0]=offset/256;	
    W5500_InitStructure[DevNum].pTxBuffer[1]=offset;
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_READ|(s*0x20+0x18));

    BSP_SPICSEnable(DevNum+1);
    BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,3);
	
    if((offset+rx_size)<S_RX_SIZE)		// 如果读出数据未超出接收缓冲区最大地址
    {
    	// 一次完成接收
        memset(W5500_InitStructure[DevNum].pTxBuffer,0x00,rx_size);
        BSP_SPIRead(DevNum+1,W5500_InitStructure[DevNum].pRxBuffer,rx_size);
        BSP_SPICSDisEnable(DevNum+1);
        memcpy(dat_ptr,W5500_InitStructure[DevNum].pRxBuffer,rx_size);
        dat_ptr=dat_ptr+rx_size;
    }
	else								// 如果读出数据超出接收缓冲区最大地址
	{
		// 分二次接收
		// 第一次接收
		offset=S_RX_SIZE-offset;
		memset(W5500_InitStructure[DevNum].pTxBuffer,0x00,offset);
        BSP_SPIRead(DevNum+1,W5500_InitStructure[DevNum].pRxBuffer,offset);
        BSP_SPICSDisEnable(DevNum+1);
        memcpy(dat_ptr,W5500_InitStructure[DevNum].pRxBuffer,offset);
        dat_ptr=dat_ptr+offset;

		// 第二次接收
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

	// 更新接收读指针地址并保存
	offset1+=rx_size;
	Write_SOCK_2_Byte(DevNum,s,Sn_RX_RD, offset1);
	Write_SOCK_1_Byte(DevNum,s,Sn_CR, RECV);
	
	return rx_size;
}
/***********************************************************************************************
* Function Name	: Write_SOCK_Data_Buffer
* Description	: 写发送缓冲区
* Input			: DevNum	2路W5500选择
				  s			socket选择(0~7)
				  *dat_ptr	缓冲区指针
				  size		数据长度
* Return		: 
* Note(s)		: 数据长度不能超过 S_TX_SIZE
* Contributor	: 140930	WB
***********************************************************************************************/
void Write_SOCK_Data_Buffer(INT8U DevNum,SOCKET s,INT8U *dat_ptr, INT16U size)
{
	INT16U offset,offset1;

	// 读出发送写指针地址
	offset=Read_SOCK_2_Byte(DevNum,s,Sn_TX_WR);
	offset1=offset;
	offset&=(S_TX_SIZE-1);		

    W5500_InitStructure[DevNum].pTxBuffer[0]=offset/256;
    W5500_InitStructure[DevNum].pTxBuffer[1]=offset;
    W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_WRITE|(s*0x20+0x10));
	
    if((offset+size)<S_TX_SIZE)		// 如果写入数据未超出发送缓冲区最大地址
    {
    	// 一次完成发送
        memcpy(&W5500_InitStructure[DevNum].pTxBuffer[3],dat_ptr,size);
        BSP_SPICSEnable(DevNum+1);
        BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,size+3);
        BSP_SPICSDisEnable(DevNum+1);
    }
	else							// 如果写入数据超出发送缓冲区最大地址
	{
		// 分两次发送
		// 第一次发送
        offset=S_TX_SIZE-offset;
        memcpy(&W5500_InitStructure[DevNum].pTxBuffer[3],dat_ptr,offset);
        BSP_SPICSEnable(DevNum+1);
        BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,offset+3);
        BSP_SPICSDisEnable(DevNum+1);

        // 第二次发送
        W5500_InitStructure[DevNum].pTxBuffer[0]=0x00;
        W5500_InitStructure[DevNum].pTxBuffer[1]=0x00;  
        W5500_InitStructure[DevNum].pTxBuffer[2]=(VDM|RWB_WRITE|(s*0x20+0x10));                 
        memcpy(&W5500_InitStructure[DevNum].pTxBuffer[3],dat_ptr+offset,size-offset);
        BSP_SPICSEnable(DevNum+1); 
        BSP_SPIWrite(DevNum+1,W5500_InitStructure[DevNum].pTxBuffer,size-offset+3);
        BSP_SPICSDisEnable(DevNum+1);
	}


	// 更新发送写指针地址并保存
	offset1+=size;
	Write_SOCK_2_Byte(DevNum,s,Sn_TX_WR, offset1);

	// 通过SEND命令发送保存在发送缓冲中的数据
	Write_SOCK_1_Byte(DevNum,s, Sn_CR, SEND);					
}

/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/