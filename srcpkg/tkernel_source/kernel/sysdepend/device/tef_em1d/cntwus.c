/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/07/28
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
__attribute__ ((noinline))
LOCAL void WaitLoop( UW count )
{
	Asm("	_loop:	subs	%0, %0, #1	\n"
	"		bhi	_loop		"
		: "=r"(count)
		: "0"(count + 1)
	);
}

/*
 * WaitUsec()  calculate the loop count for a given time (in microseconds)
 *	interrupt-disabled state is assumed.
 */
EXPORT void CountWaitUsec( void )
{
	UW	t0, t1, t2;
	UW	cnt;
	UW	d;

	/* stop timer */
	out_w(TI_OP, 0);

	/* select clock */
	out_w(TI0TIN_SEL, (in_w(TI0TIN_SEL) & ~3) | TITIN_PLL3);

	/* supply clock */
	out_w(GCLKCTRL3ENA, in_w(GCLKCTRL3ENA) | TI0_TIN_GCK);
	out_w(GCLKCTRL3,    in_w(GCLKCTRL3)    | TI0_TIN_GCK);

	/* enable timer */
	out_w(TI_OP, TM_EN);
	while ( (in_w(TI_SCLR) & TM_SCLR) != 0 );
	WaitLoop(100);

	/* set counter */
	out_w(TI_SET, 0xffffffff);

	/* start timer counting */
	out_w(TI_OP, TSTART|TM_EN);

	WaitLoop(100);		/* wait for a little (just in case) */

	/* measurement */
	t0 = in_w(TI_RCR);
	WaitLoop(1001);
	t1 = in_w(TI_RCR);
	WaitLoop(21001);
	t2 = in_w(TI_RCR);

	/* stop timer */
	out_w(TI_OP, 0);

	/* stop clock */
	out_w(GCLKCTRL3, in_w(GCLKCTRL3) & ~TI0_TIN_GCK);

	/* the time for 20000 loops is calculated excluding the
           overhead for the rest of measurement time. */
	cnt = (t2 - t1) - (t1 - t0);

	/* Calculate the loop count that spends 64 microseconds
	 *		         20000 loops
	 *	loop64us = ---------------------- * 64usec
	 *		     cnt * (1 / TIN_CLK)
	 *	TIN_CLK = input clock to the timer [Hz]
	 */
	d = in_w(DIVTIMTIN);
	SCInfo.loop64us = TIN_CLK(d) / (cnt * (50/2)) * (64/2);
}
