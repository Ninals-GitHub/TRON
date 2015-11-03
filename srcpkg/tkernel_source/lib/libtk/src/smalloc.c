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
 *	@(#)smalloc.c (libtk)
 *
 *	Non-resident shared memory allocation
 */

#include <tk/libtk.h>

#ifndef SMALLOCTEST

EXPORT void* Smalloc( size_t size )
{
	void	*p;

	MEMLOCK( return NULL )
	p = _mem_malloc(size, &_Smacb);
	MEMUNLOCK()

	return p;
}

EXPORT void* Scalloc( size_t nmemb, size_t size )
{
	void	*p;

	MEMLOCK( return NULL )
	p = _mem_calloc(nmemb, size, &_Smacb);
	MEMUNLOCK()

	return p;
}

EXPORT void* Srealloc( void *ptr, size_t size )
{
	void	*p;

	MEMLOCK( return NULL )
	p = _mem_realloc(ptr, size, &_Smacb);
	MEMUNLOCK()

	return p;
}

EXPORT void Sfree( void *ptr )
{
	MEMLOCK( return )
	_mem_free(ptr, &_Smacb);
	MEMUNLOCK()
}

#else /* SMALLOCTEST */

EXPORT void Smalloctest( int mode )
{
	MEMLOCK( return )
	_mem_malloctest(mode, &_Smacb);
	MEMUNLOCK()
}

EXPORT BOOL Smalloccheck( void *ptr )
{
	BOOL	v;

	MEMLOCK( return FALSE )
	v = _mem_malloccheck(ptr, &_Smacb);
	MEMUNLOCK()

	return v;
}

#endif /* SMALLOCTEST */
