/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */

/*
 *	@(#)typedef.h
 *
 *	Standard data type definitions
 */

#ifndef	__TYPEDEF_H__
#define __TYPEDEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TKERNEL_CHECK_CONST
#define	CONST	const
#else
#define	CONST
#endif

#include <stdint.h>
#include <compiler.h>
#include <stddef.h>

typedef int8_t		B;	/* Signed 8 bit integer */
typedef int16_t		H;	/* Signed 16 bit integer */
typedef int32_t		W;	/* Signed 32 bit integer */
typedef int64_t		D;	/* Signed 64 bit integer */
typedef uint8_t		UB;	/* Unsigned 8 bit integer */
typedef uint16_t	UH;	/* Unsigned 16 bit integer */
typedef uint32_t	UW;	/* Unsigned 32 bit integer */
typedef uint64_t	UD;	/* Unsigned 64 bit integer */

typedef int8_t		VB;	/* Nonuniform type 8 bit data */
typedef int16_t		VH;	/* Nonuniform type 16 bit data */
typedef int32_t		VW;	/* Nonuniform type 32 bit data */
typedef int64_t		VD;	/* Nonuniform type 64 bit data */
typedef void		*VP;	/* Nonuniform type data pointer */

typedef volatile B	_B;	/* Volatile statement attached */
typedef volatile H	_H;
typedef volatile W	_W;
typedef volatile D	_D;
typedef volatile UB	_UB;
typedef volatile UH	_UH;
typedef volatile UW	_UW;
typedef volatile UD	_UD;

typedef signed int	INT;	/* Processor bit width signed integer */
typedef unsigned int	UINT;	/* Processor bit width unsigned integer */

typedef INT		ID;	/* ID general */
typedef	W		MSEC;	/* Time general (millisecond) */

typedef void		(*FP)();	/* Function address general */
typedef INT		(*FUNCP)();	/* Function address general */

#define LOCAL		static		/* Local symbol definition */
#define EXPORT				/* Global symbol definition */
#define IMPORT		extern		/* Global symbol reference */
#define	_INIT_				/* bk initialization function statement	*/

typedef	int (*INITCALL)(void);		/* bk initialization function type	*/
#define	INITCALL_DEFINE(func)	IMPORT int _INIT_ func(void)

/*
 * Boolean value
 *	Defined as TRUE = 1, but it is always true when not 0.
 *	Thus, comparison such as bool = TRUE are not permitted.
 *	Should be as per bool !=FALSE.
 */
typedef INT		BOOL;
#define TRUE		(1)	/* True */
#define FALSE		(0)	/* False */

/*
 * TRON character code
 */
typedef UH		TC;		/* TRON character code */
#define TNULL		((TC)0)		/* End of TRON code character string */


/*
 * For x86
 */
typedef	uint8_t			UINT8;
typedef	int8_t			INT8;
typedef	uint16_t		UINT16;
typedef	int16_t			INT16;
typedef	uint32_t		UINT32;
typedef	int32_t			INT32;
typedef	uint64_t		UINT64;
typedef	int64_t			INT64;

typedef int			BOOLEAN;
//typedef int64_t		off_t;
/*
 * miscellaneous types
 */
#ifndef __mtxcb__
#define __mtxcb__
typedef struct mutex_control_block	MTXCB;
#endif

#ifndef __tcb__
#define __tcb__
typedef struct task			TCB;
#endif

#ifndef __ctxb__
#define __ctxb__
typedef struct task_context_block	CTXB;
#endif

#if 0
/*
 * General-purpose data type
 */
typedef signed char	B;	/* Signed 8 bit integer */
typedef signed short	H;	/* Signed 16 bit integer */
typedef signed long	W;	/* Signed 32 bit integer */
typedef signed long long D;	/* Signed 64 bit integer */
typedef unsigned char	UB;	/* Unsigned 8 bit integer */
typedef unsigned short  UH;	/* Unsigned 16 bit integer */
typedef unsigned long	UW;	/* Unsigned 32 bit integer */
typedef unsigned long long UD;	/* Unsigned 64 bit integer */

typedef char		VB;	/* Nonuniform type 8 bit data */
typedef short		VH;	/* Nonuniform type 16 bit data */
typedef long		VW;	/* Nonuniform type 32 bit data */
typedef long long	VD;	/* Nonuniform type 64 bit data */
typedef void		*VP;	/* Nonuniform type data pointer */

typedef volatile B	_B;	/* Volatile statement attached */
typedef volatile H	_H;
typedef volatile W	_W;
typedef volatile D	_D;
typedef volatile UB	_UB;
typedef volatile UH	_UH;
typedef volatile UW	_UW;
typedef volatile UD	_UD;

typedef signed int	INT;	/* Processor bit width signed integer */
typedef unsigned int	UINT;	/* Processor bit width unsigned integer */

typedef INT		ID;	/* ID general */
typedef	W		MSEC;	/* Time general (millisecond) */

typedef void		(*FP)();	/* Function address general */
typedef INT		(*FUNCP)();	/* Function address general */

#define LOCAL		static		/* Local symbol definition */
#define EXPORT				/* Global symbol definition */
#define IMPORT		extern		/* Global symbol reference */

/*
 * Boolean value
 *	Defined as TRUE = 1, but it is always true when not 0.
 *	Thus, comparison such as bool = TRUE are not permitted.
 *	Should be as per bool !=FALSE.
 */
typedef INT		BOOL;
#define TRUE		(1==1)	/* True */
#define FALSE		(1!=0)	/* False */

/*
 * TRON character code
 */
typedef UH		TC;		/* TRON character code */
#define TNULL		((TC)0)		/* End of TRON code character string */


/*
 * For x86
 */
typedef unsigned int		UINT32;
typedef unsigned long long	UINT64;
#endif

#ifdef __cplusplus
}
#endif
#endif /* __TYPEDEF_H__ */
