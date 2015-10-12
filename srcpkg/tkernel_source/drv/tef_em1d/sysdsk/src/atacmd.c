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
 *	atacmd.c	System disk driver
 *
 *	ATA (IDE) Disk command processing
 */

#include "sdisk.h"
#include "ata.h"

/*
 *	Set the wait-for-interrupt
 */
EXPORT	void	ataSelDrv(DrvTab *drv)
{
	/* Set the wait-for-interrupt */
	sdSetIntWait(drv);

	/* Reset the interrupt */
	ataStatusIn(drv);
}

/*
 *	Interrupt handler
 */
EXPORT	void	ataIntHdr(DrvTab *drv)
{
	if (drv->EnbInt) {			/* Valid interrupt?	*/
		DIS_INT(drv);			/* Disable the interrupt	*/
		drv->wrk.ub[0] = ataStatusIn(drv); /* Clear the interrupt	*/
		tk_wup_tsk(drv->WaitTskId);	/* Wake up the idle task	*/
	} else {
		/* Illegal interrupt : Clear the interrupt */
		ataStatusIn(drv);
	}
}

/*
 *	Wait-for-interrupt
 */
LOCAL	ER	WaitInt(DrvTab *drv, W dcnt, W cmd)
{
	ER	er;
	W	sts;

	/* Waken up from the interrupt handler, or wait until the time-out.
		However, do no wait if it is already aborted */
	er = (drv->Aborted) ? E_RLWAI : tk_slp_tsk(TMO_ATACMD);

	/* Disable interrupt */
	DIS_INT(drv);

	/* Fetch the status */
	sts = drv->wrk.ub[0];

	/* Time-out or abort request */
	if (er == E_TMOUT) {
		return ERR_TMOUT;
	}
	if (er == E_RLWAI) {
		return ERR_ABORT;
	}

	/* Other errors */
	if (er < E_OK) return er;

	if ((sts & stBSY) != 0) {
		return E_OK;
	}

	/* Error occurs when error exit(ERR = 1) or "DRQ != drq" */
	if ((sts & (stERR | stDRQ)) != (dcnt > 0 ? stDRQ : 0)) {
		if ((sts & stERR) == 0) {
			er = ERR_HARD;
		} else {
			/* er = DF UNC MC IDNF MCR ABRT TK0 AMNF */
			er = (ataErrorIn(drv) & 0x7F) | ((sts << 2) & 0x80);
			er = ERR_ATA(er);
		}
		return er;
	}
	return E_OK;
}

/*
 *	Abort processing
 */
EXPORT	void	ataAbort(DrvTab *drv)
{
	ER	er;

	/* Main task may be in "READY" status.
			Therefore, an abort flag shall be set up. */
	drv->Aborted = TRUE;

	/* Forcibly release the wait-for-main task (WaitInt)  */
	er = tk_rel_wai(drv->WaitTskId);
}

/*
 *	ATA command execution
 */
EXPORT	ER	ataCmd(DrvTab *drv, W cmd, UW lba, W len, void *buf)
{
	ER	er;
	W	cm, cnt, nsec, sc, cy, hd;
	BOOL	wrt;

#define	ATA_HSECSZ	(ATA_SECSZ / sizeof(H))

	wrt = FALSE;
	nsec = 1;
	cnt = sc = cy = NOSET;
	hd = 0;

	switch(cm = cmd) {
	case ATA_WRITE:
		wrt = TRUE;
	case ATA_READ:
		/* Count setting for "R/W-MULTIPLE" */
		nsec = drv->MultiCnt;
		if (nsec == 0 || len == 1) nsec = 1;
		else if (nsec > 1) {
			cm = (wrt) ? ATA_MWRITE : ATA_MREAD;
		}
		/* Logical block number -> Conversion to  CHS / LBA number */
		if (drv->UseLBA) {			/* Use LBA */
			sc = lba & 0xFF,		/* sec: LBA 00-07 */
			cy = (lba >> 8) & 0xFFFF;	/* cyl: LBA 08-23 */
			hd = ((lba >> 24) & 0xF) | drLBA; /* head:LBA 24-27 */
		} else {				/*  Use CHS*/
			sc = (lba % drv->nSec) + 1;	/* sec */
			cy = lba / drv->nSec;
			hd = cy % drv->nHead;		/* head */
			cy /= drv->nHead;		/* cyl */
		}
		cnt = len;
		break;
	case ATA_SETMULTI:
		cnt = drv->MultiCnt;
		break;
	case ATA_IDENTIFY:
		break;
	default:
		return ERR_ATA(erABRT);
	}

	/* ATA command output */
	er = ataCommandOut(drv, cm, cnt, sc, cy, hd);
	if (er < E_OK) return er;

	if (wrt == TRUE) {
		/* Wait to initially transfer for "nsec" sector  */
		er = ataWaitData(drv);
		if (er < E_OK) return er;

		/* Write data by each "nsec" sector and receive an interrupt */
		while (len > 0) {
			ENB_INT(drv);			/* Enable interrupt */

			/* Writing of data : There is also "buf == NULL" */
			if (len < nsec) nsec = len;
			ataDataOut(drv, (UH*)buf, nsec * ATA_HSECSZ);
			if (buf != NULL) buf += ATA_SECSZ * nsec;

			/* Wait-for-interrupt of completion of writing */
			er = WaitInt(drv, len -= nsec, cmd);
			if (er < E_OK) {	/* "ABRT" shall be "ERR_RONLY" */
				if (ERR_ATAMSK(er, erABRT) == ERR_ATA(erABRT))
					er = ERR_RONLY;
				break;
			}
		}
	} else {
		do {
			/* Wait-for-interrupt of completion of reading */
			er = WaitInt(drv, len, cmd);
			if (er < E_OK || len == 0) break;

			if (len > nsec)	ENB_INT(drv);	/* Enable the next interrupt */
			else		nsec = len;	/* Exit	*/

			/* Read data */
			ataDataIn(drv, (UH*)buf, nsec * ATA_HSECSZ);
			buf += ATA_SECSZ * nsec;
		} while ((len -= nsec) > 0);
	}

	/* Post-error processing*/
	if (er < E_OK) {
		if (((ERR_ATAMSK(er, erABRT) == ERR_ATA(erABRT))
						&& cmd == ATA_READ) ||
				er == ERR_TMOUT || er == ERR_HARD) {
			ataReset(drv);		/* ATA-reset */
		}
	}
	return er;
}
