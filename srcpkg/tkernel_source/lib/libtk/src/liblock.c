/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/11/03
 *
 *----------------------------------------------------------------------
 */

/*
 *	@(#)liblock.c (libtk)
 *
 *	Shared exclusive control lock in library
 */

#include <tk/libtk.h>
#include <sys/util.h>
#include <tk/util.h>

LOCAL	FastMLock	LibLock;

/* Set Object Name in .exinf for DEBUG */
#define OBJNAME_LIBLOCK		"lltk"	/* multi-lock object name for liblock */

EXPORT ER _init_liblock( void )
{
	return CreateMLock(&LibLock, OBJNAME_LIBLOCK);
}

EXPORT ER _lib_locktmo( W lockno, W tmo, BOOL ignore_mintr )
{
	return MLockTmo(&LibLock, lockno, tmo);
}

EXPORT ER _lib_lock( W lockno, BOOL ignore_mintr )
{
	return MLock(&LibLock, lockno);
}

EXPORT void _lib_unlock( W lockno )
{
	MUnlock(&LibLock, lockno);
}

EXPORT void _delete_liblock( void )
{
	DeleteMLock(&LibLock);
}
