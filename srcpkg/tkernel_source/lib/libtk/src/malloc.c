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
 *	@(#)malloc.c (libtk)
 *
 *	Non resident system memory allocation
 */

#include "libtk.h"
#include <sys/util.h>

EXPORT void* malloc( size_t size )
{
	return Vmalloc(size);
}

EXPORT void* calloc( size_t nmemb, size_t size )
{
	return Vcalloc(nmemb, size);
}

EXPORT void* realloc( void *ptr, size_t size )
{
	return Vrealloc(ptr, size);
}

EXPORT void free( void *ptr )
{
	Vfree(ptr);
}
