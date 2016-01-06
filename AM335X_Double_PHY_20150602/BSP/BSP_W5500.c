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

/* Private define-----------------------------------------------------------------------------*/

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/

_BSPW5500_CONFIG W5500_Config[2];

/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function Name	: Close_Common_INT
* Description	: 关闭普通中断
* Input			: DevNum		2路W5500选择
                  IntNum        中断ID:     IM_IR7
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
* Description	: 打开普通中断
* Input			: DevNum		2路W5500选择
                  IntNum        中断ID:     IM_IR7
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
* Description	: 打开指定socket中断
* Input			: DevNum		2路W5500选择
                  S             socket号
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
* Description	: 操作指定socket中断
* Input			: DevNum		2路W5500选择
                  IntNum        中断号
* Return		:
* Note(s)		: 打开关闭操作都在这，1打开，0关闭，低5位有效
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
* Description	: 获取Socket中断号，即第几号Socket中断了
* Input			: DevNum		2路W5500选择
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
* Description	: 获取Socket中断类型
* Input			: DevNum		2路W5500选择
* Return		: IR_SEND_OK	发送完成
                  IR_TIMEOUT	超时
                  IR_RECV		接收到数据
                  IR_DISCON		即受到FIN/ACK回复包
                  IR_CON		成功建立连接
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
* Description	: 获取Socket状态
* Input			: DevNum			2路W5500选择
				  s					socket选择(0~7)
* Return		: SOCK_CLOSED		关闭状态
                  SOCK_INIT			端口打开并处于TCP工作模式
                  SOCK_LISTEN		服务器模式等待连接请求
                  SOCK_ESTABLISHED	连接状态
                  SOCK_CLOSE_WAIT	收到断开连接请求
                  SOCK_UDP			UDP模式下
                  SOCK_MACRAW		MACRAW模式下
* Note(s)		:
* Contributor	: 140930	WB
***********************************************************************************************/
INT8U Get_Socket_Status(INT8U DevNum)
{
    return Read_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_SR);
}

/***********************************************************************************************
* Function Name	: Disconnect
* Description	: 发送断开连接请求
* Input			: DevNum			2路W5500选择
				  s					socket选择(0~7)
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
* Description   : 关闭socket
* Input		  : DevNum			  2路W5500选择
* Return		  :
* Note(s) 	  :
* Contributor   : 141017	  code
***********************************************************************************************/
void Close(INT8U DevNum)
{
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_CR,CLOSE);		// 关闭指定的socket
	W5500_Config[DevNum].State = 0;
}

  /***********************************************************************************************
* Function Name	: Socket_Connect
* Description	: 设置socket处于TCP客户端模式
* Input			: DevNum			2路W5500选择
* Return		: FALSE/TRUE		失败/成功
* Note(s)		:
* Contributor	: 140930	WB
***********************************************************************************************/
INT32U BSP_SocketConnect(INT8U DevNum)
{
	// 设置目的IP
	Write_SOCK_4_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_DIPR, W5500_Config[DevNum].D_IP);

	// 设置目的端口
	Write_SOCK_2_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_DPORTR, W5500_Config[DevNum].D_Port);

	// 配置socket为TCP客户端
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_CR,CONNECT);
    OSTimeDlyHMSM(0,0,0,5);	/* Wait for a moment */

	return TRUE;
}
/***********************************************************************************************
* Function Name	: Socket_Listen
* Description	: 设置socket处于TCP服务器模式
* Input			: DevNum			2路W5500选择
* Return		: FALSE/TRUE		失败/成功
* Note(s)		:
* Contributor	: 140930	WB
***********************************************************************************************/
INT32U BSP_SocketListen(INT8U DevNum)
{
	// 配置socket为TCP服务器
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_CR,LISTEN);

	// 读socket状态寄存器，判断socket是否处于监听状态
	while(Read_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_SR)!=SOCK_LISTEN);

	return TRUE;
}

/***********************************************************************************************
* Function Name	: BSP_W5500Config
* Description	: W5500 spi接口初始化以及网络配置
* Input			: DevNum		2路W5500选择
				  *Initstruct	配置信息结构体指针
* Return		: none
* Note(s)		: 添加延时目的是确保MAC真实写入寄存器
* Contributor	: 140930	WB
***********************************************************************************************/
void BSP_W5500Config(INT8U DevNum,_BSPW5500_CONFIG *Initstruct)
{
	memcpy(&W5500_Config[DevNum],Initstruct,sizeof(_BSPW5500_CONFIG));

	// spi初始化
	Init_W5500_SPI(DevNum);
	OSTimeDlyHMSM(0,0,0,5);

	// 硬件复位
	while((Read_1_Byte(DevNum,PHYCFGR)&LINK)==0)
        OSTimeDlyHMSM(0,0,1,0);
    
	// 软件复位
	Write_1_Byte(DevNum,MR, RST);
	OSTimeDlyHMSM(0,0,0,50);
   
    Set_INT_IntervalTime(DevNum,0x1000);

    Set_ReTran_Num(DevNum,2);

	// 网关设置
	Write_Bytes(DevNum,GAR, Initstruct->GW_IP, 4);
	OSTimeDlyHMSM(0,0,0,50);

	// 子网掩码设置
	Write_Bytes(DevNum,SUBR, Initstruct->Sub_Mask, 4);
	OSTimeDlyHMSM(0,0,0,50);

	// MAC设置
	Write_Bytes(DevNum,SHAR, Initstruct->MAC, 6);
	OSTimeDlyHMSM(0,0,0,50);

	// IP设置
	Write_Bytes(DevNum,SIPR, Initstruct->S_IP, 4);
	OSTimeDlyHMSM(0,0,0,50);
}


/***********************************************************************************************
* Function Name	: Process_IR
* Description	: 中断处理
* Input			: DevNum	2路W5500选择
				  s 		socket号(0~7)
* Return		:
* Note(s)		:
* Contributor	: 140930	WB
***********************************************************************************************/
INT8U BSP_W5500GetIR(INT8U DevNum)
{
	INT8U flag;

	// 获取产生中断的socket号
	flag=Get_Socket_INTNum(DevNum);

	// 判断指定的socket是否存在中断
	if(flag&(0x01<<W5500_Config[DevNum].s_num))
	{
		// 获取socket中断类型
		flag=Get_Socket_INTType(DevNum);

		if(flag&IR_CON)		                                // socket成功建立连接
		    return IR_CON;
		else if(flag&IR_DISCON)		                        // socket连接断开
		    return IR_DISCON;
        else if(flag&IR_RECV)                               // 接收中断
    	    return IR_RECV;
		else if(flag&IR_TIMEOUT)							// 超时中断
		    return IR_TIMEOUT;
		else if(flag&IR_SEND_OK)							// 发送完成
		    return IR_SEND_OK;        
	}
    
    return 0;
}

INT8U BSP_W5500Process(INT8U DevNum,INT8U EventNum)
{
    static INT8U S_Data_Buffer[4096]; 							// socket收发缓冲
    INT16U      Rx_len;
    
    switch(EventNum)
    {
        case IR_CON:        // 连接成功
            break;
        case IR_DISCON:     // 连接断开
            // 重新开始开始监听
			Disconnect(DevNum);
            BSP_W5500Socket(DevNum,MR_TCP|ND_MC_MMB);
            BSP_SocketListen(DevNum);
            break;
        case IR_RECV:       // 有数据需要接收
            Rx_len = BSP_W5500Recv(DevNum,S_Data_Buffer);
            if(Rx_len == 0)
                return 1;
            BSP_W5500Send(DevNum,S_Data_Buffer,Rx_len);
            break;
        case SEND_REQ:      // 有数据需要发送
            BSP_W5500Send(DevNum,S_Data_Buffer,1);
            break;
        case IR_TIMEOUT:    // 超时
            return 1;
        case IR_SEND_OK:    // 发送完成
            break;
        default:
            return 1;
            
    }
    return 0;
}

/***********************************************************************************************
* Function Name	: BSP_W5500Recv
* Description	: 读接收缓冲区数据
* Input			: DevNum	2路W5500选择
				  *R_Buf 	数据存放地址指针
* Return		: len		读取的数据长度
* Note(s)		:
* Contributor	: 141029	code
***********************************************************************************************/
INT16U BSP_W5500Recv(INT8U DevNum,INT8U *R_Buf)
{
	INT16U len;

	len = Read_SOCK_Data_Buffer(DevNum,W5500_Config[DevNum].s_num,R_Buf);

    // 如果是UDP协议
	if(W5500_Config[DevNum].Prtl & MR_UDP)
	{
		if(len > 0)
		{
			// 从接受数据中提取对方IP地址
			memcpy(W5500_Config[DevNum].D_IP,R_Buf,4);
			// 将提取的对方IP地址写入socket的目的IP寄存器
			Write_SOCK_4_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_DIPR, W5500_Config[DevNum].D_IP);

			// 从接收数据中提取对方端口号
			W5500_Config[DevNum].D_Port= R_Buf[4]*0x100+R_Buf[5];
			//将提取的对方端口号写入socket的目的端口寄存器
			Write_SOCK_2_Byte(DevNum,W5500_Config[DevNum].s_num, Sn_DPORTR, W5500_Config[DevNum].D_Port);
		}
	}

	return len;
}

/***********************************************************************************************
* Function Name	: BSP_W5500Send
* Description	: 写数据到发送缓冲区
* Input			: DevNum	2路W5500选择
				  *T_Buf	数据存放地址指针
				  len		写入的数据长度
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
* Description	: socket配置
* Input			: DevNum	2路W5500选择
				  protocol 	socket协议:TCP/UDP/MACRAW(未实现)
* Return		:
* Note(s)		:
* Contributor	: 141029	code
***********************************************************************************************/
void BSP_W5500Socket(INT8U DevNum,INT8U protocol)
{
	// 设置源端口号
	Write_SOCK_2_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_PORT, W5500_Config[DevNum].S_Port);

    // 关闭所有普通中断
    Close_Common_INT(DevNum,IM_IR4 | IM_IR5 | IM_IR6 | IM_IR7);
    // 打开所有普通中断
    //Open_Common_INT(DevNum,IM_IR4 | IM_IR5 | IM_IR6 | IM_IR7);

    // 打开指定socket中断,目前只使用单端口工作模式
	Open_Socket_INT(DevNum,W5500_Config[DevNum].s_num);

    // 打开指定socket的指定中断,这里除了IMR_SENDOK外全部打开
    Operat_Socket_INT_Indiv(DevNum,IMR_CON | IMR_DISCON | IMR_RECV | IMR_TIMEOUT);
    
	// 配置最大传输单元
	Write_SOCK_2_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_MSSR, 1460);

	// 设置RX 缓冲尺寸为 2K
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_RXBUF_SIZE, 0x04);

	// 设置TX 缓冲尺寸为 2K
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_TXBUF_SIZE, 0x04);

	// 设置socket协议
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_MR,protocol);

	// 打开socket
	Write_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_CR,OPEN);

	// 读socket状态寄存器，判断socket是否处于指定协议模式
	if(protocol & MR_TCP)
		while(Read_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_SR)!=SOCK_INIT);
	else if(protocol & MR_UDP)
		while(Read_SOCK_1_Byte(DevNum,W5500_Config[DevNum].s_num,Sn_SR)!=SOCK_UDP);

    Set_KeepAlive(DevNum,5);
    
	W5500_Config[DevNum].State = 1;
	W5500_Config[DevNum].Prtl  = protocol;
}

/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/
