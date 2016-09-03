/*
 *
 *  Filename        : xbuffer.c
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


/****************************************************************************/
/*								INCLUDE										*/
/****************************************************************************/
#include "xbuffer.h"

/****************************************************************************/
/*								FUNCTION									*/
/****************************************************************************/

/*
 *	Function :	xbuffer_create_f
 *	Describer:	Buffer Create
 *	Parameter:	size			Buffer max size
 *	Return   :	Buffer Handle
*/
XBUFFER_S *xbuffer_create_f(u32_t size)
{
	XBUFFER_S		*h; 

	h = (XBUFFER_S *)malloc(sizeof(XBUFFER_S));
	if (h == NULL)
		return NULL;
	
	h->ptr = (s8_t *)malloc(size);
	if (h->ptr == NULL) {
		free(h);
		return NULL;
	}

	h->mutex = mutex_create_f();
	if (h->mutex == NULL) {
		free(h->ptr);
		free(h);
		return NULL;
	}

	h->p_r = h->p_w = 0;
	h->size = size;
	h->len = 0;

	return h;
}

/*
 *	Function :	xbuffer_destroy_f
 *	Describer:	Buffer Destroy
 *	Parameter:	h				Buffer Handle
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_destroy_f(XBUFFER_S *h)
{
	if (h == NULL)
		return ERROR_T;

	if (h->mutex)
		mutex_destroy_f(h->mutex);

	if (h->ptr)
		free(h->ptr);

	free(h);

	return OK_T;
}

/*
 *	Function :	xbuffer_write_f
 *	Describer:	Buffer Write
 *	Parameter:	h				Buffer Handle
 *				ptr				write buffer point
 *				size			write size
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_write_f(XBUFFER_S *h, s8_t *ptr, u32_t size)
{
	status_t		retb;

	if (h == NULL)
		return ERROR_T;

	retb = mutex_lock_f(h->mutex, -1);
	if ((h->size - h->len) <= size) {
		if (retb == OK_T)
			mutex_unlock_f(h->mutex);
		return ERROR_T;
	}

	if ((h->size - h->p_w) >= size) {
		memcpy(h->ptr + h->p_w, ptr, size);
		h->p_w += size;
		if (h->p_w == h->size)
			h->p_w = 0;
	}
	else {
		memcpy(h->ptr + h->p_w, ptr, h->size - h->p_w);
		memcpy(h->ptr, ptr + h->size - h->p_w, size + h->p_w - h->size);
		h->p_w = size + h->p_w - h->size;
	}
	h->len += size;

	if (retb == OK_T)
		mutex_unlock_f(h->mutex);

	return OK_T;
}

/*
 *	Function :	xbuffer_read_f
 *	Describer:	Buffer read
 *	Parameter:	h				Buffer Handle
 *				ptr				read buffer point
 *				size			read size
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_read_f(XBUFFER_S *h, s8_t *ptr, u32_t size)
{
	status_t		retb;

	if (h == NULL)
		return ERROR_T;

	retb = mutex_lock_f(h->mutex, -1);
	if (h->len < size) {
		if (retb == OK_T)
			mutex_unlock_f(h->mutex);
		return ERROR_T;
	}

	if ((h->size - h->p_r) >= size) {
		memcpy(ptr, h->ptr + h->p_r, size);
		h->p_r += size;
		if (h->p_r == h->size)
			h->p_r = 0;
	}
	else {
		memcpy(ptr, h->ptr + h->p_r, h->size - h->p_r);
		memcpy(ptr + h->size - h->p_r, h->ptr, size + h->p_r - h->size);
		h->p_r = size + h->p_r - h->size;
	}

	h->len -= size;

	if (retb == OK_T)
		mutex_unlock_f(h->mutex);

	return OK_T;
}

/*
 *	Function :	xbuffer_size_get_f
 *	Describer:	Buffer use size get
 *	Parameter:	h				Buffer Handle
 *				size			size return point
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_size_get_f(XBUFFER_S *h, u32_t *size)
{
	status_t		retb;
	if (h == NULL)
		return ERROR_T;
	retb = mutex_lock_f(h->mutex, 500);
	*size = h->len;
	if (retb == OK_T)
		mutex_unlock_f(h->mutex);

	return OK_T;
}


/*
 *	Function :	xbuffer_free_get_f
 *	Describer:	Buffer free size get
 *	Parameter:	h				Buffer Handle
 *				size			size return point
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_free_get_f(XBUFFER_S *h, u32_t *size)
{
	status_t		retb;
	if (h == NULL)
		return ERROR_T;
	retb = mutex_lock_f(h->mutex, 500);
	*size = h->size - h->len;
	if (retb == OK_T)
		mutex_unlock_f(h->mutex);
	
	return OK_T;
}

/*
 *	Function :	xbuffer_reset_f
 *	Describer:	Buffer reset
 *	Parameter:	h				Buffer Handle
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t xbuffer_reset_f(XBUFFER_S *h)
{
	status_t		retb;
	if (h == NULL)
		return ERROR_T;

	retb = mutex_lock_f(h->mutex, -1);
	h->p_r = 0;
	h->p_w = 0;
	h->len = 0;
	if (retb == OK_T)
		mutex_unlock_f(h->mutex);

	return OK_T;
}







