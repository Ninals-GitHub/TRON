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
 *	cpu_init.c (EM1-D512)
 *	CPU-Dependent Initialization/Finalization
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include "cpu_insn.h"

EXPORT MONHDR	SaveMonHdr;	/* For saving monitor exception handler */
EXPORT ATR	available_cop;	/* Available coprocessor */

/*
 * CPU-dependent initialization
 */
EXPORT ER cpu_initialize( void )
{
IMPORT void dispatch_entry( void );	/* calling dispatcher */
IMPORT void call_entry( void );		/* calling system call */
IMPORT void _tk_ret_int( void );	/* exclusively used to invoke tk_ret_int() */
IMPORT void call_dbgspt( void );	/* calling debugger support */
IMPORT void rettex_entry( void );	/* return from task exception */

	UW	r;

	/* Save monitor exception handler */
	SaveMonHdr.default_hdr = SCArea->intvec[VECNO_DEFAULT];
	SaveMonHdr.idebug_hdr  = SCArea->intvec[VECNO_IDEBUG];
	SaveMonHdr.ddebug_hdr  = SCArea->intvec[VECNO_DDEBUG];
	SaveMonHdr.monitor_hdr = SCArea->intvec[VECNO_MONITOR];
	SaveMonHdr.abortsw_hdr = SCArea->intvec[VECNO_ABORTSW];
	SaveMonHdr.gio_hdr[0]  = SCArea->intvec[VECNO_GIO0];
	SaveMonHdr.gio_hdr[1]  = SCArea->intvec[VECNO_GIO1];
	SaveMonHdr.gio_hdr[2]  = SCArea->intvec[VECNO_GIO2];
	SaveMonHdr.gio_hdr[3]  = SCArea->intvec[VECNO_GIO3];
	SaveMonHdr.gio_hdr[4]  = SCArea->intvec[VECNO_GIO4];
	SaveMonHdr.gio_hdr[5]  = SCArea->intvec[VECNO_GIO5];
	SaveMonHdr.gio_hdr[6]  = SCArea->intvec[VECNO_GIO6];
	SaveMonHdr.gio_hdr[7]  = SCArea->intvec[VECNO_GIO7];

	/* Initialize task space */
	Asm("mrc p15, 0, %0, cr2,  c0, 1": "=r"(r));	/* TTBR1 */
	Asm("mcr p15, 0, %0, cr2,  c0, 0":: "r"(r));	/* TTBR0 */
	Asm("mcr p15, 0, %0, cr13, c0, 1":: "r"(0));	/* CONTEXTIDR */
	ISB();
	PurgeTLB();	/* invlidate TLB */

	/* available coprocessor(s) */
	available_cop = TA_NULL;

	/* install the exception handler used by the OS */
	define_inthdr(SWI_SVC,	    call_entry);
	define_inthdr(SWI_RETINT,   _tk_ret_int);
	define_inthdr(SWI_DISPATCH, dispatch_entry);
	define_inthdr(SWI_RETTEX,   rettex_entry);
#if USE_DBGSPT
	define_inthdr(SWI_DEBUG,    call_dbgspt);
#endif

	return E_OK;
}

/*
 * CPU-dependent finalization
 */
EXPORT void cpu_shutdown( void )
{
	/* Restore saved monitor exception handler */
	SCArea->intvec[VECNO_DEFAULT] = SaveMonHdr.default_hdr;
	SCArea->intvec[VECNO_IDEBUG]  = SaveMonHdr.idebug_hdr;
	SCArea->intvec[VECNO_DDEBUG]  = SaveMonHdr.ddebug_hdr;
	SCArea->intvec[VECNO_MONITOR] = SaveMonHdr.monitor_hdr;
	SCArea->intvec[VECNO_ABORTSW] = SaveMonHdr.abortsw_hdr;
	SCArea->intvec[VECNO_GIO0]    = SaveMonHdr.gio_hdr[0];
	SCArea->intvec[VECNO_GIO1]    = SaveMonHdr.gio_hdr[1];
	SCArea->intvec[VECNO_GIO2]    = SaveMonHdr.gio_hdr[2];
	SCArea->intvec[VECNO_GIO3]    = SaveMonHdr.gio_hdr[3];
	SCArea->intvec[VECNO_GIO4]    = SaveMonHdr.gio_hdr[4];
	SCArea->intvec[VECNO_GIO5]    = SaveMonHdr.gio_hdr[5];
	SCArea->intvec[VECNO_GIO6]    = SaveMonHdr.gio_hdr[6];
	SCArea->intvec[VECNO_GIO7]    = SaveMonHdr.gio_hdr[7];
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
	FP	texhdr;
	INT	texcd;
	UINT	m;
	UW	*usp;

	/* called in interrupt-disabled state */

	ctxtsk->reqdct = 0;	/* release DCT */

	/* obtain exception code */
	m = 0x00000001;
	for ( texcd = 0; texcd <= 31; texcd++ ) {
		if ( (ctxtsk->exectex & m) != 0 ) break;
		m <<= 1;
	}
	if ( texcd > 31 ) return; /* exception is not generated / released */

	ctxtsk->exectex = 0;
	ctxtsk->pendtex &= ~m;
	ctxtsk->texflg |= ( texcd == 0 )? TEX0_RUNNING: TEX1_RUNNING;
	texhdr = ctxtsk->texhdr;

	/* obtain user stack pointer */
	Asm("stmia %0, {sp}^ ; nop":: "r"(&usp));

	/* reset user stack to the initial value if exception code is 0 */
	if ( texcd == 0 ) usp = ctxtsk->istack;

	usp -= 3;

	/* set up user stack pointer */
	Asm("ldmia %0, {sp}^ ; nop":: "r"(&usp));

	ENABLE_INTERRUPT;

	/* adjust stack
	 *	we need to access user stack, and this may cause
	 *	a page fault.
	 */
	*(usp + 0) = texcd;
	*(usp + 1) = *(ssp + 1);	/* retadr */
	*(usp + 2) = *(ssp + 2);	/* CPSR */
	*(ssp + 1) = (UW)texhdr & ~1;
	*(ssp + 2) &= ~(PSR_T|PSR_J);
	if ( ((UW)texhdr & 1) != 0 ) *(ssp + 2) |= PSR_T;
}
