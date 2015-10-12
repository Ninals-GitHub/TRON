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
 *      @(#)str_align_depend.h (sys/EM1-D512)
 *
 *	Bit alignment definitions for structure
 */

#ifndef __SYS_STR_ALIGN_DEPEND_H__
#define __SYS_STR_ALIGN_DEPEND_H__

/* 32 bit alignment */
#if BIGENDIAN
#  define _pad_b(n)	int :n;
#  define _pad_l(n)
#else
#  define _pad_b(n)
#  define _pad_l(n)	int :n;
#endif

#define _align64

#endif /* __SYS_STR_ALIGN_DEPEND_H__ */
