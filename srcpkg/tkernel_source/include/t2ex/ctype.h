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
 *	@(#)ctype.h
 *
 */

#ifndef _CTYPE_H_
#define	_CTYPE_H_

#include <basic.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	_toupper(c)	((c) - ('a' - 'A'))
#define	_tolower(c)	((c) + ('a' - 'A'))

IMPORT	const char	*_ctype_;
IMPORT	const short	*_tolower_tab_;
IMPORT	const short	*_toupper_tab_;

IMPORT	int	isalnum(int c);
IMPORT	int	isalpha(int c);
IMPORT	int	isascii(int c);
IMPORT	int	isblank(int c);
IMPORT	int	iscntrl(int c);
IMPORT	int	isdigit(int c);
IMPORT	int	isgraph(int c);
IMPORT	int	islower(int c);
IMPORT	int	isprint(int c);
IMPORT	int	ispunct(int c);
IMPORT	int	isspace(int c);
IMPORT	int	isupper(int c);
IMPORT	int	isxdigit(int c);
IMPORT	int	toascii(int c);
IMPORT	int	tolower(int c);
IMPORT	int	toupper(int c);

#ifdef __cplusplus
}
#endif
#endif /* _CTYPE_H_ */

