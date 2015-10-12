/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	@(#)atomic_common.h (sys)
 *
 *	Atomic integer operations
 */

#ifndef __SYS_ATOMIC_COMMON_H__
#define __SYS_ATOMIC_COMMON_H__

#include <basic.h>
#include <tk/syslib.h>

#if TEF_EM1D
#  include <sys/sysdepend/tef_em1d/atomic_depend.h>
#endif

/*
 * From here, generic implementations of atomic integer operations 
 * are defined. Each of these definitions is used only when 
 * ATOMIC_xxx_USER_MODE remains undefined at this point.
 *
 * The value of ATOMIC_xxx_USER_MODE shall be defined as follows:
 *	0: atomic_xxx implementation cannot be used in user mode
 *	1: atomic_xxx implementation can be used in user mode
 */

#ifndef ATOMIC_INC_USER_MODE
#define ATOMIC_INC_USER_MODE 0
Inline UINT atomic_inc(volatile UINT* addr)
{
	UINT imask;
	UINT re;
	DI(imask);
	re = ++*addr;
	EI(imask);
	return re;
}
#endif /* ATOMIC_INC_USER_MODE */

#ifndef ATOMIC_DEC_USER_MODE
#define ATOMIC_DEC_USER_MODE 0
Inline UINT atomic_dec(volatile UINT* addr)
{
	UINT imask;
	UINT re;
	DI(imask);
	re = --*addr;
	EI(imask);
	return re;
}
#endif /* ATOMIC_DEC_USER_MODE */

#ifndef ATOMIC_ADD_USER_MODE
#define ATOMIC_ADD_USER_MODE 0
Inline UINT atomic_add(volatile UINT* addr, UINT val)
{
	UINT imask;
	UINT re;
	DI(imask);
	*addr += val;
	re = *addr;
	EI(imask);
	return re;
}
#endif /* ATOMIC_ADD_USER_MODE */

#ifndef ATOMIC_SUB_USER_MODE
#define ATOMIC_SUB_USER_MODE 0
Inline UINT atomic_sub(volatile UINT* addr, UINT val)
{
	UINT imask;
	UINT re;
	DI(imask);
	*addr -= val;
	re = *addr;
	EI(imask);
	return re;
}
#endif /* ATOMIC_SUB_USER_MODE */

/* TAS: test-and-set */
#ifndef ATOMIC_XCHG_USER_MODE
#define ATOMIC_XCHG_USER_MODE 0
Inline UINT atomic_xchg(volatile UINT* addr, UINT val)
{
	UINT imask;
	UINT re;
	DI(imask);
	re = *addr;
	*addr = val;
	EI(imask);
	return re;
}
#endif /* ATOMIC_XCHG_USER_MODE */

/* CAS: compare-and-swap */
#ifndef ATOMIC_CMPXCHG_USER_MODE
#define ATOMIC_CMPXCHG_USER_MODE 0
Inline UINT atomic_cmpxchg(volatile UINT* addr, UINT val, UINT cmp)
{
	UINT imask;
	UINT re;
	DI(imask);
	re = *addr;
	if ( *addr == cmp ) {
		*addr = val;
	}
	EI(imask);
	return re;
}
#endif /* ATOMIC_CMPXCHG_USER_MODE */

#ifndef ATOMIC_BITSET_USER_MODE
#define ATOMIC_BITSET_USER_MODE 0
Inline UINT atomic_bitset(volatile UINT* addr, UINT setptn)
{
	UINT imask;
	UINT re;
	DI(imask);
	re = *addr;
	*addr |= setptn;
	EI(imask);
	return re;
}
#endif /* ATOMIC_BITSET_USER_MODE */

#ifndef ATOMIC_BITCLR_USER_MODE
#define ATOMIC_BITCLR_USER_MODE 0
Inline UINT atomic_bitclr(volatile UINT* addr, UINT clrptn)
{
	UINT imask;
	UINT re;
	DI(imask);
	re = *addr;
	*addr &= clrptn;
	EI(imask);
	return re;
}
#endif /* ATOMIC_BITCLR_USER_MODE */

#endif /* __SYS_ATOMIC_COMMON_H__ */
