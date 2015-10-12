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
 *	inthdr.c	System disk driver
 *
 *	Common interrupt handler ("ATA" and "PC" card)
 */

#include "sdisk.h"

/*
 *	Register the interrupt handler
 */
EXPORT	ER	sdDefIntHdr(DrvTab *drv, BOOL regist)
{
	ER	er;
	T_DINT	dint;

	dint.intatr = TA_HLNG;

	if (drv->Spec.accif == MEM_ACCIF) return E_OK;	/* Ignore */

	if (drv->Spec.pccard) {		/* PC card */
		er = E_NOSPT;

	} else {			/* Fixed hardware */
		er = E_NOSPT;
	}
	if (er >= E_OK) {
		if (regist && drv->Spec.accif == ATA_ACCIF) {
			ataEnbInt(drv);		/* ATA interrupt enabled */
		}
		return E_OK;
	}
	return er;
}
/*
 *	Set the wait-for-interrupt
 */
EXPORT	void	sdSetIntWait(DrvTab *drv)
{
	/* Get the task number that waits for interrupt */
	drv->WaitTskId = tk_get_tid();

	/* Clear the previous wakeup request */
	tk_can_wup(drv->WaitTskId);
}
