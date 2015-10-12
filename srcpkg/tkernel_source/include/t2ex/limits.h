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
 *	@(#)limits.h
 *
 */

#ifndef __LIMITS_H__
#define __LIMITS_H__

#define	CHAR_BIT	(8)
#define	SCHAR_MAX	(+127)			/* 0x7f */
#define	SCHAR_MIN	(-128)			/* 0x80 */
#define	UCHAR_MAX	(255)			/* 0xff */
#define	CHAR_MAX	SCHAR_MAX
#define	CHAR_MIN	SCHAR_MIN
#define	MB_LEN_MAX	(1)

#define	SHRT_MAX	(+32767)		/* 0x7fff */
#define	SHRT_MIN	(-32767)		/* 0x8001 */
#define	USHRT_MAX	(65535)			/* 0xffff */

#define	LONG_BIT	(32)
#define	LONG_MAX	(+2147483647L)		/* 0x7fffffff */
#define	LONG_MIN	(-2147483647L)		/* 0x80000001 */
#define	ULONG_MAX	(4294967295UL)		/* 0xffffffff */

#define	LLONG_MAX	(+9223372036854775807LL)	/* 0x7fffffffffffffff */
#define	LLONG_MIN	(-9223372036854775807LL)	/* 0x8000000000000001 */
#define	ULLONG_MAX	(18446744073709551615ULL)	/* 0xffffffffffffffff */

#define	INT_MAX		LONG_MAX
#define	INT_MIN		LONG_MIN
#define	UINT_MAX	ULONG_MAX

#define	SSIZE_MAX	LONG_MAX

#define	WORD_BIT	(32)
#define	NAME_MAX	(255)
#define	PATH_MAX	(1024)

#endif /* __LIMITS_H__ */

