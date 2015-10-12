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
 *	ataio.c		System disk driver
 *
 *	ATA (IDE) Drive I/O processing
 */

#include "sdisk.h"
#include "ata.h"
#include "pccardio.h"

/*
 *	Time-out time
 */
#define	STATUS_TMO	(50 * 1000)		/* x usec = 50 msec	*/
#define	READY_TMO	(10 * 1000)		/* x msec = 10 sec	*/
#define	RESET_TMO	(31 * 1000)		/* x msec = 31 sec	*/

/*
 *	Status input
 */
EXPORT	W	ataStatusIn(DrvTab *drv)
{
	return InForceB(drv->IOB, REG_STS);
}

/*
 *	Error information input
 */
EXPORT	W	ataErrorIn(DrvTab *drv)
{
	UW	iob = drv->IOB;

	return InB(REG_ERR);
}

/*
 *	Cylinder register input
 */
EXPORT	W	ataCylIn(DrvTab *drv)
{
	UW	iob = drv->IOB;

	return (InB(REG_CYL_H) << 8) | InB(REG_CYL_L);
}

/*
 *	Status check
 */
LOCAL	W	ataChkSts(DrvTab *drv, UB chk, UB ok)
{
	UW	iob = drv->IOB;
	W	i;

	for (i = 0; i < STATUS_TMO; i += 10, WaitUsec(10)) {
		if ((InB(REG_STS) & chk) == ok) return E_OK;
	}
	return ERR_DATABUSY;
}

/*
 *	Drive setting
 */
EXPORT	ER	ataSetDrive(DrvTab *drv, W drvno)
{
	UW	iob = drv->IOB;
	UB	dno;

	if (drvno < 0) drvno = drv->DrvNo;

	OutB(REG_DRVHEAD, dno = drDRV(drvno));
	WaitUsec(4);		/* Waiting is necessary depending on drive */
	if (InB(REG_DRVHEAD) == dno) return E_OK;
	return ERR_NOPORT;
}

/*
 *	Interrupt-enabled
 */
EXPORT	void	ataEnbInt(DrvTab *drv)
{
	UW	iob = drv->IOB;
	UW	reg;

	reg = (drv->d.pcc.IOConf[0] & IOC_IO2_NONE)? REG_DEVCTL : REG_DEVCTL2;
	OutB(reg, dcNORM);
}

/*
 *	Wait until a data transfer is OK
 */
EXPORT	ER	ataWaitData(DrvTab *drv)
{
	/* Wait until a data transfer is OK ("BSY=0", "DRDY=1", and "DRQ=1")  */
	return ataChkSts(drv, stBSY | stDRDY | stDRQ, stDRDY | stDRQ);
}

/*
 *	Wait for a long time until it becomes ready-status (BUSY = 0)
 */
EXPORT	ER	ataWaitReady(DrvTab *drv)
{
	UW	tm, st;

	for (tm = 0;;) {
		st = ataStatusIn(drv);
		if ((st & stBSY) == 0) return E_OK;
		if (drv->Spec.pccard && st == 0xff) break;   /* There is no card */
		if (sdChkTmout(&tm, READY_TMO, 10)) break;
	}
	return ERR_CMDBUSY;
}

/*
 *	ATA (soft) reset
 */
EXPORT	void	ataReset(DrvTab *drv)
{
	UW	iob = drv->IOB;
	UW	reg, tm;
	BOOL	master;

	/* Set it to drive 0 */
	ataSetDrive(drv, 0);

	/* Check the existence of drive 0 (Master) */
	master = (InB(REG_DRVHEAD) == 0xff && InB(REG_STS) == 0xff) ?
							FALSE : TRUE;

	/* Reset pulse output */
	reg = (drv->d.pcc.IOConf[0] & IOC_IO2_NONE)? REG_DEVCTL : REG_DEVCTL2;
	OutB(reg, dcSRST);
	WaitUsec(20);				/* Wait for "20 usec" */
	OutB(reg, dcNORM);

	/* Wait until it becomes ready status:
			Firstly set it to drive 1 when there is no "Master" */
	for (tm = 0;;) {
		if (master == TRUE || ataSetDrive(drv, 0x100) == E_OK) {
			if ((ataStatusIn(drv) & stBSY) == 0) break;
		}
		if (drv->Spec.pccard && ataStatusIn(drv) == 0xff) break;
							/* There is no card */
		if (sdChkTmout(&tm, RESET_TMO, 10)) break;
	}
	/* Set Drive (Master & Slave) to the reset status */
	if (drv->Top != NULL) drv = drv->Top;
	for (; drv != NULL; drv = drv->Next) drv->Reset = TRUE;
}

/*
 *	ATA command output
 */
EXPORT	ER	ataCommandOut(DrvTab *drv, W cmd, W cnt, W sec, W cyl, W head)
{
	UW	iob = drv->IOB;

	/* Wait for a while until it becomes ready status (BSY = 0) */
	if (ataChkSts(drv, stBSY, 0) != E_OK) goto EEXIT;

	/* The drive/header number register setting */
	OutB(REG_DRVHEAD, drDRV(drv->DrvNo) | head);

	/* Wait for a while until It gets to "BSY = 0" and "DRDY = 1" */
	if (ataChkSts(drv, stBSY | stDRDY, stDRDY) != E_OK) goto EEXIT;

	/* Register setting of the number of sectors */
	if (cnt != NOSET) {
		OutB(REG_SECCNT, cnt);
	}

	/* Register setting of sector number */
	if (sec != NOSET) {
		OutB(REG_SECNO, sec);
	}

	/* Register setting of cylinder number */
	if (cyl != NOSET) {
		OutB(REG_CYL_L, cyl);
		OutB(REG_CYL_H, cyl >> 8);
	}

	/* Future register setting */
	if ((cmd & 0xFF00) != 0) OutB(REG_FEATURE, cmd >> 8);

	/*  Enable interrupt:
		Interrupt may be caused immediately after the command setting.
		Therefore, the interrupt shall be enabled at this point. However, the interrupt
		is not caused yet at the time of writing, so it shall be enabled later.*/

	if (cmd != ATA_WRITE && cmd != ATA_MWRITE) ENB_INT(drv);

	/* Command register setting */
	OutB(REG_CMD, cmd);
	return E_OK;

EEXIT:
	ataReset(drv);			/* ATA reset*/
	return ERR_CMDBUSY;
}

/*
 *	Data input : H unit
 */
EXPORT	void	ataDataIn(DrvTab *drv, UH *buf, W cnt)
{
	UW	iob = drv->IOB;
	UH	d;

	pcIO_16bits();	/* 16 bits I/O */
	while (--cnt >= 0) {d = InH(REG_DATA); *buf++ = CnvHIO(d);}
	pcIO_8bits();	/* 8 bits I/O */
}

/*
 *	Skip the data input : H unit
 */
EXPORT	void	ataDataSkip(DrvTab *drv, W cnt)
{
	UW	iob = drv->IOB;

	pcIO_16bits();	/* 16 bits I/O */
	while (--cnt >= 0) InH(REG_DATA);
	pcIO_8bits();	/* 8 bits I/O */
}

/*
 *	Data output : H unit
 *		Output the "0xFFFF" when "buf == NULL"
 */
EXPORT	void	ataDataOut(DrvTab *drv, UH *buf, W cnt)
{
	UW	iob = drv->IOB;

	pcIO_16bits();	/* 16 bits I/O */
	if (buf != NULL) {
		UH	d;
		while (--cnt >= 0) {d = *buf++;	OutH(REG_DATA, CnvHIO(d));}
	} else {
		while (--cnt >= 0) OutH(REG_DATA, 0xffff);
	}
	pcIO_8bits();	/* 8 bits I/O */
}
