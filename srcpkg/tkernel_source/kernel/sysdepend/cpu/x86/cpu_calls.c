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
 *	cpu_calls.c (EM1-D512)
 *	CPU-Dependent System Call
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include "check.h"
#include <cpu.h>

#include <sys/sysinfo.h>
#include <tk/sysdef.h>
#include <cpu/x86/cpu_insn.h>

/*
 * Dispatch enable/disable
 */
SYSCALL ER _tk_dis_dsp( void )
{
	//CHECK_CTX(!in_loc());

	dispatch_disabled = DDS_DISABLE;

	return E_OK;
}

/*
 * Dispatch enable
 */
SYSCALL ER _tk_ena_dsp( void )
{
	//CHECK_CTX(!in_loc());

	dispatch_disabled = DDS_ENABLE;
	if ( ctxtsk != schedtsk ) {
		dispatch();
	}

	return E_OK;
}

/* ------------------------------------------------------------------------ */

/*
 * High level programming language
 */

/* High level programming language interrupt handler entry */
EXPORT FP hll_inthdr[N_INTVEC];

/* High level programming language routine (Interrupt) */
IMPORT void inthdr_startup();

/* High level programming language routine (Exception) */
IMPORT void exchdr_startup();

/* For default handler */
IMPORT void defaulthdr_startup();

/*
 * Interrupt handler definition
 */
SYSCALL ER _tk_def_int( UINT dintno, CONST T_DINT *pk_dint )
{
	return E_OK;
}

/* ------------------------------------------------------------------------ */

/*
 * Get task space
 */
SYSCALL ER _tk_get_tsp( ID tskid, T_TSKSPC *pk_tskspc )
{
	ER		ercd = E_OK;
	
	return ercd;
}

/*
 * Set task space
 */
SYSCALL ER _tk_set_tsp( ID tskid, CONST T_TSKSPC *pk_tskspc )
{
	ER		ercd = E_OK;
	
	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Set task register contents
 */
LOCAL void set_reg( TCB *tcb, CONST T_REGS *regs, CONST T_EIT *eit, CONST T_CREGS *cregs )
{

}

/*
 * Set task register contents
 */
SYSCALL ER _tk_set_reg( ID tskid,
	CONST T_REGS *pk_regs, CONST T_EIT *pk_eit, CONST T_CREGS *pk_cregs )
{
	ER		ercd = E_OK;

	return ercd;
}

/*
 * Get task register contents
 */
LOCAL void get_reg( TCB *tcb, T_REGS *regs, T_EIT *eit, T_CREGS *cregs )
{

}

/*
 * Get task register contents
 */
SYSCALL ER _tk_get_reg( ID tskid,
		T_REGS *pk_regs, T_EIT *pk_eit, T_CREGS *pk_cregs )
{
	ER		ercd = E_OK;

	return ercd;
}

/*
 * Set task coprocessor register
 */
SYSCALL ER _tk_set_cpr( ID tskid, INT copno, CONST T_COPREGS *pk_copregs )
{
	ER	ercd = E_OK;

	return ercd;
}

/*
 * Get task coprocessor register
 */
SYSCALL ER _tk_get_cpr( ID tskid, INT copno, T_COPREGS *pk_copregs )
{
	ER	ercd = E_OK;

	return ercd;
}

/* ------------------------------------------------------------------------ */
/*
 *	Debugger support function
 */
#if USE_DBGSPT

/*
 * Set task register
 */
SYSCALL ER _td_set_reg( ID tskid, CONST T_REGS *regs, CONST T_EIT *eit, CONST T_CREGS *cregs )
{
	ER	ercd = E_OK;

	return ercd;
}

/*
 * Get task register
 */
SYSCALL ER _td_get_reg( ID tskid, T_REGS *regs, T_EIT *eit, T_CREGS *cregs )
{
	ER	ercd = E_OK;

	return ercd;
}

#endif /* USE_DBGSPT */
