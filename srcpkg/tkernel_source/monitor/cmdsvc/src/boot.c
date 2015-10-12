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
 *	boot.c
 *
 *       boot processing
 */

#include "cmdsvc.h"

/*
 * boot information
 *       Information passed to primary boot loader (PBOOT).
 *      This address (&bootinfo) is passed to the primary boot loader and is directly referenced.
 */
LOCAL BootInfo	bootInfo;

/*
 * Loading of the primary bootloader (PBOOT).
 *      return value     boot partition number
 */
LOCAL W loadPBoot( const UB *devnm, DISKCB **dcb_p )
{
	W	retry = 2;
	W	pno;
	DISKCB	*dcb;
	ER	err;

	while ( --retry >= 0 ) {

		/* opening of a disk device */
		pno = openDisk(devnm, &dcb);
		if ( pno < E_OK ) {
			err = pno;
			if ( err == E_IO ) continue; /* retry */
			return err;
		}

                /* If there is no partition specification in the device name, we assume the boot partition. */
		if ( pno == 0 ) pno = dcb->boot;

                /* read the boot block inside the target partiion */
		err = (*dcb->rwdisk)(dcb, dcb->part[pno].sblk, 1,
						PBootAddr, FALSE);
		if ( err < E_OK ) {
			if ( err == E_IO ) continue; /* retry */
			return err;
		}

                /* check the signature in the boot block */
		if ( *(UH*)(PBootAddr + 510) != BootSignature ) return E_BOOT;

		*dcb_p = dcb;
		return pno;
	}

	return E_IO;
}

/*
 * disk boot
 *       devnm   device name (possibly with the partition number)
 *               if it is NULL, the standard search order is used to look for a bootable device.
 *       return value error code
 */
EXPORT ER bootDisk( const UB *devnm )
{
	DISKCB	*dcb;
	W	pno, i, c;
	ER	err;

	if ( devnm != NULL ) {
                /* boot from the specified device */
		pno = loadPBoot(devnm, &dcb);
		if ( pno < E_OK ) return pno;
	} else {
                /* Boot using the standard boot order */
		pno = E_BOOT;
		for ( i = 0;; i++ ) {
			devnm = bootDevice(i);
			if ( devnm == NULL ) break; /* end is seen */

			pno = loadPBoot(devnm, &dcb);
			if ( pno >= 0 ) break;
		}
	}
	if ( pno >= 0 ) {
                /* Length of the device name without the partition number */
		i = strlen(devnm);
		c = devnm[i - 1];
		if ( i >= 2 && c >= '0' && c <= '3' ) --i;

                /* Set boot information */
		strncpy(bootInfo.devnm, devnm, L_DEVNM);
		bootInfo.devnm[i] = '\0'; /* erase partition number */
		bootInfo.part  = pno - 1;
		bootInfo.start = dcb->part[pno].sblk;
		bootInfo.secsz = dcb->blksz;

                /* prepare for primary boot execution */
		setUpBoot(PBootAddr, &bootInfo);

	} else {
                /* if boot from disk fails
                   try invoking ROM kernel */
		err = bootROM();
		if ( err < E_OK ) {
			if ( err != E_ABORT ) err = pno;
			return err;
		}
	}

	return E_OK;
}
