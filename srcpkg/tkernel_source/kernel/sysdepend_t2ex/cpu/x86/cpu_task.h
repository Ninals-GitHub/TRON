/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	cpu_task.h (EM1-D512)
 *	CPU-Dependent Task Start Processing
 */

#ifndef _CPU_TASK_
#define _CPU_TASK_

#include <cpu.h>

/*
 * System stack configuration at task startup
 */
#if 0
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
#endif
typedef struct {
	unsigned long	r[2];		/*  */
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
#define RESERVE_SSTACK(tskatr)	8

/*
 * Initial value for task startup
 */
#if USE_MMU
#define INIT_PSR(rng)	( ( (rng) == 0 )? PSR_SVC: \
			  ( (rng) >= MMU_MIN_USER_LEVEL )? PSR_USR: PSR_SYS )
#else
#define INIT_PSR(rng)	( ( (rng) == 0 )? PSR_SVC: PSR_SYS )
#endif

#define INIT_TMF(rng)	( TMF_PPL(rng) | TMF_CPL(rng) )

/*
 * Switch task space
 */
Inline void change_space( void *uatb, INT lsid )
{
	UW	ttbr;

	/* if no task space to switch to is not specified, use system default. */
	//Asm("mrc p15, 0, %0, cr2, c0, 1": "=r"(ttbr));	/* TTBR1 */
	if ( uatb != NULL ) {
		ttbr = (UW)uatb | (ttbr & 0x07f);
	}

	/* To synchronize ASID and TTBR change, set ASID to a meaningless value temporarily. */
	//Asm("mcr p15, 0, %0, cr13, c0, 1":: "r"(0));	/* CONTEXTIDR */
	ISB();
	//Asm("mcr p15, 0, %0, cr2,  c0, 0":: "r"(ttbr)); /* TTBR0 */
	//Asm("mcr p15, 0, %0, cr13, c0, 1":: "r"(lsid)); /* CONTEXTIDR */
	ISB();
}


/*
 * Set task startup code
 *	Called by 'tk_sta_tsk()' processing.
 */
Inline void setup_stacd( TCB *tcb, INT stacd )
{
	W rng;
	unsigned long *sp;
	
	rng = (tcb->tskatr & TA_RNG3) >> 8;
	
	if (rng) {
		sp = (unsigned long*)tcb->tskctxb.sp;
	} else {
		sp = (unsigned long*)tcb->tskctxb.ssp;
	}
	
	//ssp->r[0] = stacd;
	//ssp->r[1] = (VW)tcb->exinf;
	*sp = (VW)tcb->exinf;
	sp--;
	*sp = stacd;
	sp--;
	
	if (rng) {
		tcb->tskctxb.sp  -= sizeof(unsigned long) * 2;
	} else {
		tcb->tskctxb.ssp -= sizeof(unsigned long) * 2;
	}
}

/*
 * Delete task contexts
 */
Inline void cleanup_context( TCB *tcb )
{
}

#endif /* _CPU_TASK_ */
