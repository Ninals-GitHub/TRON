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
 *	device.h
 *
 *       T-Monitor device-related definitions
 *
 *       since machines support different devices, functions defined here
 *       may not be usable all the time.
 *       Available functions differ from one system to the other.
 */

#ifndef __MONITOR_DEVICE_H__
#define __MONITOR_DEVICE_H__

/* ------------------------------------------------------------------------ */
/*
 *       serial port
 */

/*
 * initialize serial port
 *       port    console port number (0 - )
 *              when it is -1, it means there is no console.
 *       speed   communication speed (bps)
 */
IMPORT ER initSIO( W port, W speed );

/*
 * serial port output (one character transmission)
 */
IMPORT void putSIO( UB c );

/*
 * serial port input (one character reception)
 *       tmo     timeout (milliseconds)
 *              You can not wait forever.
 *       return value       >= 0 : character code
 *                 -1 : timeout
 *       input data using buffer.
 *       receive error is ignored.
 */
IMPORT W getSIO( W tmo );

#define	SIO_RCVBUFSZ	64		/* receive buffer size */
#define	SIO_PTRMSK	(SIO_RCVBUFSZ - 1)

/*
 * control block for a serial port
 */
typedef struct siocb	SIOCB;
struct siocb {
	UW	info;			/* driver can use this for arbitrary purposes */
	UB	rcvbuf[SIO_RCVBUFSZ];	/* receive buffer */
	UW	iptr;			/* input pointer */
	UW	optr;			/* output pointer */

	/*
         * I/O functions
         *       same specification as putSIO(), and getSIO()
	 */
	void (*put)( SIOCB*, UB c );	/* send a character */
	W    (*get)( SIOCB*, W tmo );	/* receive a character */
};

/*
 * serial port configuration information
 */
typedef struct cfgsio	CFGSIO;
struct cfgsio {
	/*
         * initialization processing
         *       serial port that is supported by the initialization of CFGSIO
         *       speed   communication speed (bps)
         *       initialize the serial port according to the specified parameters and set SIOCB
         *       SIOCB is given in 0-cleared state initially.
         *       Subsequent I/O operations uses the SIOCB.
	 */
	ER (*initsio)( SIOCB*, const CFGSIO*, W speed );

	UW	info;			/* additional information (driver-dependent) */
};

IMPORT W	ConPort;	/* console port number */
IMPORT UW	ConPortBps;	/* console port communication speed (bps) */

/* ------------------------------------------------------------------------ */
/*
 *	disk drive
 */

#define	L_DEVNM		8		/* device name length */
#define	N_PARTITION	4		/* number of partitions */

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
IMPORT ER rwDisk( const UB *devnm, W blk, W nblk, void *buf, BOOL wrt );

/*
 * obtain disk information
 *       devnm   device name (possibly with the partition number)
 *       blksz   return block size (* )
 *       tblks   return the number of all blocks (* )
 *       return value error code
 *       if there is a partition number to the device name, return the specific information to that partition.
 *       argument marked with (* ) may be an address specified from external sources.
 */
IMPORT ER infoDisk( const UB *devnm, W *blksz, W *tblks );

/*
 * control block for a disk drive
 *       During the first initialization of the disk driver, please do the following:
 *       assume the first initialization if blksz == 0.
 *       blksz   block size
 *       boot    0xff if there is a partition
 *               0 if there is no partition
 *       part[0] the starting block number of a whole disk and number of blocks
 *       rwdisk  I/O processing function
 *
 *      after the disk driver initialization, boot and part[1 - (and up) ] are
 *       set by upper level software.
 */
typedef struct diskcb	DISKCB;
struct diskcb {
	UW	info;			/* driver can use this for arbitrary purposes */
	UW	blksz;			/* block size (in bytes) */
	UB	boot;			/* boot partition number (0: no partition) */
	struct partition {		/* partition information */
		UW	sblk;		/* start block number */
		UW	nblk;		/* number of blocks */
	} part[1 + N_PARTITION];	/* [0]:whole, [1- (and up)]: each partition */

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
	ER (*rwdisk)( DISKCB*, W blk, W nblk, void *buf, BOOL wrt );
};

/*
 * disk drive configuration information
 */
typedef struct cfgdisk	CFGDISK;
struct cfgdisk {
	UB	name[L_DEVNM];		/* device name */
	UW	attr;			/* disk drive attribute */

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
	ER (*initdisk)( DISKCB*, const CFGDISK* );

	UW	info;			/* additional information (driver-dependent) */
};

/* disk drive attribute */
#define	DA_RONLY	0x0001		/* read-only */

/*
 * open disk (obtain disk drive control block)
 *       open a device indicated by `devnm', and return the disk information in `dcb'.
 *       devnm can specify a device name with partition number.
 *       return the partition number specified by devnm.
 *       return value     0   : entire disks
 *               1 - : partition number
 *               < 0 : error
 *       usually openDisk() is internally called implicitly, and you don't have to call it explicitly.
 *       it is provided here so that you can call it independently
 */
IMPORT W openDisk( const UB *devnm, DISKCB **dcb );

/* ------------------------------------------------------------------------ */
/*
 *	Flash ROM
 */

IMPORT const UW	FROM_SECSZ;	/* Flash ROM sector size (bytes) */

/*
 * Flash ROM sector erase / write
 *       addr    Flash ROM write start address (must be on a sector boundary)
 *       data    write data start address (RAM)
 *       nsec    number of sectors to write
 *       msg      0 : no message display  no verify write
 *                1 : message display    with verify write
 *               -1 : message display no verify write
 *       return value error code
 */
IMPORT ER writeFrom( UW addr, UW data, W nsec, W msg );

/*
 * set up Flash ROM loading processing
 *       mode     0 : set up for loading write data
 *               -1 : set up for writing already loaded data
 *
 *       in the case of setting up loading (mode= 0)
 *         addr returns the following value.
 *           addr[0]  the start address in RAM area for loading data
 *           addr[1]  the end address in RAM area for loading data
 *               addr[1] - addr[0] + 1 = load area size
 *               in principle, load area size matches the size of FLASH ROM.
 *               But if RAM is small, there may be cases
 *               in which load area size is smaller than that of Flash ROM size.
 *           addr[2]  the distance between the data load RAM area and Flash ROM area
 *               adjustment is made so that the addr[0] position is written to the beginning of Flash ROM.
 *               addr[2] = addr[0] - Flash ROM start address
 *
 *       in the case of setting up for writing (mode = -1),
 *         we set the writing area based on the addr value when we called this function using mode = 0.
 *           addr[0]  starting address of loaded data in RAM area (to be written)
 *            addr[1]  ending address of loaded data in RAM area (to be written)
 *               addr[1] - addr[0] + 1 = size of written data
 *           addr[2]  the value remains the same after it was set by mode = 0 (ignored)
 *         the modified values are returned in addr.
 *           addr[0]  Flash ROM write start address
 *           addr[1]  start address of write data in RAM
 *               address will be adjusted to the sector boundary of Flash ROM.
 *           addr[2]  number of sectors to write
 *               Since writing is done in the unit of sectors, the writing will be done from the sector boundary,
 *               areas immediately before and after the designated area may be part of the write operation.
 */
IMPORT void setupFlashLoad( W mode, UW addr[3] );

/* ------------------------------------------------------------------------ */
/*
 *	PCMCIA
 */

/*
 * initialize slot
 */
IMPORT void initPcSlot( void );

/* ------------------------------------------------------------------------ */
/*
 *       PCI device
 */

/*
 * initialize PCI
 */
IMPORT void initPCI( void );

/*
 * submodule extended SVC function
 *       fno     function code
 *       p1-p3 parameter(s)
 *       er_p    returns error code
 *       return value      if the function is handled, returns TRUE.
 *               if not, returns FALSE
 */
IMPORT BOOL pciSVC( W fno, W p1, W p2, W p3, W *er_p );

/* ------------------------------------------------------------------------ */
/*
 *       memory disk
 */

/*
 * initialization processing
 */
IMPORT ER initMemDisk( DISKCB *dcb, const CFGDISK *cfg );

/*
 * submodule extended SVC function
 *       fno     function code
 *       p1-p3 parameter(s)
 *       er_p    returns error code
 *       return value      if the function is handled, returns TRUE.
 *               if not, returns FALSE
 */
IMPORT BOOL memDiskSVC( W fno, W p1, W p2, W p3, W *er_p );

/*
 * exclude area used for RAM disk
 *       top     the start of RAM area (the lower address)
 *       end     the end of RAM area (upper area)
 *       excluding the area used for RAM disk from the upper end of RAM area,
 *       return the address (the end of RAM area) as return value.
 */
IMPORT UW omitRAMDiskArea( UW top, UW end );

/* ------------------------------------------------------------------------ */
/*
 *       display
 */

/*
 * display power off message
 */
IMPORT void DispPowerOff( void );

/*
 * display the progression (two hexadecimal digits) on the screen
 *       monitor uses n = 0x01 - 0x0f.
 *       OS uses values 0x10 and above, and 0x00 means the booting has completed.
 */
IMPORT void DispProgress( W n );

/* ------------------------------------------------------------------------ */

#endif /* __MONITOR_DEVICE_H__ */
