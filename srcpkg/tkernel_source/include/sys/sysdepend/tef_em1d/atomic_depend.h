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
 *	@(#)atomic_depend.h (sys/EM1-D512)
 *
 *	Atomic integer operations
 */

#ifndef __SYS_ATOMIC_DEPEND_H__
#define __SYS_ATOMIC_DEPEND_H__

#define ATOMIC_INC_USER_MODE 1
Inline UINT atomic_inc(volatile UINT* addr)
{
	UINT excl, re;

	do {
		Asm("	ldrex	%2, [%3]\n"
                    "	add	%2, %2, #1\n"
                    "	strex	%1, %2, [%3]"
			: "+m"(*addr), "=r"(excl), "=r"(re)
			: "r"(addr)
			: "cc", "memory");
	} while (excl);

	return re;
}

#define ATOMIC_DEC_USER_MODE 1
Inline UINT atomic_dec(volatile UINT* addr)
{
	UINT excl, re;

	do {
		Asm("	ldrex	%2, [%3]\n"
                    "	sub	%2, %2, #1\n"
                    "	strex	%1, %2, [%3]"
			: "+m"(*addr), "=r"(excl), "=r"(re)
			: "r"(addr)
			: "cc", "memory");
	} while (excl);

	return re;
}

#define ATOMIC_ADD_USER_MODE 1
Inline UINT atomic_add(volatile UINT* addr, UINT val)
{
	UINT excl, re;

	do {
		Asm("	ldrex	%2, [%3]\n"
                    "	add	%2, %2, %4\n"
                    "	strex	%1, %2, [%3]"
			: "+m"(*addr), "=r"(excl), "=r"(re)
			: "r"(addr), "r"(val)
			: "cc", "memory");
	} while (excl);

	return re;
}

#define ATOMIC_SUB_USER_MODE 1
Inline UINT atomic_sub(volatile UINT* addr, UINT val)
{
	UINT excl, re;

	do {
		Asm("	ldrex	%2, [%3]\n"
                    "	sub	%2, %2, %4\n"
                    "	strex	%1, %2, [%3]"
			: "+m"(*addr), "=r"(excl), "=r"(re)
			: "r"(addr), "r"(val)
			: "cc", "memory");
	} while (excl);

	return re;
}

/* TAS: test-and-set */
#define ATOMIC_XCHG_USER_MODE 1
Inline UINT atomic_xchg(volatile UINT* addr, UINT val)
{
	UINT excl, re;

	do {
		Asm("	ldrex	%2, [%3]\n"
                    "	strex	%1, %4, [%3]"
			: "+m"(*addr), "=r"(excl), "=r"(re)
			: "r"(addr), "r"(val)
			: "cc", "memory");
	} while (excl);

	return re;
}

/* CAS: compare-and-swap */
#define ATOMIC_CMPXCHG_USER_MODE 1
Inline UINT atomic_cmpxchg(volatile UINT* addr, UINT val, UINT cmp)
{
	UINT excl, re;

	do {
		Asm("	mov	%1, #0\n"
		    "	ldrex	%2, [%3]\n"
		    "	cmp	%2, %5\n"
                    "	strexeq	%1, %4, [%3]\n"
                    "	clrex"
			: "+m"(*addr), "=r"(excl), "=r"(re)
			: "r"(addr), "r"(val), "r"(cmp)
			: "cc", "memory");
	} while (excl);

	return re;
}

#define ATOMIC_BITSET_USER_MODE 1
Inline UINT atomic_bitset(volatile UINT* addr, UINT setptn)
{
	UINT excl, re, tmp;

	do {
		Asm("	ldrex	%2, [%4]\n"
                    "	orr	%3, %2, %5\n"
                    "	strex	%1, %3, [%4]"
			: "+m"(*addr), "=r"(excl), "=r"(re), "=r"(tmp)
			: "r"(addr), "r"(setptn)
			: "cc", "memory");
	} while (excl);

	return re;
}

#define ATOMIC_BITCLR_USER_MODE 1
Inline UINT atomic_bitclr(volatile UINT* addr, UINT clrptn)
{
	UINT excl, re, tmp;

	do {
		Asm("	ldrex	%2, [%4]\n"
                    "	and	%3, %2, %5\n"
                    "	strex	%1, %3, [%4]"
			: "+m"(*addr), "=r"(excl), "=r"(re), "=r"(tmp)
			: "r"(addr), "r"(clrptn)
			: "cc", "memory");
	} while (excl);

	return re;
}

#endif /* __SYS_ATOMIC_DEPEND_H__ */
