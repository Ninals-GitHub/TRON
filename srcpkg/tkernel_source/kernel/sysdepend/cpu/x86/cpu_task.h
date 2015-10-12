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
 *	cpu_task.h (EM1-D512)
 *	CPU-Dependent Task Start Processing
 */

#ifndef _CPU_TASK_
#define _CPU_TASK_

#include <cpu/x86/cpu_insn.h>

/*
 * System stack configuration at task startup
 */
typedef struct {
	VW	r[12];		/* R0-R11 */
	UW	taskmode;
	void	*usp;		/* R13_usr */
	void	*lr_usr;	/* R14_usr */
	void	*lr_svc;	/* R14_svc */
	VW	ip;		/* R12 */
	void	*pc;		/* R15 */
	VW	spsr_svc;
} SStackFrame;

/*
 * User stack configuration at task startup (only RNG 1-3)
 */
typedef struct {
	/* Empty */
} UStackFrame;

/*
 * Size of system stack area destroyed by 'make_dormant()'
 * In other words, the size of area required to write by 'setup_context().'
 */
#define DORMANT_STACK_SIZE	( sizeof(VW) * 7 )	/* To 'taskmode' */

/*
 * Size of area kept for special use from system stack
 */
#define RESERVE_SSTACK(tskatr)	0

/*
 * Initial value for task startup
 */
#if USE_MMU
#define INIT_PSR(rng)	( ( (rng) == 0 )? PSR_SVC: \
			  ( (rng) == 3 )? PSR_USR: PSR_SYS )
#else
#define INIT_PSR(rng)	( ( (rng) == 0 )? PSR_SVC: PSR_SYS )
#endif

#define INIT_TMF(rng)	( TMF_PPL(rng) | TMF_CPL(rng) )

/*
 * Switch task space
 */
Inline void change_space( void *uatb, INT lsid )
{

}

/*
 * Set task startup code
 *	Called by 'tk_sta_tsk()' processing.
 */
Inline void setup_stacd( TCB *tcb, INT stacd )
{
	//SStackFrame	*ssp = tcb->tskctxb.ssp;

	//ssp->r[0] = stacd;
	//ssp->r[1] = (VW)tcb->exinf;
}

/*
 * Delete task contexts
 */
Inline void cleanup_context( TCB *tcb )
{
}

#endif /* _CPU_TASK_ */
