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
 *	wait.h (T-Kernel/OS)
 *	Definition of Common Routine for Synchronization
 */

#ifndef _WAIT_
#define _WAIT_

#include <sys/queue.h>
#include <tk/timer.h>
#include <tk/task.h>

/*
 * Release wait state of the task.
 *	Remove the task from the timer queue and the wait queue, then
 *	update the task state. 'wait_release_ok' sends E_OK to the
 *	wait released task.
 *	'wait_release_ok_recd' is normal wait release as well as
 *	'wait_release_ok', but it sends 'ercd' to the wait released task.
 *	It needs to be ercd >= 0.
 *	'wait_release_ng' sends 'ercd' to the wait released task. Use for
 *	releasing the forced wait task. It needs to be ercd < 0.
 *	'wait_release_tmout' don't remove from the timer queue. Use for
 *	time out processing.
 */
IMPORT void wait_release_ok( TCB *tcb );
IMPORT void wait_release_ok_ercd( TCB *tcb, ER ercd );
IMPORT void wait_release_ng( TCB *tcb, ER ercd );
IMPORT void wait_release_tmout( TCB *tcb );

/*
 * Cancel task wait state.
 *	Remove the task from the timer queue and the wait queue.
 *	Do not update the task state.
 */
Inline void wait_cancel( TCB *tcb )
{
	timer_delete(&tcb->wtmeb);
	QueRemove(&tcb->tskque);
}

/*
 * Change the active task to wait state and connect to the
 * timer event queue.
 */
IMPORT void make_wait( TMO_U tmout, ATR atr );
IMPORT void make_wait_reltim( RELTIM_U tmout, ATR atr );

/*
 * Release wait state of all tasks connected to the wait queue,
 * and set it as E_DLT error.
 * Use when synchronization between tasks or communication
 * object is deleted.
 */
IMPORT void wait_delete( QUEUE *wait_queue );

/*
 * Get ID of the head task in the wait queue.
 */
IMPORT ID wait_tskid( QUEUE *wait_queue );

/*
 * Connect the task to the prioritized wait queue.
 */
Inline void queue_insert_tpri( TCB *tcb, QUEUE *queue )
{
	QUEUE *q;
	QUEUE *start, *end;
	UB val;
	W offset;

	start = end = queue;
	val = tcb->priority;
	offset = offsetof(TCB,priority);

	for ( q = start->next; q != end; q = q->next ) {
		if ( *(UB*)((VB*)q + offset) > val ) {
			break;
		}
	}

	QueInsert(&tcb->tskque, q);
}

/*
 * Common part of control block
 *	For synchronization between tasks and communication object,
 *	the head part of control block is common. The followings are
 *	common routines.
 *	Define common part as GCB (generic control block) type.
 *	Cannot use these routines if an object has multiple wait queues
 *	and when it operates a wait queue after the first one.
 *	Cannot use these routines if TA_TPRI, TA_NODISWAI object attribute
 *	bits are used for other purposes since these bits are checked.
 */
typedef struct generic_control_block {
	QUEUE	wait_queue;	/* Wait queue */
	ID	objid;		/* Object ID */
	void	*exinf;		/* Extended information */
	ATR	objatr;		/* Object attribute */
	/* It is OK to have another field after this point, */
	/* but it is not used for generic operation routines. */
} GCB ;

/*
 * Change the active task to wait state and connect to the timer event
 * queue and the object wait queue. Also, set 'wid' in 'ctxtsk'.
 */
IMPORT void gcb_make_wait( GCB *gcb, TMO_U tmout );

/*
 * Check wait disable
 */
Inline BOOL is_diswai( GCB *gcb, TCB *tcb, UINT tskwait )
{
	return ( (tcb->waitmask & tskwait) != 0
	      && (gcb->objatr & TA_NODISWAI) == 0 );
}

/*
 * gcb_make_wait with wait disable check function.
 */
IMPORT void gcb_make_wait_with_diswai( GCB *gcb, TMO_U tmout );

/*
 * When the task priority changes, adjust the task position in the
 * wait queue.
 * Do nothing if TA_TPRI is not specified in the object attribute.
 */
IMPORT void gcb_change_priority( GCB *gcb, TCB *tcb );

/*
 * Search the first task of wait queue include "tcb" with target.
 * (Not insert "tcb" into wait queue.)
 *
 */
IMPORT TCB* gcb_top_of_wait_queue( GCB *gcb, TCB *tcb );

#endif /* _WAIT_ */
