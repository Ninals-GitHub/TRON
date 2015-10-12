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
 *	klock.c (T-Kernel/OS)
 *	Kernel Lock
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include "ready_queue.h"

/*
 * Object lock
 *	Do not call from critical section
 */
EXPORT void LockOBJ( OBJLOCK *loc )
{
	BOOL	klocked;

  retry:
	BEGIN_CRITICAL_SECTION;
	klocked = ctxtsk->klocked;
	if ( !klocked ) {
		if ( loc->wtskq.next == NULL ) {
			/* Lock */
			QueInit(&loc->wtskq);

			ctxtsk->klocked = klocked = TRUE;
			ready_queue.klocktsk = ctxtsk;
		} else {
			/* Ready for lock */
			ready_queue_delete(&ready_queue, ctxtsk);
			ctxtsk->klockwait = TRUE;
			QueInsert(&ctxtsk->tskque, &loc->wtskq);

			schedtsk = ready_queue_top(&ready_queue);
			dispatch_request();
		}
	}
	END_CRITICAL_SECTION;
	/* Since wait could be freed without getting lock,
	   need to re-try if lock is not got */
	if ( !klocked ) {
		goto retry;
	}
}

/*
 * Object unlock
 *	It may be called from a critical section.
 */
EXPORT void UnlockOBJ( OBJLOCK *loc )
{
	TCB	*tcb;

	BEGIN_CRITICAL_SECTION;
	ctxtsk->klocked = FALSE;
	ready_queue.klocktsk = NULL;

	tcb = (TCB*)QueRemoveNext(&loc->wtskq);
	if ( tcb == NULL ) {
		/* Free lock */
		loc->wtskq.next = NULL;
	} else {
		/* Wake lock wait task */
		tcb->klockwait = FALSE;
		tcb->klocked = TRUE;
		ready_queue_insert_top(&ready_queue, tcb);
	}

	schedtsk = ready_queue_top(&ready_queue);
	if ( ctxtsk != schedtsk ) {
		dispatch_request();
	}
	END_CRITICAL_SECTION;
}

/* ------------------------------------------------------------------------ */

/*
 * Extended SVC lock
 *	Do not call from critical section
 */
EXPORT void LockSVC( SVCLOCK *loc )
{
	BOOL	lock;

  retry:
	BEGIN_CRITICAL_SECTION;
	lock = ( ctxtsk->svclocked == loc );
	if ( !lock ) {
		if ( loc->wtskq.next == NULL ) {
			/* Lock */
			QueInit(&loc->wtskq);

			loc->locklist = ctxtsk->svclocked;
			ctxtsk->svclocked = loc;
			lock = TRUE;
		} else {
			/* Ready for lock */
			ready_queue_delete(&ready_queue, ctxtsk);
			ctxtsk->klockwait = TRUE;
			QueInsert(&ctxtsk->tskque, &loc->wtskq);

			schedtsk = ready_queue_top(&ready_queue);
			dispatch_request();
		}
	}
	END_CRITICAL_SECTION;
	/* Since wait could be freed without getting lock,
	   need to re-try if lock is not got */
	if ( !lock ) {
		goto retry;
	}
}

/*
 * Extended SVC unlock
 */
EXPORT void UnlockSVC( void )
{
	SVCLOCK	*loc;
	TCB	*tcb;

	BEGIN_CRITICAL_SECTION;
	loc = ctxtsk->svclocked;
	ctxtsk->svclocked = loc->locklist;

	tcb = (TCB*)QueRemoveNext(&loc->wtskq);
	if ( tcb == NULL ) {
		/* Free lock */
		loc->wtskq.next = NULL;
	} else {
		/* Wake lock wait task */
		tcb->klockwait = FALSE;
		loc->locklist = tcb->svclocked;
		tcb->svclocked = loc;
		ready_queue_insert(&ready_queue, tcb);

		schedtsk = ready_queue_top(&ready_queue);
		if ( ctxtsk != schedtsk ) {
			dispatch_request();
		}
	}
	END_CRITICAL_SECTION;
}

/*
 * Unlock all extended SVCs in which 'tcb' tasks are locked
 */
EXPORT void AllUnlockSVC( TCB *tcb )
{
	SVCLOCK	*loc;

	BEGIN_CRITICAL_SECTION;
	while ( (loc = ctxtsk->svclocked) != NULL ) {
		ctxtsk->svclocked = loc->locklist;

		tcb = (TCB*)QueRemoveNext(&loc->wtskq);
		if ( tcb == NULL ) {
			/* Free lock */
			loc->wtskq.next = NULL;
		} else {
			/* Wake lock wait task */
			tcb->klockwait = FALSE;
			loc->locklist = tcb->svclocked;
			tcb->svclocked = loc;
			ready_queue_insert(&ready_queue, tcb);
		}
	}
	schedtsk = ready_queue_top(&ready_queue);
	if ( ctxtsk != schedtsk ) {
		dispatch_request();
	}
	END_CRITICAL_SECTION;
}
