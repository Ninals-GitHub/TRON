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
 *	@(#)chkspc2.c (libtk)
 *
 *	Address check
 */

#include <basic.h>
#include <tk/syslib.h>
#include <sys/segment.h>

#if VIRTUAL_ADDRESS

#include "getsvcenv.h"

/*
 * Check address space (TC string)
 *	Checks to see that the memory area from str through to
 *	either TNULL or the max character is valid.
 *	If max = 0, the number of characters (max) is ignored.
 * 	If the memory area is valid, the value returned is either
 *	the number of characters or the max value (through to the
 *	max character with max !=0 character and no TNULL).
 *	If the memory area is not valid, the error code is returned.
 */
EXPORT INT ChkSpaceTstrR( CONST TC *str, INT max )
{
	return ChkSpaceTstr(str, max, MA_READ, getsvcenv());
}
EXPORT INT ChkSpaceTstrRW( CONST TC *str, INT max )
{
	return ChkSpaceTstr(str, max, MA_READ|MA_WRITE, getsvcenv());
}

#else /* VIRTUAL_ADDRESS */

LOCAL INT chklen( CONST TC *p, INT max )
{
	INT	len = 0;

	while ( *p++ != TNULL ) {
		len++;
		if ( --max == 0 ) {
			break;
		}
	}
	return len;
}

EXPORT INT ChkSpaceTstrR( CONST TC *str, INT max )
{
	return chklen(str, max);
}
EXPORT INT ChkSpaceTstrRW( CONST TC *str, INT max )
{
	return chklen(str, max);
}

#endif /* VIRTUAL_ADDRESS */
