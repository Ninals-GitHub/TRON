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
 *	cpu_status.h (x86)
 *	x86 Dependent Definition
 */

#ifndef _CPU_STATUS_
#define _CPU_STATUS_

#include <cpu.h>
#include <stdint.h>

#include <tk/syslib.h>
#include <tk/sysdef.h>

#include <bk/uapi/ldt.h>

IMPORT TCB	*ctxtsk;
IMPORT TCB	*schedtsk;

IMPORT INT	dispatch_disabled;
IMPORT UINT	lowpow_discnt;

/*
 * Start/End critical section
 */
//				  && schedtsk				
#define BEGIN_CRITICAL_SECTION	{ uint32_t _cpsr_ = disint();
#define END_CRITICAL_SECTION	if ( !isDI(_cpsr_)			\
				  && ctxtsk != schedtsk			\
				  && !isTaskIndependent()		\
				  && !dispatch_disabled ) {		\
					dispatch();			\
				}					\
				enaint(_cpsr_);}

/*
 * Start/End interrupt disable section
 */
#define BEGIN_DISABLE_INTERRUPT	{ uint32_t _cpsr_ = disint();
#define END_DISABLE_INTERRUPT	enaint(_cpsr_); }

/*
 * Start/End interrupt enable section
 */
#define BEGIN_ENABLE_INTERRUPT	{ uint32_t _cpsr_ = saveFlags();	\
				enableInt( );
#define END_ENABLE_INTERRUPT	restoreFlags(_cpsr_); }

/*
 * Interrupt enable/disable
 */
#define ENABLE_INTERRUPT	{ enableInt(); }
#define DISABLE_INTERRUPT	{ disint(); }

/*
 * Enable interrupt nesting
 *	Enable the interrupt that has a higher priority than 'level.'
 */
#define ENABLE_INTERRUPT_UPTO(level)	{ enableInt(); }

/*
 * Move to/Restore task independent part
 */
#define ENTER_TASK_INDEPENDENT	{ EnterTaskIndependent(); }
#define LEAVE_TASK_INDEPENDENT	{ LeaveTaskIndependent(); }

/* ----------------------------------------------------------------------- */
/*
 *	Check system state
 */

/*
 * When a system call is called from the task independent part, TRUE
 */
#define in_indp()	( isTaskIndependent() || ctxtsk == NULL )

/*
 * When a system call is called during dispatch disable, TRUE
 * Also include the task independent part as during dispatch disable.
 */
#define in_ddsp()	( dispatch_disabled	\
			|| in_indp()		\
			|| isDI(saveFlags()) )
//			|| isDI(getCPSR()) )

/*
 * When a system call is called during CPU lock (interrupt disable), TRUE
 * Also include the task independent part as during CPU lock.
 */
#define in_loc()	( isDI(saveFlags())		\
			|| in_indp() )

/*
 * When a system call is called during executing the quasi task part, TRUE
 * Valid only when in_indp() == FALSE because it is not discriminated from
 * the task independent part.
 */
#define in_qtsk()	( ctxtsk->sysmode > ctxtsk->isysmode )

/* ----------------------------------------------------------------------- */
/*
 *	Task dispatcher startup routine
 */

/*
 * Request for task dispatcher startup
 *	Do nothing at this point because there is no delayed
 *	interrupt function in ARM.
 *	Perform dispatcher startup with END_CRITICAL_SECTION.
 */
#define dispatch_request()	/* */

/*
 * Throw away the current task context.
 * and forcibly dispatch to the task that should be performed next.
 *	Use at system startup and 'tk_ext_tsk, tk_exd_tsk.'
 */
#if 0
Inline void force_dispatch( void )
{
IMPORT	void	dispatch_to_schedtsk();
	dispatch_to_schedtsk();
	return;
}
#endif

/*
 * Start task dispatcher
 */
#if 0
Inline void dispatch( void )
{
}
#endif

/* ----------------------------------------------------------------------- */
/*
 *	Task exception
 */

/*
 * Task exception handler startup reservation
 */
IMPORT void request_tex( TCB *tcb );

/* ----------------------------------------------------------------------- */
/*
 * Task context block
 */
struct task_context_block {
	struct segment_desc	tls_desc[NR_TLS_ENTRYIES];
	struct pt_regs		*pt_regs;
	unsigned long		sp0;
	unsigned long		sp;
	unsigned long		ip;
	unsigned long		sysenter_cs;
	unsigned long		flags;
	uint16_t		ds;
	uint16_t		es;
	uint16_t		gs;
	uint16_t		fs;
	unsigned long		cr2;
	unsigned long		trap_nr;
	unsigned long		error_code;
	unsigned long		*io_bitmap;
	unsigned long		iopl;
	INT			io_bitmap_max;

	void			*ssp;		/* System stack pointer */
	void			*uatb;		/* Task space page table */
	INT			lsid;		/* Task space ID */
	
	uint32_t		need_iret:1;	/* need iret for first dispatch	*/
};

#define	KERNEL_STACK_SIZE	8192

/*
 * CPU information
 */
IMPORT ATR	available_cop;	/* Enabled coprocessor (TA_COPn) */

#endif /* _CPU_STATUS_ */
