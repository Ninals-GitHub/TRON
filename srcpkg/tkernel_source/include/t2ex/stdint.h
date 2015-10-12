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
 *	@(#)stdint.h
 *
 */

#ifndef _STDINT_H_
#define	_STDINT_H_

#include <basic.h>

typedef	char	int8_t;
typedef	short	int16_t;
typedef	long	int32_t;
typedef	long long	int64_t;

typedef	unsigned char	uint8_t;
typedef	unsigned short	uint16_t;
typedef	unsigned long	uint32_t;
typedef	unsigned long long	uint64_t;

typedef	int8_t	int_least8_t;
typedef	int16_t	int_least16_t;
typedef	int32_t	int_least32_t;
typedef	int64_t	int_least64_t;

typedef	uint8_t	uint_least8_t;
typedef	uint16_t	uint_least16_t;
typedef	uint32_t	uint_least32_t;
typedef	uint64_t	uint_least64_t;

typedef	int32_t	int_fast8_t;
typedef	int32_t	int_fast16_t;
typedef	int32_t	int_fast32_t;
typedef	int64_t	int_fast64_t;

typedef	uint32_t	uint_fast8_t;
typedef	uint32_t	uint_fast16_t;
typedef	uint32_t	uint_fast32_t;
typedef	uint64_t	uint_fast64_t;

typedef	int32_t	intptr_t;
typedef	uint32_t	uintptr_t;

typedef	int64_t	intmax_t;
typedef	uint64_t	uintmax_t;

typedef long	ssize_t;

#define	INT8_MAX	(0x7f)
#define	INT8_MIN	(-INT8_MAX-1)

#define	INT16_MAX	(0x7fff)
#define	INT16_MIN	(-INT16_MAX-1)

#define	INT32_MAX	(0x7fffffff)
#define	INT32_MIN	(-INT32_MAX-1)

#define	INT64_MAX	(0x7fffffffffffffffLL)
#define	INT64_MIN	(-INT64_MAX-1LL)

#define	UINT8_MAX	(0xff)
#define	UINT16_MAX	(0xffff)
#define	UINT32_MAX	(0xffffffffU)
#define	UINT64_MAX	(0xffffffffffffffffULL)

#define	INT_LEAST8_MAX	INT8_MAX
#define	INT_LEAST8_MIN	INT8_MIN
#define	INT_LEAST16_MAX	INT16_MAX
#define	INT_LEAST16_MIN	INT16_MIN
#define	INT_LEAST32_MAX	INT32_MAX
#define	INT_LEAST32_MIN	INT32_MIN
#define	INT_LEAST64_MAX	INT64_MAX
#define	INT_LEAST64_MIN	INT64_MIN
#define	UINT_LEAST8_MAX	UINT8_MAX
#define	UINT_LEAST16_MAX	UINT16_MAX
#define	UINT_LEAST32_MAX	UINT32_MAX
#define	UINT_LEAST64_MAX	UINT64_MAX

#define	INT_FAST8_MAX	INT32_MAX
#define	INT_FAST8_MIN	INT32_MIN
#define	INT_FAST16_MAX	INT32_MAX
#define	INT_FAST16_MIN	INT32_MIN
#define	INT_FAST32_MAX	INT32_MAX
#define	INT_FAST32_MIN	INT32_MIN
#define	INT_FAST64_MAX	INT64_MAX
#define	INT_FAST64_MIN	INT64_MIN
#define	UINT_FAST8_MAX	UINT32_MAX
#define	UINT_FAST16_MAX	UINT32_MAX
#define	UINT_FAST32_MAX	UINT32_MAX
#define	UINT_FAST64_MAX	UINT64_MAX

#define	INTPTR_MIN	INT32_MIN
#define	INTPTR_MAX	INT32_MAX
#define	UINTPTR_MAX	UINT32_MAX

#define	INTMAX_MIN	INT64_MIN
#define	INTMAX_MAX	INT64_MAX
#define	UINTMAX_MAX	UINT64_MAX

#define	PTRDIFF_MIN	INT32_MIN
#define	PTRDIFF_MAX	INT32_MAX
#define	SIZE_MAX	UINT32_MAX
#define	WCHAR_MIN	0
#define	WCHAR_MAX	UINT32_MAX

#endif /* _STDINT_H_ */

