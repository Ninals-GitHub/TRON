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

#ifndef	__INTERRUPT_H__
#define	__INTERRUPT_H__

#ifndef _in_asm_source_
#include <stdint.h>
#include <tk/typedef.h>
#endif	// _in_asm_source_
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
 * software interrupt number for T-Monitor
 */
#define	SWI_MONITOR		0xC0	/* T-Monitor service call */
#define	int_syscall_mon		int_syscall_0xC0

/*
 * software interrupt number for T-Kernel
 */
#define	SWI_SVC			0xC1	/* T-Kernel system call and extended SVC */
#define	int_tksyscall_svc	int_tksyscall_0xC1
#define	SWI_RETINT		0xC2	/* tk_ret_int() system call */
#define	int_tksyscall_retint	int_syscall_0xC2
#define	SWI_DISPATCH		0xC3	/* task dispatcher */
#define	int_tksyscall_dispatch	int_syscall_0xC3
#define	SWI_DEBUG		0xC4	/* debug support function */
#define	int_tksyscall_debug	int_tksyscall_0xC4
#define	SWI_RETTEX		0xC5	/* return from task exception */
#define	int_tksyscall_rettex	int_syscall_0xC5

/*
 * software interrupt number for Extension
 */
#define	SWI_KILLPROC		0xC6	/* request to forcibly kill process */

#define	SWI_SYSCALL		0x80
#define	int_syscall		int_syscall_0x80
#define	int_syscall_return 	int_syscall_return_0x80


#define	INT_DIV_0		0
#define	INT_DEBUG		1
#define	INT_NMI			2
#define	INT_BREAKPOINT		3
#define	INT_INT0		4
#define	INT_BOUND		5
#define	INT_INVALID_OPCODE	6
#define	INT_CP_NO_AVAILABLE	7
#define	INT_DOUBLE_FAULT	8
#define	INT_CP_SEG_OVERRUN	9
#define	INT_TSS_ERROR		10
#define	INT_SEG_NOT_PRESENT	11
#define	INT_STACK_FAULT		12
#define	INT_GPF			13
#define	INT_PAGE_FAULT		14
#define	INT_RESERVED		15
#define	INT_FP_ERROR		16
#define	INT_ALIGN_CHECK		17
#define	INT_MACHINE_CHECK	18
#define	INT_SIMD_FP_EXCEPTION	19
#define	NUM_EXCEPTION		20

#define	INT_IRQ0		32
#define	INT_IRQ1		33
#define	INT_IRQ2		34
#define	INT_IRQ3		35
#define	INT_IRQ4		36
#define	INT_IRQ5		37
#define	INT_IRQ6		38
#define	INT_IRQ7		39
#define	INT_IRQ8		40
#define	INT_IRQ9		41
#define	INT_IRQ10		42
#define	INT_IRQ11		43
#define	INT_IRQ12		44
#define	INT_IRQ13		45
#define	INT_IRQ14		46
#define	INT_IRQ15		47


#ifndef _in_asm_source_
/*
----------------------------------------------------------------------------------
	saved register before entering interrupt context
----------------------------------------------------------------------------------
*/
struct ctx_reg {
	uint32_t	edi;
	uint32_t	esi;
	uint32_t	ebx;
	uint32_t	edx;
	uint32_t	ecx;
	uint32_t	eax;
	uint32_t	eip;
	uint32_t	cs;
	uint32_t	eflags;
	uint32_t	esp;
	uint32_t	ss;
};

/*
----------------------------------------------------------------------------------
	saved register before entering system call
----------------------------------------------------------------------------------
*/
struct pt_regs {
	uint32_t	bx;			// 0
	uint32_t	cx;			// 4
	uint32_t	dx;			// 8
	uint32_t	si;			// 12
	uint32_t	di;			// 16
	uint32_t	bp;			// 20
	uint32_t	ax;			// 24
	uint32_t	ds;			// 28
	uint32_t	es;			// 32
	uint32_t	fs;			// 36
	uint32_t	gs;			// 40
	uint32_t	orig_ax;		// 44
	uint32_t	ip;			// 48
	uint32_t	cs;			// 52
	uint32_t	flags;			// 56
	uint32_t	sp;			// 60
	uint32_t	ss;			// 64
};

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
 Funtion	:init_default_int_handlers
 Input		:void
 Output		:void
 Return		:void
 Description	:intialize default handlers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void init_default_int_handlers(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:register_int_handler
 Input		:int int_num
 		 < interrupt number to register the handler >
 		 void (*handler)(struct ctx_reg*)
 		 < interrupt handler to register >
 Output		:void
 Return		:ER
 		 < request result >
 Description	:request for registering interrupt handler
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER
register_int_handler(int int_num, void (*handler)(struct ctx_reg*));

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:unregister_int_handler
 Input		:int int_num
 		 < interrupt number to unregister the handler >
 Output		:void
 Return		:void
 Description	:request for unregistering interrupt handler
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void unregister_int_handler(int int_num);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:timer_handler
 Input		:void
 Output		:void
 Return		:void
 Description	:t-kernel standard timer handler
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void timer_handler( void );
#endif	// _in_asm_source_
#endif	// __INTERRUPT_H__
