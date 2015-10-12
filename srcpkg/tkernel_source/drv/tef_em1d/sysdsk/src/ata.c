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
 *	ata.c		System disk driver
 *
 *	ATA Disk access processing
 */

#include "sdisk.h"
#include "ata.h"

/*
 *	Drive initialization
 */
LOCAL	void	ataDriveInit(DrvTab *drv)
{
	ER	er;

	/* Since power has been just turned ON, wait for a while until it become ready status  */
	ataWaitReady(drv);

	/* Issue "SET MULTIPLE" command when "MULTIPLE" command is valid. */
	if (drv->MultiCnt >= 2) {
		/* Output the "SET MULTIPLE" command */
		er = ataCmd(drv, ATA_SETMULTI, 0, 0, NULL);
		if (er < E_OK) drv->MultiCnt = 0;
	}

	/* Release the drive's reset status */
	drv->Reset = FALSE;
}

/*
 *	The actual read-in/writing of disk block
 *		"buf == NULL" at the time of "FORMAT"
 *		return	The number of processed blocks
 */
LOCAL	INT	ataRWblks(DrvTab *drv, W blk, W cnt, void *buf, W cmd)
{
	ER	er;
	W	nblks, nsec, maxlen;

#define	WRKBUFSZ	(64 * 1024)

	/* Read-in/Write every "MAX_IOSEC" for the number of the requested blocks */

	ataSelDrv(drv);		/* Wait-for-interrupt setting */

	/* Reinitialize drive : response to the power-OFF status */
	if (drv->Reset) ataDriveInit(drv);

	maxlen = MAX_IOSEC * ATA_SECSZ;

	/* The maximum number of sectors at one time transfer */
	maxlen /= drv->SecSize;
	er = E_OK;

	for (nblks = 0; cnt > 0; cnt -= nsec) {
		/* The number of processing request sectors */
		if ((nsec = cnt) > maxlen) nsec = maxlen;

		er = ataCmd(drv, cmd, blk, nsec, buf);
		if (er < E_OK) break;

		nblks += nsec;
		blk += nsec;
		if (buf != NULL) buf += nsec * drv->SecSize;
	}
	return (er < E_OK) ? er  : nblks;
}

/*
 *	The read-in/writing of the disk block
 */
EXPORT	INT	ataReadWrite(DrvTab *drv, W blk, W cnt, void *mptr, BOOL write)
{
	/* The validity of "blk" and "cnt" is already checked (cnt > 0) */

	/* The actual Reading/writing of the disk block */
	return ataRWblks(drv, blk, cnt, mptr, (write) ? ATA_WRITE : ATA_READ);
}

/*
 *	Format processing
 *		ATA's format commander is vendor definition, and
 *		the refraining from using is recommended.
 *		Therefore, specific data shall be written in format processing
 */
EXPORT	ER	ataFormat(DrvTab *drv, DiskFormat *fmt, W dcnt)
{
	ER	er;
	W	blk, cnt, nfm;

	nfm = *fmt;
	if (nfm != DiskFmt_STD) return E_PAR;

	/* Start block & The number of the blocks */
	blk = drv->s.SUnit[drv->CurSUnit].StartBlock;
	cnt = drv->s.SUnit[drv->CurSUnit].EndBlock - blk;
	if (cnt <= 0) return E_PAR;

	/* Initialize the number of the valid units in the case of physical unit */
	if (drv->CurSUnit == 0) drv->nSUnit = 0;

	/* Actual writing of the disk block */
	er = ataRWblks(drv, blk, cnt, NULL, ATA_WRITE);
	return (er >= E_OK) ? E_OK : er;
}

/*
 *	Set the CHS information to be notified to external :
 *		Calculate CHS for "BIOS"setting by "Phoenix method"
 *		(Increase the number of heads by "2 ** n" times so that "the number of cylinders <= 1024" is available)
 */
EXPORT	W	ataSetXCHS(DrvTab *drv)
{
	W	xt, xc, xh, xs;

	/* Set the actual "IDE" */
	xc = drv->nCyl;
	xh = drv->nHead;
	xs = drv->nSec;
	xt = xc * xh * xs;

	/* Adjust the number of heads so that "the number of cylinders <= 1024" is available */
	while (xc > 1024) {xc >>= 1; xh <<= 1;}

	if (xs > 63) xs = 63;			/* The number of sectors <= 63	*/
	if (xh > 255) xh = 255;			/* The number of heads <= 255 	*/
	if (xt != 0) xc = xt / xh / xs;
	if (xc > 1024) xc = 1024;		/* The number of cylinders <= 1024 	*/
	if (xc > 0) xc--;			/* The last cylinder is unused	*/

	/* CHS information to be notified to external */
	drv->nXSec = xs;
	drv->nXHead = xh;
	drv->nXCyl = xc;
	return xc * xh * xs;
}

/*
 *	Reading of disk information
 */
EXPORT	ER	ataIdentify(DrvTab *drv, BOOL check)
{
	ER	er;
	W	cyl, head, sec, tsec, n, i;
	BOOL	uselba;
	UH	buf[ATA_SECSZ / sizeof(UH)];

	/* Set drive */
	for (n = 0; (er = ataSetDrive(drv, -1)) != E_OK; n++) {
		/* Ignore errors because it might not be the ready-status yet
				when PC card is inserted */
		if (drv->Spec.pccard) break;

		/* Reset and re-try it when it is not PC card */
		if (n != 0) goto E_EXIT;
		ataReset(drv);
	}

	/* Since power has been just turned ON, wait for a while until it become ready status */
	if ((er = ataWaitReady(drv)) != E_OK) goto E_EXIT;

	/* Wait-for-interrupt setting */
	ataSelDrv(drv);

	/* "IDENTIFY" command output */
	for (;;) {
		er = ataCmd(drv, ATA_IDENTIFY, 0, 1, (void*)buf);
		if (er >= E_OK) break;
		goto E_EXIT;
	}

	/* Move the necessary data that follows "IDENT_DTSZ"   */
	buf[2] = buf[49];
	buf[4] = buf[80];
	buf[5] = CnvLeH(buf[53]) & 0x00FF;	/* Upper level is the internal flag */
	buf[7] = buf[54];
	buf[8] = buf[55];
	buf[9] = buf[56];
	buf[20] = buf[60];
	buf[21] = buf[61];
	buf[22] = 0;				/* DMA No Support */
	buf[127] = 0;				/* MSN No Support */
	buf[5] = CnvLeH(buf[5]);

	tsec = 0;
	drv->DiskFmt = DiskFmt_STD;
	drv->SecSize = ATA_SECSZ;
	drv->SecBias = 0;

	/* Set the number of sectors for "R/W- MULTIPLE" : any of  0,2,4,8,16,and 32 */
	n = CnvLeH(buf[47]) & 0xFF;		/* Maximum value */
	for (i = 32; i > n; i >>= 1);
	drv->MultiCnt = (i > 1) ? i : 0;
	drv->DiskFmt = DiskFmt_STD;

	/* Initialize the drive */
	drv->MediaOK = TRUE;
	ataDriveInit(drv);

	if (check) {	/* Check whether it is same as before or not :
		Compare the absence or presence of media, write-protect, and ID data */
		if (!drv->MediaOK ||
		    drv->Wprotect != drv->Spec.readonly ||
		    memcmp((void*)drv->Ident, (void*)&buf[0], IDENT_DTSZ) != 0) {
			er = ERR_NOTSAME;
			goto E_EXIT;
		}
		return E_OK;
	}

	/* Save ID data */
	memcpy((void*)drv->Ident, (void*)&buf[0], IDENT_DTSZ);

	/* Set write-protect */
	if (!drv->Spec.wlock) drv->Spec.readonly = drv->Wprotect;

	sec = head = cyl = 0;

	if (!drv->MediaOK) {		/* There is no media	*/
		tsec = 0		;
		uselba = FALSE;		/* It is meaninless, however... */

	} else {			/* ATA device*/
		/* Fetch the information of "Cyl", "Head", and "Sec"  */
		cyl  = CnvLeH(buf[1]);			/* The number of cylinders	*/
		head = CnvLeH(buf[3]);			/* The number of heads	*/
		sec = CnvLeH(buf[6]);			/* The number of sectors */
		tsec = cyl * head * sec;		/* The total number of sectors	*/

		n = CnvLeH(buf[80]);
		if ((n & ~0x7FFE) == 0 && n >= (1 << 3)) {
			uselba = TRUE;		/* ATA-3 or more : LBA OK */
		} else {
			uselba = (CnvLeH(buf[49]) & 0x200) ? TRUE : FALSE;
		}

		if (uselba) {
			/* When LBA is used, checking shall be required on
		whether "Total Number of Sectors on LBA [60][61]" is correctly set or not */
			n = CnvLeW(*((UW*)&buf[60]));
			if (tsec == n || tsec ==
				(((n >> 16) & 0xFFFF)|((n & 0xFFFF) << 16))) {
				;	/* OK */
			} else if (n < tsec) {
				uselba = FALSE;		/* Illegal */

			} else if (tsec > 1032192) {
				tsec = n;	/* Actual total number of sectors */
				cyl = n / head / sec;	/* The actual number of "Cyl"s */
				if (cyl > 32767) cyl = 32767;

			} else if (tsec == 0 && n > 16514064) {
				tsec = n;	/* The actual number of sectors */
				/* Set "CHS" as a matter of convenience though it is unsupported */
				cyl = 16383; head = 15; sec = 63;
			}
		}

		/* Use "CHS" when "LBA" is unused (translation mode) */
		if (!uselba && (CnvLeH(buf[53]) & 0x1) != 0) {
			cyl = CnvLeH(buf[54]);		/* The number of the current cylinders	*/
			head = CnvLeH(buf[55]);		/* The number of the current heads	*/
			sec = CnvLeH(buf[56]);		/* The number of the current sectors	*/
			tsec = cyl * head * sec;	/* The total number of sectors	*/
		}
	}

	drv->UseLBA = uselba;			/* Use "LBA"	*/

	/* Set "CHS" information to be used internally */
	drv->nSec = sec;			/* The number of sectors / head */
	drv->nHead = head;			/* The number of heads / cylinder */
	drv->nCyl = cyl;			/* The number of cylinders		*/

	/* Set physical unit information */
	drv->s.SUnit[0].SystemId = DSID_NONE;
	drv->s.SUnit[0].StartBlock = 0;
	drv->s.SUnit[0].EndBlock = tsec;
	drv->s.SUnit[0].OpenCnt = 0;

	/* Set "CHS" information (tentative) to be notified to outside  */
	tsec = ataSetXCHS(drv);
	drv->SetXCHS = FALSE;

	if (drv->MediaOK) {
		if (drv->s.SUnit[0].EndBlock <= 0 || tsec <= 0) {
			er = ERR_NOBLK;
			goto E_EXIT;
		}
	}
	return E_OK;

E_EXIT:
	/* Set it to drive 0 to make sure */
	ataSetDrive(drv, 0);
	return er;
}

/*
 *	Set the partition information (MBR)
 */
EXPORT	ER	ataSetupPart(DrvTab *drv)
{
	ER	er;
	W	i, n, k;
	UW	sblk, eblk;
	UB	buf[ATA_SECSZ];
	PartTab	*part;
	W	xCyl, xSec, xHead;

	/* Confirm the sector size */
	/* if (drv->SecSize != ATA_SECSZ) return ERR_MEDIA; */

	/* The number of the sectors and heads in BIOS setting */
	xSec = xHead = 0;

	/* Reading of master boot block */
	if ((er = (*drv->ReadWrite)(drv, 0, 1, (void*)buf, FALSE))
							< E_OK) return er;

	/* Check the validity of boot block */
	if (*((UH*)&buf[OFS_SIGN]) != VALID_SIGN) {

		/* Consider it as the unused partition table */
		memset((void*)buf, 0, SIZE_PARTTAB);

	} else {
		/* Move the partition table to adjust to the word align */
		memmove((void*)buf, (void*)&buf[OFS_PART], SIZE_PARTTAB);
	}

	part = (PartTab*)buf;

	/* Set the unit information according to partition information:
			Simply register in order of partition information */

	for (i = n = 0; i < N_PART; i++) {
		sblk = part[i].StartBlock;
		sblk = CnvLeW(sblk);
		eblk = part[i].BlockCnt;
		eblk = sblk + CnvLeW(eblk);

		if (part[i].SysInd == DSID_NONE) {
INVALID_PART:
			/* Register it as the unused partition  */
			if (n < MAX_PART) {
				drv->s.SUnit[++n].SystemId = DSID_NONE;
				drv->s.SUnit[n].StartBlock = 0;
				drv->s.SUnit[n].EndBlock = 0;
				drv->s.SUnit[n].OpenCnt = 0;
			}
			continue;
		}

		/* Check whether it is illegal or not */
		if (sblk >= eblk || eblk > drv->s.SUnit[0].EndBlock) {
			goto INVALID_PART;
		}

		/* Check the duplication of the already registered partition */
		for (k = n; k > 0; k--) {
			if (sblk < drv->s.SUnit[k].EndBlock &&
			    eblk > drv->s.SUnit[k].StartBlock) break;
		}
		if (k > 0) {	/* There is duplication */
			goto INVALID_PART;
		}
		/* There is no duplication: Register */
		if (n < MAX_PART) {
			drv->s.SUnit[++n].SystemId = part[i].SysInd;
			drv->s.SUnit[n].StartBlock = sblk;
			drv->s.SUnit[n].EndBlock = eblk;
			drv->s.SUnit[n].OpenCnt = 0;

			/* Seek the number of sectors and heads in "BIOS" setting
		from the partition setting (cylinder align) */
			if (xSec == 0) {
				xSec = part[i].EndSec & 0x3F;
				xHead = (UH)part[i].EndHead + 1;
				xCyl = (UH)part[i].EndCyl + 1 +
					(((UH)part[i].EndSec & 0xC0) << 2);
				if ((k = xSec * xHead) != 0) {
					k = ((eblk % k) != 0) ? 0 : eblk / k;
				}
				if (xCyl != 1024 && xCyl != k && xCyl != k - 1)
					xSec = -1;	/* Illegal */
			} else if (xSec > 0) {
				if (xSec != (part[i].EndSec & 0x3F) ||
				    xHead != ((UH)part[i].EndHead + 1))
					xSec = -1;	/* Illegal */
			}
		}
	}
	/* Set the number of valid units */
	drv->nSUnit = n;

	/* Reset the CHS information to be notified to outside by prioritizing the partition setting information */
	if (!drv->SetXCHS) {
		if (xSec > 0) {
			drv->nXSec = xSec;
			drv->nXHead = xHead;
			drv->nXCyl = (drv->nCyl * drv->nHead * drv->nSec) /
				xHead / xSec - 1;  /* Last cylinder is unused */
			if (drv->nXCyl >= 1024) drv->nXCyl = 1023;
		}
		drv->SetXCHS = TRUE;
	}
	return E_OK;
}

/*
 *	Disk initialization (set the partition information)
 */
EXPORT	ER	ataDiskInit(DrvTab *drv)
{
	ER	er;

	/* Initialize the number of valid units */
	drv->nSUnit = 0;
	drv->CurSUnit = 0;

	/* "DISKMBR" is unsupported */
	drv->SuptMBR = FALSE;

	if (drv->Aborted) {
		/* No processing shall be executed when there is already an abort request :
			Response to the case where format is aborted. */
		er = ERR_ABORT;

	} else if (!drv->MediaOK) {
		er = ERR_NOMEDIA;	/* There is no media */

	} else if (drv->SecSize != ATA_SECSZ) {
		er = ERR_BLKSZ;		/* Illegal block size */

	} else {			/* HD */
		/* Set the partition information (MBR)  */
		er = ataSetupPart(drv);
		if (er == E_OK) drv->SuptMBR = TRUE;	/* "DISKMBR" support*/
	}
	if (er < E_OK) goto E_EXIT;

	return E_OK;

E_EXIT:
	return er;
}

/*
 *	The open/close processings
 */
LOCAL	void	ataOpenClose(DrvTab *drv, W cmd)
{
	switch(cmd) {
	case DC_OPEN:		/* Open processing */
		/* Check the injectable media in the case of physical unit */
		break;

	case DC_CLOSE:		/* Close processing */
	case DC_CLOSEALL:
		ataSelDrv(drv);		/* Wait-for-interrupt setting	*/
	}
}

/*
 *	Various processings
 */
EXPORT	ER	ataMisc(DrvTab *drv, W cmd)
{
	switch(cmd) {
	case DC_TSKINIT:	/* Task initialization processing(Top "drv")	*/
	case DC_TSKTMO:		/* Task time-out processing      (Top "drv")	*/
	case DC_DRVCHG:		/* Drive change processing       (Top "drv")	*/
		break;

	case DC_OPEN:		/* Open processing		 (Top "drv")	*/
	case DC_CLOSE:		/* Close processing		 (Top "drv")	*/
	case DC_CLOSEALL:	/* Close processing		 (Top "drv")	*/
		ataOpenClose(drv, cmd);
		break;
	}

	drv->ReqTmout = TMO_FEVR;
	return E_OK;
}
