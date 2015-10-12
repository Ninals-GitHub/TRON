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
 *	mem.c	System disk driver
 *
 *	Memory disk access processing
 */

#include "sdisk.h"

/*
 *	Abort processing
 */
EXPORT	void	memAbort(DrvTab *drv)
{
	/* Execute nothing because it does not enter a wait status in memory disk */
}

/*
 *	Reading/Writing of memory block
 */
LOCAL	ER	memRWblks(DrvTab *drv, W blk, W cnt, void *mptr, BOOL write)
{
	void*	madr;

	/* Convert to byte unit */
	blk *= drv->SecSize;
	cnt *= drv->SecSize;

	/* Writing/Reading to memory */
	madr = (void*)(drv->IOB + blk);	/* Address is logical address */
	if (write) {
		if (mptr != NULL) memcpy(madr, mptr, cnt);
		else		  memset(madr, 0, cnt);
	} else {
		memcpy(mptr, madr, cnt);
	}
	return E_OK;
}

/*
 *	Reading/Writing to disk block
 */
EXPORT	INT	memReadWrite(DrvTab *drv, W blk, W cnt, void *mptr, BOOL write)
{
	ER	er;

	/* The validity of  "blk" and "cnt" is already checked */

	/* Reading /Writing of memory block  */
	er = memRWblks(drv, blk, cnt, mptr, write);
	if (er >= E_OK) return cnt;
	return er;
}

/*
 *	Format processing
 */
EXPORT	ER	memFormat(DrvTab *drv, DiskFormat *fmt, W dcnt)
{
	ER	er;
	W	blk, cnt;

	if (*fmt == DiskFmt_MEMINIT) {

		if (dcnt < sizeof(DiskFormat) + sizeof(W) * 2)
			{er = E_PAR; goto E_EXIT;}

		fmt++;
		blk = ((W*)fmt)[0];	/* Block size	*/
		cnt = ((W*)fmt)[1];	/* Total number of blocks		*/
		if (cnt < 0 || blk < 512 || blk > 8192 || (blk % 512) != 0)
			{er = E_PAR; goto E_EXIT;}

		if (drv->IOB != 0) {
			Kfree((void*)drv->IOB);
			drv->IOB = 0;
		}
		er = E_OK;
		if (cnt > 0) {
			if ((drv->IOB = (UW)Kcalloc(blk, cnt)) == 0) {
				er = E_NOMEM;
				cnt = 0;
			}
		}
		drv->MiscPar = cnt * blk;
		drv->SecSize = blk;
		drv->nXSec = drv->nSec = (cnt < 0x10000) ? cnt : 0xffff;
		drv->s.SUnit[0].EndBlock = cnt;

	} else if (*fmt == DiskFmt_MEM) {
		/* Start block/The number of blocks */
		blk = drv->s.SUnit[/*drv->CurSUnit*/ 0].StartBlock;
		cnt = drv->s.SUnit[/*drv->CurSUnit*/ 0].EndBlock - blk;

		/* Initialize the number of valid units in the case of physical unit */
		/* if (drv->CurSUnit == 0) drv->nSUnit = 0; */

		/* Write 0 into memory block */
		er = memRWblks(drv, blk, cnt, NULL, TRUE);
	} else {
		er = E_PAR;
	}
	if (er >= E_OK) return E_OK;

E_EXIT:
	return er;
}

/*
 *	Reading of disk information
 */
EXPORT	ER	memIdentify(DrvTab *drv, BOOL check)
{
	W	sz;

	/* Media shall be always existent */
	drv->MediaOK = TRUE;

	if (check) return E_OK;

	sz = drv->MiscPar;			/* The total memory size	*/
	sz /= drv->SecSize;			/* The total number of sectors	*/

	/* Set the information */
	drv->nXSec = drv->nSec = (sz < 0x10000) ? sz : 0xffff;
	drv->nXHead = drv->nHead = 1;		/* Dummy	*/
	drv->nXCyl = drv->nCyl = 1;		/* Dummy	*/

	/* Set the physical unit */
	drv->s.SUnit[0].SystemId = DSID_NONE;
	drv->s.SUnit[0].StartBlock = 0;
	drv->s.SUnit[0].EndBlock = sz;		/* The total number of sectors	*/
	drv->s.SUnit[0].OpenCnt = 0;
	return E_OK;
}
/*
 *	Disk initialization
 */
EXPORT	ER	memDiskInit(DrvTab *drv)
{
	/* Subunit does not exist in memory disk */
	drv->nSUnit = 0;
	return E_OK;
}
/*
 *	Various processings
 */
EXPORT	ER	memMisc(DrvTab *drv, W cmd)
{
#if	0
	switch(cmd) {
	case DC_TSKINIT:	/* Task initialization processing    (Top drv)	*/
	case DC_TSKTMO:		/* Task time-out processing          (Top drv)	*/
	case DC_DRVCHG:		/* Drive change processing	     (Target drv)	*/
	case DC_OPEN:		/* Open processing		     (Target drv)	*/
	case DC_CLOSE:		/* Close processing		     (Target drv)	*/
	case DC_CLOSEALL:	/* Close processing	             (Target drv)	*/
	}
#endif
	return E_OK;
}
