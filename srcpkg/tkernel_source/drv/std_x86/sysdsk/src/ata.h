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
 *	ata.h		System disk driver
 *
 *	ATA (IDE) / ATAPI disk-related definition
 */

/*
 *	Endian conversion
 *	!! Note that the data from ATA disk is the little endian !!
 */
Inline	UH	swapH(UH x)
{
	return (x << 8) | (x >> 8);
}
Inline	UW	swapW(UW x)
{
	return (UW)swapH((UH)x) << 16 | swapH((UH)(x >> 16));
}

#if	BIGENDIAN
#define	CnvLeH(x)	((UH)swapH((UH)(x)))
#define	CnvLeW(x)	((UW)swapW((UW)(x)))
#define	CnvBeH(x)	(x)
#define	CnvBeW(x)	(x)
#else
#define	CnvLeH(x)	(x)
#define	CnvLeW(x)	(x)
#define	CnvBeH(x)	((UH)swapH((UH)(x)))
#define	CnvBeW(x)	((UW)swapW((UW)(x)))
#endif

/*
 *	ATA disk physical block size : fixed
 */
#define	ATA_SECSZ	(512)		/* Sector size	*/

/*
 *	Time-out (msec)
 */
#define	TMO_ATACMD	(10 * 1000)	/* Command execution time-out	*/
#define	TMO_ATAPIREADY	(10 * 1000)	/* Ready time-out		*/

/*
 *	The maximum number of I/O sectors at one time
 */
#define	MAX_IOSEC	(256)		/* The number of sectors at "ATA_SECSZ"	*/

/*
 *	ATA disk partition information
 */
#define	N_PART		4		/* The number of partitions	*/

typedef	struct {
	UB	BootInd;		/* Boot indicator		*/
	UB	StartHead;		/* Start head number		*/
	UB	StartSec;		/* Start sector number		*/
	UB	StartCyl;		/* Start cylinder number	*/
	UB	SysInd;			/* System indicator		*/
	UB	EndHead;		/* End head number		*/
	UB	EndSec;			/* End sector number		*/
	UB	EndCyl;			/* End cylinder number		*/
	UW	StartBlock;		/* Relative start sector number	*/
	UW	BlockCnt;		/* The number of sectors		*/
} PartTab;

#define	OFS_PART	(446)		/* Offset toward partition information 	*/
#define	SIZE_PARTTAB	(sizeof(PartTab) * N_PART)
#define	OFS_SIGN	(OFS_PART + SIZE_PARTTAB)
#define	VALID_SIGN	CnvLeH((UH)0xAA55)	/* Signature	*/

/*
 *	ATA disk master boot block structure
 *	!! "PartTab[].StartBlock" in this structure doesn't match a word align,
 *	   so cannot be directly used unchanged according to machines!!
 */
typedef	struct {
	UB	BootCode[OFS_PART];	/* Code for boot(DOS/V)	*/
	PartTab	Part[N_PART];		/* Partition information		*/
	UH	Sign;			/* Signature:0xAA55			*/
} MasterBoot;

/*
 *	ATA command
 */
#define	NOSET		(-1)		/* Unnecessary register setting		*/

#define	ATA_IDENTIFY	(0xEC)		/* "IDENTIFY" command		*/
#define	ATA_READ	(0x20)		/* READ w. Retry command	*/
#define	ATA_WRITE	(0x30)		/* WRITE w. Retry command	*/

#define	ATA_MREAD	(0xC4)		/* READ MULTIPLE command	*/
#define	ATA_MWRITE	(0xC5)		/* WRITE MULTIPLE command	*/
#define	ATA_SETMULTI	(0xC6)		/* SET MULTIPLE command	*/

/*
 *	ATA card setting information
 */
#define	ATA_CNFIX	(0x01)		/* Continuous IO address		*/
#define	ATA_OFFIO2	(0x3F6 - 0x1F0)	/* Offset toward IO 2 address	*/

/*
 *	IOConf[0] definition :	 The things execepting (*1)is valid only in the case of PC card
 *				 All are 0 in the case of non PC card
 *
 *	SSSS ABCD xxII IIII	I: CONF INDEX
 *				A: ATAPI	0 = ATA		1 = ATAPI
 *				B: IO2 with or without	0 = with, 	1 = without
 *				C: IO2 range	0 = 1		1 = 2
 *				D: IO1 range	0 = 8		1 = 16
 *
 */
#define	IOC_SPECIAL	0xF000		/* Special processing type	*/
#define	IOC_ATAPI	0x0800		/* ATAPI device 		*/
#define	IOC_IO2_NONE	0x0400		/* here is no IO2	   (*1)	*/
#define	IOC_IO2_2	0x0200		/* IO2 range 2			*/
#define	IOC_IO1_16	0x0100		/* IO1 range 16	        	*/
#define	IOC_CONFIX	0x003F		/* Config index	                */

/*
 *	ATA I/O register address (Offset from base address)
 */
#define	REG_DATA	0x00		/* Data (RW)			*/
#define	REG_FEATURE	0x01		/* Features (W)			*/
#define	REG_ERR 	0x01		/* Error (R)			*/

#define	REG_SECCNT	0x02		/* (ATA) Sector Count (RW)	*/
#define	REG_INTR	0x02		/* (ATAPI) Interrupt Reason(R)	*/

#define	REG_SECNO	0x03		/* (ATA) Sector Number (RW)	*/

#define	REG_CYL_L	0x04		/* (ATA) Cylinder Low (RW)	*/
#define	REG_CYL_H	0x05		/* (ATA) Cylinder High (RW)	*/

#define	REG_BCNT_L	0x04		/* (ATAPI) Byte Count Low (RW)	*/
#define	REG_BCNT_H	0x05		/* (ATAPI) Byte Count High (RW)	*/

#define	REG_DRVHEAD	0x06		/* Drive/Head	(RW)		*/
#define	REG_CMD 	0x07		/* Command  (W)			*/
#define	REG_STS 	0x07		/* Status  (R)			*/

#define	REG_ALTSTS	0x0E		/* Alternate Status (R)		*/
#define	REG_DEVCTL	0x0E		/* Device Control (W)		*/

#define	REG_ALTSTS2	ATA_OFFIO2	/* Alternate Status (R)		*/
#define	REG_DEVCTL2	ATA_OFFIO2	/* Device Control (W)		*/

/*
	I/O register:	7    6	  5    4    3	 2    1    0
	(ATA)
	7:REG_STS	BSY  DRDY DF   DSC  DRQ  CORR IDX  ERR	 (R)
	6:REG_DRVHEAD	1    LBA  1    DRV  HN3  HN2  HN1  HN0	 (RW)
	1:REG_ERR	(BBK)UNC  MC   IDNF MCR  ABRT TK0  AMNF	 (R)
	(MSC cmd)	0    WP	  MC   0    MCR	 0    NM   0	 (R)
	3x6:REG_DEVCTL	x    x	  x    x    1	 SRST ~IEN 0	 (W)

	(ATAPI)
	7:REG_STS	BSY  DRDY DF   DSC  DRQ  CORR x    CHK	 (R)
	6:REG_DRVHEAD	1    x	  1    DRV  x	 x    x    x	 (RW)
	1:REG_ERR	SK0  SK1  SK2  SK3  MCR  ABRT EOM  ILI	 (R)
	3x6:REG_DEVCTL	x    x	  x    x    1	 SRST ~IEN 0	 (W)
	1:REG_FEATURE	x    x	  x    x    x	 x    OVLP DMA	 (W)
	2:REG_INTR	x    x	  x    x    x	 REL  IO   Cod	 (R)
*/

/* REG_STS */
#define	stBSY		0x80
#define	stDRDY		0x40
#define	stDF		0x20
#define	stDSC		0x10
#define	stDRQ		0x08
#define	stCORR		0x04
#define	stIDX		0x02
#define	stERR		0x01

/* REG_DRVHEAD */
#define	drDRV(n)	(0xA0 | (((n) & 0x100) >> 4))
#define	drLBA		0x40

/* REG_ERR */
#define	erMC		0x20			/* Media Change		*/
#define	erMCR		0x08			/* Media Change Request	*/
#define	erABRT		0x04			/* Command Abort	*/

#define	erWP		0x40			/* Write Protect	*/
#define	erNM		0x02			/* No Midia		*/

/* REG_INTR             */
#define	irIO		0x02			/* 0: Write, 1: Read	*/
#define	irCOD		0x01			/* 0: Data,  1: Command	*/

/* REG_DEVCTL */
#define	dcNORM		(0x08)			/* SRST = 0, ~IEN = 0	*/
#define	dcSRST		(0x08 | 0x04 | 0x02)	/* SRST = 1, ~IEN = 1	*/
