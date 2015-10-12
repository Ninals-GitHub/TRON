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
 *	bitsset.c (common)
 */

#include <bitop.h>
#include <libstr.h>

#define TSD_BST_N_8		8
#define TSD_BST_MSK_7		7U
#define TSD_BST_VAL_0XFF	0xffU

/*
 * Set the bit string.
 *	Set the width bits from the position of specified bit.
 */
EXPORT void BitsSet( VP base, W offset, W width )
{
	UB	*bp;
	INT	n;

	n = offset / TSD_BST_N_8;
	bp = (UB*)base + n;

	n = (W)((UW)offset & TSD_BST_MSK_7);
	if ( n > 0 ) {
		for ( ; n < TSD_BST_N_8; ++n ) {
			if ( --width < 0 ) {
				return;
			}
			BitSet(bp, (UW)n);
		}
		bp++;
	}

	n = width / TSD_BST_N_8;
	if ( n > 0 ) {
		memset(bp, TSD_BST_VAL_0XFF, (size_t)n);
	}
	bp += n;
	width -= n * TSD_BST_N_8;

	for ( n = 0; n < width; ++n ) {
		BitSet(bp, (UW)n);
	}
}
