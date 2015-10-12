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
 *	memdsk.c
 *
 *       memory disk
 */

#include <tmonitor.h>
#include <tm/tmonitor.h>
#include <sys/rominfo.h>

/*
 * memory disk information
 *       Use the ROM disk information in ROM info as the starting point.
 */
typedef struct {
	UW	rd_type;	/* disk type */
	UW	rd_blksz;	/* disk block size */
	UW	rd_saddr;	/* disk start address */
	UW	rd_eaddr;	/* disk end address */
} MDINFO;

/*
 * disk types
 */
#define	ROMDISK		1

/* ------------------------------------------------------------------------ */

/*
 * submodule extended SVC function
 *       fno     function code
 *       p1-p3 parameter(s)
 *       er_p    returns error code
 *       return value      if the function is handled, returns TRUE.
 *               if not, returns FALSE
 */
EXPORT BOOL memDiskSVC( W fno, W p1, W p2, W p3, W *er_p )
{
	MDINFO	*mdi;
	UW	type;
	W	er, n, v;

	switch ( fno ) {
	  case TMEF_RDAINFO:	/* ROM disk information */
		mdi = (MDINFO*)&ROMInfo->rd_type;
		type = ROMDISK;
		goto info;

	  info:
		if ( mdi->rd_blksz > 0 && mdi->rd_type == type ) {
			v = (UW)mdi;
			n = writeMem(p1, &v, sizeof(W), sizeof(W));
			er = ( n == sizeof(W) )? E_OK: E_MACV;
		} else {
			er = E_NOEXS;
		}
		break;

	  default:
		return FALSE;	/* unsupported */
	}

	*er_p = er;
	return TRUE;
}

/* ------------------------------------------------------------------------ */

/*
 * I/O functions
 *       blk     start block number
 *               this is not a block number within a partition, but
 *               it is a disk-wide block number unique inside the whole disk.
 *       nblk    number of blocks
 *       buf     buffer (* )
 *       wrt     FALSE : read
 *               TRUE  : write
 *       return value error code
 *       argument marked with (* ) may be an address specified from external sources.
 */
LOCAL ER rwdisk( DISKCB *dcb, W blk, W nblk, void *buf, BOOL wrt )
{
	MDINFO	*mdi = (MDINFO*)dcb->info;
	W	sz, asz;
	void	*adr;

	if ( dcb->blksz <= 0 ) return E_NOEXS;  /* not yet initialized */

	adr = (void*)(mdi->rd_saddr + blk * dcb->blksz);
	sz = nblk * dcb->blksz;

	if ( wrt ) {
                /* write */
		if ( mdi->rd_type == ROMDISK ) return E_RONLY;
		asz = readMem((UW)buf, adr, sz, 1);
	} else {
                /* read */
		asz = writeMem((UW)buf, adr, sz, 1);
	}
	if ( asz < sz ) return E_IO;

	return E_OK;
}

/*
 * initialization processing
 *       disk drive that is supported by the initialization by CFGDISK
 *       disk drive is initialized and DISKCB is set up.
 *       DISKCB is given in 0-cleared state initially. Subsequently,
 *       DISKCB returned in the previous call is passed.
 *       I/O function receives this DISKCB.
 *
 *       In principle, this function is called every time an I/O processing is performed.
 *       Hence, there is no need to perform hardware initialization on the second call and afterward.
 *       but whether the hardware status remains as it was the last time the initialization took place is not guaranteed,
 *     so,
 *      it is desirable to initialize hardware from time to as necessary.
 *     If we perform re-initialization, DISKCB is to be re-initialized.
 */
EXPORT ER initMemDisk( DISKCB *dcb, const CFGDISK *cfg )
{
	MDINFO	*mdi;
	UW	type;

	if ( dcb->blksz > 0 ) return E_OK;  /* already initialized  */

        /* select the target disk */
	switch ( cfg->info ) {
	  case 0:	/* ROM disk */
		mdi = (MDINFO*)&ROMInfo->rd_type;
		type = ROMDISK;
		break;

	  default:
		return E_PAR;
	}

	if ( mdi->rd_blksz <= 0 || mdi->rd_type != type ) return E_NOEXS;

        /* set up disk drive control block (DISKCB) */
	memset(dcb, 0, sizeof(DISKCB));
	dcb->info         = (UW)mdi;
	dcb->blksz        = mdi->rd_blksz;
	dcb->part[0].sblk = 0;
	dcb->part[0].nblk = (mdi->rd_eaddr - mdi->rd_saddr) / mdi->rd_blksz;
	dcb->rwdisk       = rwdisk;

	return E_OK;
}
