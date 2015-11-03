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
 *	timer.c (T-Kernel/OS)
 *	Timer Control
 */

#include <tk/kernel.h>
#include <tk/timer.h>
#include <tk/task.h>
#include "tkdev_timer.h"
#include <sys/rominfo.h>

/*
 * Timer interrupt interval (us)
 */
EXPORT RELTIM_U	TIMER_PERIOD;

/*
 * Current time (Software clock)
 *	'current_time' shows the total operation time since
 *	operating system Starts. 'real_time_ofs' shows difference
 *	between the current time and the operating system clock
 *	(current_time). Do not change 'current_time' when setting
 *	time by 'set_tim()'. Set 'real_time_ofs' with the time  	
 *   	difference between 'current_time' and setup time.
 *	Therefore 'current_time' does not affect with time change
 *	and it increases simply.
 */
EXPORT LSYSTIM	current_time;	/* System operation time */
EXPORT LSYSTIM	real_time_ofs;	/* Actual time - System operation time */

/*
 * Timer event queue
 */
LOCAL QUEUE	timer_queue;

/*
 * Initialization of timer module
 */
#ifdef _STD_X86_
EXPORT ER timer_initialize(void)
#else
EXPORT void timer_initialize(void);
#endif
{
	INT	n, t[2];

	/* Get system information */
	n = _tk_get_cfn(SCTAG_TTIMPERIOD, t, 2);
	if ( n < 1 ) return E_SYS;
	if ( n < 2 ) t[1] = 0;
	//TIMER_PERIOD = to_usec(t[0]) + t[1];
	TIMER_PERIOD = t[0] + t[1] * 1000;
	if ( TIMER_PERIOD < MIN_TIMER_PERIOD
	  || TIMER_PERIOD > MAX_TIMER_PERIOD ) {
		return E_SYS;
	}

	current_time = real_time_ofs = ltoll(0);
	QueInit(&timer_queue);

	/* Start timer interrupt */
#ifdef _STD_X86_
	return(start_hw_timer());
#else
	start_hw_timer();

	return;
#endif
}

/*
 * Stop timer
 */
EXPORT void timer_shutdown( void )
{
	terminate_hw_timer();
}

/*
 * Obtain the time for correction
 *	Obtain the time for correction used for setting timeout, etc.
 *     (in microseconds).
 *      This returns the elapsed time from the last timer interrupt.
 */
EXPORT RELTIM_U adjust_time( void )
{
	return get_hw_timer_usec();
}

/*
 * Insert timer event to timer event queue
 */
LOCAL void enqueue_tmeb( TMEB *event )
{
	QUEUE	*q;

	for ( q = timer_queue.next; q != &timer_queue; q = q->next ) {
		if ( ll_cmp(event->time, ((TMEB*)q)->time) < 0) {
			break;
		}
	}
	QueInsert(&event->queue, q);
}

/*
 * Set timeout event
 *	Register the timer event 'event' onto the timer queue to
 *	start after the timeout 'tmout'. At timeout, start with the
 *	argument 'arg' on the callback function 'callback'.
 *	When 'tmout' is TMO_FEVR, do not register onto the timer
 *	queue, but initialize queue area in case 'timer_delete'
 *	is called later.
 *
 *	"include/tk/typedef.h"
 *	typedef	INT		TMO;
 *	typedef UINT		RELTIM;
 *	#define TMO_FEVR	(-1)
 */
EXPORT void timer_insert( TMEB *event, TMO_U tmout, CBACK callback, void *arg )
{
	event->callback = callback;
	event->arg = arg;

	if ( tmout == TMO_FEVR ) {
		QueInit(&event->queue);
	} else {
		/* To guarantee longer wait time specified by 'tmout',
		   add adjust_time() on wait time */
		event->time = ll_add( ll_add(current_time, ltoll(tmout)),
					uitoll(adjust_time()) );
		enqueue_tmeb(event);
	}
}

EXPORT void timer_insert_reltim( TMEB *event, RELTIM_U tmout, CBACK callback, void *arg )
{
	event->callback = callback;
	event->arg = arg;

	/* To guarantee longer wait time specified by 'tmout',
	   add adjust_time() on wait time */
	event->time = ll_add( ll_add(current_time, ultoll(tmout)),
				uitoll(adjust_time()) );
	enqueue_tmeb(event);
}

/*
 * Set time specified event
 *	Register the timer event 'evt' onto the timer queue to start at the
 *	(absolute) time 'time'.
 *	'time' is not an actual time. It is system operation time.
 */
EXPORT void timer_insert_abs( TMEB *evt, LSYSTIM time, CBACK cback, void *arg )
{
	evt->callback = cback;
	evt->arg = arg;
	evt->time = time;
	enqueue_tmeb(evt);
}

/* ------------------------------------------------------------------------ */

/*
 * Timer interrupt handler
 *	Timer interrupt handler starts every TIMER_PERIOD millisecond
 *	interval by hardware timer. Update the software clock and start the
 *	timer event upon arriving at start time.
 */
EXPORT void timer_handler( void )
{
	TMEB	*event;

	clear_hw_timer_interrupt();		/* Clear timer interrupt */

	BEGIN_CRITICAL_SECTION;
	current_time = ll_add(current_time, to_usec(uitoll(TIMER_PERIOD)));

	if ( ctxtsk != NULL ) {
		/* Task at execution */
		if ( ctxtsk->sysmode > 0 ) {
			ctxtsk->stime += TIMER_PERIOD;
		} else {
			ctxtsk->utime += TIMER_PERIOD;
		}

		if ( schedtsk == ctxtsk ) {
			schedtsk = time_slice_schedule(ctxtsk);
			if ( schedtsk != ctxtsk ) {
				dispatch_request();
			}
		}
	}

	/* Execute event that passed occurring time. */
	while ( !isQueEmpty(&timer_queue) ) {
		event = (TMEB*)timer_queue.next;
		
		if ( ll_cmp(event->time, current_time) > 0 ) {
			break;
		}

		QueRemove(&event->queue);
		if ( event->callback != NULL ) {
			(*event->callback)(event->arg);
		}
	}

	//END_CRITICAL_SECTION;
if ( !isDI(_cpsr_)
 && ctxtsk != schedtsk
  && !isTaskIndependent()
 && !dispatch_disabled ) {
 	//vd_printf("start dispatch\n");
					dispatch();
				}
	//vd_printf("end dispatch\n");
	
				enaint(_cpsr_); }
	end_of_hw_timer_interrupt();		/* Clear timer interrupt */
}
