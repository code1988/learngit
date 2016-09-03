/*
 *
 *  Filename        : thread.c
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
 *  2.	Date		: 2009/9/17
 *		Author		: Xiaole
 *		Modification: rename function name
 *					  support thread_create_f's parameter
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
#include <unistd.h>
#endif

#include <errno.h>

#include "utypes.h"


/****************************************************************************/
/*								STRUCT										*/
/****************************************************************************/
/*
 * thread struct
*/

typedef struct tagThreadS {
#if defined(LINUX)
	pthread_t		taskID;
#endif
#if defined(WIN32)
	HANDLE			taskID;
#endif
}thread_s;

/****************************************************************************/
/*								FUNCTION									*/
/****************************************************************************/
/*
 *	Function :	thread_create_f
 *	Describer:	create thread
 *	Parameter:	name			thread name		(string length max 31 bytes)
 *				priority		thread priority (1-254)
 *				stack			thread stack
 *				start_routine	thread entry
 *				arg				entry parameter
 *	return   :	thread handle
*/
void_t* thread_create_f(s8_t *name, u8_t priority, u32_t stack, void_t *func, void_t *arg)
{
#if defined(LINUX)
	int					ret;
	pthread_attr_t		attr;
//	struct sched_param	param;
	size_t					size;
#endif
	thread_s			*pthread = NULL_T;

	pthread = (thread_s *)malloc(sizeof(thread_s));
	if (pthread == NULL) 
		return NULL_T;

#if defined(LINUX)

	pthread_attr_init(&attr);

	/* set thread priority */
//	pthread_attr_getschedparam(&attr, &param);
//	param.sched_priority = priority;
//	pthread_attr_setschedparam(&attr, &param);

	/* get default thread stack size */
	pthread_attr_getstacksize(&attr, &size);
	/* set thread stack size */
	pthread_attr_setstacksize(&attr, stack);

#if 0
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
#endif

	ret = pthread_create(&(pthread->taskID), &attr, func, arg);
	if (ret != 0) {
		switch(ret) {
			case EAGAIN:
				break;
			case EINVAL:
				break;
			case EPERM:
				break;
			default:
				break;
		}
		free((void *)pthread);
		return NULL_T;
	}
#endif

#if defined(WIN32)
	pthread->taskID = CreateThread(NULL, stack, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL);
	if (pthread->taskID == NULL) {
		free((void *)pthread);
		return NULL_T;
	}
#endif

	return (void_t *)pthread;
}

/*
 *	Function :	thread_destroy_f
 *	Describer:	thread destroy
 *	Parameter:	h				thread handle
 *	return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t thread_destroy_f(void_t *handle)
{
	thread_s	*h = (thread_s *)handle;

	if (h == NULL_T) 
		return ERROR_T;

#if defined(LINUX)
	pthread_exit(&(h->taskID));
#endif

#if defined(WIN32)
	if (h->taskID)
		TerminateThread(h->taskID, 0);
#endif
	free((void *)h);

	return OK_T;
}

/*
 *	Function :	thread_join_f
 *	Describer:	thread verity
 *	Parameter:	h		thread handle
 *				msec	timeout value, if msec = -1. It will be waitforever until thread exit.
 *						notice it is invalid in linux. function will be waitforever until
 *						thread exit
 *	return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t thread_join_f(void_t *handle, s32_t msec)
{
#if defined(WIN32)
	DWORD		ret;
#endif
#if defined(LINUX)
	int			reti;
#endif
	thread_s	*h = (thread_s *)handle;

	if (h == NULL)
		return ERROR_T;

#if defined(LINUX)
	/* notice: pthread_join can not support timeout mode. so the second parameter set NULL */
	reti = pthread_join(h->taskID, NULL);
	if (reti != 0) {
		switch(reti) {
			case EINVAL:
				break;
			case ESRCH:
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
	ret = WaitForSingleObject(h->taskID, msec);
	if (ret != WAIT_OBJECT_0) {
		free(h);
		return ERROR_T;
	}
#endif

	return OK_T;
}

/*
 *	Function :	msleep_f
 *	Describer:	thread sleep
 *	Parameter:	msecond			sleep timeout (ms)
 *	return   :	NONE
*/
void_t msleep_f(u32_t msecond)
{
#if defined(LINUX)
	usleep(msecond*1000);
#endif

#if defined(WIN32)
	Sleep(msecond);
#endif

	return;
}



