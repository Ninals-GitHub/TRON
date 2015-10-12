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
 *	misc.c
 *
 */

#include "sysdepend.h"

/*
 * obtain switch status
 */
EXPORT UW getDipSw( void )
{
	return DipSw;
}

/*
 * Obtain boot selection information
 */
EXPORT W bootSelect( void )
{
	if ( (DipSw & (SW_MON|SW_ABT)) != 0 ) return BS_MONITOR;

	return BS_AUTO;
}

/*
 * obtain the console port number
 */
EXPORT W getConPort( void )
{
	return 0;
}
