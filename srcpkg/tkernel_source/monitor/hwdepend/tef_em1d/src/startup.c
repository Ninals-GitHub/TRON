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
 *	startup.c
 *
 *       system boot processing
 */

#include "hwdepend.h"

/* No support for the progress report yet */
#define	DispProgress(n)		/* nop */

/*
 * debug port speed
 *       default setting is LO:38400bps, and HI:115200bps. But
 *       if you need a different set value, then define the following macros in {cpu}/{machine}/setup_xxx.h :
 *       LO_BAUD_RATE, and HI_BAUD_RATE.
 */
#ifndef	LO_BAUD_RATE
#  define LO_BAUD_RATE	 38400
#endif
#ifndef	HI_BAUD_RATE
#  define HI_BAUD_RATE	115200
#endif

/*
 * initial processing after reset
 */
EXPORT void procReset( void )
{
	const MEMSEG	*mp;
	W	i;
	W	speed;

	DispProgress(0x01);

        /* system basic set up */
	resetSystem(0);
	DispProgress(0x06);

        /* setting up the initial count for micro-wait */
	setupWaitUsec();
	DispProgress(0x07);

        /* initialize console serial port */
#if SW_BHI == 0
	speed = HI_BAUD_RATE; /* HI speed is fixed. */
#else
	speed = ( (DipSw & SW_BHI) != 0 )? HI_BAUD_RATE: LO_BAUD_RATE;
#endif
	initSIO(getConPort(), speed);
	DispProgress(0x08);

        /* initialize hardware (peripherals) */
	initHardware();
	DispProgress(0x0d);

        /* memory clear is not done to save time when automatic reboot is under way. */
	if ( bootSelect() == BS_MONITOR ) {
		cpuLED(LED_MEMCLR);

                /* all memory clear (save the monitor area) */
		for ( i = 1;; ++i ) {
			mp = MemArea(MSA_OS|MSA_ERAM, i);
			if ( mp == NULL ) break;

			memset((void*)mp->top, 0, mp->end - mp->top);
		}
	}
	cpuLED(LED_POWERUP);
	DispProgress(0x0e);

        /* initialize break processing */
	initBreak();

        /* Invoking user reset initialization routine */
	callUserResetInit();
	DispProgress(0x0f);
}
