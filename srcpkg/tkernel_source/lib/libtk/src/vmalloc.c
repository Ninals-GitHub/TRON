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
 *	@(#)vmalloc.c (libtk)
 *
 *	Non resident system memory allocation
 */

#include "libtk.h"

#ifndef VMALLOCTEST

EXPORT void* Vmalloc( size_t size )
{
	void	*p;

	MEMLOCK( return NULL )
	p = _mem_malloc(size, &_Vmacb);
	MEMUNLOCK()

	return p;
}

EXPORT void* Vcalloc( size_t nmemb, size_t size )
{
	void	*p;

	MEMLOCK( return NULL )
	p = _mem_calloc(nmemb, size, &_Vmacb);
	MEMUNLOCK()

	return p;
}

EXPORT void* Vrealloc( void *ptr, size_t size )
{
	void	*p;

	MEMLOCK( return NULL )
	p = _mem_realloc(ptr, size, &_Vmacb);
	MEMUNLOCK()

	return p;
}

EXPORT void Vfree( void *ptr )
{
	MEMLOCK( return )
	_mem_free(ptr, &_Vmacb);
	MEMUNLOCK()
}

#else /* VMALLOCTEST */

EXPORT void Vmalloctest( int mode )
{
	MEMLOCK( return )
	_mem_malloctest(mode, &_Vmacb);
	MEMUNLOCK()
}

EXPORT BOOL Vmalloccheck( void *ptr )
{
	BOOL	v;

	MEMLOCK( return FALSE )
	v = _mem_malloccheck(ptr, &_Vmacb);
	MEMUNLOCK()

	return v;
}

#endif /* VMALLOCTEST */
