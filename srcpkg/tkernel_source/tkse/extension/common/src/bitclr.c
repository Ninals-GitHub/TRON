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
 *	bitclr.c (common)
 */

#include <bitop.h>

#define TSD_BTC_SFT_3		3
#define TSD_BTC_BIT_0X80	0x80U
#define TSD_BTC_BIT_0X01	0x01U
#define TSD_BTC_MSK_0X00000007	0x00000007U


EXPORT void BitClr( VP base, UW offset )
{
#if BIGENDIAN
	((UB*)base)[offset >> TSD_BTC_SFT_3] &= ~(UB)(TSD_BTC_BIT_0X80 >> (offset & TSD_BTC_MSK_0X00000007));
#else
	((UB*)base)[offset >> TSD_BTC_SFT_3] &= ~(UB)(TSD_BTC_BIT_0X01 << (offset & TSD_BTC_MSK_0X00000007));
#endif
}
