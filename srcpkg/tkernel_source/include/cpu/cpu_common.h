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

#ifndef	__CPU_COMMON_H__
#define	__CPU_COMMON_H__

#if STD_SH7727
#endif
#if STD_SH7751R
#endif
#if MIC_M32104
#endif
#if STD_S1C38K
#endif
#if STD_MC9328
#endif
#if MIC_VR4131
#endif
#if STD_VR5500
#endif
#if STD_MB87Q1100
#endif
#if STD_SH7760
#endif
#if TEF_EM1D
#endif
#if _STD_X86_
#  include <cpu/x86/cpu_task.h>
#  include <cpu/x86/cpu_insn.h>
#  include <cpu/x86/cpuid.h>
#  include <cpu/x86/descriptor.h>
#  include <cpu/x86/interrupt.h>
#  include <cpu/x86/register.h>
#  include <cpu/x86/dispatch.h>
#  include <cpu/x86/cpu_status.h>
#  include <cpu/x86/break.h>
#  include <cpu/x86/bitsperlong.h>
#endif

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

#endif	// __CPU_COMMON_H__
