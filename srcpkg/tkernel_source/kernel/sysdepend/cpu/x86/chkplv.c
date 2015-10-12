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
 *	chkplv.c (EM1-D512)
 *	Check Call Protected Level
 */

#include "sysmgr.h"
#include <sys/sysinfo.h>

IMPORT INT	svc_call_limit;	/* SVC protected level (T-Kernel/OS) */

Inline INT PPL( void )
{
	return (SCInfo.taskmode >> 16) & 3;
}

/*
 * Check protected level of extended SVC caller
 */
EXPORT ER ChkCallPLevel( void )
{
	return ( PPL() > svc_call_limit )? E_OACV: E_OK;
}
