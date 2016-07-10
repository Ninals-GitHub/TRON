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
 *	task.c (T-Kernel/OS)
 *	Task Control
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include <tk/wait.h>
#include "ready_queue.h"
#include <cpu.h>
#include "tkdev_timer.h"
#include "check.h"
#include <sys/rominfo.h>

/*
 * Task dispatch disable state
 */
EXPORT INT	dispatch_disabled;	/* DDS_XXX see task.h */

/*
 * Task execution control
 */
EXPORT TCB	*ctxtsk;	/* Task in execution */
EXPORT TCB	*schedtsk;	/* Task which should be executed */
EXPORT RDYQUE	ready_queue;	/* Ready queue */

/*
 * Task control information
 */
EXPORT TCB	*tcb_table;	/* Task control block */
EXPORT QUEUE	free_tcb;	/* FreeQue */
EXPORT ID	max_tskid;	/* Maximum task ID */

EXPORT INT	default_sstksz;	/* Default system stack size */
EXPORT INT	svc_call_limit;	/* Protect level of system call */

/*
 * TCB Initialization
 */
EXPORT ER task_initialize( void )
{
	INT	i;
	TCB	*tcb;
	ID	tskid;

	/* Get system information */
	i = _tk_get_cfn(SCTAG_TMAXTSKID, &max_tskid, 1);
	if ( i < 1 || NUM_TSKID < 1 ) {
		return E_SYS;
	}
	i = _tk_get_cfn(SCTAG_TSYSSTKSZ, &default_sstksz, 1);
	if ( i < 1 || default_sstksz < MIN_SYS_STACK_SIZE ) {
		return E_SYS;
	}
	i = _tk_get_cfn(SCTAG_TSVCLIMIT, &svc_call_limit, 1);
	if ( i < 1 || svc_call_limit < 0 || svc_call_limit > 3 ) {
		return E_SYS;
	}

	/* Allocate TCB area */
	tcb_table = Imalloc((UINT)NUM_TSKID * sizeof(TCB));
	if ( tcb_table == NULL ) {
		return E_NOMEM;
	}
	
	/* Initialize task execution control information */
	ctxtsk = schedtsk = NULL;
	ready_queue_initialize(&ready_queue);
	dispatch_disabled = DDS_ENABLE;

	/* Register all TCBs onto FreeQue */
	QueInit(&free_tcb);
	for ( tcb = tcb_table, i = 0; i < NUM_TSKID; tcb++, i++ ) {
		tskid = ID_TSK(i);
		tcb->tskid = tskid;
		tcb->state = TS_NONEXIST;
#ifdef NUM_PORID
		tcb->wrdvno = tskid;
#endif
		InitSVCLOCK(&tcb->svclock);
		tcb->svclocked = NULL;

		QueInsert(&tcb->tskque, &free_tcb);
	}

	return E_OK;
}

/*
 * Prepare task execution.
 */
EXPORT void make_dormant( TCB *tcb )
{
	/* Initialize variables which should be reset at DORMANT state */
	tcb->state	= TS_DORMANT;
	tcb->priority	= tcb->bpriority = tcb->ipriority;
	tcb->sysmode	= tcb->isysmode;
	tcb->wupcnt	= 0;
	tcb->suscnt	= 0;
	tcb->waitmask	= 0;

	tcb->nodiswai	= FALSE;
	tcb->klockwait	= FALSE;
	tcb->klocked	= FALSE;

	tcb->slicetime	= 0;
	tcb->stime	= 0;
	tcb->utime	= 0;

#ifdef NUM_MTXID
	tcb->mtxlist	= NULL;
#endif
	tcb->tskevt	= 0;

	tcb->reqdct	= 0;	/* clear DCT request */

	tcb->texhdr	= NULL; /* Undefined task exception handler */
	tcb->texmask	= 0;
	tcb->pendtex	= 0;
	tcb->exectex	= 0;
	tcb->texflg	= 0;
	tcb->execssid	= 0;

	/* Set context to start task */
	setup_context(tcb);
}

/* ------------------------------------------------------------------------ */

/*
 * Scheduling by time slice
 *	Add TIMER_PERIOD to the 'tcb' task's continuous execution
 *	time counter, and then if the counter exceeds the maximum
 *	continuation time, put the task at the end of the ready queue.
 */
EXPORT TCB* time_slice_schedule( TCB *tcb )
{
	if ( tcb != NULL && tcb->slicetime > 0 ) {

		tcb->slicecnt += TIMER_PERIOD;
		if ( tcb->slicecnt > tcb->slicetime ) {
			tcb = ready_queue_move_last(&ready_queue, tcb);
		}
	}

	return tcb; /* New head task */
}

/*
 * Reselect task to execute
 *	Set 'schedtsk' to the head task at the ready queue.
 */
Inline void reschedule( void )
{
	TCB	*toptsk;

	toptsk = ready_queue_top(&ready_queue);
	if ( schedtsk != toptsk ) {
		/*
		 * When the state becomes RUN to READY,
		 * execute the time slice scheduling.
		 */
		if ( schedtsk == ctxtsk ) {
			time_slice_schedule(schedtsk);
		}

		schedtsk = toptsk;
		dispatch_request();
	}
}

/*
 * Set task to READY state.
 *	Update the task state and insert in the ready queue. If necessary,
 *	update 'schedtsk' and request to start task dispatcher.
 */
EXPORT void make_ready( TCB *tcb )
{
	tcb->state = TS_READY;
	if ( ready_queue_insert(&ready_queue, tcb) ) {
		/*
		 * When the state becomes RUN to READY,
		 * execute the time slice scheduling.
		 */
		if ( schedtsk == ctxtsk ) {
			time_slice_schedule(schedtsk);
		}

		schedtsk = tcb;

		dispatch_request();
	}
}

/*
 * Set task to non-executable state.
 *	Delete the task from the ready queue.
 *	If the deleted task is 'schedtsk', set 'schedtsk' to the
 *	highest priority task in the ready queue.
 *	'tcb' task must be READY.
 */
EXPORT void make_non_ready( TCB *tcb )
{
	ready_queue_delete(&ready_queue, tcb);
	if ( schedtsk == tcb ) {
		schedtsk = ready_queue_top(&ready_queue);
		dispatch_request();
	}
}

/*
 * Change task priority.
 */
EXPORT void change_task_priority( TCB *tcb, INT priority )
{
	INT	oldpri;

	if ( tcb->state == TS_READY ) {
		/*
		 * When deleting a task from the ready queue,
		 * a value in the 'priority' field in TCB is needed.
		 * Therefore you need to delete the task from the
		 * ready queue before changing 'tcb->priority.'
		 */
		ready_queue_delete(&ready_queue, tcb);
		tcb->priority = (UB)priority;
		ready_queue_insert(&ready_queue, tcb);
		reschedule();
	} else {
		oldpri = tcb->priority;
		tcb->priority = (UB)priority;

		/* If the hook routine at the task priority change is defined,
		   execute it */
		if ( (tcb->state & TS_WAIT) != 0 && tcb->wspec->chg_pri_hook) {
			(*tcb->wspec->chg_pri_hook)(tcb, oldpri);
		}
	}
}

/*
 * Rotate ready queue.
 */
EXPORT void rotate_ready_queue( INT priority )
{
	ready_queue_rotate(&ready_queue, priority);
	reschedule();
}

/*
 * Rotate the ready queue including the highest priority task.
 */
EXPORT void rotate_ready_queue_run( void )
{
	if ( schedtsk != NULL ) {
		ready_queue_rotate(&ready_queue,
				ready_queue_top_priority(&ready_queue));
		reschedule();
	}
}

/* ------------------------------------------------------------------------ */
/*
 *	Debug support function
 */
#if USE_DBGSPT

/*
 * Refer ready queue
 */
SYSCALL INT _td_rdy_que( PRI pri, ID list[], INT nent )
{
	QUEUE	*q, *tskque;
	INT	n = 0;

	CHECK_PRI(pri);

	BEGIN_DISABLE_INTERRUPT;
	tskque = &ready_queue.tskque[int_priority(pri)];
	for ( q = tskque->next; q != tskque; q = q->next ) {
		if ( n++ < nent ) {
			*(list++) = ((TCB*)q)->tskid;
		}
	}
	END_DISABLE_INTERRUPT;

	return n;
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_task_id
 Input		:struct task *task
 		 < task to get its id >
 Output		:void
 Return		:ID
 		 < task id >
 Description	:get task id calculated from task_table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ID get_task_id(struct task *task)
{
	ID index = (ID)(task - tcb_table);
	
	return(ID_TSK(index));
}

#endif /* USE_DBGSPT */
