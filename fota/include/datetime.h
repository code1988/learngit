#ifndef _DATE_TIME_H_
#define _DATE_TIME_H_

#include "utypes.h"

/*
 *	Function :	tick_get_f
 *	Describer:	get current tick (tick count after cpu reset)
 *	Parameter:	NONE
 *	return   :	tick count
*/
u32_t tick_get_f(void);

/*
 *	Function :	time_set_f
 *	Describer:	set system time
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t time_set_f(s32_t hour, s32_t minute, s32_t second);

/*
 *	Function :	time_get_f
 *	Describer:	get system time
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t time_get_f(s32_t *hour, s32_t *minute, s32_t *second);

/*
 *	Function :	date_set_f
 *	Describer:	set system date
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t date_set_f(s32_t year, s32_t months, s32_t day);

/*
 *	Function :	date_get_f
 *	Describer:	get system date
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t date_get_f(s32_t *year, s32_t *months, s32_t *day);

/*
 *	Function :	datetime_set_f
 *	Describer:	set system datetime
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t datetime_set_f(s32_t year, s32_t months, s32_t day, s32_t hour, s32_t minute, s32_t second);

/*
 *	Function :	datetime_get_f
 *	Describer:	get system datetime
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t datetime_get_f(s32_t *year, s32_t *months, s32_t *day, s32_t *hour, s32_t *minute, s32_t *second);

/*
 *	Function :	zone_set_f
 *	Describer:	set system time zone
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t zone_set_f(s32_t zone);

/*
 *	Function :	zone_get_f
 *	Describer:	get system time zone
 *	return   :	time zone
*/
s32_t zone_get_f(void);

/*
 *	Function :	week_get_f
 *	Describer:	get system week day
 *	return   :	week day
*/
s32_t week_get_f(void);

s32_t datetime_get_ex(void);

status_t datetime_set_ex(s32_t second);

status_t datetime_separate_f(s32_t time, s32_t *year, s32_t *month, s32_t *day, 
				s32_t *hour, s32_t *minute, s32_t *second);

status_t datetime_compose_f(s32_t *time, s32_t year, s32_t month, s32_t day, 
				s32_t hour, s32_t minute, s32_t second);


#endif	//_DATE_TIME_H_



