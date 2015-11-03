/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */

/*
 *	@(#)asm_common.h (tk)
 *
 *	Assembler Macro
 */

#ifndef __TK_ASM_COMMON_H__
#define __TK_ASM_COMMON_H__

#if STD_SH7727
#  include <tk/sysdepend/std_sh7727/asm_depend.h>
#endif
#if STD_SH7751R
#  include <tk/sysdepend/std_sh7751r/asm_depend.h>
#endif
#if MIC_M32104
#  include <tk/sysdepend/mic_m32104/asm_depend.h>
#endif
#if STD_S1C38K
#  include <tk/sysdepend/std_s1c38k/asm_depend.h>
#endif
#if STD_MC9328
#  include <tk/sysdepend/std_mc9328/asm_depend.h>
#endif
#if MIC_VR4131
#  include <tk/sysdepend/mic_vr4131/asm_depend.h>
#endif
#if STD_VR5500
#  include <tk/sysdepend/std_vr5500/asm_depend.h>
#endif
#if STD_MB87Q1100
#  include <tk/sysdepend/std_mb87q1100/asm_depend.h>
#endif
#if STD_SH7760
#  include <tk/sysdepend/std_sh7760/asm_depend.h>
#endif
#if TEF_EM1D
#  include <tk/sysdepend/tef_em1d/asm_depend.h>
#endif
#if _STD_X86_
#  include <tk/sysdepend/std_x86/asm_depend.h>
#endif

#endif /* __TK_ASM_COMMON_H__ */
