#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXNUM 40

HANDLE hCom;									//���ھ��			
COMSTAT comstat;								//ͨ�Ŷ˿�״̬�Ľṹ��������Ҫ�õ���������������/���뻺�����е��ֽ���
OVERLAPPED m_ov;

/****************************************************************************************
*	�첽ģʽ�������ڶ�дʱ�����õȵ�I/O����������ŷ��أ�����첽���Ը������Ӧ�û�����
*	ͬ��ģʽ����Ӧ��I/O����������ɺ����ŷ��أ����������߳�
****************************************************************************************/
bool openport(char *portname)					//�򿪴���
{
	hCom = CreateFile(portname,					//�����������ַ���	
					GENERIC_READ|GENERIC_WRITE,	//�ɶ���д
					0,							//���ڲ�֧�ֹ���ģʽ���������������0
					NULL,						//���尲ȫ���ԣ�һ�㲻��
					OPEN_EXISTING,				//�������������OPEN_EXISTING
					0,							//ͬ����ʽ				
					//FILE_FLAG_OVERLAPPED,		//�첽��ʽ
					NULL);						//������ģ�壬����������NULL
	if(hCom == INVALID_HANDLE_VALUE)			//�������ɹ����򷵻ؾ�������򷵻� INVALID_HANDLE_VALUE
	{
		//CloseHandle(hCom);						//�رմ���
		return FALSE;
	}
	return TRUE;
}

bool setupDCB(void)								//���ô���
{
	DCB dcb;									//ȡһ���ڴ�ռ䶨��Ϊ�豸���ƿ�

	memset(&dcb,0,sizeof(dcb));					//�豸���ƿ��ڴ�ռ�����

	if(!GetCommState(hCom,&dcb))				//GetCommState����ȡ�ô��ڵ�ǰ״̬
		return FALSE;
	dcb.DCBlength		= sizeof(dcb);			//DCB�ṹ���С
	dcb.BaudRate		= 38400;				//������
	dcb.ByteSize		= 8;					//���ݿ�ȣ�ͨ��Ϊ8
	dcb.Parity			= NOPARITY;				//��żУ��ѡ��
	dcb.fParity			= 0;					//�Ƿ������żУ��
	dcb.StopBits		= ONESTOPBIT;			//ֹͣλ��ͨ��Ϊ1
	dcb.fOutxCtsFlow	= 0;					//��ʹ��CTS���ϵ�Ӳ������
	dcb.fOutxDsrFlow	= 0;					//��ʹ��DTS���ϵ�Ӳ������
	dcb.fDsrSensitivity	= 0;					
	dcb.fDtrControl		= DTR_CONTROL_DISABLE;	
	dcb.fRtsControl		= RTS_CONTROL_DISABLE;	
	dcb.fOutX			= 0;					
	dcb.fInX			= 0;					

	dcb.fErrorChar		= 0;					
	dcb.fBinary			= 1;					//������
	dcb.fNull			= 0;
	dcb.wReserved		= 0;					
	dcb.fAbortOnError	= 0;
	dcb.XonLim			= 2;
	dcb.XoffLim			= 4;
	dcb.XonChar			= 0x13;
	dcb.XoffChar		= 0x19;
	dcb.EvtChar			= 0;

	if(!SetCommState(hCom,&dcb))				//SetCommState�������ô���״̬
		return FALSE;
	return TRUE;
}

bool setuptimeout(DWORD ReadInterval,
				  DWORD ReadTotalMultiplier,
				  DWORD ReadTotalConstant,
				  DWORD WriteTotalMultiplier,
				  DWORD WriteTotalConstant)						//���ó�ʱ����
{
	COMMTIMEOUTS timeouts;										//��ʱ���Ʋ����ṹ��
	
	//������������ʱʱ��=��/д�����ֽ�����ʱ��ϵ��+ʱ�䳣����
	//���ReadIntervalTimeout = MAXDWORD,ͬʱReadTotalTimeoutConstant��ReadTotalTimeoutMultiplier��Ϊ0��
	//���ʾ������Ҫ���������ѽ��յ����ַ�����ʹδ���յ���������ҲҪ����
	timeouts.ReadIntervalTimeout = ReadInterval;				//�Ժ���Ϊ��λָ����ȡ�����ֽڵ���֮������ʱ�䣻0ֵ��ʾ���ü����ʱ
	timeouts.ReadTotalTimeoutConstant = ReadTotalConstant;		//�Ժ���Ϊ��λָ��һ�����������ڼ��������������ʱʱ�䣻0ֵ��ʾ������ʱ
	timeouts.ReadTotalTimeoutMultiplier = ReadTotalMultiplier;	//ָ��һ�����������ڼ��������������ʱʱ�䣻0ֵ��ʾ����ʱ
	timeouts.WriteTotalTimeoutConstant = WriteTotalConstant;	//д�¼�����
	timeouts.WriteTotalTimeoutMultiplier = WriteTotalMultiplier;//д�¼�ϵ��

	if(!SetCommTimeouts(hCom,&timeouts))						//�����úò���д�볬ʱ����SetCommTimeouts
		return FALSE;
	return TRUE;
}

bool RecivedChar(BYTE *RXBuffer)						//������
{
	BOOL bRead = TRUE;
	BOOL bResult = TRUE;
	DWORD dwError = 0;
	DWORD BytesRead = 0;
	
	memset(RXBuffer,0,MAXNUM);							//���㽫Ҫ�洢���������ݵ�����
	
	//ʹ��readfile������ǰ��������ʹ��clearcommerror������������־
	bResult = ClearCommError(hCom,&dwError,&comstat);	//������ڴ��󲢶�ȡ��������/����������ֽ���
	if(comstat.cbInQue == 0)							//�ж����뻺�����������ֽڸ���
		return FALSE;
	
	if(bRead)
	{
		//����������
		bResult = ReadFile(hCom,						
						RXBuffer,						//���������ݴ洢�����׵�ַ
						comstat.cbInQue,				//׼���������ֽڸ���
						&BytesRead,						//ʵ�ʶ������ֽڸ���
						&m_ov);							//�첽ģʽ��ָ��OVERLAPPED�ṹ��ͬ��ģʽ��ΪNULL				
		printf("%s",RXBuffer);

		if(!bResult)									//��ReadFile����FALSEʱ����һ�����ǲ���ʧ��								
		{
			dwError = GetLastError();					//����Ҫͨ������GetLastError�������ؽ��
			if(dwError == ERROR_IO_PENDING)				//������ؽ����ERROR_IO_PENDING����˵�����첽������δ��ɵ�����ReadFile����FALSE
				bRead = FALSE;
		}
		else
			bRead = TRUE;
	}
	

	if(!bRead)												//����첽������δ���
	{
		bRead = TRUE;
		//����GetOverlappedResult�ȴ��첽�������
		bResult = GetOverlappedResult(hCom,					
									&m_ov,					//�ж��첽�����Ƿ���ɣ�ʵ�����жϸ�OVERLAPPED�ṹ�е�hEvent�Ƿ���λ			
									&BytesRead,
									TRUE);					//�Ƿ�ȴ�������첽������ɣ���ΪTRUE,��˺�������ȵ�������ɺ�ŷ���
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
						m_szWriteBuffer,					//ָ��Ҫд�봮�ڷ������ݻ����������ݵ��׵�ַ
						m_nToSend,							//Ҫд��������ֽ���
						&BytesSent,							//ʵ��д��������ֽ���
						&m_ov);								//�첽ģʽ��ָ��OVERLAPPED�ṹ��ͬ��ģʽ��ΪNULL

		if(!bResult)										//��WriteFile����FALSEʱ����һ�����ǲ���ʧ��
		{
			dwError = GetLastError();						//����Ҫͨ������GetLastError�������ؽ��
			if(dwError == ERROR_IO_PENDING)					//������ؽ����ERROR_IO_PENDING����˵�����첽������δ��ɵ�����WriteFile����FALSE
			{
				BytesSent = 0;
				bWrite = FALSE;
			}
		}		
	}
	if(!bWrite)												//����첽������δ���		
	{
		bWrite = TRUE;
		//����GetOverlappedResult�ȴ��첽�������
		bResult = GetOverlappedResult(hCom,					
									&m_ov,					//�ж��첽�����Ƿ���ɣ�ʵ�����жϸ�OVERLAPPED�ṹ�е�hEvent�Ƿ���λ			
									&BytesSent,
									TRUE);					//�Ƿ�ȴ�������첽������ɣ���ΪTRUE,��˺�������ȵ�������ɺ�ŷ���
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

	//SetCommMask(hCom,EV_RXCHAR);					//���ô���ͨ���¼�������ָ����������ض��Ĵ����¼�
													//EV_RXCHAR �������ѽ��յ�����
													//EV_TXEMPTY ����������е������ѱ���ȫ�ͳ�
	
	PurgeComm(hCom,									//��д����֮ǰ��Ҫ��PurgeComm��ջ�����������ֹ���ڽ��еĶ�д����	
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

