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
 *	@(#)rominfo.c (EM1D-512)
 *
 *	ROM information
 */

#include <basic.h>
#include <sys/rominfo.h>
#include "rominfo_conf.h"

RomInfo rominfo = {
	RI_KERNEL_START,	/* Kernel startup address */
	RI_SYSCONF,		/* SYSCONF top */
	RI_DEVCONF,		/* DEVCONF top */
	RI_USERAREA_TOP,	/* RAM user area top */
	RI_USERINIT,		/* User initialization program address */
	RI_RESETINIT,		/* Reset initialization program address */
	{0},			/* Reserved (always 0) */

	RI_RDSK_TYPE,		/* ROM disk type */
	RI_RDSK_BLOCK,		/* ROM disk block size */
	RI_RDSK_START,		/* ROM disk start address */
	RI_RDSK_END,		/* ROM disk end address */
	{0}			/* Reserved (always 0) */
};
