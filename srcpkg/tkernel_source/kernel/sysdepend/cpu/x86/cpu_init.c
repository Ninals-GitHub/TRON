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
 *	cpu_init.c (x86)
 *	CPU-Dependent Initialization/Finalization
 */

#include <cpu.h>

#include <tk/kernel.h>
#include <tk/task.h>


//EXPORT MONHDR	SaveMonHdr;	/* For saving monitor exception handler */
EXPORT ATR	available_cop;	/* Available coprocessor */

/*
 * CPU-dependent initialization
 */
EXPORT ER cpu_initialize( void )
{
	/* -------------------------------------------------------------------- */
	/* setup idt								*/
	/* -------------------------------------------------------------------- */
	init_default_int_handlers( );
	
	initIdt();
	
	/* -------------------------------------------------------------------- */
	/* collect cpuid information						*/
	/* -------------------------------------------------------------------- */
	initCpuInformation();
	
	return E_OK;
}

/*
 * CPU-dependent finalization
 */
EXPORT void cpu_shutdown( void )
{
}

/* ------------------------------------------------------------------------- */

/*
 * Task exception handler startup reservation
 */
EXPORT void request_tex( TCB *tcb )
{
	/* Cannot set to the task operating at protected level 0 */
	if ( tcb->isysmode == 0 ) {
		tcb->reqdct = 1;
	}
}

/*
 * Setting up the start of task exception handler
 *
 *	Initial stack status
 *		system stack			user stack
 *		+---------------+		+---------------+
 *	ssp ->	| R12	  = ip	|	usp ->	| (xxxxxxxxxxx) |
 *		| R14_svc = lr	|		|		|
 *		| SPSR_svc	|
 *		+---------------+
 *
 *	Modified stack status ( modified parts are maked with * )
 *		+---------------+		+---------------+
 *	ssp ->	| R12		|	usp* -> | texcd		|*
 *		| texhdr	|*		| retadr	|*
 *		| SPSR_svc	|*		| CPSR		|*
 *		+---------------+		+---------------+
 *						| (xxxxxxxxxxx) |
 */
EXPORT void setup_texhdr( UW *ssp )
{
}

