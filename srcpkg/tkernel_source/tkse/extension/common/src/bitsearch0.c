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
 *	bitsearch0.c (common)
 */

#include <bitop.h>

#define TSD_BS0_MSK_3		3U
#define TSD_BS0_VAL_8		8U
#define TSD_BS0_VAL_32		32
#define TSD_BS0_MSK_31		31
#define TSD_BS0_RTN_M1		(-1)
#define TSD_BS0_MSK_0X00000001	0x00000001U
#define TSD_BS0_MSK_0X80000000	0x80000000U

/*
 * Retrieve bit 0.
 *	Retrieve the width bits from the position of specified bit and return the position.
 *	If not found, return -1.
 *	The found position is returned in relative position from the retrieval start position.
 *	When found, the return value is between 0 and width-1.
 */
EXPORT W BitSearch0( VP base, W offset, W width )
{
	UW	*wp, w;
	INT	pos, end, n;

	wp = (UW*)((UINT)base & ~TSD_BS0_MSK_3);
	offset += (W)(((UINT)base & TSD_BS0_MSK_3) * TSD_BS0_VAL_8);
	n = offset / TSD_BS0_VAL_32;
	wp += n;
	offset -= n * TSD_BS0_VAL_32;
	pos = offset;
	end = offset + width;

	n = pos & TSD_BS0_MSK_31;
	if ( n > 0 ) {
		n = TSD_BS0_VAL_32 - n;
		while ( n-- > 0 ) {
			if ( pos >= end ) {
				return TSD_BS0_RTN_M1;
			}
			if ( !BitTest(wp, (UW)pos) ) {
				return pos - offset;
			}
			pos++;
		}
		wp++;
	}

	while ( (end - pos) >= TSD_BS0_VAL_32 ) {
		w = *wp;
		if ( w != ~0U ) {
			for ( n = 0; n < TSD_BS0_VAL_32; ++n ) {
#if BIGENDIAN
				if ( (w & TSD_BS0_MSK_0X80000000) == 0U ) {
					return pos + n - offset;
				}
				w <<= 1U;
#else
				if ( (w & TSD_BS0_MSK_0X00000001) == 0U ) {
					return pos + n - offset;
				}
				w >>= 1U;
#endif
			}
		}
		wp++;
		pos += TSD_BS0_VAL_32;
	}

	n = pos;
	while ( pos < end ) {
		if ( !BitTest(wp, (UW)(pos - n)) ) {
			return pos - offset;
		}
		pos++;
	}

	return TSD_BS0_RTN_M1;
}
