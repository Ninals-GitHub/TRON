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
 *	share.h		Device shared manager definition
 */

#ifndef	__DEVICE_SHARE_H__
#define	__DEVICE_SHARE_H__

#include <basic.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	Register/Deregister the shared interrupt handler
 *
 * IMPORT	ER	def_inthdr(INT dintno, FP inthdr);
 *
 * When interrupt vector is shared,
 * interrupt handler shall be registered by using "def_inthdr()" instead of "tk_def_int()".
 *
 * When interrupt occurs, registered interrupt is sequentially called in order of registration.
 * Therefore, whether the interrupt targeted by yourself occurs or not shall be checked.
 * And the corresponding interrupt processing shall be executed when it occurs.
 *
 * How to call an interrupt handler is just the same as how to register in "tk_def_int()".
 *
 *	dintno:	Interrupt number  (> 0): registration
 *		- Interrupt number (< 0): deregistration
 *		Interrupt number is same as the number specified at "tk_def_int()".
 *
 *	inthdr: Interrupt handler ("TA_HLG" attribute)
 *		Specify the registered interrupt handler also on deregistration.
 *
 *	return: = 1 Normal completion (the first registration or the final registration)
 *		= 0 Normal completion (except for the above)
 *		< 0 Error exit
 *
 * Operations such as the enabling/disabling of interrupt is not automatically executed.
 * Therefore, when "return == 1", the necessary enabling/disabling of interrupt shall be executed on the driver side.
 *
 */

/*
 *	Definition for interface library automatic creation (mkiflib).
 */
/*** DEFINE_IFLIB
[INCLUDE FILE]
<device/share.h>

[PREFIX]
CONSIO
***/

/*
 *	Device shared manager service function
 */
/* [BEGIN SYSCALLS] */

/* ORIGIN_NO 0x30 */
IMPORT	ER	def_inthdr(INT dintno, FP inthdr);
/* [END SYSCALLS] */

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_SHARE_H__ */
