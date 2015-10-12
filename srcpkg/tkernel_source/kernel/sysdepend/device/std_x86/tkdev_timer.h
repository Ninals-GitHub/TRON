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
 *	tkdev_timer.h (EM1-D512)
 *	Hardware-Dependent Timer Processing
 */

#ifndef _TKDEV_TIMER_
#define _TKDEV_TIMER_

#include <tk/syslib.h>
#include <sys/sysinfo.h>
#include <tk/tkdev_conf.h>

#include <device/std_x86/pit.h>
#include <device/std_x86/pic.h>


/*
 * Range of settable period  (microseconds)
 */
#define	MIN_TIMER_PERIOD	10
#define	MAX_TIMER_PERIOD	50000

/*
 * Setting up timer
 */
Inline void init_hw_timer( void )
{
	return;
}

/*
 * Timer start processing
 *	Initialize timer, and start periodic timer interrupt.
 */
Inline ER start_hw_timer( void )
{
	/* -------------------------------------------------------------------- */
	/* initialize programmable interval controller				*/
	/* -------------------------------------------------------------------- */
	return(initPit());
}

/*
 * Clear timer interrupt
 *	clear timer interrupt. Depending on hardware, we have to clear
 *	the request of the timer interrupt at the beginning of the
 *      timer handler, or clear it at the end.
 *	clear_hw_timer_interrupt() is called at the beginning of the
 *	timer interrupt handler.
 *	end_of_hw_timer_interrupt() is called at the end.
 *	Either one of them, or both are used according hardware requirements.
 */
Inline void clear_hw_timer_interrupt( void )
{
	return;
}
Inline void end_of_hw_timer_interrupt( void )
{
	/* Enable the current interrupt */
	EndOfInterrupt1();
}

/*
 * Timer stop processing
 *	stop timer
 *	called during system shutdown
 */
Inline void terminate_hw_timer( void )
{
	return;
}

/*
 * Obtain the elapsed time (nanoseconds) from the last timer interrupt
 * To compensate for the possibility that the timer interrupt may have
 * occurred during the interval when the interrupt was disabled,
 * we calculate the time in the following range:
 *		0 <= elapsed time < TIMER_PERIOD * 2
 */
Inline UINT get_hw_timer_nsec( void )
{
	return 0;
}

/*
 * Similar to the function as above, but returns value in microseconds.
 */
Inline UINT get_hw_timer_usec( void )
{
	return 0;
}

#endif /* _TKDEV_TIMER_ */
