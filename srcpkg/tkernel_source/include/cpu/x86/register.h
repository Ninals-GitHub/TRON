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

#ifndef	__REGISTER_H__
#define	__REGISTER_H__

#include <tstdlib/bitop.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	EFLAGS register
----------------------------------------------------------------------------------
*/
#define	EFLAGS_CF	MAKE_BIT32( 0)	/* carry flag				*/
#define	EFLAGS_PF	MAKE_BIT32( 2)	/* parity flag				*/
#define	EFLAGS_AF	MAKE_BIT32( 4)	/* auxiliary carry flag			*/
#define	EFLAGS_ZF	MAKE_BIT32( 6)	/* zero flag				*/
#define	EFLAGS_SF	MAKE_BIT32( 7)	/* sign flag				*/
#define	EFLAGS_TF	MAKE_BIT32( 8)	/* trap flag				*/
#define	EFLAGS_IF	MAKE_BIT32( 9)	/* interrupt enable flag		*/
#define	EFLAGS_DF	MAKE_BIT32(10)	/* direction flag			*/
#define	EFLAGS_OF	MAKE_BIT32(11)	/* overflow flag			*/
#define	EFLAGS_IOPL	MAKE_MASK32(12, 13)	/* i/o privilege level		*/
#define	EFLAGS_NT	MAKE_BIT32(14)	/* nested task				*/
#define	EFLAGS_RF	MAKE_BIT32(16)	/* resume flag				*/
#define	EFLAGS_VM	MAKE_BIT32(17)	/* virtual-8086 mode			*/
#define	EFLAGS_AC	MAKE_BIT32(18)	/* alignment check			*/
#define	EFLAGS_VIF	MAKE_BIT32(19)	/* virtual interrupt flag		*/
#define	EFLAGS_VIP	MAKE_BIT32(20)	/* virtual interrupt pending		*/
#define	EFLAGS_ID	MAKE_BIT32(21)	/* id flag				*/

/*
----------------------------------------------------------------------------------
	CR0 register
----------------------------------------------------------------------------------
*/
#define	CR0_PE		MAKE_BIT32( 0)	/* protection enable			*/
#define	CR0_MP		MAKE_BIT32( 1)	/* monitor coporocessor			*/
#define	CR0_EM		MAKE_BIT32( 2)	/* emulation				*/
#define	CR0_TS		MAKE_BIT32( 3)	/* task switched			*/
#define	CR0_ET		MAKE_BIT32( 4)	/* extension type			*/
#define	CR0_NE		MAKE_BIT32( 5)	/* numeric error			*/
#define	CR0_WP		MAKE_BIT32(16)	/* write protect			*/
#define	CR0_AM		MAKE_BIT32(18)	/* alignment mask			*/
#define	CR0_NW		MAKE_BIT32(29)	/* not write-through			*/
#define	CR0_CD		MAKE_BIT32(30)	/* cache disable			*/
#define	CR0_PG		MAKE_BIT32(31)	/* paging				*/

/*
----------------------------------------------------------------------------------
	CR3 register
----------------------------------------------------------------------------------
*/
#define	CR3_PWT		MAKE_BIT32( 3)	/* page-level write-through		*/
#define	CR3_PCD		MAKE_BIT32( 4)	/* page-level cache disable		*/
#define	CR3_PDB		MAKE_MASK32(12, 31)	/* page-directory base		*/

/*
----------------------------------------------------------------------------------
	CR4 register
----------------------------------------------------------------------------------
*/
#define	CR4_VME		MAKE_BIT32( 0)	/* virtual-8086 mode extensions		*/
#define	CR4_PVI		MAKE_BIT32( 1)	/* protected-mode virtual interrupts	*/
#define	CR4_TSD		MAKE_BIT32( 2)	/* time stamp disable			*/
#define	CR4_DE		MAKE_BIT32( 3)	/* debugging extensions			*/
#define	CR4_PSE		MAKE_BIT32( 4)	/* page size extensions			*/
#define	CR4_PAE		MAKE_BIT32( 5)	/* physical address extension		*/
#define	CR4_MCE		MAKE_BIT32( 6)	/* machine-check enable			*/
#define	CR4_PGE		MAKE_BIT32( 7)	/* page global enable			*/
#define	CR4_PCE		MAKE_BIT32( 8)	/* performance-monitoring counter enable*/
#define	CR4_OSFXSR	MAKE_BIT32( 9)	/* os support for FXSAVE/FXRSTOR	*/
#define	CR4_OSXMMEXCPT	MAKE_BIT32(10)	/* os support for unmasked SIMD fp-excpt*/
#define	CR4_VMXE	MAKE_BIT32(13)	/* SMX-enable				*/
#define	CR4_FSGSBASE	MAKE_BIT32(16)	/* FSGSBASE-enable			*/
#define	CR4_PCIDE	MAKE_BIT32(17)	/* PCID-enable				*/
#define	CR4_OSXSAVE	MAKE_BIT32(18)	/* XSAVE/proc extended states-enable	*/
#define	CR4_SMEP	MAKE_BIT32(20)	/* SMEP-enalbe				*/



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
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	// _REGISTER_H__
