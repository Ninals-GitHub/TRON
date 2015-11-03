/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by T-Engine Forum at 2012/10/24.
 *    Modified by Nina Petipa at 2015/09/22
 *
 *----------------------------------------------------------------------
 */

/*
 *	task.h (T-Kernel/OS)
 *	Task Definition
 */

#ifndef _TASK_
#define _TASK_

#include <sys/queue.h>
#include <sys/str_align.h>
#include <tk/timer.h>
#include <tk/winfo.h>
#include <tstdlib/list.h>


struct process;

/*
 * Internal expression of task state
 *	Can check with 'state & TS_WAIT' whether the task is in the wait state.
 *	Can check with 'state & TS_SUSPEND' whether the task is in the forced
 *	wait state.
 */
typedef enum {
	TS_NONEXIST	= 0,	/* Unregistered state */
	TS_READY	= 1,	/* RUN or READY state */
	TS_WAIT		= 2,	/* WAIT state */
	TS_SUSPEND	= 4,	/* SUSPEND state */
	TS_WAITSUS	= 6,	/* Both WAIT and SUSPEND state */
	TS_DORMANT	= 8	/* DORMANT state */
} TSTAT;

/*
 * If the task is alive ( except NON-EXISTENT,DORMANT ), return TRUE.
 */
Inline BOOL task_alive( TSTAT state )
{
	return ( (state & (TS_READY|TS_WAIT|TS_SUSPEND)) != 0 );
}

/*
 * Task priority internal/external expression conversion macro
 */
#define int_priority(x)		( (INT)((x) - MIN_PRI) )
#define ext_tskpri(x)		( (PRI)((x) + MIN_PRI) )

/*
 * Task control block (TCB)
 */
//struct task_control_block {
struct task {
	QUEUE	tskque;		/* Task queue */
	ID	tskid;		/* Task ID */
	void	*exinf;		/* Extended information */
	ATR	tskatr;		/* Task attribute */
	FP	task;		/* Task startup address */
	ID	resid;		/* Assigned resource group ID */

	INT	stksz;		/* User stack size */
	INT	sstksz;		/* System stack size */

	INT	:0;		/* ### From here */
	UB	reqdct;		/* DCT request (Depends on implementation,
				   the usage is different) */
	B	isysmode;	/* Task operation mode initial value */
	H	sysmode;	/* Task operation mode, quasi task part
				   call level */
	INT	:0;		/* ### To here, since it might be accessed
				   from outside of the critical section,
				   need to be assigned as an independent
				   word. Also, there is a case where one
				   word is read from 'reqdct' and is read
				   all at once from 'reqdct', 'isysmode',
				   and 'sysmode', so do not change the
				   order and size. */

	UB	ipriority;	/* Priority at task startup */
	UB	bpriority;	/* Base priority */
	UB	priority;	/* Current priority */

	UB /*TSTAT*/	state;	/* Task state (Int. expression) */

	BOOL	nodiswai:1;	/* TRUE at wait disable
				   (Valid only for wait state) */
	BOOL	klockwait:1;	/* TRUE at wait kernel lock */
	BOOL	klocked:1;	/* TRUE at hold kernel lock */

CONST	WSPEC	*wspec;		/* Wait specification */
	ID	wid;		/* Wait object ID */
	INT	wupcnt;		/* Number of wakeup requests queuing */
	INT	suscnt;		/* Number of SUSPEND request nests */
	ER	*wercd;		/* Wait error code set area */
	WINFO	winfo;		/* Wait information */
	TMEB	wtmeb;		/* Wait timer event block */
	UINT	waitmask;	/* Disabled wait factor */

#ifdef NUM_PORID
	RNO	wrdvno;		/* For creating rendezvous number */
#endif
#ifdef NUM_MTXID
	MTXCB	*mtxlist;	/* List of hold mutexes */
#endif
	UB	tskevt;		/* Task event occurrence state */

	SVCLOCK	svclock;	/* Kernel lock for Extended SVC exclusion */
	SVCLOCK	*svclocked;	/* List of lock */

	FP	texhdr;		/* Task exception handler */
	UINT	texmask;	/* Task exception which is enabled */
	UINT	pendtex;	/* Task exception which is now suspended */
	UINT	exectex;	/* Task exception in break processing */

	UH	texflg;		/* Task exception control flag */
#define TEX0_RUNNING	0x0001U	/* Task exception code '0' handler
				   is in execution */
#define TEX1_RUNNING	0x0002U	/* Task exception code '1-31' handler
				   is in execution */
#define SSFN_RUNNING	0x0004U	/* Subsystem function is in execution
				   (Task exception is suspended) */

	ID	execssid:16;	/* Subsystem ID of extended SVC in
				   execution (MSB=1 break handler was
				   already executed) */
#define BREAK_RAN	0x8000U	/* execssid MSB */

	RELTIM_U slicetime;	/* Maximum continuous execution time (us) */
	RELTIM_U slicecnt;	/* Continuous execution time counter (us) */

	RELTIM_U stime;		/* System execution time (us) */
	RELTIM_U utime;		/* User execution time (us) */

	void	*istack;	/* User stack pointer initial value */
	void	*isstack;	/* System stack pointer initial value */
#if TA_GP
	void	*gp;		/* Global pointer */
#endif
	_align64		/* alignment for CTXB.ssp */
	CTXB	tskctxb;	/* Task context block */
#if USE_OBJECT_NAME
	UB	name[OBJECT_NAME_LENGTH];	/* name */
#endif
	struct process	*proc;		/* main process			*/
	struct list	task_node;	/* node of task group list	*/
};

/*
 * Task dispatch disable state
 *	0 = DDS_ENABLE		 : ENABLE
 *	1 = DDS_DISABLE_IMPLICIT : DISABLE with implicit process
 *	2 = DDS_DISABLE		 : DISABLE with tk_dis_dsp()
 *	|	|
 *	|	use in *.c
 *	use in *.S
 *	  --> Do NOT change these literals, because using in assembler code
 *
 *	'dipatch_disabled' records dispatch disable status set by tk_dis_dsp()
 *	for some CPU, that accepts delayed interrupt.
 *	In this case, you can NOT refer the dispatch disabled status
 *	only by 'dispatch_disabled'.
 *	Use 'in_ddsp()' to refer the task dispatch status.
 *	'in_ddsp()' is a macro definition in CPU-dependent definition files.
 */
#define DDS_ENABLE		(0)
#define DDS_DISABLE_IMPLICIT	(1)	/* set with implicit process */
#define DDS_DISABLE		(2)	/* set by tk_dis_dsp() */
IMPORT INT	dispatch_disabled;

/*
 * Task in execution
 *	ctxtsk is a variable that indicates TCB task in execution
 *	(= the task that CPU holds context). During system call processing,
 *	when checking information about the task that requested system call,
 *	use 'ctxtsk'. Only task dispatcher changes 'ctxtsk'.
 */
IMPORT TCB	*ctxtsk;

/*
 * Task which should be executed
 *	'schedtsk' is a variable that indicates the task TCB to be executed.
 *	If a dispatch is delayed by the delayed dispatch or dispatch disable,
 *	it does not match with 'ctxtsk.'
 */
IMPORT TCB	*schedtsk;

/*
 * Task control information
 */
IMPORT TCB	*tcb_table;	/* Task control block */
IMPORT QUEUE	free_tcb;	/* FreeQue */

/*
 * Get TCB from task ID.
 */
#define get_tcb(id)		( &tcb_table[INDEX_TSK(id)] )
#define get_tcb_self(id)	( ( (id) == TSK_SELF )? ctxtsk: get_tcb(id) )

/*
 * Prepare task execution.
 */
IMPORT void make_dormant( TCB *tcb );

/*
 * Make task executable.
 *	If the 'tcb' task priority is higher than the executed task,
 *	make it executable. If the priority is lower, connect the task to the
 *	ready queue.
 */
IMPORT void make_ready( TCB *tcb );

/*
 * Make task non-executable.
 *	Change the 'tcb' task state to be a non-executable state (wait state,
 *	forced wait, or dormant state). When calling this function, the
 *	task must be executable. Change 'tcb->state' on the caller side
 *	after returning from this function.
 */
IMPORT void make_non_ready( TCB *tcb );

/*
 * Change task priority.
 *	Change 'tcb' task priority to 'priority'.
 *	Then make the required task state transition occur.
 */
IMPORT void change_task_priority( TCB *tcb, INT priority );

/*
 * Rotate ready queue.
 *	'rotate_ready_queue' rotates the priority ready queue at 'priority'.
 *	'rotate_ready_queue_run' rotates the ready queue including the highest
 *	priority task in the ready queue.
 */
IMPORT void rotate_ready_queue( INT priority );
IMPORT void rotate_ready_queue_run( void );

/*
 * Scheduling by time slice
 */
IMPORT TCB* time_slice_schedule( TCB *tcb );
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_current_task
 Input		:void
 Output		:void
 Return		:struct task*
		 < current task >
 Description	:get current task
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct task* get_current_task(void);


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_scheduled_task
 Input		:void
 Output		:void
 Return		:struct task*
		 < next task >
 Description	:get next scheduled task
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct task* get_scheduled_task(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_task
 Input		:void
 Output		:void
 Return		:struct task*
		 < allocated task >
 Description	:allocate a empty task
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct task* alloc_task(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_task
 Input		:struct task *task
		 < to be freed >
 Output		:void
 Return		:void
 Description	:free a task
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER free_task(struct task *task);

#endif /* _TASK_ */
