
#include "comm.h"

#define COMM_MAX_LISTEN		2

static SERVER_S		*hManager = NULL;

static status_t commThread(void_t *arg);
static s32_t comm_fdset_f(fd_set *rfd, fd_set *wfd, fd_set *efd);
static status_t comm_socket_listen_f(u16_t port);
static SOCKET_S *comm_create_f(s32_t sock);
static status_t comm_destroy_f(void_t *handle);
static status_t comm_thread_create_f(void);
static status_t comm_thread_destroy_f(void);
static status_t comm_socket_close_f(void);
static status_t comm_data_read_f(fd_set *rfd);
static status_t comm_data_write_f(fd_set *wfd);


status_t comm_server_init(u16_t port)
{
	hManager = (SERVER_S *)malloc(sizeof(SERVER_S));
	if (hManager == NULL)
		return ERROR_T;
	memset(hManager, 0x00, sizeof(SERVER_S));

	hManager->hMutex = mutex_create_f();
	if (hManager->hMutex == NULL) {
		comm_server_release();
		return ERROR_T;
	}
	
	if (comm_socket_listen_f(port) != OK_T) {
		comm_server_release();
		return ERROR_T;
	}

	if (comm_thread_create_f() != OK_T) {
		comm_server_release();
		return ERROR_T;
	}

	return OK_T;
}

status_t comm_server_release(void)
{
	status_t		retb;
	SOCKET_S		*pNode;

	if (hManager == NULL)
		return OK_T;
	retb = mutex_lock_f(hManager->hMutex, -1);
	comm_thread_destroy_f();
	comm_socket_close_f();
	pNode = hManager->header;
	while(pNode) {
		SOCKET_S		*tail;
		tail = pNode->next;
		comm_destroy_f(pNode);
		pNode = tail;
	}
	hManager->header = NULL;
	if (retb == OK_T)
		mutex_unlock_f(hManager->hMutex);
	if (hManager->hMutex) {
		mutex_destroy_f(hManager->hMutex);
		hManager->hMutex = NULL;
	}
	free(hManager);
	hManager = NULL;
	return OK_T;
}

static status_t commThread(void_t *arg)
{
	hManager->b_loop = TRUE_T;
	while(hManager->b_loop) {
		struct timeval		tv = {0, 200000};
		fd_set				rfd, wfd, efd;
		s32_t				nsock = -1;

		nsock = comm_fdset_f(&rfd, &wfd, &efd);

		if (select(nsock + 1, &rfd, &wfd, &efd, &tv) <= 0)
			continue;
		
		if ((hManager->listen_sock > 0) && FD_ISSET(hManager->listen_sock, &rfd)) {
			s32_t				sock;
			u32_t				sin_size;
			struct sockaddr_in	caddr;
			printf("socket connect\n");
			memset(&caddr, 0x00, sizeof(struct sockaddr_in));
			sin_size = sizeof(struct sockaddr_in);
			sock = (s32_t)accept(hManager->listen_sock, (struct sockaddr *)&caddr, &sin_size);
			printf("sock = %d\n", sock);
			if (sock > 0) {
				if (comm_create_f(sock) == NULL) 
					close(sock);
			}
		}
		comm_data_read_f(&rfd);
		comm_data_write_f(&wfd);	
	}
	return OK_T;
}

static s32_t comm_fdset_f(fd_set *rfd, fd_set *wfd, fd_set *efd)
{
	s32_t		nsock = -1;
	status_t	retb;
	SOCKET_S	*pNode;

	FD_ZERO(rfd);
	FD_ZERO(wfd);
	FD_ZERO(efd);

	FD_SET(hManager->listen_sock, rfd);
	FD_SET(hManager->listen_sock, efd);
	nsock = hManager->listen_sock;
	retb = mutex_lock_f(hManager->hMutex, -1);
	pNode = hManager->header;
	while (pNode) {
		u32_t           size;
		SOCKET_S         *tail;
                
		tail = pNode->next;
		if (pNode->sock > nsock)
			nsock = pNode->sock;
		FD_SET(pNode->sock, rfd);
		FD_SET(pNode->sock, efd);
		if (xbuffer_size_get_f(pNode->sbuf, &size) == ERROR_T) {
			pNode = tail;
			continue;
		}
		if (size == 0) {
			pNode = tail; 
			continue;
		}
		FD_SET(pNode->sock, wfd);

		pNode = tail;
	}

	if (retb == OK_T)
		mutex_unlock_f(hManager->hMutex);
	return nsock;
}

static status_t comm_socket_listen_f(u16_t port)
{
	struct sockaddr_in	saddr;
	int					on;
	hManager->listen_sock = (s32_t)socket(AF_INET, SOCK_STREAM, 0);
	printf("listen_sock = %d\n", hManager->listen_sock);

	if (hManager->listen_sock <= 0)
		return ERROR_T;
	on = 1;
	setsockopt(hManager->listen_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	printf("bind start\n");
	if(bind(hManager->listen_sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr)) == -1) {
		close(hManager->listen_sock);
		return ERROR_T;
	}
	printf("listen start\n");
	if (listen(hManager->listen_sock, COMM_MAX_LISTEN) == -1) {
		close(hManager->listen_sock);
		return ERROR_T;
	}
	printf("listen ok port = %d\n", port);
	hManager->listen_port = port;

	return OK_T;
}

static SOCKET_S *comm_create_f(s32_t sock)
{
	SOCKET_S		*h = NULL;
	status_t		retb;
	
	if (hManager == NULL)
		return NULL;
	retb = mutex_lock_f(hManager->hMutex, 500);
	h = (SOCKET_S *)malloc(sizeof(SOCKET_S));
	if (h == NULL) {
		if (retb == OK_T)
			mutex_unlock_f(hManager->hMutex);
		return NULL;
	}
	memset(h, 0x00, sizeof(SOCKET_S));
	h->fota = fota_frame_create(NULL);
	if (h->fota == NULL) {
		free(h);
		if (retb == OK_T)
			mutex_unlock_f(hManager->hMutex);
		return NULL;
	}
	h->sbuf = xbuffer_create_f(SEND_BUFFER_MAX_SIZE);
	if (h->sbuf == NULL) {
		free(h->fota);
		free(h);
		if (retb == OK_T)
			mutex_unlock_f(hManager->hMutex);
		return NULL;
	}
	h->sock = sock;

	if (hManager->header)
		hManager->header->prev = h;
	h->next = hManager->header;
	hManager->header = h;
	if (retb == OK_T)
		mutex_unlock_f(hManager->hMutex);
	return h;
}

static status_t comm_thread_create_f(void)
{
	hManager->hThread = thread_create_f("commThread", 100, 1024*128, commThread, NULL);
	if (hManager->hThread == NULL) 
		return ERROR_T;
	return OK_T;
}

static status_t comm_thread_destroy_f(void)
{
	hManager->b_loop = FALSE_T;
	if (hManager->hThread) {
		thread_join_f(hManager->hThread, -1);
		hManager->hThread = NULL;
	}

	return OK_T;
}

static status_t comm_socket_close_f(void)
{
	if (hManager->listen_sock > 0)
		close(hManager->listen_sock);
	hManager->listen_sock = 0;
	hManager->listen_port = 0;

	return OK_T;
}

static status_t comm_destroy_f(void_t *handle)
{
	SOCKET_S	*h = (SOCKET_S *)handle;
	SOCKET_S	*pNode;
	status_t	retb;

	if (h == NULL)
		return ERROR_T;

	if (hManager == NULL)
		return ERROR_T;
	retb = mutex_lock_f(hManager->hMutex, 500);
	pNode = hManager->header;
	while(pNode) {
		if (h == pNode)
			break;
		pNode = pNode->next;
	}
	if (pNode == NULL) {
		if (retb == OK_T)
			mutex_unlock_f(hManager->hMutex);
		return ERROR_T;
	}
	if (h == hManager->header) {
		hManager->header = h->next;
		if (hManager->header)
			hManager->header->prev = NULL;
	}
	else {
		h->prev->next = h->next;
		if (h->next)
			h->next->prev = h->prev;
	}
	if (h->sbuf)
		xbuffer_destroy_f(h->sbuf);
	if (h->fota)
		free(h->fota);
	if (h->sock > 0)
		close(h->sock);
	free(h);
	if (retb == OK_T)
		mutex_unlock_f(hManager->hMutex);

	return OK_T;
}

static status_t comm_data_read_f(fd_set *rfd)
{
	status_t		retb;
	SOCKET_S		*pNode;
	retb = mutex_lock_f(hManager->hMutex, 500);
	pNode = hManager->header;
	while(pNode) {
		SOCKET_S         *tail;
		tail = pNode->next;
		if ((pNode->sock > 0) && FD_ISSET(pNode->sock, rfd)) {
			s32_t           reti;
			s8_t            buf[2048];
			reti = recv(pNode->sock, buf, 2048, 0);
			printf("reti = %d\n", reti);
			if (reti > 0) {
				fota_analyze(pNode->fota, (u8_t *)buf, reti);
			}
		}
		pNode = tail;
	}
	if (retb == OK_T)
		mutex_unlock_f(hManager->hMutex);
	return OK_T;
}

static status_t comm_data_write_f(fd_set *wfd)
{
	status_t                retb;
	SOCKET_S         *pNode;

	retb = mutex_lock_f(hManager->hMutex, 500);
	pNode = hManager->header;
	while(pNode) {
		SOCKET_S         *tail;

		tail = pNode->next;
		if ((pNode->sock > 0) && FD_ISSET(pNode->sock, wfd)) {
			u32_t           size;
			s32_t           reti;
			s8_t            buf[1400];

			if (xbuffer_size_get_f(pNode->sbuf, &size) != OK_T) {
				pNode = tail;
				continue;
			}
			if (size == 0) {
				pNode = tail;
				continue;
			}
			if (size > 1400)
				size = 1400;
			if (xbuffer_read_f(pNode->sbuf, buf, size) != OK_T) {
				pNode = tail;
				continue;
			}
			reti = send(pNode->sock, buf, size, 0);
			if (reti < 0)
				comm_destroy_f(pNode);
			else if (reti == 0)
				comm_destroy_f(pNode);
		}
		pNode = tail;
	}
	if (retb == OK_T)
		mutex_unlock_f(hManager->hMutex);

	return OK_T;
}

status_t comm_send_data(void_t* h, s8_t *ptr, s32_t size)
{
	SOCKET_S	*hsock;
	hsock = (SOCKET_S *)h;
	if (hManager == NULL)
		return ERROR_T;
	if (hManager->header == NULL)
		return ERROR_T;
	xbuffer_write_f(hManager->header->sbuf, ptr, size);
//	xbuffer_write_f(hsock->sbuf, ptr, size);
	return OK_T;
}





