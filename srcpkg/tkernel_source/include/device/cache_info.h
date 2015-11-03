/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------
 */

/*
 *	@(#)sysinfo_common.h (sys)
 *
 *	System cache information
 */

#ifndef __DEVICE_CACHE_INFO_H__
#define __DEVICE_CACHE_INFO_H__

#if STD_SH7727
#  include <device/std_sh7727/cache_info.h>
#endif
#if STD_SH7751R
#  include <device/std_sh7751r/cache_info.h>
#endif
#if MIC_M32104
#  include <device/mic_m32104/cache_info.h>
#endif
#if STD_S1C38K
#  include <device/std_s1c38k/cache_info.h>
#endif
#if STD_MC9328
#  include <device/std_mc9328/cache_info.h>
#endif
#if MIC_VR4131
#  include <device/mic_vr4131/cache_info.h>
#endif
#if STD_VR5500
#  include <device/std_vr5500/cache_info.h>
#endif
#if STD_MB87Q1100
#  include <device/std_mb87q1100/cache_info.h>
#endif
#if STD_SH7760
#  include <device/std_sh7760/cache_info.h>
#endif
#if TEF_EM1D
#  include <device/tef_em1d/cache_info.h>
#endif
#if _STD_X86_
#  include <device/std_x86/cache_info.h>
#endif

#endif /* __DEVICE_CACHE_INFO_H__ */
