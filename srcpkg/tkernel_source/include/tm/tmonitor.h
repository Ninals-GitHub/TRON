/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by T-Engine Forum at 2012/11/27.
 *
 *----------------------------------------------------------------------
 */

/*
 *	@(#)tmonitor.h (tm)
 *
 *	T-Monitor
 */

#ifndef __TM_TMONITOR_H__
#define __TM_TMONITOR_H__

#include <basic.h>

#ifdef __cplusplus
extern "C" {
#endif

#define L_DEVNM		8	/* Device name length */

/*
 * Boot information
 */
typedef struct BootInfo {
	UB	devnm[L_DEVNM];	/* Physical device name */
	INT	part;		/* Partition number
				   (-1: no partition) */
	INT	start;		/* Partition first sector number
				   (0: no partition) */
	INT	secsz;		/* Sector size (byte) */
} BootInfo;

/*
 * Extended service call function number (tm_extsvc)
 */
#define TMEF_PORTBPS	0x00U	/* Debug port speed (bps) */
#define TMEF_RDAINFO	0x01U	/* ROM disk information */
#define TMEF_PCIINFO	0x02U	/* PCI device information */
#define TMEF_DIPSW	0x10U	/* DIPSW state */
#define TMEF_WROM	0x20U	/* Flash ROM write */

/*
 *	ROM disk information
 */
typedef struct	{
	UW	rd_type;	/* ROM disk type (1: ROM disk) */
	UW	rd_blksz;	/* ROM disk block size (normally 512) */
	UW	rd_saddr;	/* ROM disk start address */
	UW	rd_eaddr;	/* ROM disk end address */
} RdaInfo;

/*
 *	PCI device information
 */
#define CADDR(bus, dev, func)	(((bus) << 8) | ((dev) << 3) | (func))

typedef struct	{
	UH	caddr;		/* Configuration address */
	UH	vendor; 	/* Vendor ID */
	UH	devid;		/* Device ID */
	UH	devclass;	/* Device class */
} PciInfo;

/*
 * Monitor service function
 */
IMPORT void tm_monitor( void );
IMPORT INT  tm_getchar( INT wait );
IMPORT INT  tm_putchar( INT c );
IMPORT INT  tm_getline( UB *buff );
IMPORT INT  tm_putstring( const UB *buff );
IMPORT INT  tm_command( const UB *buff );
IMPORT INT  tm_readdisk( const UB *dev, INT sec, INT nsec, void *addr );
IMPORT INT  tm_writedisk( const UB *dev, INT sec, INT nsec, void *addr );
IMPORT INT  tm_infodisk( const UB *dev, INT *blksz, INT *nblks );
IMPORT void tm_exit( INT mode );
IMPORT INT  tm_extsvc( INT fno, INT par1, INT par2, INT par3 );

/*
 * Monitor related library
 */
IMPORT int tm_sprintf( char *str, const char *format, ... );
IMPORT int tm_printf( const char *format, ... );
#define	bms_printf	tm_printf

#ifdef __cplusplus
}
#endif
#endif /* __TM_TMONITOR_H__ */
