/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *
 *----------------------------------------------------------------------
 */

/*
 *	memset_w.c (common)
 *
 *	Memory operation
 */

#include <basic.h>


EXPORT void* memset_w( void *mem, int ch, size_t len )
{
	W	*Mem = (W *)mem;

	while ((len--) != 0U) {
		*(Mem++) = ch;
	}
	return mem;
}
