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
 *	@(#)cpuattr.h (tk/EM1-D512)
 *
 *	Type Specific Function
 */

#ifndef __TK_CPUATTR_H__
#define __TK_CPUATTR_H__

/*
 * Global pointer support
 *   0: No global pointer support
 */
#define TA_GP		0		/* No global pointer support */

/*
 * Using FPU (depend on CPU)
 *   0: not using FPU
 *   TA_COPn(n = 0-3): using FPU
 */
#define TA_FPU		0

#endif /* __TK_CPUATTR_H__ */
