/*
 *
 *  Filename        : xbuffer.h
 *  Version         : 1.0.0.0
 *  Author          : Xiaole
 *	Created         : 2009/3/28
 *  Description     :
 *
 *  History         :
 *  1.  Date        : 2009/3/28
 *      Author      : Xiaole
 *      Modification: Created file
 *
*/


#ifndef _XBUFFER_H_
#define _XBUFFER_H_

/****************************************************************************/
/*								INCLUDE										*/
/****************************************************************************/
#include "utypes.h"

/****************************************************************************/
/*								STRUCT										*/
/****************************************************************************/
typedef struct tagXBufferS {
	s8_t			*ptr;
	u32_t			size;
	u32_t			len;
	u32_t			p_w;
	u32_t			p_r;
	void_t			*mutex;	
}XBUFFER_S;

/****************************************************************************/
/*								FUNCTION									*/
/****************************************************************************/

/*
 *	Function :	xbuffer_create_f
 *	Describer:	Buffer Create
 *	Parameter:	size			Buffer max size
 *	Return   :	Buffer Handle
*/
XBUFFER_S *xbuffer_create_f(u32_t size);

/*
 *	Function :	xbuffer_destroy_f
 *	Describer:	Buffer Destroy
 *	Parameter:	h				Buffer Handle
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_destroy_f(XBUFFER_S *h);

/*
 *	Function :	xbuffer_write_f
 *	Describer:	Buffer Write
 *	Parameter:	h				Buffer Handle
 *				ptr				write buffer point
 *				size			write size
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_write_f(XBUFFER_S *h, s8_t *ptr, u32_t size);

/*
 *	Function :	xbuffer_read_f
 *	Describer:	Buffer read
 *	Parameter:	h				Buffer Handle
 *				ptr				read buffer point
 *				size			read size
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_read_f(XBUFFER_S *h, s8_t *ptr, u32_t size);

/*
 *	Function :	xbuffer_size_get_f
 *	Describer:	Buffer use size get
 *	Parameter:	h				Buffer Handle
 *				size			size return point
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_size_get_f(XBUFFER_S *h, u32_t *size);

/*
 *	Function :	xbuffer_free_get_f
 *	Describer:	Buffer free size get
 *	Parameter:	h				Buffer Handle
 *				size			size return point
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_free_get_f(XBUFFER_S *h, u32_t *size);

/*
 *	Function :	xbuffer_reset_f
 *	Describer:	Buffer reset
 *	Parameter:	h				Buffer Handle
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_reset_f(XBUFFER_S *h);


#endif /* _XBUFFER_H_ */




