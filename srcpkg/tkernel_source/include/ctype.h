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
 *	@(#)ctype.h
 *
 *	C language:  character operation
 */

#ifndef	__CTYPE_H__
#define __CTYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define _U_	01U
#define _L_	02U
#define _N_	04U
#define _S_	010U
#define _P_	020U
#define _C_	040U
#define _X_	0100U
#define _B_	0200U

extern int isalnum( int );
extern int isalpha( int );
extern int iscntrl( int );
extern int isdigit( int );
extern int isgraph( int );
extern int islower( int );
extern int isprint( int );
extern int ispunct( int );
extern int isspace( int );
extern int isupper( int );
extern int isxdigit( int );
extern int tolower( int );
extern int toupper( int );

extern int isascii( int );
extern int toascii( int );

extern const unsigned char __ctype[];

#define isalnum(c)	( (__ctype+1)[(c)&0377U] & (_U_|_L_|_N_) )
#define isalpha(c)	( (__ctype+1)[(c)&0377U] & (_U_|_L_) )
#define iscntrl(c)	( (__ctype+1)[(c)&0377U] & _C_ )
#define isdigit(c)	( (__ctype+1)[(c)&0377U] & _N_ )
#define isgraph(c)	( (__ctype+1)[(c)&0377U] & (_P_|_U_|_L_|_N_) )
#define islower(c)	( (__ctype+1)[(c)&0377U] & _L_ )
#define isprint(c)	( (__ctype+1)[(c)&0377U] & (_P_|_U_|_L_|_N_|_B_) )
#define ispunct(c)	( (__ctype+1)[(c)&0377U] & _P_ )
#define isspace(c)	( (__ctype+1)[(c)&0377U] & _S_ )
#define isupper(c)	( (__ctype+1)[(c)&0377U] & _U_ )
#define isxdigit(c)	( (__ctype+1)[(c)&0377U] & _X_ )
#define tolower(c)	( (__ctype+258)[(c)&0377U] )
#define toupper(c)	( (__ctype+515)[(c)&0377U] )

#define isascii(c)	( ((c) & ~0177U) == 0 )
#define toascii(c)	( (c) & 0177U )

#define _tolower(c)	tolower(c)
#define _toupper(c)	toupper(c)

#ifdef __cplusplus
}
#endif
#endif /* __CTYPE_H__ */
