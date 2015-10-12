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
 *	@(#)rominfo_conf.h (EM1D-512)
 *
 *	RomInfo configuration
 */

#ifndef _ROMINFO_CONF_
#define _ROMINFO_CONF_

IMPORT UB SYSCONF[], DEVCONF[];
IMPORT void START();

/* Kernel address */
#define RI_KERNEL_START	&START		/* Kernel start address */
#define RI_SYSCONF	(UB*)SYSCONF	/* SYSCONF top */
#define RI_DEVCONF	(UB*)DEVCONF	/* DEVCONF top */

/* User definition */
#define RI_USERAREA_TOP	(void*)0x40000000	/* RAM user area top */
#define RI_USERINIT	(FP)NULL	/* User initialization program */
#define RI_RESETINIT	(FP)NULL	/* Reset initialization program */

/* Rom disk */
#define RI_RDSK_TYPE	(UW)0		/* ROM disk type */
#define RI_RDSK_BLOCK	(UW)0		/* ROM disk block size */
#define RI_RDSK_START	(UW)0		/* ROM disk start address */
#define RI_RDSK_END	(UW)0		/* ROM disk end address */

#endif /* _ROMINFO_CONF_ */
