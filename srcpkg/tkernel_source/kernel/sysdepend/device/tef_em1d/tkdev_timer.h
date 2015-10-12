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
	UW	n, d, imask;

	DI(imask);

	/* stop timer */
	out_w(TI_OP, 0);

	/* choose clock */
	out_w(TI0TIN_SEL, (in_w(TI0TIN_SEL) & ~3) | TITIN_PLL3);
	d = in_w(DIVTIMTIN);

	/* supply clock */
	out_w(GCLKCTRL3ENA, in_w(GCLKCTRL3ENA) | TI0_TIN_GCK);
	out_w(GCLKCTRL3,    in_w(GCLKCTRL3)    | TI0_TIN_GCK);

	/* enable timer */
	out_w(TI_OP, TM_EN);
	while ( (in_w(TI_SCLR) & TM_SCLR) != 0 );
	WaitUsec(100);

	/* set counter */
	n = (TIMER_PERIOD * TIN_CLK(d)) / 1000000 - 1;
	out_w(TI_SET, n);

	/* start timer count */
	out_w(TI_OP, TO_EN|TSTART|TM_EN);

	EI(imask);
}

/*
 * Timer start processing
 *	Initialize timer, and start periodic timer interrupt.
 */
Inline void start_hw_timer( void )
{
IMPORT	void	timer_handler_startup( void );

	/* set up timer */
	init_hw_timer();

	/* define interrupt handler */
	define_inthdr(VECNO_TIMER, timer_handler_startup);

	/* enable timer interrupt */
	SetIntMode(VECNO_TIMER, IM_ENA);
	ClearInt(VECNO_TIMER);
	EnableInt(VECNO_TIMER);
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
	/* Mask the current interrupt to allow multiple interrupts */
	out_w(IT0_IDS1, IRQM(IRQ_TIMER));

	/* Clear timer interrupt */
	out_w(IT0_IIR, IRQM(IRQ_TIMER));
}
Inline void end_of_hw_timer_interrupt( void )
{
	/* Enable the current interrupt */
	out_w(IT0_IEN1, IRQM(IRQ_TIMER));
}

/*
 * Timer stop processing
 *	stop timer
 *	called during system shutdown
 */
Inline void terminate_hw_timer( void )
{
	UW	imask;

	/* disable timer interrupt */
	DisableInt(VECNO_TIMER);

	DI(imask);

	/* stop timer */
	out_w(TI_OP, 0);

	/* stop clock */
	out_w(GCLKCTRL3, in_w(GCLKCTRL3) & ~TI0_TIN_GCK);

	EI(imask);
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
	UW	ofs, max, ovf, imask, d;

	DI(imask);

	d = in_w(DIVTIMTIN);

	max = in_w(TI_SET);
	do {
		ovf = in_w(IT0_RAW1) & IRQM(IRQ_TIMER);
		ofs = in_w(TI_RCR);
	} while ( ovf != (in_w(IT0_RAW1) & IRQM(IRQ_TIMER)) );
	if ( ovf != 0 ) ofs += max + 1;

	EI(imask);

	return ofs * 1000000000LL / TIN_CLK(d);
}

/*
 * Similar to the function as above, but returns value in microseconds.
 */
Inline UINT get_hw_timer_usec( void )
{
	UW	ofs, max, ovf, imask, d;

	DI(imask);

	d = in_w(DIVTIMTIN);

	max = in_w(TI_SET);
	do {
		ovf = in_w(IT0_RAW1) & IRQM(IRQ_TIMER);
		ofs = in_w(TI_RCR);
	} while ( ovf != (in_w(IT0_RAW1) & IRQM(IRQ_TIMER)) );
	if ( ovf != 0 ) ofs += max + 1;

	EI(imask);

	return (ofs + 1) * 1000000LL / TIN_CLK(d);
}

#endif /* _TKDEV_TIMER_ */
