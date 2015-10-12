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
 *	version.c (T-Kernel/OS)
 *	Version Information
 */

#include <basic.h>
#include <tk/tkernel.h>
#include "version.h"

EXPORT const T_RVER kernel_version = {
	RV_MAKER,		/* Kernel manufacturing code */
	RV_PRODUCT_ID,		/* Kernel ID number */
	RV_SPEC_VER,		/* Specification version number */
	RV_PRODUCT_VER,		/* Kernel version number */

	{
		/* Product management information */
		RV_PRODUCT_NO1,
		RV_PRODUCT_NO2,
		RV_PRODUCT_NO3,
		RV_PRODUCT_NO4
	}
};
