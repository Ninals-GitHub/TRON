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
#define	_in_asm_source_
#include <cpu/x86/descriptor.h>
#include <cpu/x86/interrupt.h>
#include <cpu/x86/syscall_table.h>

#include <machine.h>
#include <tk/errno.h>
#include <tk/sysdef.h>
#include <tk/asm.h>
#include <sys/sysinfo.h>

#include <tk/config.h>
#include <tk/cpu_conf.h>
#include <tk/isysconf.h>
#include <tk/tkdev_conf.h>
#include "offset.h"

#include <bk/uapi/btron/errno.h>
#undef	_in_asm_source_

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
.extern exception_handler_err
.extern interrupt_handler
.extern page_fault_handler
.extern no_support
.extern svc_ientry
.extern _tdsvctbl
.extern syscall_table



/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	common procedure for exception with error code
----------------------------------------------------------------------------------
*/
.macro setup_exception_err exp_num
.globl interrupt_\exp_num

interrupt_\exp_num:
	xchg	(%esp), %eax			# save error code to eax
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ds
	pushl	%es
	pushl	%gs
	pushl	%fs
	movw	$SEG_KERNEL_DS, %cx		# set up kernel data segment
	movw	%cx, %ds
	movw	%cx, %es
	movw	%cx, %gs
	movw	%cx, %fs
	pushl	%eax				# argment3:error code
	pushl	%ebp
	addl	$4, (%esp)			# argument2:pushed context
	movl	$\exp_num, %ebx			# argument1:interrupt number
	pushl	%ebx
	cld
	call	exception_handler_err		# call handler
	addl	$12, %esp			# remove 3 arguments
	jmp	exit_interrupt
.endm

/*
----------------------------------------------------------------------------------
	procedure for page fault exception with error code
----------------------------------------------------------------------------------
*/
.macro setup_page_fault_err exp_num
.globl interrupt_\exp_num

interrupt_\exp_num:
	xchg	(%esp), %eax			# save error code to eax
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ds
	pushl	%es
	pushl	%gs
	pushl	%fs
	movw	$SEG_KERNEL_DS, %cx		# set up kernel data segment
	movw	%cx, %ds
	movw	%cx, %es
	movw	%cx, %gs
	movw	%cx, %fs
	movl	%cr2, %ecx			# falult address
	pushl	%ecx				# argment4:fault address
	pushl	%eax				# argment3:error code
	pushl	%ebp
	addl	$4, (%esp)			# argument2:pushed context
	movl	$\exp_num, %ebx			# argument1:interrupt number
	pushl	%ebx
	cld
	call	page_fault_handler		# call handler
	addl	$16, %esp			# remove 3 arguments and fault address
	jmp	exit_interrupt
.endm

/*
----------------------------------------------------------------------------------
	common procedure for interrupt
----------------------------------------------------------------------------------
*/
.macro setup_interrupt int_num
.globl interrupt_\int_num

interrupt_\int_num:
	pushl	%eax
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ds
	pushl	%es
	pushl	%gs
	pushl	%fs
	movw	$SEG_KERNEL_DS, %cx		# set up kernel data segment
	movw	%cx, %ds
	movw	%cx, %es
	movw	%cx, %gs
	movw	%cx, %fs
	pushl	%ebp
	addl	$4, (%esp)			# argument2:pushed context
	movl	$\int_num, %ebx			# argument1:interrupt number
	pushl	%ebx
	cld
	call	interrupt_handler		# call handler
	addl	$8, %esp			# remove 2 arguments
	jmp	exit_interrupt
.endm


/*
----------------------------------------------------------------------------------
	common procedure for system call interrupt
----------------------------------------------------------------------------------
*/
.macro setup_int_syscall int_num
.globl int_syscall_\int_num
.globl int_syscall_return_\int_num

#if 1
int_syscall_\int_num:
	cmpl	$SYSCALL_NUM, %eax
	jg	ret_nosys
#	pushl	%eax
#	pushl	%ecx
#	pushl	%edx
#	pushl	%ebx
#	pushl	%esi
#	pushl	%edi
	pushl	%ebp
	pushl	%ds
	pushl	%es
	pushl	%gs
	pushl	%fs
	pushl	%eax				# argument7 for test
	pushl	%ebp				# argument6
	pushl	%edi				# argument5
	pushl	%esi				# argument4
	pushl	%edx				# argument3
	pushl	%ecx				# argument2
	pushl	%ebx				# argument1
	
	movw	$SEG_KERNEL_DS, %cx		# set up kernel data segment
	movw	%cx, %ds
	movw	%cx, %es
	movw	%cx, %gs
	movw	%cx, %fs
	cld
	call	*syscall_table(,%eax,4)		# systemcall dispatch
int_syscall_return_\int_num:
	#addl	$28, %esp			# remove 7 arguments
	popl	%ebx
	popl	%ecx
	addl	$4, %esp			# remove edx argument
	popl	%esi
	popl	%edi
	addl	$8, %esp			# remove eaxa and ebp argument
	
	popl	%fs
	popl	%gs
	popl	%es
	popl	%ds
	leave					# restore ebp
#	popl	%edi
#	popl	%esi
#	popl	%ebx
#	popl	%edx
#	popl	%ecx
#	popl	%eax
	iret					# bye exception/interrupt
.endm
#endif


/*
----------------------------------------------------------------------------------
	common procedure for t-kernel system call interrupt
----------------------------------------------------------------------------------
*/
.macro setup_before_int_tksyscall int_num
.globl int_tksyscall_\int_num

int_tksyscall_\int_num:
	pushl	%ds
	pushl	%es
	pushl	%gs
	pushl	%fs
	
	subl	$8, %esp			# argument7 + argument 8
	
	pushl	%ebp				# argument6
	pushl	%edi				# argument5
	pushl	%esi				# argument4
	pushl	%edx				# argument3
	pushl	%ecx				# argument2
	
	movw	$SEG_KERNEL_DS, %cx		# set up kernel data segment
	movw	%cx, %ds
	movw	%cx, %es
	movw	%cx, %gs
	movw	%cx, %fs
	
.endm

.macro setup_after_int_tksyscall int_num
after_int_tksyscall_\int_num:
	popl	%fs
	popl	%gs
	popl	%es
	popl	%ds
	iret					# bye exception/interrupt
.endm

/*
----------------------------------------------------------------------------------
	common procedure for reserved interrupt
----------------------------------------------------------------------------------
*/
.macro setup_res_interrupt int_num
.globl interrupt_\int_num

interrupt_\int_num:
	pushl	%eax
	movl	\int_num, %eax
	xchg	(%esp), %eax			# save interrupt number to eax
	jmp	enter_res_interrupt
.endm

/*
==================================================================================

	Management 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	T-Kernel standard svc
----------------------------------------------------------------------------------
*/
	.text
	.balign	4
	.globl _svctbl
_svctbl:
	.long	no_support
#define	_tk_ret_int	no_support
#include <sys/svc/tksvctbl.h>
#undef	_tk_ret_int

/*
----------------------------------------------------------------------------------
	T-Kernel/DS svc
----------------------------------------------------------------------------------
*/
#if USE_DBGSPT
	.text
	.balign	4
	.globl _tdsvctbl
_tdsvctbl:
	.long	Csym(no_support)
#include <sys/svc/tdsvctbl.h>
#endif /* USE_DBGSPT */



/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
.text
.align 4
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:enter_res_interrupt
 Input		:void
 Output		:void
 Return		:void
 Description	:hello reserved exception/interrupt
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

enter_res_interrupt:
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ds
	pushl	%es
	pushl	%gs
	pushl	%fs
	movw	$SEG_KERNEL_DS, %cx		# set up kernel data segment
	movw	%cx, %ds
	movw	%cx, %es
	movw	%cx, %gs
	movw	%cx, %fs
	pushl	%ebp
	addl	$4, (%esp)			# argument2:pushed context
	movl	%eax, %ebx			# argument1:interrupt number
	pushl	%ebx
	cld
	call	interrupt_handler		# call handler
	addl	$8, %esp			# remove 2 arguments
	jmp	exit_interrupt

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:exit_interrupt
 Input		:void
 Output		:void
 Return		:void
 Description	:bye exception/interrupt
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

exit_interrupt:
	popl	%fs
	popl	%gs
	popl	%es
	popl	%ds
	leave					# restore ebp
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%edx
	popl	%ecx
	popl	%eax
	iret					# bye exception/interrupt
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:no_support
 Input		:void
 Output		:void
 Return		:ER
 		 < return E_RSFN error code >
 Description	:unsupported system call for t-kernel
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
	.text
	.balign	4
	.globl	Csym(no_support)
	.type	Csym(no_support), %function
no_support:
	pushl	%ebp
	movl	%esp, %ebp
	mov	$-10, %eax	# E_RSFN
	popl	%ebp
	ret

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:interrupt_0 - interrupt_47
 Input		:void
 Output		:void
 Return		:void
 Description	:call interrupt handler
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
setup_interrupt		0		# division by 0
setup_interrupt		1		# debug
setup_interrupt		2		# nmi
setup_interrupt		3		# breakpoint
setup_interrupt		4		# int0
setup_interrupt		5		# bound
setup_interrupt		6		# invalid opcode
setup_interrupt		7		# coprocessor not available
setup_exception_err	8		# double fault
setup_interrupt		9		# coprocessor segment overrun
setup_exception_err	10		# tss error
setup_exception_err	11		# segment not present
setup_exception_err	12		# stack fault
setup_exception_err	13		# general protection fault
setup_page_fault_err	14		# page fault

setup_interrupt		16		# floating-point error (math fault)
setup_interrupt		17		# alignment check
setup_interrupt		18		# machine check
setup_interrupt		19		# simd floating-point exception

setup_interrupt		32		# irq 0
setup_interrupt		33		# irq 1
setup_interrupt		34		# irq 2
setup_interrupt		35		# irq 3
setup_interrupt		36		# irq 4
setup_interrupt		37		# irq 5
setup_interrupt		38		# irq 6
setup_interrupt		39		# irq 7
setup_interrupt		40		# irq 8
setup_interrupt		41		# irq 9
setup_interrupt		42		# irq 10
setup_interrupt		43		# irq 11
setup_interrupt		44		# irq 12
setup_interrupt		45		# irq 13
setup_interrupt		46		# irq 14
setup_interrupt		47		# irq 15

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:systemcall 0x80
 Input		:void
 Output		:void
 Return		:void
 Description	:dispatch systemcall
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
setup_int_syscall	SWI_SYSCALL	# system call 0x80

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:systemcall for t-kernel
 Input		:void
 Output		:void
 Return		:void
 Description	:dispatch t-kernel standard systemcall or extended system call
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
setup_before_int_tksyscall	SWI_SVC		# system call 0xC0 for t-kernel
	pushl	%ebx				# argument1
	test	$0x80000000, %eax
	jz	call_svc_ientry
call_svctbl:
	andl	$0x7FFF0000, %eax
	shr	$16, %eax
	cld
	call	*_svctbl(,%eax,4)		# systemcall dispatch
	addl	$32, %esp			# remove 8 arguments
setup_after_int_tksyscall	SWI_SVC
call_svc_ientry:
	movl	%esp, %ebx
	pushl	%eax				# argument2
	pushl	%ebx				# argument1
	cld
	call	svc_ientry			# extended system call
	addl	$40, %esp			# remove 2 + 8 arguments
setup_after_int_tksyscall	999

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:systemcall for t-kernel/DS
 Input		:void
 Output		:void
 Return		:void
 Description	:dispatch t-kernel/DS systemcall
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
setup_before_int_tksyscall	SWI_DEBUG	# system call 0xC1 for t-kernel
	andl	$0x7FFF0000, %eax
	shr	$16, %eax
	pushl	%edi				# argument5
	pushl	%esi				# argument4
	pushl	%edx				# argument3
	pushl	%ecx				# argument2
	pushl	%ebx				# argument1
	cld
#if USE_DBGSPT
	call	*_tdsvctbl(,%eax,4)		# systemcall dispatch
#endif
	addl	$20, %esp			# remove 5 arguments
setup_after_int_tksyscall	SWI_DEBUG

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:interrupt_255
 Input		:void
 Output		:void
 Return		:void
 Description	:call reserved interrupt handler
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
setup_interrupt		255		# for invalid interrupt

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:ret_nosys
 Input		:void
 Output		:void
 Return		:void
 Description	:return with invalid system call
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
ret_nosys:
	movl	(-ENOSYS), %eax
	iret


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
