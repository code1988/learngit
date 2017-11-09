#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "utypes.h"

void_t* thread_create_f(s8_t	*name, 
						u8_t	priority, 
						u32_t	stack, 
						void_t	*func, 
						void_t	*arg);

status_t thread_destroy_f(void_t *handle);
status_t thread_join_f(void_t *handle, s32_t msec);
void_t msleep_f(u32_t msecond);
void_t* mutex_create_f(void);
status_t mutex_destroy_f(void_t *handle);
status_t mutex_lock_f(void_t *handle, s32_t msec);
status_t mutex_unlock_f(void_t *handle);
void_t* semaphore_create_f(void);
status_t semaphore_destroy_f(void_t *handle);
status_t semaphore_lock_f(void_t *handle, s32_t msec);
status_t semaphore_unlock_f(void_t *handle);

#endif	/* _UTILITY_H_ */

