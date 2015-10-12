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
 *	bittest.c (common)
 */

#include <bitop.h>

#define TSD_BTT_SFT_3	3
#define TSD_BTT_VAL_7	7
#define TSD_BTT_MSK_7	7U


EXPORT BOOL BitTest( VP base, UW offset )
{
#if BIGENDIAN
	return (BOOL)((((UB*)base)[offset >> TSD_BTT_SFT_3] >> (UW)(TSD_BTT_VAL_7 - (offset & TSD_BTT_MSK_7))) & 1U);
#else
	return (BOOL)((((UB*)base)[offset >> TSD_BTT_SFT_3] >> (UW)(offset & TSD_BTT_MSK_7)) & 1U);
#endif
}
