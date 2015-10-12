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
 *	@(#)stdlib.h
 *
 */

#ifndef _STDLIB_H_
#define	_STDLIB_H_

#include <basic.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	RAND_MAX	0x7fffffff

typedef struct {
	int	quot;
	int	rem;
} div_t;

typedef struct {
	long	quot;
	long	rem;
} ldiv_t;

typedef struct {
	long long	quot;
	long long	rem;
} lldiv_t;

struct rand48_data {
	unsigned short	xi[3];
	unsigned short	mult[3];
	unsigned short	add;
};

IMPORT	int	abs(int i);
IMPORT	long	labs(long i);
IMPORT	long long llabs(long long i);

IMPORT	int	atoi(const char *str);
IMPORT	long	atol(const char *str);
IMPORT	long long	atoll(const char *str);
IMPORT	double	atof(const char *str);

IMPORT	div_t	div(int n, int d);
IMPORT	ldiv_t	ldiv(long n, long d);
IMPORT	lldiv_t	lldiv(long long n, long long d);

IMPORT	void	*malloc(size_t size);
IMPORT	void	*calloc(size_t nelem, size_t elsize);
IMPORT	void	*realloc(void *ptr , size_t size);
IMPORT	void	free(void *ptr);

IMPORT	void	*bsearch(const void *key, const void *base, size_t nel, size_t width, int (*compare)(const void *, const void *));
IMPORT	void	qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));

IMPORT	int	rand_r(unsigned int *seed);
IMPORT	double	drand48_r(struct rand48_data *buffer);
IMPORT	long	lrand48_r(struct rand48_data *buffer);
IMPORT	long	mrand48_r(struct rand48_data *buffer);
IMPORT	void	srand48_r(long int seedval, struct rand48_data *buffer);
IMPORT	void	seed48_r(unsigned short int seed16v[3], struct rand48_data *buffer, unsigned short oldxi[3]);
IMPORT	void	lcong48_r(unsigned short int param[7], struct rand48_data *buffer);

IMPORT	char	*realpath_eno(const char *path, char *resolved_path, errno_t *eno);
IMPORT	char	*realpath(const char *path, char *resolved_path);
IMPORT	char	*realpath2_eno(const char *path1, const char* path2, char *resolved_path, errno_t *eno);
IMPORT	char	*realpath2(const char *path1, const char* path2, char *resolved_path);

IMPORT	void	abort(void);
IMPORT	int	setabort(void (*func)(void));

IMPORT	long	strtol(const char *str, char **endptr, int base);
IMPORT	long long	strtoll(const char *str, char **endptr, int base);
IMPORT	unsigned long	strtoul(const char *str, char **endptr, int base);
IMPORT	unsigned long long	strtoull(const char *str, char **endptr, int base);
IMPORT	float	strtof(const char *nptr, char **endptr);
IMPORT	double	strtod(const char *nptr, char **endptr);
IMPORT	long double	strtold(const char *nptr, char **endptr);

#ifdef __cplusplus
}
#endif
#endif /* _STDLIB_H_ */

