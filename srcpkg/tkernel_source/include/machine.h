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
 *	@(#)machine.h
 *
 *	Machine type definition
 *
 *	Describes differences between platforms, such as hardware
 *	and OS. Where the source code must be differentiated
 *	according to platform, macros and other definitions given
 *	here should be used.
 *
 *	Where possible, classifications should be based on general
 *	attributes rather than the machine type. Thus, the endian,
 *	for instance, should be described as BIGENDIAN rather than
 *	using a classification based on the machine type.
 *
 *	* Machine type definition is not used solely by the machine
 *	C language source file, so non-macro definitions are not
 *	permitted.
 */

#ifndef __MACHINE_H__
#define __MACHINE_H__

/* ===== System dependencies definitions ================================ */

#include <sys/sysdepend/machine_common.h>

/* ===== Common definitions ============================================= */

#ifndef Inline
#ifdef __cplusplus
#define Inline		inline
#else
#define Inline		static __inline__
#endif
#endif

#ifndef Asm
#ifdef __GNUC__
#define Asm		__asm__ volatile
#endif
#endif

/*
 * C symbol format
 *	_Csym = 0	do not append _
 *	_Csym = 1	append _
 *
 *	* In the UNIX System V Release 4 C compiler,
 *	   _ is not appended to symbols.
 */
#if _Csym == 0
#define Csym(sym)	sym
#else
#define Csym(sym)	_##sym
#endif

#endif /* __MACHINE_H__ */
