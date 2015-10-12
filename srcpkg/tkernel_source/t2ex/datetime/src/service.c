/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	@(#)service.c
 *
 *       T2EX: calendar functions
 *       manager initialization
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <t2ex/datetime.h>
#include "ifdatetime.h"

IMPORT ER ChkT2EXLevel( void );

/* Manager lock */
LOCAL	FastLock	dtLock;

/* System timezone */
LOCAL	struct tzinfo	systemTimeZone = {
	.tzname = {"JST", ""},
	.offset = -9 * 60 * 60,
	.daylight = 0,
};

/*
 * Set system timezone
 */
EXPORT	ER	_dt_setsystz(const struct tzinfo* tz)
{
	ER	er;

	er = ChkSpaceR(tz, sizeof(*tz));
	if (er < E_OK) {
		return er;
	}

	systemTimeZone = *tz;
	return E_OK;
}

/*
 * Get system timezone
 */
EXPORT	ER	_dt_getsystz(struct tzinfo* tz)
{
	ER	er;

	er = ChkSpaceRW(tz, sizeof(*tz));
	if (er < E_OK) {
		return er;
	}

	*tz = systemTimeZone;
	return E_OK;
}

/*
 * Service call entries
 */
LOCAL	ER	svcEntry(void* para, W fn)
{
	ER	er;

	er = ChkT2EXLevel();
	if (er < E_OK) {
		return er;
	}

	Lock(&dtLock);

	switch (fn) {
	case DT_DT_SETSYSTZ_FN: {
		DT_DT_SETSYSTZ_PARA* p = (DT_DT_SETSYSTZ_PARA*)para;
		er = _dt_setsystz(p->tz);
		break;
	}
	case DT_DT_GETSYSTZ_FN: {
		DT_DT_GETSYSTZ_PARA* p = (DT_DT_GETSYSTZ_PARA*)para;
		er = _dt_getsystz(p->tz);		
		break;
	}
	default:
		er = E_RSFN;
		break;
	}

	Unlock(&dtLock);

	return er;
}

/*
 * Calendar functions entry
 */
EXPORT	ER	dt_main(INT ac, UB* av[])
{
	ER	er;
	T_DSSY	dssy;

	if (ac < 0) {
		/* Stop other tasks from executing SVC */
		Lock(&dtLock);

		/* Unregister subsystem */
		tk_def_ssy(DT_SVC, NULL);

		/* Delete lock */
		DeleteLock(&dtLock);
		return E_OK;
	}

	/* Create lock */
	er = CreateLock(&dtLock, "dtLk");
	if (er < E_OK) {
		goto err_ret0;
	}

	/* Register subsystem */
	dssy.ssyatr	= TA_NULL;
	dssy.ssypri	= DT_PRI;
	dssy.svchdr	= (FP)svcEntry;
	dssy.breakfn	= NULL;
	dssy.startupfn	= NULL;
	dssy.cleanupfn	= NULL;
	dssy.eventfn	= NULL;
	dssy.resblksz	= 0;
	er = tk_def_ssy(DT_SVC, &dssy);
	if (er < E_OK) {
		goto err_ret1;
	}

	return E_OK;

err_ret1:
	DeleteLock(&dtLock);
	
err_ret0:
	return er;
}
