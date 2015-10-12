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
 *	@(#)cpudef.h (tk/EM1-D512)
 *
 *	EM1-D512 dependent definition
 */

#ifndef __TK_CPUDEF_H__
#define __TK_CPUDEF_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * General purpose register			tk_get_reg tk_set_reg
 */
typedef struct t_regs {
	VW	r[13];		/* General purpose register R0-R12 */
	void	*lr;		/* Link register R14 */
} T_REGS;

/*
 * Exception-related register		tk_get_reg tk_set_reg
 */
typedef struct t_eit {
	void	*pc;		/* Program counter R15 */
	UW	cpsr;		/* Program status register */
	UW	taskmode;	/* Task mode flag */
} T_EIT;

/*
 * Control register			tk_get_reg tk_set_reg
 */
typedef struct t_cregs {
	void	*ssp;		/* System stack pointer R13_svc */
	void	*usp;		/* User stack pointer R13_ usr */
	void	*uatb;		/* Address of task specific space page table */
	UW	lsid;		/* Task logical space ID */
} T_CREGS;

/*
 * Coprocessor register	tk_get_cpr tk_set_cpr
 */
typedef union {
#if 0
	T_COP0REGS	cop0;
	T_COP1REGS	cop1;
	T_COP2REGS	cop2;
	T_COP3REGS	cop3;
#else
	UW		copDummy;
#endif
} T_COPREGS;

#ifdef __cplusplus
}
#endif
#endif /* __TK_CPUDEF_H__ */
