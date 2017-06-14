


#ifndef _UTYPES_H_
#define _UTYPES_H_

#ifndef WIN32
#ifndef LINUX
#define LINUX
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32)
#include <windows.h>
#include <winsock.h>
#endif

#if defined(LINUX)
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif



typedef void				void_t;

typedef char				s8_t;
typedef unsigned char		u8_t;

typedef short				s16_t;
typedef unsigned short		u16_t;

typedef int					s32_t;
typedef unsigned int		u32_t;

#if defined(LINUX)
typedef long long			s64_t;
typedef unsigned long long	u64_t;
#endif
#if defined(WIN32)
typedef __int64				s64_t;
typedef unsigned __int64	u64_t;
#endif
typedef int					status_t;
#define	OK_T				0
#define ERROR_T				-1

#ifndef __BOOL_T_
#define __BOOL_T_
typedef int					bool_t;
#endif

#define TRUE_T				1
#define FALSE_T				0

#define NULL_T				NULL


typedef struct tagPosition {
	s32_t		xpos;				
	s32_t		ypos;				
}position_t;



typedef struct tagRectangle {
	s32_t		x;					
	s32_t		y;					
	s32_t		w;					
	s32_t		h;					
}rectangle_t;


#endif



