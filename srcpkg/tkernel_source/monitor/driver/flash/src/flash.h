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
 *	flash.h
 *
 *       Flash ROM write processing
 */

#include <tmonitor.h>

/* ------------------------------------------------------------------------ */

/*
 * JEDEC Flash ROM specification
 *       bsec, and tsec show the sector configuration of boot block.
 *       Ordinary sectors are divided into 8KB units, and
 *       lower address is mapped to MSB side (this is the bitwise mapping ) The leading bit of a sector is 1, and
 *       if this is a continued part of a sector, then it is 0.
 *
 *                             range
 *		bit15	0x00000 - 0x01fff
 *		bit14	0x02000 - 0x03fff
 *		bit13	0x04000 - 0x05fff
 *		bit12	0x06000 - 0x07fff
 *		  :	   :         :
 *		bit3	0x18000 - 0x19fff
 *		bit2	0x1a000 - 0x1bfff
 *		bit1	0x1c000 - 0x1dfff
 *		bit0	0x1e000 - 0x1ffff
 *
 *       example: if sector size if 64KB, and boot block is configured as follows,
 *               address size
 *		 0x0000    32KB
 *		 0x8000     8KB
 *		 0xa000     8KB
 *		 0xc000    16KB
 *               the configured value is 1000_1110_0000_0000 (binary) = 0x8e00
 */
typedef struct {
	UH	man;		/* manufacturer ID */
	UH	dev;		/* device ID */
	UH	ex1;		/* extended device ID 1 */
	UH	ex2;		/* extended device ID 2 2 */
	UH	size;		/* capacity (MB) */
	UH	wrmode:1;	/* write mode */
	UH	bsec;		/* bottom (low address) boot sector configuration */
	UH	tsec;		/* top (upper address) boot sector configuration */
} JEDEC_SPEC;

/* write mode */
#define	JD_WRSINGLE	0	/* single write */
#define	JD_WRMULTI	1	/* multiple write (command 0x20) */

IMPORT const JEDEC_SPEC	JedecSpec[];
IMPORT const W		N_JedecSpec;

/* ------------------------------------------------------------------------ */

IMPORT const UW FROM_SECSZ;	/* Flash ROM sector size (byte)
				   in the ccase of FWM, it is block size */
/*
 * function definition
 */
IMPORT ER   flashwr( UW addr, void *data, W nsec, BOOL reset );
IMPORT void flashwr_reset( void );
IMPORT ER   flashwr_protect( UW addr, W nsec );
IMPORT void flashwr_setup( BOOL reset );
IMPORT void flashwr_done( void );
