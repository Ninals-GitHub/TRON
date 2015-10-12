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
 *	wrkbuf.c
 *
 *       work buffer
 *
 *       this is in an independent separate file so that we can specify the layout at link-time.
 *       this is also used to store FlashROM writing program.
 *
 */

#include "cmdsvc.h"

EXPORT	UB	wrkBuf[WRKBUF_SZ];
