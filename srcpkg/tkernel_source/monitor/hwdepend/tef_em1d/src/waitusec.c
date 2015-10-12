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
 *	waitusec.c
 *
 *       EM1-D512: micro wait
 */

#include "sysdepend.h"
#include <arm/em1d512.h>

LOCAL	UW	delay64us;		 // wait for 64 microsec

/*
 * wait for nanoseconds
 */
EXPORT	void	waitNsec(_UW nsec)
{
	for (nsec = nsec * delay64us / 64000; nsec > 0; nsec--);

	return;
}

/*
 * wait for microseconds
 */
EXPORT	void	waitUsec(_UW usec)
{
	for (usec = usec * delay64us / 64; usec > 0; usec--);

	return;
}

/*
 * wait for milliseconds
 */
EXPORT	void	waitMsec(UW msec)
{
	while (msec-- > 0) waitUsec(1000);

	return;
}

/* ------------------------------------------------------------------------ */

/*
 * setting up the initial count for micro-wait()
 */
EXPORT	void	setupWaitUsec(void)
{
	UW	t0, t1, t2;

#define	MAX_CNT		(ACPU_CLK * 64 / 10)	// 1 Clock
#define	MIN_CNT		(ACPU_CLK * 64 / 1280)	// 128 Clock

        /* use TI0 timer, and assume clock is PLL3 / 8 */
	out_w(Txx_OP(TI0), 0);			// Timer stop, count clear
	while (in_w(Txx_RCR(TI0)));

	out_w(Txx_SET(TI0), 0xffffffff);	// maximum count
	out_w(Txx_OP(TI0), 0x00000003);		// Timer start

	delay64us = 64;
	waitUsec(1000);				// wait for a while until things settle down

	t0 = in_w(Txx_RCR(TI0));
	waitUsec(1000);
	t1 = in_w(Txx_RCR(TI0));
	waitUsec(3000);
	t2 = in_w(Txx_RCR(TI0));

	out_w(Txx_OP(TI0),0);			// Timer stop, count clear
	while (in_w(Txx_RCR(TI0)));
	
	t2 -= t1;	// count for 3000 times
	t1 -= t0;	// count for 1000 times
	t2 -= t1;	// count for 2000 times

	/*
         * calculate the count for 64 microsec
	 *
         *                    2000 loops x timer clock [MHz] x 64 [microsec]
	 *	delay64us = ------------------------------------------------
	 *	                                 t2
	 *
         * * considering the representation of PLL3_CLK (1/1000MHz unit), and setting of pre scaler,
         * it can be written down as follows.
	 *
         *                    2 loops x PLL3_CLK [1/1000MHz] x 8 [microsec]
	 *	delay64us = -------------------------------------------
	 *	                                 t2
	 *
	 */
	delay64us = (t2 == 0) ? MAX_CNT : ((2 * PLL3_CLK * 8) / t2);
	if (delay64us > MAX_CNT) delay64us = MAX_CNT;
	else if (delay64us < MIN_CNT) delay64us = MIN_CNT;

	return;
}

