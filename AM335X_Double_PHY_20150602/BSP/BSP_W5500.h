/*****************************************Copyright(C)******************************************
*******************************************浙江方泰*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_W5500.h
**硬          件: am335x
**创    建    人: WB
**创  建  日  期: 140930
**最  新  版  本: V0.1
**描          述: 
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_W5500_H_
#define	__BSP_W5500_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "w5500_socket.h"
/* Private define-----------------------------------------------------------------------------*/

/*SOCKET NUM*/
#define SOCKET0          0
#define SOCKET1          1
#define SOCKET2          2
#define SOCKET3          3
#define SOCKET4          4
#define SOCKET5          5
#define SOCKET6          6
#define SOCKET7          7
/*PORT NUM*/
#define PORT_NUM1        5000
#define PORT_NUM2        5001
#define PORT_NUM3        5002
#define DESPORT_NUM     5000
/*w5500 Slect*/
#define W5500A		0x00
#define W5500B		0x01

#define SEND_REQ    0x20
/* Private typedef----------------------------------------------------------------------------*/

typedef struct 
{
	INT8U S_IP[4];
	INT8U GW_IP[4];
	INT8U Sub_Mask[4];
	INT8U MAC[6];
	INT16U S_Port;
	INT8U D_IP[4];
	INT16U D_Port;
	INT8U S0_SendOK;								// 发送空闲标志，1表示空闲
	INT8U S0_TimeOut;								// 超时标志
	SOCKET s_num;									// socket号 0～7
	INT8U State;									// socket状态：0 关闭，1 空闲，2 忙碌
	INT8U Prtl;										// socket协议
}_BSPW5500_CONFIG;

/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/  
/* Private functions--------------------------------------------------------------------------*/
void Close_Common_INT(INT8U DevNum,INT8U IntNum);

void Open_Common_INT(INT8U DevNum,INT8U IntNum);
    
void Open_Socket_INT(INT8U DevNum,SOCKET s);

void Operat_Socket_INT_Indiv(INT8U DevNum,INT8U IntNum);

INT8U Get_Socket_INTNum(INT8U DevNum);

INT8U Get_Socket_INTType(INT8U DevNum);

INT8U Get_Socket_Status(INT8U DevNum);

void Disconnect(INT8U DevNum);

void Set_INT_IntervalTime(INT8U DevNum,INT16U intvtime);

INT16U Read_INT_IntervalTime(INT8U DevNum);

INT32U BSP_SocketConnect(INT8U DevNum);

INT32U BSP_SocketListen(INT8U DevNum);

void BSP_W5500Config(INT8U DevNum,_BSPW5500_CONFIG *Initstruct);

INT16U BSP_W5500Recv(INT8U DevNum,INT8U *R_Buf);

void BSP_W5500Send(INT8U DevNum,INT8U *T_Buf,INT16U len);

void BSP_W5500Socket(INT8U DevNum,INT8U protocol);

INT8U BSP_W5500Process(INT8U DevNum,INT8U EventNum);

INT8U BSP_W5500GetIR(INT8U DevNum);
#endif
/************************(C)COPYRIGHT 2013 浙江方泰*****END OF FILE****************************/