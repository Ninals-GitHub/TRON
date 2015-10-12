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
 *	@(#)machine_common.h (sys)
 *
 *	Machine type definition
 */

#ifndef __SYS_MACHINE_COMMON_H__
#define __SYS_MACHINE_COMMON_H__

#ifdef _STD_SH7727_
#  include <sys/sysdepend/std_sh7727/machine_depend.h>
#endif
#ifdef _STD_SH7751R_
#  include <sys/sysdepend/std_sh7751r/machine_depend.h>
#endif
#ifdef _MIC_M32104_
#  include <sys/sysdepend/mic_m32104/machine_depend.h>
#endif
#ifdef _STD_S1C38K_
#  include <sys/sysdepend/std_s1c38k/machine_depend.h>
#endif
#ifdef _STD_MC9328_
#  include <sys/sysdepend/std_mc9328/machine_depend.h>
#endif
#ifdef _MIC_VR4131_
#  include <sys/sysdepend/mic_vr4131/machine_depend.h>
#endif
#ifdef _STD_VR5500_
#  include <sys/sysdepend/std_vr5500/machine_depend.h>
#endif
#ifdef _STD_MB87Q1100_
#  include <sys/sysdepend/std_mb87q1100/machine_depend.h>
#endif
#ifdef _STD_SH7760_
#  include <sys/sysdepend/std_sh7760/machine_depend.h>
#endif
#ifdef _TEF_EM1D_
#  include <sys/sysdepend/tef_em1d/machine_depend.h>
#endif
#ifdef _STD_X86_
#  include <sys/sysdepend/std_x86/machine_depend.h>
#endif

#endif /* __SYS_MACHINE_COMMON_H__ */
