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
 *	bitop.h (tstdlib)
 *
 *	T-Kernel common standard library
 *
 */

#ifndef	_BITOP_
#define _BITOP_

#include <stdtype.h>
#include <compiler.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/


#ifdef	__size_t
typedef __size_t	size_t;
#undef	__size_t
#endif

#ifdef	__wchar_t
typedef __wchar_t	wchar_t;
#undef	__wchar_t
#endif

#define NULL		0

/*
==================================================================================

	Management 

==================================================================================
*/
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
extern void tstdlib_bitclr( void *base, W offset );
extern void tstdlib_bitset( void *base, W offset );
extern BOOL tstdlib_bittest( void *base, W offset );
 
extern W tstdlib_bitsearch0( void *base, W offset, W width );
extern W tstdlib_bitsearch1( void *base, W offset, W width );
extern W tstdlib_bitsearch1_binsearch( UW* base, W offset, W width );

#define	MAKE_BIT32(bit_num)		((uint32_t)1UL << bit_num)
#define	MAKE_MASK_SHIFT32(start, end)	((uint32_t)~1UL >> (31 - (end - start)))
#define	MAKE_MASK32(start, end)		(MAKE_MASK_SHIFT32(start,end) << start)

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:bit_value
 Input		:uint32_t value
 		 < bit value >
 		 uint32_t start
 		 < start bit to extract >
 		 uint32_t end
 		 < end bit to extract >
 Output		:void
 Return		:uint32_t
 		 < extracted value from bit value >
 Description	:extract specified bit value
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT
uint32_t bit_value(uint32_t value, uint32_t start, uint32_t end);

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/


#ifdef __cplusplus
}
#endif
#endif /* _BITOP_ */