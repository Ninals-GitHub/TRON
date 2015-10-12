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
 *	@(#)machine_depend.h (sys/EM1-D512)
 *
 *	Machine type definition
 */

#ifndef __SYS_MACHINE_DEPEND_H__
#define __SYS_MACHINE_DEPEND_H__

/*
 * CPU_xxxx		CPU type
 * ALLOW_MISALIGN	1 if access to misalignment data is allowed
 * BIGENDIAN		1 if big endian
 * VIRTUAL_ADDRESS	1 if virtual memory
 * ALLOCA_NOSPT		1 if alloca() is not supported
 */

/* ----- T-Engine Reference Board EM1-D512 definition ----- */
#undef _TEF_EM1D_

#define _TEF_EM1D_		1
#define CPU_ARM1176		1
#define CPU_ARMv6		1
#define CPU_ARM			1
#define TEF_EM1D		1
#define ALLOW_MISALIGN		0
#define BIGENDIAN		0
#define VIRTUAL_ADDRESS		1
#define ALLOCA_NOSPT		0

#ifndef _Csym
#define _Csym			0
#endif

#endif /* __SYS_MACHINE_DEPEND_H__ */
