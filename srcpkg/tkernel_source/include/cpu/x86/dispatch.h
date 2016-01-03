/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------
 */

#ifndef	__DISPATCH_H__
#define	__DISPATCH_H__

#include <typedef.h>
#include <tk/typedef.h>
#include <tk/kernel.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
//struct task_context_block;

/*
==================================================================================

	DEFINE 

==================================================================================
*/

//#ifndef __tcb__
//#define __tcb__
////typedef struct task_control_block	TCB;
//typedef struct task			TCB;
//#endif

/*
==================================================================================

	Management 

==================================================================================
*/



/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:start_task
 Input		:unsigned long start_text
 		 < start address >
 Output		:void
 Return		:int
 		 < result >
 Description	:execute new user process forcibly
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int start_task(unsigned long start_text);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:Throw away the current task context. and forcibly dispatch to
 		 the task that should be performed next.
 		 Use at system startup and 'tk_ext_tsk, tk_exd_tsk.'
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void force_dispatch(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setup_context
 Input		:TCB *tcb
 		 < task control block to be set up >
 		 unsigned long fork
 		 < boolean fork flag >
 Output		:void
 Return		:void
 Description	:Create stack frame for task startup Call from 'make_dormant()'
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void setup_context( TCB *tcb );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dispatch
 Input		:void
 Output		:void
 Return		:void
 Description	:context switch
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void dispatch(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_task_context
 Input		:struct task *from
 		 < copy from >
 		 struct task *new
 		 < copy to >
 Output		:void
 Return		:int
 		 < resutl >
 Description	:copy current task context to the new one
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int copy_task_context(struct task *from, struct task *new);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setup_fork_user_context
 Input		:struct task *new
 		 < forked task >
 		 struct pt_regs *regs
 		 < context for new task >
 Output		:void
 Return		:void
 Description	:setup forked task context
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void setup_fork_user_context(struct task *new, struct pt_regs *regs);

#endif	// __DISPATCH_H__
