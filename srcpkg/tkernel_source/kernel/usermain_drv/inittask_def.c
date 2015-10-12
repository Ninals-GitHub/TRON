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
 *	inittask_def.c (extension)
 *	Initial task definition
 */

#include "inittask_def.h"

IMPORT void init_task(void);

/*
 * Initial task creation parameter
 */
EXPORT const T_CTSK c_init_task = {
	(void*)INITTASK_EXINF,	/* exinf */
	INITTASK_TSKATR,	/* tskatr */
	(FP)&init_task,		/* task */
	INITTASK_ITSKPRI,	/* itskpri */
	INITTASK_STKSZ,		/* stksz */
	INITTASK_SSTKSZ,	/* sstksz */
	(void*)INITTASK_STKPTR,	/* stkptr */
	(void*)INITTASK_UATB,	/* uatb */
	INITTASK_LSID,		/* lsid */
	INITTASK_RESID,		/* resid */
	"init"
};
