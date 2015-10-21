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
status_t ifaddr_set_f(s8_t* ifname, s32_t ip, s32_t mask, s32_t gate);
status_t ifether_get_f(s8_t *ifname, s32_t *ip, s32_t *mask);
status_t ifether_status_f(s8_t *ifname);
status_t ifmac_get_f(s8_t *ifname, s8_t *mac);
status_t datetime_set_f(s32_t year, s32_t months, s32_t day, s32_t hour, s32_t minute, s32_t second);
status_t datetime_get_f(s32_t *year, s32_t *months, s32_t *day, s32_t *hour, s32_t *minute, s32_t *second);
status_t datetime_separate_f(s32_t time, s32_t *year, s32_t *month, s32_t *day, 
								s32_t *hour, s32_t *minute, s32_t *second);	




#endif	/* _UTILITY_H_ */

