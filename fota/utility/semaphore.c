/*
 *
 *  Filename        : utility.h
 *  Version         : 1.0.0.0
 *  Author          : Xiaole
 *	Created         : 2009/3/28
 *  Description     :
 *
 *  History         :
 *  1.  Date        : 2009/3/28
 *      Author      : Xiaole
 *      Modification: Created file
 *	2.	Date		: 2009/9/19
 *		Author		: Xiaole
 *		Modification: add function and struct describe
 *
*/

/****************************************************************************/
/*								INCLUDE										*/
/****************************************************************************/
#if defined(WIN32)
#include <windows.h>
#endif

#if defined(LINUX)
#include <pthread.h>
#include <semaphore.h>
#endif

#include <errno.h>
#include "utypes.h"

/****************************************************************************/
/*								STRUCT										*/
/****************************************************************************/
/*
 * 同步信号结构
*/
typedef struct tagSemaphoreS {
#if defined(LINUX)
	sem_t				hsem;
#endif
#if defined(WIN32)
	HANDLE				hsem;
#endif
}semaphore_s;

/****************************************************************************/
/*								FUNCTION									*/
/****************************************************************************/

/*
 *	Function :	semaphore_create_f
 *	Describer:	semaphore create
 *	Parameter:	NONE
 *	return   :	semaphore handle
*/
void_t* semaphore_create_f(void)
{
	semaphore_s			*h;
#if defined(LINUX)
	int					reti;
#endif

	h = (semaphore_s *)malloc(sizeof(semaphore_s));
	if (h == NULL)
		return NULL;

#if defined(LINUX)
	reti = sem_init(&(h->hsem), 0, 0);
	if (reti != 0) {
		switch(reti) {
			case EAGAIN:
				break;
			case ENOSPC:
				break;
			case EPERM:
				break;
			default:
				break;
		}
		free(h);
		return NULL;
	}
#endif

#if defined(WIN32)
	h->hsem = CreateEvent(NULL, 0, 0, NULL);
	if (h->hsem == NULL) {
		free(h);
		return NULL;
	}
#endif

	return (void_t *)h;
}

/*
 *	Function :	semaphore_destroy_f
 *	Describer:	semaphore destroy
 *	Parameter:	h				semaphore handle
 *	return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t semaphore_destroy_f(void_t *handle)
{
#if defined(LINUX)
	int			reti;
#endif	
	semaphore_s		*h = (semaphore_s *)handle;

	if (h == NULL_T) 
		return ERROR_T;

#if defined(LINUX)
	reti = sem_destroy(&(h->hsem));
	if (reti != 0) {
		switch(reti) {
			case EBUSY:
				break;
			case EINVAL:
				break;
			default:
				break;
		}
		return ERROR_T;
	}
#endif

#if defined(WIN32)
	CloseHandle(h->hsem);
#endif

	free(h);

	return OK_T;
}

/*
 *	Function :	semaphore_lock_f
 *	Describer:	semaphore lock
 *	Parameter:	h		semaphore handle
 *				msec	timeout value, if msec = -1. It will be waitforever until thread exit.
 *	return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t semaphore_lock_f(void_t *handle, s32_t msec)
{
#if defined(LINUX)
	int			reti;
#endif
#if defined(WIN32)
	DWORD		ret;
#endif
	semaphore_s		*h = (semaphore_s *)handle;

	if (h == NULL_T) 
		return ERROR_T;

#if defined(LINUX)
	reti = sem_wait(&(h->hsem));
	if (reti != 0) {
		switch(reti) {
			case EINVAL:
				break;
			case EDEADLK:
				break;
			case EINTR:
				break;
			default:
				break;
		}
		return ERROR_T;
	}
#endif

#if defined(WIN32)
	ret = WaitForSingleObject(h->hsem, -1);
	if (ret != WAIT_OBJECT_0)
		return ERROR_T;
#endif

	return OK_T;
}

/*
 *	Function :	semaphore_unlock_f
 *	Describer:	semaphore unlock
 *	Parameter:	h				semaphore handle
 *	return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t semaphore_unlock_f(void_t *handle)
{
#if defined(LINUX)
	int			reti;
#endif
	semaphore_s		*h = (semaphore_s *)handle;

	if (h == NULL_T) 
		return ERROR_T;

#if defined(LINUX)
	reti = sem_post(&(h->hsem));
	if (reti != 0) {
		switch(reti) {
			case EINVAL:
				break;
			default:
				break;
		}
		return ERROR_T;
	}
#endif

#if defined(WIN32)
	SetEvent(h->hsem);
#endif

	return OK_T;
}





