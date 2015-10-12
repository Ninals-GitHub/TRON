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
 *	@(#)sysdef_depend.h (tk/EM1-D512)
 *
 *	Definition about EM1-D512
 *
 *	Included also from assembler program.
 */

#ifndef __TK_SYSDEF_DEPEND_H__
#define __TK_SYSDEF_DEPEND_H__

/*
 * Program Status Regsiter (PSR)
 */
#define	PSR_N		0x80000000	/* conditional flag  negative */
#define	PSR_Z		0x40000000	/*		     zero */
#define	PSR_C		0x20000000	/*		     carry */
#define	PSR_V		0x10000000	/*		     overflow */
#define	PSR_Q		0x08000000	/* Sticky overflow */
#define	PSR_GE		0x000f0000	/* SIMD >= flag  */

#define	PSR_J		0x01000000	/* Jazelle mode */
#define	PSR_IT		0x0600fc00	/* Thumb If-Then state */
#define	PSR_E		0x00000200	/* data endian */
#define	PSR_A		0x00000100	/* disable asynchronous abort */
#define	PSR_I		0x00000080	/* disable interrupt(IRQ) */
#define	PSR_F		0x00000040	/* disable fast interrupt(FIQ) */
#define	PSR_T		0x00000020	/* Thumb mode */

#define	PSR_M(n)	( n )		/* processor mode 0-31 */
#define	PSR_USR		PSR_M(16)	/* user mode */
#define	PSR_FIQ		PSR_M(17)	/* fast interrupt(FIQ) mode */
#define	PSR_IRQ		PSR_M(18)	/* interrupt (IRQ) mode */
#define	PSR_SVC		PSR_M(19)	/* supervisor mode */
#define	PSR_ABT		PSR_M(23)	/* abort mode */
#define	PSR_UND		PSR_M(27)	/* undefined instruction mode */
#define	PSR_SYS		PSR_M(31)	/* system mode */

#define PSR_DI		( PSR_I|PSR_A )	/* disable (ordinary) interrupt */

/*
 * task mode flag
 *	system wide (shared) taskmode
 */
#define	TMF_CPL(n)	( (n) )		/* current protection level	(0-3) */
#define	TMF_PPL(n)	( (n) << 16 )	/* previous protection level	(0-3) */

/*
 * system control coprocessor(CP15) : control register(CR1)
 */
#define	CR1_M	0x0001	/* enable MMU  */
#define	CR1_A	0x0002	/* enable alignment check */
#define	CR1_C	0x0004	/* enable (data) cache */
#define	CR1_W	0x0008	/* enable write buffer */
#define	CR1_B	0x0080	/* endian (1 - big endian)  */
#define	CR1_S	0x0100	/* protect system */
#define	CR1_R	0x0200	/* protect ROM  */
#define	CR1_SW	0x0400	/* enable SWP/SWPB */
#define	CR1_Z	0x0800	/* enable branch prediction */
#define	CR1_I	0x1000	/* enable instruction cache */
#define	CR1_V	0x2000	/* high vector */
#define	CR1_RR	0x4000	/* cache : prediction strategy (round robin) */
#define	CR1_L4	0x8000	/* ARMv4 compatible mode */

#define	CR1_HA	 0x00020000	/* hardware access flag */
#define	CR1_FI	 0x00200000	/* fast interrupt */
#define	CR1_U	 0x00400000	/* unalligned access */
#define	CR1_XP	 0x00800000	/* subpage AP bit */
#define	CR1_VE	 0x01000000	/* interrupt vector mechanism */
#define	CR1_EE	 0x02000000	/* endian for exception processing */
#define	CR1_L2	 0x04000000	/* enable L2 integrated cache */
#define	CR1_NMFI 0x08000000	/* enable non-maskable FIQ */
#define	CR1_TRE	 0x10000000	/* enable TEX remap */
#define	CR1_AFE	 0x20000000	/* enable access flag */
#define	CR1_TE	 0x40000000	/* enable Thumb exception */

#define	CR1_nF	0x40000000	/* clock mode (Fast)  ARM920T */
#define	CR1_iA	0x80000000	/* clock mode (Async) ARM920T */

/*
 * Fault Status(CP15:CR5)
 *	(v7) ARMv7 definition
 */
#define	FSR_Alignment	0x1	/* 00x1 : misaligned access */
#define	FSR_BusErrorT	0xc	/* 11x0 : bus error during address translation */
#define	FSR_Translation	0x5	/* 01x1 : missing page during address translation */
#define	FSR_Domain	0x9	/* 10x1 : domain access violation */
#define	FSR_Permission	0xd	/* 11x1 : illegal access permission */
#define	FSR_BusErrorL	0x4	/* 01x0 : bus error during line fetch */
#define	FSR_BusErrorO	0x8	/* 10x0 : other bus errors */
#define	FSR_Section	0x0	/* xx0x : section */
#define	FSR_Page	0x2	/* xx1x : page */
#define	FSR_TypeMask	0xd	/* type mask */

#define	FSR_ICacheM	0x0004	/* 00100 : instruction cache management(v7) */
#define	FSR_AccessFlagS	0x0003	/* 00011 : access flag - section(v7) */
#define	FSR_AccessFlagP	0x0006	/* 00110 : access flag - page(v7) */
#define	FSR_DebugEvent	0x0002	/* 00010 : debug event (v7) */
#define	FSR_TypeMaskAll	0x140f	/* type bit mask for all types(v7) */
#define	FSR_WnR		0x0800	/* 1=write/0=read(v7) */

/*
 * memory barrier instruction
 *	ISB()	Instruction Synchronization Barrier
 *	DSB()	Data Synchronization Barrier
 *	DMB()	Data Memory Barrier
 */
#define ISB()		Asm("mcr p15, 0, %0, cr7, c5,  4":: "r"(0))
#define DSB()		Asm("mcr p15, 0, %0, cr7, c10, 4":: "r"(0))
#define DMB()		Asm("mcr p15, 0, %0, cr7, c10, 5":: "r"(0))

/* ------------------------------------------------------------------------ */

/*
 * software interrupt number for T-Monitor
 */
#define	SWI_MONITOR	4	/* T-Monitor service call */

/*
 * software interrupt number for T-Kernel
 */
#define	SWI_SVC		6	/* T-Kernel system call and extended SVC */
#define	SWI_RETINT	7	/* tk_ret_int() system call */
#define	SWI_DISPATCH	8	/* task dispatcher */
#define	SWI_DEBUG	9	/* debug support function */
#define	SWI_RETTEX	10	/* return from task exception */

/*
 * software interrupt number for Extension
 */
#define	SWI_KILLPROC	11	/* request to forcibly kill process */

#endif /* __TK_SYSDEF_DEPEND_H__ */
