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
 *	ready_queue.h (T-Kernel/OS)
 *	Ready Queue Operation Routine
 */

#ifndef _READY_QUEUE_
#define _READY_QUEUE_

#include <libstr.h>
//#include "bitop.h"
#include <tstdlib/bitop.h>

/*
 * Definition of ready queue structure
 *	In the ready queue, the task queue 'tskque' is provided per priority.
 *	The task TCB is registered onto queue with the applicable priority.
 *	For effective ready queue search, the bitmap area 'bitmap' is provided
 *	to indicate whether there are tasks in task queue per priority.
 *	
 *	Also, to search a task at the highest priority in the ready queue
 *    	effectively, put the highest task priority in the 'top_priority' field.
 *	If the ready queue is empty, set the value in this field to NUM_PRI.
 *	In this case, to return '0' with refering 'tskque[top_priority]',
 *      there is 'null' field which is always '0'.
 *
 *	Multiple READY tasks with kernel lock do not exist at the same time.
 */

#define BITMAPSZ	( sizeof(UW) * 8 )
#define NUM_BITMAP	( (NUM_PRI + BITMAPSZ - 1) / BITMAPSZ )

typedef	struct ready_queue {
	INT	top_priority;		/* Highest priority in ready queue */
	QUEUE	tskque[NUM_PRI];	/* Task queue per priority */
	TCB	*null;			/* When the ready queue is empty, */
	UW	bitmap[NUM_BITMAP];	/* Bitmap area per priority */
	TCB	*klocktsk;		/* READY task with kernel lock */
} RDYQUE;

IMPORT RDYQUE	ready_queue;

/*
 * Ready queue initialization
 */
Inline void ready_queue_initialize( RDYQUE *rq )
{
	INT	i;

	rq->top_priority = NUM_PRI;
	for ( i = 0; i < NUM_PRI; i++ ) {
		QueInit(&rq->tskque[i]);
	}
	rq->null = NULL;
	rq->klocktsk = NULL;
	memset(rq->bitmap, 0, sizeof(rq->bitmap));
}

/*
 * Return the highest priority task in ready queue
 */
Inline TCB* ready_queue_top( RDYQUE *rq )
{
	/* If there is a task at kernel lock, that is the highest priority task */
	if ( rq->klocktsk != NULL ) {
		return rq->klocktsk;
	}

	return (TCB*)rq->tskque[rq->top_priority].next;
}

/*
 * Return the priority of the highest priority task in the ready queue
 */
Inline INT ready_queue_top_priority( const RDYQUE *rq )
{
	return rq->top_priority;
}

/*
 * Insert task in ready queue
 *	Insert it at the end of the same priority tasks with task priority
 *	indicated with 'tcb'. Set the applicable bit in the bitmap area and
 *	update 'top_priority' if necessary. When updating 'top_priority,'
 *	return TRUE, otherwise FALSE.
 */
Inline BOOL ready_queue_insert( RDYQUE *rq, TCB *tcb )
{
	INT	priority = tcb->priority;

	tcb->slicecnt = 0;

	QueInsert(&tcb->tskque, &rq->tskque[priority]);
	tstdlib_bitset(rq->bitmap, priority);

	if ( tcb->klocked ) {
		rq->klocktsk = tcb;
	}

	if ( priority < rq->top_priority ) {
		rq->top_priority = priority;
		return TRUE;
	}
	return FALSE;
}

/*
 * Insert task at head in ready queue
 */
Inline void ready_queue_insert_top( RDYQUE *rq, TCB *tcb )
{
	INT	priority = tcb->priority;

	QueInsert(&tcb->tskque, rq->tskque[priority].next);
	tstdlib_bitset(rq->bitmap, priority);

	if ( tcb->klocked ) {
		rq->klocktsk = tcb;
	}

	if ( priority < rq->top_priority ) {
		rq->top_priority = priority;
	}
}

/*
 * Delete task from ready queue
 *	Take out TCB from the applicable priority task queue, and if the task
 *	queue becomes empty, clear the applicable bit from the bitmap area.
 *	In addition, update 'top_priority' if the deleted task had the highest
 *	priority. In such case, use the bitmap area to search the second
 *	highest priority task.
 */
Inline void ready_queue_delete( RDYQUE *rq, TCB *tcb )
{
	INT	priority = tcb->priority;
	INT	i;

	if ( rq->klocktsk == tcb ) {
		rq->klocktsk = NULL;
	}

	QueRemove(&tcb->tskque);
	if ( tcb->klockwait ) {
		/* Delete from kernel lock wait queue */
		tcb->klockwait = FALSE;
		return;
	}
	if ( !isQueEmpty(&rq->tskque[priority]) ) {
		return;
	}

	tstdlib_bitclr(rq->bitmap, priority);
	if ( priority != rq->top_priority ) {
		return;
	}

	i = tstdlib_bitsearch1_binsearch(rq->bitmap, priority, NUM_PRI - priority);
	if ( i >= 0 ) {
		rq->top_priority = priority + i;
	} else {
		rq->top_priority = NUM_PRI;
	}
}

/*
 * Move the task, whose ready queue priority is 'priority', at head of
 * queue to the end of queue. Do nothing, if the queue is empty.
 */
Inline void ready_queue_rotate( RDYQUE *rq, INT priority )
{
	QUEUE	*tskque = &rq->tskque[priority];
	TCB	*tcb;

	tcb = (TCB*)QueRemoveNext(tskque);
	if ( tcb != NULL ) {
		QueInsert((QUEUE*)tcb, tskque);

		tcb->slicecnt = 0;
	}
}

/*
 * Put 'tcb' to the end of ready queue.
 */
Inline TCB* ready_queue_move_last( RDYQUE *rq, TCB *tcb )
{
	QUEUE	*tskque = &rq->tskque[tcb->priority];

	QueRemove(&tcb->tskque);
	QueInsert(&tcb->tskque, tskque);

	tcb->slicecnt = 0;

	return (TCB*)tskque->next;	/* New task at head of queue */
}

#endif /* _READY_QUEUE_ */
