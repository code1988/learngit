#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXNUM 40

HANDLE hCom;									//串口句柄			
COMSTAT comstat;								//通信端口状态的结构变量，主要用到两个参数：输入/输入缓冲区中的字节数
OVERLAPPED m_ov;

/****************************************************************************************
*	异步模式：串口在读写时，不用等到I/O操作完后函数才返回，因此异步可以更快的响应用户操作
*	同步模式：响应的I/O操作必须完成后函数才返回，否则阻塞线程
****************************************************************************************/
bool openport(char *portname)					//打开串口
{
	hCom = CreateFile(portname,					//串口名，是字符串	
					GENERIC_READ|GENERIC_WRITE,	//可读可写
					0,							//串口不支持共享模式，所以这里必须是0
					NULL,						//定义安全属性，一般不用
					OPEN_EXISTING,				//串口这里必须是OPEN_EXISTING
					0,							//同步方式				
					//FILE_FLAG_OVERLAPPED,		//异步方式
					NULL);						//串口无模板，所以这里是NULL
	if(hCom == INVALID_HANDLE_VALUE)			//串口若成功打开则返回句柄，否则返回 INVALID_HANDLE_VALUE
	{
		//CloseHandle(hCom);						//关闭串口
		return FALSE;
	}
	return TRUE;
}

bool setupDCB(void)								//设置串口
{
	DCB dcb;									//取一块内存空间定义为设备控制块

	memset(&dcb,0,sizeof(dcb));					//设备控制块内存空间清零

	if(!GetCommState(hCom,&dcb))				//GetCommState用于取得串口当前状态
		return FALSE;
	dcb.DCBlength		= sizeof(dcb);			//DCB结构体大小
	dcb.BaudRate		= 38400;				//波特率
	dcb.ByteSize		= 8;					//数据宽度，通常为8
	dcb.Parity			= NOPARITY;				//奇偶校验选择
	dcb.fParity			= 0;					//是否进行奇偶校验
	dcb.StopBits		= ONESTOPBIT;			//停止位，通常为1
	dcb.fOutxCtsFlow	= 0;					//不使能CTS线上的硬件握手
	dcb.fOutxDsrFlow	= 0;					//不使能DTS线上的硬件握手
	dcb.fDsrSensitivity	= 0;					
	dcb.fDtrControl		= DTR_CONTROL_DISABLE;	
	dcb.fRtsControl		= RTS_CONTROL_DISABLE;	
	dcb.fOutX			= 0;					
	dcb.fInX			= 0;					

	dcb.fErrorChar		= 0;					
	dcb.fBinary			= 1;					//二进制
	dcb.fNull			= 0;
	dcb.wReserved		= 0;					
	dcb.fAbortOnError	= 0;
	dcb.XonLim			= 2;
	dcb.XoffLim			= 4;
	dcb.XonChar			= 0x13;
	dcb.XoffChar		= 0x19;
	dcb.EvtChar			= 0;

	if(!SetCommState(hCom,&dcb))				//SetCommState用于设置串口状态
		return FALSE;
	return TRUE;
}

bool setuptimeout(DWORD ReadInterval,
				  DWORD ReadTotalMultiplier,
				  DWORD ReadTotalConstant,
				  DWORD WriteTotalMultiplier,
				  DWORD WriteTotalConstant)						//设置超时限制
{
	COMMTIMEOUTS timeouts;										//超时限制参数结构体
	
	//读操作的总限时时间=读/写操作字节数×时间系数+时间常量；
	//如果ReadIntervalTimeout = MAXDWORD,同时ReadTotalTimeoutConstant和ReadTotalTimeoutMultiplier都为0，
	//则表示读操作要立即返回已接收到的字符，即使未接收到，读操作也要返回
	timeouts.ReadIntervalTimeout = ReadInterval;				//以毫秒为单位指定读取两个字节到达之间的最大时间；0值表示不用间隔限时
	timeouts.ReadTotalTimeoutConstant = ReadTotalConstant;		//以毫秒为单位指定一个常数，用于计算读操作的总限时时间；0值表示不用限时
	timeouts.ReadTotalTimeoutMultiplier = ReadTotalMultiplier;	//指定一个乘数，用于计算读操作的总限时时间；0值表示不限时
	timeouts.WriteTotalTimeoutConstant = WriteTotalConstant;	//写事件常数
	timeouts.WriteTotalTimeoutMultiplier = WriteTotalMultiplier;//写事件系数

	if(!SetCommTimeouts(hCom,&timeouts))						//将配置好参数写入超时函数SetCommTimeouts
		return FALSE;
	return TRUE;
}

bool RecivedChar(BYTE *RXBuffer)						//读操作
{
	BOOL bRead = TRUE;
	BOOL bResult = TRUE;
	DWORD dwError = 0;
	DWORD BytesRead = 0;
	
	memset(RXBuffer,0,MAXNUM);							//清零将要存储被读出数据的区域
	
	//使用readfile读操作前，必须先使用clearcommerror函数清除错误标志
	bResult = ClearCommError(hCom,&dwError,&comstat);	//清除串口错误并读取串口输入/输出缓冲区字节数
	if(comstat.cbInQue == 0)							//判断输入缓冲区内数据字节个数
		return FALSE;
	
	if(bRead)
	{
		//读串口数据
		bResult = ReadFile(hCom,						
						RXBuffer,						//被读出数据存储区域首地址
						comstat.cbInQue,				//准备读出的字节个数
						&BytesRead,						//实际读出的字节个数
						&m_ov);							//异步模式下指向OVERLAPPED结构；同步模式下为NULL				
		printf("%s",RXBuffer);

		if(!bResult)									//当ReadFile返回FALSE时，不一定就是操作失败								
		{
			dwError = GetLastError();					//而是要通过调用GetLastError分析返回结果
			if(dwError == ERROR_IO_PENDING)				//如果返回结果是ERROR_IO_PENDING，则说明是异步操作还未完成导致了ReadFile返回FALSE
				bRead = FALSE;
		}
		else
			bRead = TRUE;
	}
	

	if(!bRead)												//如果异步操作还未完成
	{
		bRead = TRUE;
		//调用GetOverlappedResult等待异步操作完成
		bResult = GetOverlappedResult(hCom,					
									&m_ov,					//判断异步操作是否完成，实际是判断该OVERLAPPED结构中的hEvent是否置位			
									&BytesRead,
									TRUE);					//是否等待悬起的异步操作完成，若为TRUE,则此函数必须等到操作完成后才返回
	}
	return TRUE;
}

bool WriteChar(char *m_szWriteBuffer,DWORD m_nToSend)
{
	BOOL bWrite = TRUE;
	BOOL bResult = TRUE;
	DWORD dwError = 0;
	DWORD BytesSent = 0;

	if(bWrite)
	{
		m_ov.Offset = 0;
		m_ov.OffsetHigh = 0;
		
		bResult = WriteFile(hCom,
						m_szWriteBuffer,					//指向要写入串口发送数据缓冲区的内容的首地址
						m_nToSend,							//要写入的数据字节数
						&BytesSent,							//实际写入的数据字节数
						&m_ov);								//异步模式下指向OVERLAPPED结构；同步模式下为NULL

		if(!bResult)										//当WriteFile返回FALSE时，不一定就是操作失败
		{
			dwError = GetLastError();						//而是要通过调用GetLastError分析返回结果
			if(dwError == ERROR_IO_PENDING)					//如果返回结果是ERROR_IO_PENDING，则说明是异步操作还未完成导致了WriteFile返回FALSE
			{
				BytesSent = 0;
				bWrite = FALSE;
			}
		}		
	}
	if(!bWrite)												//如果异步操作还未完成		
	{
		bWrite = TRUE;
		//调用GetOverlappedResult等待异步操作完成
		bResult = GetOverlappedResult(hCom,					
									&m_ov,					//判断异步操作是否完成，实际是判断该OVERLAPPED结构中的hEvent是否置位			
									&BytesSent,
									TRUE);					//是否等待悬起的异步操作完成，若为TRUE,则此函数必须等到操作完成后才返回
		if (!bResult)
		{
			printf("GetOverlappedResults() in WriteFile()");
			return FALSE;
		}
	}

	if(BytesSent != m_nToSend)
	{
		printf("WARNING: WriteFile() error.. Bytes Sent: %d; Message Length: %d\n", BytesSent, strlen((char*)m_szWriteBuffer));
		return FALSE;
	}
	return TRUE;
}

void main()
{
	bool open;
	unsigned char rxbuffer[MAXNUM];

	open = openport("COM2");
	if(open)
		printf("open COM successful\n");
	else
		printf("open COM fail\n");

	if(setupDCB())
		printf("COM set successful\n");
	else
		printf("COM set fail\n");

	if(setuptimeout(0,0,0,0,0))
		printf("set up timeout successful\n");
	else
		printf("set up timeout fail\n");

	//SetCommMask(hCom,EV_RXCHAR);					//设置串口通信事件，用来指定程序接收特定的串口事件
													//EV_RXCHAR 缓冲区已接收到数据
													//EV_TXEMPTY 输出缓冲区中的数据已被完全送出
	
	PurgeComm(hCom,									//读写串口之前，要用PurgeComm清空缓冲区，并终止正在进行的读写操作	
			PURGE_RXCLEAR									
			|PURGE_TXCLEAR
			|PURGE_RXABORT
			|PURGE_TXABORT);

	while(1)
	{
		//if(!RecivedChar(rxbuffer))
		//	break;
		if(WriteChar("please send data now\n",22))
			RecivedChar(rxbuffer);
	}

}

