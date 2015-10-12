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
 *	@(#)waitnsec.c (libtk/EM1-D512)
 *
 *	Busy loop wait time in nanoseconds
 */

#include <basic.h>
#include <sys/sysinfo.h>

EXPORT void WaitNsec( UINT nsec )
{
	UW	count = nsec * SCInfo.loop64us / 64000U;

	Asm("	loop:	subs	%0, %0, #1	\n"
	"		bhi	loop		"
		: "=r"(count)
		: "0"(count + 1)
	);
}
