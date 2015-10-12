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
 *	@(#)prreg.c (libtk/EM1-D512)
 *
 *	Display task register value
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <sys/misc.h>

/*
 * Uses prfn to display the contents of gr, er, and cr.
 * prfn must be a printf compatible function.
 */
EXPORT W PrintTaskRegister( int (*prfn)( const char *format, ... ),
				T_REGS *gr, T_EIT *er, T_CREGS *cr )
{
/*
 *	PC: 12345678             CPSR:12345678 TMF:12345678
 *	R0: 12345678 R1: 12345678 R2: 12345678 R3: 12345678
 *	R4: 12345678 R5: 12345678 R6: 12345678 R7: 12345678
 *	R8: 12345678 R9: 12345678 R10:12345678 R11:12345678
 *	IP: 12345678 LR: 12345678
 *	USP:12345678 SSP:12345678 LSID:1234   UATB:12345678
 */
	(*prfn)("PC: %08x             CPSR:%08x TMF:%08x\n",
		(UW)er->pc, er->cpsr, er->taskmode);
	(*prfn)("R0: %08x R1: %08x R2: %08x R3: %08x\n",
		gr->r[0], gr->r[1], gr->r[2], gr->r[3]);
	(*prfn)("R4: %08x R5: %08x R6: %08x R7: %08x\n",
		gr->r[4], gr->r[5], gr->r[6], gr->r[7]);
	(*prfn)("R8: %08x R9: %08x R10:%08x R11:%08x\n",
		gr->r[8], gr->r[9], gr->r[10], gr->r[11]);
	(*prfn)("IP: %08x LR: %08x\n",
		gr->r[12], (UW)gr->lr);
	(*prfn)("USP:%08x SSP:%08x LSID:%-4d   UATB:%08x\n",
		(UW)cr->usp, (UW)cr->ssp, cr->lsid, (UW)cr->uatb);
	return 6;  /* Number of display rows */
}
