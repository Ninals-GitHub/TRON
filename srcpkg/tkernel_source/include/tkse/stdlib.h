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
 *	stdlib.h
 *
 *	C language: General utility
 */

#ifndef	__STDLIB_H__
#define	__STDLIB_H__

#include "stdtype.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef	struct {
	int		quot, rem;
} div_t;

typedef	struct {
	long int	quot, rem;
} ldiv_t;

#define	NULL		0

#define	EXIT_SUCCESS	0
#define	EXIT_FAILURE	(-1)

#define	RAND_MAX	32767

/* Storage area management function */
extern void*	malloc( size_t size );
extern void*	calloc( size_t nmemb, size_t size );
extern void*	realloc( void *ptr, size_t size );
extern void	free( void *ptr );

extern double			atof( const char *nptr );
extern int			atoi( const char *nptr );
extern long int			atol( const char *nptr );
extern double			strtod( const char *nptr, char **endptr );
extern long int			strtol( const char *nptr, char **endptr, int base );
extern unsigned long int	strtoul( const char *nptr, char **endptr, int base );

extern int	rand( void );
extern void	srand( unsigned int seed );

extern void	abort( void );
extern int	atexit( void (*func)( void ) );
extern void	exit( int status );

extern int	abs( int j );
extern long int	labs( long int j );
extern div_t	div( int numer, int denom );
extern ldiv_t	ldiv( long int numer, long int denom );

#ifdef __cplusplus
}
#endif
#endif /* __STDLIB_H__ */
