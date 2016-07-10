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
 *	@(#)libstr.h
 *
 *	Standard library for kernel link
 *
 */

#ifndef	__LIBSTR_H__
#define __LIBSTR_H__

#include <typedef.h>
#include <stdtype.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	__size_t
typedef __size_t	size_t;
#undef	__size_t
#endif

#ifdef	__wchar_t
typedef __wchar_t	wchar_t;
#undef	__wchar_t
#endif

#define NULL		0

IMPORT void* memset( void *s, int c, size_t n );
IMPORT int memcmp( const void *s1, const void *s2, size_t n );
IMPORT void* memcpy( void *dst, const void *src, size_t n );
IMPORT void* memmove( void *dst, const void *src, size_t n );
IMPORT void bzero( void *s, size_t n );

IMPORT size_t strlen( const char *s );
IMPORT size_t strnlen(const char *s, size_t maxlen);
IMPORT int strcmp( const char *s1, const char *s2 );
IMPORT int strncmp( const char *s1, const char *s2, size_t n );
IMPORT char* strcpy( char *dst, const char *src );
IMPORT char* strncpy( char *dst, const char *src, size_t n );
IMPORT char* strcat( char *dst, const char *src );
IMPORT char* strncat( char *dst, const char *src, size_t n );

IMPORT long int strtol( const char *nptr, char **endptr, int base );

#ifdef __cplusplus
}
#endif
#endif /* __LIBSTR_H__ */
