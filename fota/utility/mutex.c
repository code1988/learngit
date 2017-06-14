/*
 *
 *  Filename        : mutex.c
 *  Version         : 1.0.0.0
 *  Author          : Xiaole
 *	Created         : 2009/5/15
 *  Description     :
 *
 *  History         :
 *  1.  Date        : 2009/5/15
 *      Author      : Xiaole
 *      Modification: Created file
 *	
 *  2.	Date		: 2009/9/19
 *		Author		: Xiaole
 *		Modification: rename function name
 *					  insert debug log information
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
#endif
#include <errno.h>
#include "utypes.h"
/****************************************************************************/
/*								STRUCT										*/
/****************************************************************************/
/*
 * 互斥信号结构
*/
typedef struct tagMutexS {
#if defined(LINUX)
	pthread_mutex_t		hmutex;
#endif
#if defined(WIN32)
	CRITICAL_SECTION	hmutex;
#endif
}mutex_s;

/****************************************************************************/
/*								FUNCTION									*/
/****************************************************************************/

/*
 *	Function :	mutex_create_f
 *	Describer:	create mutex
 *	Parameter:	NONE
 *	return   :	mutex handle
*/
void_t* mutex_create_f(void)
{
	mutex_s			*h;
#if defined(LINUX)
	int				reti;
	pthread_mutexattr_t	mattr;
#endif
	h = (mutex_s *)malloc(sizeof(mutex_s));
	if (h == NULL)
		return NULL;

#if defined(LINUX)
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE_NP);
	reti = pthread_mutex_init(&(h->hmutex), &mattr);
	if (reti != 0) {
		switch(reti) {
			case EAGAIN:
				break;
			case ENOMEM:
				break;
			case EPERM:
				break;
			case EBUSY:
				break;
			case EINVAL:
				break;
			default:
				break;
		}
		free(h);
		return NULL;
	}
#endif

#if defined(WIN32)
	InitializeCriticalSection(&(h->hmutex));
#endif

	return (void_t *)h;
}

/*
 *	Function :	mutex_destroy_f
 *	Describer:	mutex destroy
 *	Parameter:	h			mutex handle
 *	return   :	OK_T		success
 *				ERROR_T		failed
*/
status_t mutex_destroy_f(void_t *handle)
{
#if defined(LINUX)
	int			reti;
#endif
	mutex_s		*h = (mutex_s *)handle;
	
	if (h == NULL_T) 
		return ERROR_T;

#if defined(LINUX)
	reti = pthread_mutex_destroy(&(h->hmutex));
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
	DeleteCriticalSection(&(h->hmutex));
#endif

	free(h);

	return OK_T;
}

/*
 *	Function :	mutex_lock_f
 *	Describer:	mutex lock
 *	Parameter:	h		mutex handle
 *				msec	timeout value, if msec = -1. It will be waitforever until thread exit.
 *	return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t mutex_lock_f(void_t *handle, s32_t msec)
{
#if defined(LINUX)
	int 		reti;
#endif
#if defined(WIN32)
	DWORD		ret;
#endif
	mutex_s		*h = (mutex_s *)handle;

	if (h == NULL_T) 
		return ERROR_T;

#if defined(LINUX)
	reti = pthread_mutex_lock(&(h->hmutex));
	if (reti != 0) {
		switch(reti) {
			case EINVAL:
				break;
			case EAGAIN:
				break;
			case EDEADLK:
				break;
			default:
				break;
		}
		return ERROR_T;
	}
#endif

#if defined(WIN32)
	EnterCriticalSection(&(h->hmutex));
#endif

	return OK_T;
}

/*
 *	Function :	mutex_unlock_f
 *	Describer:	mutex unlock
 *	Parameter:	h				mutex handle
 *	return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t mutex_unlock_f(void_t *handle)
{
	mutex_s		*h = (mutex_s *)handle;

#if defined(LINUX)
	int 		reti;
#endif

	if (h == NULL_T) 
		return ERROR_T;

#if defined(LINUX)
	reti = pthread_mutex_unlock(&(h->hmutex));
	if (reti != 0) {
		switch(reti) {
			case EINVAL:
				break;
			case EAGAIN:
				break;
			case EPERM:
				break;
			default:
				break;
		}
		return ERROR_T;
	}
#endif

#if defined(WIN32)
	LeaveCriticalSection(&(h->hmutex));
#endif

	return OK_T;
}




