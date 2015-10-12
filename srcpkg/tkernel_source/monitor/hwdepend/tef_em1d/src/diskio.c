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
 *	diskio.c
 *
 *       Disk I/O
 */

#include "hwdepend.h"
#include <device/disk.h>

/*
 * misaligned data is read as a little-endian data
 */
#define	GMW(vp)		( (UW)((UB*)vp)[0]       \
			| (UW)((UB*)vp)[1] << 8  \
			| (UW)((UB*)vp)[2] << 16 \
			| (UW)((UB*)vp)[3] << 24 )
#define	GMH(vp)		( (UH)((UB*)vp)[0]       \
			| (UH)((UB*)vp)[1] << 8  )

/*
 * control block for a disk drive
 *       When their number is smaller than the number of disks,
 *       already used block is re-used in place of missing block after it is cleared
 */
#define	N_DISKCB	3
LOCAL struct dcblist {
	const CFGDISK	*cfg;	/* target device */
	DISKCB		dcb;
} dcbList[N_DISKCB];

LOCAL W	last_dcb = 0;	/* the last allocated number from dcbList */

/*
 * allocating a disk control block
 */
LOCAL DISKCB* getDISKCB( const CFGDISK *cfg )
{
	W	i, n;
	DISKCB	*dcb;

        /* searching for disk drive control block */
	n = last_dcb;
	do {
		i = n;
		if ( dcbList[i].cfg == cfg ) break;
		if ( ++n >= N_DISKCB ) n = 0;
	} while ( n != last_dcb );
	dcb = &dcbList[i].dcb;
	last_dcb = i;
	if ( dcbList[i].cfg != cfg ) {
                /* re-use after clear */
		memset(dcb, 0, sizeof(DISKCB));
		dcbList[i].cfg = cfg;
	}

	return dcb;
}

/*
 * search for a device
 *       return the configuration information of device (indicated by devnm) to cfg_p.
 *       return value is a partition number or error.
 */
LOCAL W searchDevice( const UB *devnm, const CFGDISK **cfg_p )
{
	UB	name[L_DEVNM + 1];
	W	i, pno, c;

        /* checking device name */
	strncpy(name, devnm, L_DEVNM + 1);
	if ( name[L_DEVNM] != '\0' ) return E_PAR;
	i = strlen(name);
	if ( i <= 0 ) return E_PAR;

        /* check for logical device (partition: 0-3) */
	pno = 0;
	c = name[i - 1];
	if ( c >= '0' && c <='3' ) {
		if ( --i <= 0 ) return E_PAR;
		name[i] = '\0';		/* partition number is removed */
		pno = c - '0' + 1;	/* partition number (1 - ) */
	}

        /* search for a device */
	for ( i = 0; i < N_ConfigDisk; i++ ) {
		if ( strncmp(name, ConfigDisk[i].name, L_DEVNM) == 0 ) break;
	}
	if ( i >= N_ConfigDisk ) return E_NOEXS;

	*cfg_p = &ConfigDisk[i];
	return pno;
}

/*
 * obtain partition information
 */
LOCAL ER readPart( DISKCB *dcb )
{
	DiskBlock0	buf;
	W		i, pno;
	ER		err;

	dcb->boot = 0;

        /* if an unexpected disk block size is seen, raise an error */
	if ( dcb->blksz != sizeof(buf) ) return E_NOSPT;

        /* read master boot record */
	err = (*dcb->rwdisk)(dcb, 0, 1, &buf, FALSE);
	if ( err < E_OK ) return err;

        /* check the signature in the boot block */
	if ( GMH(&buf.signature) != 0xaa55 ) return E_OK; /* no partition */

        /* obtain partition information */
	for ( i = 0; i < MAX_PARTITION; i++ ) {
		pno = i + 1;
		dcb->part[pno].sblk = GMW(buf.part[i].StartBlock);
		dcb->part[pno].nblk = GMW(buf.part[i].BlockCnt);

		if ( buf.part[i].BootInd == 0x80
		  && dcb->part[pno].nblk > 0
		  && dcb->boot == 0 ) dcb->boot = pno;
	}

	return E_OK;
}

/*
 * open disk (obtain disk drive control block)
 *       open a device indicated by `devnm', and return the disk information in `dcb'.
 *       devnm can specify a device name with partition number.
 *       return the partition number specified by devnm.
 *       return value     0   : entire disks
 *               1 - : partition number
 *               < 0 : error
 */
EXPORT W openDisk( const UB *devnm, DISKCB **dcb_p )
{
	const CFGDISK	*cfg;
	DISKCB		*dcb;
	W		pno;
	ER		err;

        /* search for a device */
	pno = searchDevice(devnm, &cfg);
	if ( pno < E_OK ) return pno;

        /* allocating a disk control block */
	dcb = getDISKCB(cfg);

        /* initialize disk */
	err = (*cfg->initdisk)(dcb, cfg);
	if ( err < E_OK ) return err;
	if ( dcb->blksz == 0 ) return E_NOMDA;

	if ( dcb->boot == 0xff ) {
                /* read the partition information */
		err = readPart(dcb);
		if ( err < E_OK ) return err;
	}

	*dcb_p = dcb;
	return pno;
}

/*
 * disk access
 *       devnm   device name (possibly with the partition number)
 *       blk     start block number
 *               if device name has a partition number, then the block number in that partition
 *               if there is no partition number in the disk anme, the block number in the entire disk
 *       nblk    number of blocks
 *       buf     buffer (* )
 *       wrt     FALSE : read
 *               TRUE  : write
 *       return value error code
 *       argument marked with (* ) may be an address specified from external sources.
 */
EXPORT ER rwDisk( const UB *devnm, W blk, W nblk, void *buf, BOOL wrt )
{
	DISKCB	*dcb;
	W	pno, nb;
	ER	err;

        /* initialize disk */
	pno = openDisk(devnm, &dcb);
	if ( pno < E_OK ) return pno;

	nb = dcb->part[pno].nblk;

        /* range check of the block number */
	if ( blk < 0 || blk >= nb
	  || nblk <= 0 || nblk > nb - blk ) return E_PAR;

        /* convert the relative block number in a partiton to a block number in the entire disk */
	blk += dcb->part[pno].sblk;

        /* read from, or write to disk */
	err = (*dcb->rwdisk)(dcb, blk, nblk, buf, wrt);
	if ( err < E_OK ) return err;

	return E_OK;
}

/*
 * obtain disk information
 *       devnm   device name (possibly with the partition number)
 *       blksz   return block size (* )
 *       tblks   return the number of all blocks (* )
 *       return value error code
 *       if there is a partition number to the device name, return the specific information to that partition.
 *       argument marked with (* ) may be an address specified from external sources.
 */
EXPORT ER infoDisk( const UB *devnm, W *blksz, W *tblks )
{
	DISKCB	*dcb;
	W	pno, buf, n;

        /* initialize disk */
	pno = openDisk(devnm, &dcb);
	if ( pno < E_OK ) return pno;

        /* obtain disk information
         *       since there are addresses specified from external source, use writeMem()
	 */
	buf = dcb->part[pno].nblk;
	n = writeMem((UW)tblks, &buf, sizeof(W), sizeof(W));
	if ( n < sizeof(W) ) return E_MACV;

	buf = dcb->blksz;
	n = writeMem((UW)blksz, &buf, sizeof(W), sizeof(W));
	if ( n < sizeof(W) ) return E_MACV;

	return E_OK;
}
