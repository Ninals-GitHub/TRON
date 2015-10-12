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
 *	@(#)chkspc3.c (libtk)
 *
 *	Address check
 */

#include <basic.h>
#include <tk/syslib.h>
#include <sys/segment.h>

#if VIRTUAL_ADDRESS

#include "getsvcenv.h"

/*
 * Check address space (B string)
 *	Checks to see that the memory area from str through to either
 *	'\0' or the max byte is valid.
 *	If max = 0, the number of bytes (max) is ignored.
 *	If the memory area is valid, the value returned is either the
 *	number of bytes or the max value (through to the max byte with
 *	max !=0 and no '\0').
 *	If the memory area is not valid, the error code is returned.
 */
EXPORT INT ChkSpaceBstrR( CONST UB *str, INT max )
{
	return ChkSpaceBstr(str, max, MA_READ, getsvcenv());
}
EXPORT INT ChkSpaceBstrRW( CONST UB *str, INT max )
{
	return ChkSpaceBstr(str, max, MA_READ|MA_WRITE, getsvcenv());
}

#else /* VIRTUAL_ADDRESS */

LOCAL INT chklen( CONST UB *p, INT max )
{
	INT	len = 0;

	while ( *p++ != '\0' ) {
		len++;
		if ( --max == 0 ) {
			break;
		}
	}
	return len;
}

EXPORT INT ChkSpaceBstrR( CONST UB *str, INT max )
{
	return chklen(str, max);
}
EXPORT INT ChkSpaceBstrRW( CONST UB *str, INT max )
{
	return chklen(str, max);
}

#endif /* VIRTUAL_ADDRESS */
