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
 *    Modified by Nina Petipa at 2015/10/27
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
 *	@(#)string.h
 *
 */

#ifndef _STRING_H_
#define	_STRING_H_

#include <basic.h>
//#include <errno.h>
#include <t2ex/errno.h>
#include <tk/typedef.h>

#ifdef __cplusplus
extern "C" {
#endif

IMPORT	void	*memccpy(void *dst, const void *src, int c, size_t n);
IMPORT	void	*memchr(const void *s, int c, size_t n);
IMPORT	int	memcmp(const void *s1, const void *s2, size_t n);
IMPORT	void	*memcpy(void *dst, const void *src, size_t n);
IMPORT	void	*memmove(void *dst, const void *src, size_t n);
IMPORT	void	*memset(void *s, int c, size_t n);
IMPORT	char	*strcat(char *dst, const char *src);
IMPORT	char	*strchr(const char *s, int c);
IMPORT	int	strcmp(const char *s1, const char *s2);
IMPORT	int	strcoll(const char *s1, const char *s2);
IMPORT	char	*strcpy(char *dst, const char *src);
IMPORT	size_t	strcspn(const char *s1, const char *s2);
IMPORT	char	*strdup(const char *s);
IMPORT	size_t	strlen(const char *s);
IMPORT	size_t	strnlen(const char *s, size_t maxlen);
IMPORT	char	*strncat(char *dst, const char *src, size_t n);
IMPORT	int	strncmp(const char *s1, const char *s2, size_t n);
IMPORT	char	*strncpy(char *dst, const char *src, size_t n);
IMPORT	char	*strpbrk(const char *s1, const char *s2);
IMPORT	char	*strrchr(const char *s, int c);
IMPORT	size_t	strspn(const char *s1, const char *s2);
IMPORT	char	*strstr(const char *s1, const char *s2);
IMPORT	char	*strtok_r(char *str, const char *sep, char **lasts);
IMPORT	size_t	strxfrm(char *dst, const char *src, size_t n);
IMPORT	int	strerror_r(errno_t errnum, char *buf, size_t buflen);
IMPORT	int	strercd_r(ER ercd, char *buf, size_t buflen);

#ifdef __cplusplus
}
#endif
#endif /* _STRING_H_ */

