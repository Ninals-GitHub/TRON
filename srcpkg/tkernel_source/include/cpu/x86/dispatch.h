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

#include <tk/typedef.h>
#include <tk/kernel.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct task_context_block;

/*
==================================================================================

	DEFINE 

==================================================================================
*/

#ifndef __tcb__
#define __tcb__
typedef struct task_control_block	TCB;
#endif

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
 		 < task control block to be setup >
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

#endif	// __DISPATCH_H__
