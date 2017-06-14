/*
 *
 *  Filename        : datetime.c
 *  Version         : 1.0.0.0
 *  Author          : Xiaole
 *  Created         : 2009/5/30
 *  Description     :
 *
 *  History         :
 *  1.  Date        : 2009/5/30
 *      Author      : Xiaole
 *      Modification: Created file
 *
*/

#if defined(WIN32)
#include <windows.h>
#include <time.h>
#endif

#if defined(LINUX)
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#endif

#include <stdio.h>
#include "utypes.h"

/*
 *	Function :	tick_get_f
 *	Describer:	get current tick (tick count after cpu reset)
 *	Parameter:	NONE
 *	return   :	tick count
*/
u32_t tick_get_f(void)
{
	u32_t		msec;
#if defined(LINUX)
	struct timeb	tp;
	
	ftime(&tp);
	msec = tp.time;
	msec = msec*1000;
	msec += tp.millitm;
#endif
#if defined(WIN32)
	int				tick;
	tick = GetTickCount();
	msec = tick*10;
#endif
	return msec;
}

/*
 *	Function :	time_set_f
 *	Describer:	set system time
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t time_set_f(s32_t hour, s32_t minute, s32_t second)
{
#if defined(WIN32)
	SYSTEMTIME			lpSystemTime;

	GetLocalTime(&lpSystemTime);
	lpSystemTime.wHour = hour;
	lpSystemTime.wMinute = minute;
	lpSystemTime.wSecond = second;
	SetLocalTime(&lpSystemTime);
#endif

#if defined(LINUX)
	struct timeval		tv;
	struct timezone		tz;
	struct tm			tm;
	
	gettimeofday(&tv, &tz);
	localtime_r(&(tv.tv_sec), &tm);
	
	tm.tm_hour = hour;
	tm.tm_min = minute;
	tm.tm_sec = second;
	tv.tv_sec = mktime(&tm);

	settimeofday(&tv, &tz);
#ifdef RTC_TIME_SUPPORT
	{
		int rtc;
		rtc = open("/dev/rtc0", O_WRONLY);
		if (rtc > 0) {
			ioctl(rtc, RTC_SET_TIME, &tm);
			close(rtc);
		}
	}
#endif
#endif
	return OK_T;
}

/*
 *	Function :	time_get_f
 *	Describer:	get system time
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t time_get_f(s32_t *hour, s32_t *minute, s32_t *second)
{
#if defined(WIN32)
	SYSTEMTIME			lpSystemTime;

	GetLocalTime(&lpSystemTime);
	*hour = lpSystemTime.wHour;
	*minute = lpSystemTime.wMinute;
	*second = lpSystemTime.wSecond;
#endif
#if defined(LINUX)
	struct timeval		tv;
	struct timezone		tz;
	struct tm			tm;
	
	gettimeofday(&tv, &tz);
	localtime_r(&(tv.tv_sec), &tm);

	*hour = tm.tm_hour;
	*minute = tm.tm_min;
	*second = tm.tm_sec;
#endif
	return OK_T;
}

/*
 *	Function :	date_set_f
 *	Describer:	set system date
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t date_set_f(s32_t year, s32_t months, s32_t day)
{
#if defined(WIN32)
	SYSTEMTIME			lpSystemTime;

	GetLocalTime(&lpSystemTime);
	lpSystemTime.wYear = year;
	lpSystemTime.wMonth = months;
	lpSystemTime.wDay = day;
	SetLocalTime(&lpSystemTime);
#endif

#if defined(LINUX)
	struct timeval		tv;
	struct timezone		tz;
	struct tm			tm;

	gettimeofday(&tv, &tz);
	localtime_r(&(tv.tv_sec), &tm);

	tm.tm_year = year - 1900;
	tm.tm_mon = months - 1;
	tm.tm_mday = day;
	tv.tv_sec = mktime(&tm);

	settimeofday(&tv, &tz);
#ifdef RTC_TIME_SUPPORT
	{
		int rtc;
		rtc = open("/dev/rtc0", O_WRONLY);
		if (rtc > 0) {
			ioctl(rtc, RTC_SET_TIME, &tm);
			close(rtc);
		}
	}
#endif
#endif
	return OK_T;
}

/*
 *	Function :	date_get_f
 *	Describer:	get system date
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t date_get_f(s32_t *year, s32_t *months, s32_t *day)
{
#if defined(WIN32)
	SYSTEMTIME			lpSystemTime;

	GetLocalTime(&lpSystemTime);
	*year = lpSystemTime.wYear;
	*months = lpSystemTime.wMonth;
	*day = lpSystemTime.wDay;
#endif
#if defined(LINUX)
	struct timeval		tv;
	struct timezone		tz;
	struct tm			tm;
	
	gettimeofday(&tv, &tz);
	localtime_r(&(tv.tv_sec), &tm);

	*year = tm.tm_year + 1900;
	*months = tm.tm_mon + 1;
	*day = tm.tm_mday;
#endif
	return OK_T;
}

/*
 *	Function :	datetime_set_f
 *	Describer:	set system datetime
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t datetime_set_f(s32_t year, s32_t months, s32_t day, s32_t hour, s32_t minute, s32_t second)
{
#if defined(WIN32)
	SYSTEMTIME			lpSystemTime;

	GetLocalTime(&lpSystemTime);
	lpSystemTime.wYear = year;
	lpSystemTime.wMonth = months;
	lpSystemTime.wDay = day;
	lpSystemTime.wHour = hour;
	lpSystemTime.wMinute = minute;
	lpSystemTime.wSecond = second;
	SetLocalTime(&lpSystemTime);
#endif

#if defined(LINUX)
	struct timeval		tv;
	struct timezone		tz;
	struct tm			tm;

	gettimeofday(&tv, &tz);
	localtime_r(&(tv.tv_sec), &tm);
	tm.tm_year = year - 1900;
	tm.tm_mon = months - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = minute;
	tm.tm_sec = second;
	tv.tv_sec = mktime(&tm);

	settimeofday(&tv, &tz);
#ifdef RTC_TIME_SUPPORT
	{
		int rtc;
		rtc = open("/dev/rtc0", O_WRONLY);
		if (rtc > 0) {
			ioctl(rtc, RTC_SET_TIME, &tm);
			close(rtc);
		}
	}
#endif
#endif	
	return OK_T;
}

/*
 *	Function :	datetime_get_f
 *	Describer:	get system datetime
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t datetime_get_f(s32_t *year, s32_t *months, s32_t *day, s32_t *hour, s32_t *minute, s32_t *second)
{
#if defined(WIN32)
	SYSTEMTIME			lpSystemTime;

	GetLocalTime(&lpSystemTime);
	*year = lpSystemTime.wYear;
	*months = lpSystemTime.wMonth;
	*day = lpSystemTime.wDay;
	*hour = lpSystemTime.wHour;
	*minute = lpSystemTime.wMinute;
	*second = lpSystemTime.wSecond;
#endif
#if defined(LINUX)
	struct timeval		tv;
	struct timezone		tz;
	struct tm			tm;
	
	gettimeofday(&tv, &tz);
	localtime_r(&(tv.tv_sec), &tm);

	*year = tm.tm_year + 1900;
	*months = tm.tm_mon + 1;
	*day = tm.tm_mday;
	*hour = tm.tm_hour;
	*minute = tm.tm_min;
	*second = tm.tm_sec;
#endif
	return OK_T;
}

/*
 *	Function :	zone_set_f
 *	Describer:	set system time zone
 *	return   :	OK_T		success
 *			 :	ERROR_T		failed
*/
status_t zone_set_f(s32_t zone)
{
#if defined(LINUX)
	struct timeval		tv;
	struct timezone		tz;
	
	gettimeofday(&tv, &tz);
	tz.tz_minuteswest = zone;
	settimeofday(&tv, &tz);
#endif
	return OK_T;
}

/*
 *	Function :	zone_get_f
 *	Describer:	get system time zone
 *	return   :	time zone
*/
s32_t zone_get_f(void)
{
#if defined(LINUX)
	struct timeval		tv;
	struct timezone		tz;

	gettimeofday(&tv, &tz);
	return tz.tz_minuteswest;
#endif
#if defined(WIN32)
	return 0;
#endif
}

/*
 *	Function :	week_get_f
 *	Describer:	get system week day
 *	return   :	week day
*/
s32_t week_get_f(void)
{
#if defined(WIN32)
	SYSTEMTIME			lpSystemTime;

	GetLocalTime(&lpSystemTime);
	return lpSystemTime.wDayOfWeek;
#endif
#if defined(LINUX)
	struct timeval		tv;
	struct timezone		tz;
	struct tm			tm;

	gettimeofday(&tv, &tz);
	localtime_r(&(tv.tv_sec), &tm);
	return tm.tm_wday;
#endif
}

s32_t datetime_get_ex(void)
{
	return (s32_t)time(NULL);
}

status_t datetime_set_ex(s32_t second)
{
#if defined(WIN32)
	struct tm			*tm;
	time_t				t;
	SYSTEMTIME			lpSystemTime;

	t = (time_t)second;

	tm = localtime(&t);
	lpSystemTime.wYear = tm->tm_year;
	lpSystemTime.wMonth = tm->tm_mon;
	lpSystemTime.wDay = tm->tm_yday;
	lpSystemTime.wHour = tm->tm_hour;
	lpSystemTime.wMinute = tm->tm_min;
	lpSystemTime.wSecond = tm->tm_sec;
	SetLocalTime(&lpSystemTime);
#endif
#if defined(LINUX)
	struct timeval		tv;
	struct timezone		tz;

	gettimeofday(&tv, &tz);
	tv.tv_sec = second;
	settimeofday(&tv, &tz);
#ifdef RTC_TIME_SUPPORT
	{
		int rtc;
		rtc = open("/dev/rtc0", O_WRONLY);
		if (rtc > 0) {
			ioctl(rtc, RTC_SET_TIME, &tm);
			close(rtc);
		}
	}
#endif
#endif

	return OK_T;
}

status_t datetime_separate_f(s32_t time, s32_t *year, s32_t *month, s32_t *day, 
								s32_t *hour, s32_t *minute, s32_t *second)
{
#if defined(LINUX)
	struct timeval		tv;
	struct tm			tm;
	
	tv.tv_sec = time;
	localtime_r(&(tv.tv_sec), &tm);
	*year = tm.tm_year + 1900;
	*month = tm.tm_mon + 1;
	*day = tm.tm_mday;
	*hour = tm.tm_hour;
	*minute = tm.tm_min;
	*second = tm.tm_sec;
#endif	
	return OK_T;
}

status_t datetime_compose_f(s32_t *time, s32_t year, s32_t month, s32_t day, 
				s32_t hour, s32_t minute, s32_t second)
{
#if defined(LINUX)
	struct tm			tm;

	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = minute;
	tm.tm_sec = second;

	*time = mktime(&tm);	
#endif
	return OK_T;
}






