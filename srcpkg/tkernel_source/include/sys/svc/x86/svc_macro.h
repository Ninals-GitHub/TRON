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

#ifndef	__SVC_MACRO_H__
#define	__SVC_MACRO_H__

#define	_in_asm_source_
#include <cpu/x86/interrupt.h>
#undef	_in_asm_source
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
	invoke svc specified without argument
----------------------------------------------------------------------------------
*/
.macro define_svc_arg0 func_name func_code
	.text
	.balign	4
	.globl	Csym(\func_name)
	.type	Csym(\func_name), %function
Csym(\func_name):
	pushl	%ebp
	movl	%esp, %ebp
	movl	$\func_code, %eax
	int	$SWI_SVC
	pop	%ebp
	ret
.endm

/*
----------------------------------------------------------------------------------
	invoke svc specified with 1 argument
----------------------------------------------------------------------------------
*/
.macro define_svc_arg1 func_name func_code
	.text
	.balign	4
	.globl	Csym(\func_name)
	.type	Csym(\func_name), %function
Csym(\func_name):
	pushl	%ebx
	pushl	%ebp
	movl	%esp, %ebp
	movl	12(%ebp), %ebx
	movl	$\func_code, %eax
	int	$SWI_SVC
	popl	%ebp
	popl	%ebx
	ret
.endm

/*
----------------------------------------------------------------------------------
	invoke svc specified with 2 arguments
----------------------------------------------------------------------------------
*/
.macro define_svc_arg2 func_name func_code
	.text
	.balign	4
	.globl	Csym(\func_name)
	.type	Csym(\func_name), %function
Csym(\func_name):
	pushl	%ecx
	pushl	%ebx
	pushl	%ebp
	movl	%esp, %ebp
	movl	20(%ebp), %ecx		# 2nd
	movl	16(%ebp), %ebx		# 1st
	movl	$\func_code, %eax
	int	$SWI_SVC
	popl	%ebp
	popl	%ebx
	popl	%ecx
	ret
.endm

/*
----------------------------------------------------------------------------------
	invoke svc specified with 3 arguments
----------------------------------------------------------------------------------
*/
.macro define_svc_arg3 func_name func_code
	.text
	.balign	4
	.globl	Csym(\func_name)
	.type	Csym(\func_name), %function
Csym(\func_name):
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%ebp
	movl	%esp, %ebp
	movl	28(%ebp), %edx		# 3rd
	movl	24(%ebp), %ecx		# 2nd
	movl	20(%ebp), %ebx		# 1st
	movl	$\func_code, %eax
	int	$SWI_SVC
	popl	%ebp
	popl	%ebx
	popl	%ecx
	popl	%edx
	ret
.endm

/*
----------------------------------------------------------------------------------
	invoke svc specified with 4 arguments
----------------------------------------------------------------------------------
*/
.macro define_svc_arg4 func_name func_code
	.text
	.balign	4
	.globl	Csym(\func_name)
	.type	Csym(\func_name), %function
Csym(\func_name):
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%ebp
	movl	%esp, %ebp
	movl	36(%ebp), %esi		# 4th
	movl	32(%ebp), %edx		# 3rd
	movl	28(%ebp), %ecx		# 2nd
	movl	24(%ebp), %ebx		# 1st
	movl	$\func_code, %eax
	int	$SWI_SVC
	popl	%ebp
	popl	%ebx
	popl	%ecx
	popl	%edx
	popl	%esi
	ret
.endm

/*
----------------------------------------------------------------------------------
	invoke svc specified with 5 arguments
----------------------------------------------------------------------------------
*/
.macro define_svc_arg5 func_name func_code
	.text
	.balign	4
	.globl	Csym(\func_name)
	.type	Csym(\func_name), %function
Csym(\func_name):
	pushl	%edi
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%ebp
	movl	%esp, %ebp
	movl	44(%ebp), %edi		# 5th
	movl	40(%ebp), %esi		# 4th
	movl	36(%ebp), %edx		# 3rd
	movl	32(%ebp), %ecx		# 2nd
	movl	28(%ebp), %ebx		# 1st
	movl	$\func_code, %eax
	int	$SWI_SVC
	popl	%ebp
	popl	%ebx
	popl	%ecx
	popl	%edx
	popl	%esi
	popl	%edi
	ret
.endm

/*
----------------------------------------------------------------------------------
	invoke svc specified with 6 arguments
----------------------------------------------------------------------------------
*/
.macro define_svc_arg6 func_name func_code
	.text
	.balign	4
	.globl	Csym(\func_name)
	.type	Csym(\func_name), %function
Csym(\func_name):
	pushl	%edi
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%ebp
	movl	48(%esp), %ebp		# 6th
	movl	44(%esp), %edi		# 5th
	movl	40(%esp), %esi		# 4th
	movl	36(%esp), %edx		# 3rd
	movl	32(%esp), %ecx		# 2nd
	movl	28(%esp), %ebx		# 1st
	movl	$\func_code, %eax
	int	$SWI_SVC
	popl	%ebp
	popl	%ebx
	popl	%ecx
	popl	%edx
	popl	%esi
	popl	%edi
	ret
.endm

/*
----------------------------------------------------------------------------------
	invoke svc specified with 7 arguments
----------------------------------------------------------------------------------
*/
.macro define_svc_arg7 func_name func_code
	.text
	.balign	4
	.globl	Csym(\func_name)
	.type	Csym(\func_name), %function
Csym(\func_name):
	pushl	%edi
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%ebp
	movl	52(%esp), %ebp
	movl	%ebp, -40(%esp)		# 7th
	movl	48(%esp), %ebp		# 6th
	movl	44(%esp), %edi		# 5th
	movl	40(%esp), %esi		# 4th
	movl	36(%esp), %edx		# 3rd
	movl	32(%esp), %ecx		# 2nd
	movl	28(%esp), %ebx		# 1st
	movl	$\func_code, %eax
	int	$SWI_SVC
	popl	%ebp
	popl	%ebx
	popl	%ecx
	popl	%edx
	popl	%esi
	popl	%edi
	ret
.endm

/*
----------------------------------------------------------------------------------
	invoke svc specified with 8 arguments
----------------------------------------------------------------------------------
*/
.macro define_svc_arg8 func_name func_code
	.text
	.balign	4
	.globl	Csym(\func_name)
	.type	Csym(\func_name), %function
Csym(\func_name):
	pushl	%edi
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	pushl	%ebp
	movl	56(%esp), %ebp
	movl	%ebp, -40(%esp)		# 8th
	movl	52(%esp), %ebp
	movl	%ebp, -44(%esp)		# 7th
	movl	48(%esp), %ebp		# 6th
	movl	44(%esp), %edi		# 5th
	movl	40(%esp), %esi		# 4th
	movl	36(%esp), %edx		# 3rd
	movl	32(%esp), %ecx		# 2nd
	movl	28(%esp), %ebx		# 1st
	movl	$\func_code, %eax
	int	$SWI_SVC
	popl	%ebp
	popl	%ebx
	popl	%ecx
	popl	%edx
	popl	%esi
	popl	%edi
	ret
.endm

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

#endif	// __SVC_MACRO_H__
