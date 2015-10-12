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
 *	inittask_def.h (extension)
 *	Initial task definition
 */

#ifndef _INITTASK_DEF_
#define _INITTASK_DEF_

#include <basic.h>
#include <tk/tkernel.h>

/*
 * Initial task parameter
 */
#define INITTASK_EXINF		(0)
#define INITTASK_TSKATR		(TA_HLNG | TA_RNG0)
#define INITTASK_ITSKPRI	(1)
#define INITTASK_STKSZ		(8 * 1024)
#define INITTASK_SSTKSZ		(0)
#define INITTASK_STKPTR		(NULL)
#define INITTASK_UATB		(NULL)
#define INITTASK_LSID		(0)
#define INITTASK_RESID		(0)

#endif /* _INITTASK_DEF_ */
