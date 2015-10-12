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
 *	bitsclr.c (common)
 */

#include <bitop.h>
#include <libstr.h>

#define TSD_BCR_N_8	8
#define TSD_BCR_MSK_7	7U


/*
 * Clear the bit string.
 *	Clear the width bits from the position of specified bit.
 */
EXPORT void BitsClr( VP base, W offset, W width )
{
	UB	*bp;
	INT	n;

	n = offset / TSD_BCR_N_8;
	bp = (UB*)base + n;

	n = (W)((UW)offset & TSD_BCR_MSK_7);
	if ( n > 0 ) {
		for ( ; n < TSD_BCR_N_8; ++n ) {
			if ( --width < 0 ) {
				return;
			}
			BitClr(bp, (UW)n);
		}
		bp++;
	}

	n = width / TSD_BCR_N_8;
	if ( n > 0 ) {
		memset(bp, 0U, (size_t)n);
	}
	bp += n;
	width -= n * TSD_BCR_N_8;

	for ( n = 0; n < width; ++n ) {
		BitClr(bp, (UW)n);
	}
}
