/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/09/22
 *
 *----------------------------------------------------------------------
 */

/*
 *	Calculate the value of required count for loop-wait for a given period (in micro seconds)
 */

#include <basic.h>
#include <sys/sysinfo.h>
#include <tk/syslib.h>
#include <tk/tkdev_conf.h>

/*
 * Loop wait
 *	disable the inline expansion caused by compiler optimization to obtain accurate
 *      measurement.
 */
#if 0
__attribute__ ((noinline))
LOCAL void WaitLoop( UW count )
{
	return;
}
#endif

/*
 * WaitUsec()  calculate the loop count for a given time (in microseconds)
 *	interrupt-disabled state is assumed.
 */
EXPORT void CountWaitUsec( void )
{
	return;
}
