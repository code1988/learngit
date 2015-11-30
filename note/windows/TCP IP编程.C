TCP/IP协议由4个层次组成：
	1.网络接口层：这层没有提供专门的协议
	2.网络层：IP，ICMP
	3.传输层：TCP		//面向连接的可靠传输协议，通信前建立三次握手，握手成功后才能通信
			  UDP		//无连接、不可靠的传输协议
	4.应用层：Talnet，HTTP,FTP,DNS,SMTP,NFS

端口：用于标识进行通信的各个进程（即应用程序）。
	1. 应用程序通过系统调用和某端口绑定连接后（如Win API函数bind），从传输层传给该端口的数据都会自动被相应的进程接收；相应进程发给传输层的数据也都通过该端口输出
	2. 端口号用一个16位整型表示，范围0～65535，其中1024以下的端口号保留给预定义的服务如FTP、HTTP等
	3. 端口号跟传输层协议相关，所以TCP和UDP各自的端口是独立的，即端口号码可以相同
	
套接字：多个应用程序进程可能需要通过同一个端口传输数据，为了区分不同的进程和连接，通信两方作约定，用套接字中断相关函数来完成通信过程
		套接字的类型：	1. 流套接字（SOCK_STREAM）	面向连接、可靠的数据传输服务，数据无差错、无重复发送，且按发送顺序接收，基于TCP协议
						2. 数据报套接字（SOCK_DGRAM）无连接的数据服务，数据有可能出现丢失或重复，且接收顺序混乱，基于UDP协议


基于TCP的socket编程：（主要是基于windows 下VC6.0环境，部分是基于linux环境的）
	1.socket编程环境：	头文件 - winsock2.h	(该头文件内包含了windows.h和pshpack4.h,所以windows常用API都可以使用)
						库文件 - ws2_32.lib(库文件加载方法有2种：#pragma comment(lib,"ws2_32.lib");点击project-》setting-》link-》object/library modules后面添加ws2_32.lib)
	2.socket数据结构：	
				1）	   	sockaddr_in - 定义了socket发送和接收数据包的地址，只适用于TCP/IP地址
							struct socket_in{
									short sin_family;				// 网络地址族，固定值AF_INET
									u_short sin_port;				// 端口
									struct in_addr sin_addr;		// 存储IP地址的联合体，有3种表达方式
									char sin_zero[8];				// 无意义，仅仅是为了确保sockaddr_in和sockaddr两种结构体长度相等,方便两者强制转换
								};
								
				2）		sockaddr - 同样是定义了socket发送和接收数据包的地址，区别在于适用范围更广
							struct sockaddr{
									u_short sa_family;				
									char sa_data[14];
								};
								
				3）		WSADATA - 描述了socket库的一些相关信息
							typedef WSAData{
									WORD wVersion;			// 存储了socket的版本号
									WORD wHighVersion;		// 以下不需要关注
									char szDescription[WSADESCRIPTION_LEN+1];
							        char szSystemStatus[WSASYS_STATUS_LEN+1];
							        unsigned short iMaxSockets;
							        unsigned short iMaxUdpDg;
							        char FAR * lpVendorInfo;
								}WSADATA;
	3.socket函数：
				1)		* Function Name	: SOCKET socket(int af,int type,int protocol)
						* Description	: 创建socket
						* Input			: int af 		- 网络地址族，固定值AF_INET
										  int type 		- 网络协议类型，SOCK_DGRAM代表UDP，SOCK_STREAM代表TCP
										  int protocol 	- 网络地址族的特殊协议，IPPROTO_UDP代表UDP,IPPROTO_TCP代表TCP
						* Return		: SOCKET		- 创建成功返回具体SOCKET,失败返回INVALID_SOCKET
							
				2）	    * Function Name	: int setsockopt(SOCKET s,int level,int optname,const char *optval,int optlen)
						* Description	: 设置socket的属性
						* Input			: SOCKET s 				- 要设置的套接字
										  int level 			- 要设置的属性所处的层次，通常取值SOL_SOCKET代表套接字层次
										  int optname 			- 要设置的参数的名称，常设参数包括：SO_SNDBUF发送缓冲区，SO_RCVBUF接收缓冲区,SO_SNDTIMEO发送超时，SO_RCVTIMEO接收超时
										  const char *optval	- 指向要设置的参数数值的指针
										  int optlen			- 要设置的参数数值变量的长度,通常就是 sizeof(int)
						* Return		: 返回0代表成功，SOCKET_ERROR代表有错误
						
				3）	    * Function Name	: int getsockopt(SOCKET s,int level,int optname,char *optval,int *optlen)
						* Description	: 读取socket的属性
						* Input			: SOCKET s 				- 要读取的套接字
										  int level 			- 要读取的属性所处的层次，通常取值SOL_SOCKET代表套接字层次
										  int optname 			- 要读取的参数的名称，常设参数包括：SO_SNDBUF发送缓冲区，SO_RCVBUF接收缓冲区,SO_SNDTIMEO发送超时，SO_RCVTIMEO接收超时
										  char *optval			- 指向存储要读取的参数数值的指针
										  int *optlen			- 指向存储要读取的参数数值变量的长度,通常就是 &sizeof(int)
						* Return		: 返回0代表成功，SOCKET_ERROR代表有错误
							
				4）		* Function Name	: int sendto(SOCKET s,const char *buf,int len,int flags,const struct sockaddr *to,int tolen)
						* Description	: socket中有2套发送函数，本套在形参中需要指明目标地址，所以适用于无连接的套接字，也就是UDP
						* Input			: SOCKET s					- 要使用的套接字
										  const char *buf			- 指向要发送的缓冲区指针
										  int len					- 要发送到数据长度
										  int flags					- 传送方式的标志，无特殊要求填0，可选标志：MSG_DONTWAIT非阻塞模式发送
										  const struct sockaddr *to - 目标地址
										  int tolen					- 目标地址的长度
						* Return 		: 成功则返回成功发送的字节数，失败则返回SOCKET_ERROR
				
				5）		* Function Name	: int send(SOCKET s,const char *buf,int len,int flags)
						* Description	: socket中有2套发送函数，本套需要事先将套接字与地址绑定，面向的是已连接的套接字，也就是TCP
						* Input			: SOCKET s					- 要使用的套接字
										  const char *buf			- 指向要发送的缓冲区指针
										  int len					- 要发送到数据长度
										  int flags					- 传送方式的标志，无特殊要求填0，可选标志：MSG_DONTWAIT非阻塞模式发送
						* Return 		: 成功则返回成功发送的字节数，失败则返回SOCKET_ERROR
				
				6）		* Function Name	: int recv(SOCKET s,char *buf,int len,int flags,struct sockaddr *from,int *fromlen)
						* Description	: socket中有2套接收函数，本套在形参中需要指明目标地址，所以适用于无连接的套接字，也就是UDP
						* Input			: SOCKET s						- 要使用的套接字
										  char *buf						- 指向接收数据的缓冲区指针
										  int len						- 要接收的数据长度
										  int flags						- 接收方式的标志，无特殊要求填0，可选标志：MSG_DONTWAIT非阻塞模式接收，MSG_PEEK读取接收缓冲区中数据且不清除
										  const struct sockaddr *from	- 目标地址
										  int *fromlen					- 指向目标地址的长度
						* Return 		: 成功则返回成功接收的字节数，失败则返回SOCKET_ERROR
							
				7）		* Function Name	: int recv(SOCKET s,char *buf,int len,int flags)
						* Description	: socket中有2套接收函数，recv可以面向连接和非连接
						* Input			: SOCKET s					- 要使用的套接字
										  char *buf					- 指向要发送的缓冲区指针
										  int len					- 要发送到数据长度
										  int flags					- 接收方式的标志，无特殊要求填0，可选标志：MSG_DONTWAIT非阻塞模式接收，MSG_PEEK读取接收缓冲区中数据且不清除
						* Return 		: 成功则返回成功接收的字节数，失败则返回SOCKET_ERROR	
															
				4）	    * Function Name	: int closesocket(SOCKET s)
						* Description	: 关闭socket
						* Input			: SOCKET s 				- 要关闭的套接字
						* Return		: 返回0代表成功，SOCKET_ERROR代表有错误
	
	
						
	服务器端程序：1. 加载套接字库
				 2. 创建套接字（socket）
				 3. 将套接字绑定到一个本地地址和端口上（bind）
				 4. 将套接字设为监听模式，准备接收客户请求（listen）
				 5. 等待客户请求的到来，当请求到来后，接受连接请求，返回一个新的、对应于此次连接的套接字（accept）
				 6. 用返回的套接字和客户端进行通信（send/recv）
				 7. 返回，等待另一个客户请求
				 8. 关闭套接字
	客户端程序：	1. 加载套接字库
				2. 创建套接字（socket）
				3. 向服务器发送连接请求（connect）
				4. 和服务器端进行通信（send/recv）
				5. 关闭套接字

服务器端程序例程：
#include <WINSOCK2.H>								//声明socket库的类
#include <iostream>

#pragma comment (lib,"WS2_32.LIB")					//包含socket库，跟WINSOCK2.H一般一起配对使用

#define PORT_NUM 9999

int main()
{
	WSADATA wsaData;               					//WSADATA结构，加载套接字库时被用来填充库版本信息
	SOCKET  ListeningSocket;       					//用于监听的套接字
	SOCKET  NewConnection = NULL;  					//accept函数反回的套接字，用于同connect方（客户端）连系。
	SOCKADDR_IN ServerAddr;        					//本地（服务器）地址
	SOCKADDR_IN ClientAddr;        					//用来接收客户端的地址信息
	int     ClientAddrLen = sizeof(ClientAddr); 	//设置客户端的地址结构长度
	int     BufLen = 0;            					//接收到的信息的长度
	char    buf[50];               					//用于存储信息

	/*加载2.2版本的套接字库*/
	if(WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		std::cout<<"WSAStartup failed"<<std::endl;
		return 1;
	}
	std::cout<<"WSAStartup successing!"<<std::endl;

	/*创建套接字*/
	if((ListeningSocket = socket(AF_INET,			//指定地址族，对于TCP/IP协议，这里只能是 AF_INET（PF_INET等价）		
								SOCK_STREAM,		//指定socket类型
								IPPROTO_TCP)) 		//指定协议，如果是0则表示会根据地址族和socket类型来自动选择合适的协议
								== INVALID_SOCKET)  			
	{
		std::cout<<"Create ListenSocket failed with error"<<" "<<WSAGetLastError()<<std::endl;
		return 1;
	}
	std::cout<<"Create ListenSocket successing!"<<std::endl;

	/*设置服务器地址*/
	ServerAddr.sin_family = AF_INET; 				//指定地址族，对于TCP/IP协议，这里只能是 AF_INET（PF_INET等价）即IP地址族 
	ServerAddr.sin_port   = htons(port);			//指定端口号，htons用来将主机字节顺序（unsigned short）转换为网络字节顺序，通常就是 小端->大端
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);	//指定IP地址，htonl跟htons类似，区别在于是unsigned int；IP地址指定为INADDR_ANY，则允许一个独立的应用接收发自多个接口的回应

	/*绑定套接字与本地地址（服务器）*/
	if(bind(ListeningSocket,						//指定要绑定的套接字
			(SOCKADDR*)&ServerAddr,					//指定该套接字的本地地址信息，注意SOCKADDR是通用地址结构，而这里TCP/IP使用特定的SOCKADDR_IN地址结构
			sizeof(ServerAddr)) 					//指定该地址结构的长度
			== SOCKET_ERROR) 
	{
		std::cout<<"Bind failed with error"<<" "<<WSAGetLastError()<<std::endl;
		return 1;
	}
	std::cout<<"Bind successing!"<<std::endl;

	/*将套接字设置为监听模式*/
	if(listen(ListeningSocket,3) == SOCKET_ERROR)	//第二个参数设置等待请求连接的最大值
	{
		std::cout<<"Listen failed with error"<<" "<<WSAGetLastError()<<std::endl;
		return 1;
	}
	else
	std::cout<<"Listening..."<<std::endl;


	/*等待请求的到来，到来后接受连接*/
	if((NewConnection 								//如果连接成功，则返回一个当前成功建立连接的套接字
		= accept(ListeningSocket,					//处于监听中的套接字	
				(SOCKADDR*)&ClientAddr,				//接收客户端地址信息
				&ClientAddrLen)) 					//指定客户端地址结构长度
		== INVALID_SOCKET)  
		std::cout<<"Accept failed with error"<<" "<<
				WSAGetLastError()<<std::endl;		//如果连接失败，则通过WSAGetLastError得到相关的失败信息
	else
		std::cout<<"Accept successing!"<<std::endl;
	
	std::cout<<inet_ntoa(ClientAddr.sin_addr)		//打印接收到的客户端IP，inet_ntoa表示网络字节序的IP地址转换为点分十进制表示的IP地址
		<<std::endl;

	/*接收并显示数据*/
	while(strcmp(buf,"exit")) 							//接收到exit则退出循环	 
	{
		BufLen = recv(NewConnection,buf,sizeof(buf),0);	//接收从客户端传递过来的信息
		buf[BufLen] = '\0';
		std::cout<<buf<<std::endl;
	}

	/*关闭套接字*/
	closesocket(ListeningSocket);						//关闭指定的套接字
	closesocket(NewConnection);
	WSACleanup();										//终止对socket库的使用
		
	return 0;
}

客户端程序历程：
#include <iostream>
#include <WINSOCK2.H>
#pragma comment (lib,"ws2_32.lib")

#define PORT_NUM 9999

int main()
{
	WSADATA wsaData;               
	SOCKET  ClientSocket;   									//客户端套接字
	SOCKADDR_IN ClientSocketAddr;  								//服务器端地址
	char    buf[1024];             								//存储消息用
	int     MessageLen = 0;        								//返回的消息长度

	//加载Winsock 2.2版本
	if(WSAStartup(MAKEWORD(2,2),&wsaData) !=0)
	{
		std::cout<<"WSAStartup failed"<<std::endl;
		return 1;
	}

	//创建套接字
	if((ClientSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == INVALID_SOCKET)
		std::cout<<"Create ServerSocket failed with error"<<" "<<WSAGetLastError()<<std::endl;

	//填写要连接的服务器地址信息
	ClientSocketAddr.sin_family = AF_INET;
	ClientSocketAddr.sin_port   = htons(port);
	ClientSocketAddr.sin_addr.s_addr  = inet_addr("127.0.0.1");	//指定服务端IP，inet_addr函数用来将点分十进制表示的IP地址转换为网络字节序IP地址

	//向服务器发出连接请求
	if(connect(ClientSocket,									//客户端套接字
			(SOCKADDR*)&ClientSocketAddr,						//指定该套接字的本地地址信息
			sizeof(ClientSocketAddr)) 							//指定该本地地址结构的长度
			==SOCKET_ERROR)
		std::cout<<"Connecting failed with error"<<" "<<WSAGetLastError()<<std::endl;
	else 
		std::cout<<"Connect successing!"<<std::endl;

	//发送数据，直到数据内容为"exit"则退出程序
	while(strcmp(buf,"exit") != 0)
	{
		std::cout<<"Please input:"<<std::endl;
		std::cin>>buf;
		if((MessageLen = send(ClientSocket,buf,strlen(buf)+1,0)) == INVALID_SOCKET)	//发送信息到服务器
			std::cout<<"Send data failed with error"<<" "<<WSAGetLastError()<<std::endl;
		else
			std::cout<<"Send"<<" "<<MessageLen<<" "<<"byte"<<"datas"<<std::endl;;
	}

	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}