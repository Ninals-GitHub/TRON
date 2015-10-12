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
 *	sdisk.h		system disk driver
 *
 *	common definition
 */
#include <tk/tkernel.h>
#include <libstr.h>
#include <tk/util.h>
#include <device/sdrvif.h>
#include <device/gdrvif.h>
#include <device/disk.h>
#include <device/pcmcia.h>
#include <sys/util.h>

/* debug message (disabled) */
#define	DP(arg)		/**/

/* Macro for 4 characters object creation   */
#if	BIGENDIAN
#define	CH4toW(c1, c2, c3, c4)	( ((c1)<<24)|((c2)<<16)|((c3)<<8)|(c4) )
#else
#define	CH4toW(c1, c2, c3, c4)	( ((c4)<<24)|((c3)<<16)|((c2)<<8)|(c1) )
#endif

/*
	Target disk information
*/
typedef struct {
	UW	accif:8;		/* Access interface	*/
	UW	blocksz:2;		/* Block size x 512	*/
	UW	rsv1:6;
	BOOL	pccard:1;		/* PCMCIA card		*/
	BOOL	rsv2:1;
	BOOL	readonly:1;		/* Readout only 		*/
	BOOL	subtsk:1;		/* Necessity of subtask (unused)	*/
	BOOL	eject:1;		/* Availability of the eject		*/
	BOOL	lockreq:1;		/* Necessity of the lock (use DMA)	*/
	BOOL	wlock:1;		/* (Forced) write-protect		*/
	BOOL	susres:1;		/* here is power-supply control		*/
	BOOL	nowprotect:1;		/* No write-protect	*/
	UW	rsv3:7;
} SDSpec;

#define MEM_ACCIF	1		/* Memory interface 	*/
#define ATA_ACCIF	2		/* ATA/ ATAPI interface	*/
#define MMC_ACCIF	3		/* MMC/SD interface */

			/* acc	    bs rs pc us ro st ej lc wl sr nw rs */
#define ROM_DISK	{MEM_ACCIF, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}
#define RAM_DISK	{MEM_ACCIF, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}
#define ATA_CARD	{ATA_ACCIF, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0}
#define MMC_CARD	{MMC_ACCIF, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0}

#define	MEM_DRV		0x00		/* Memory disk drive number	*/
#define	PC_DRV		0x0C		/* PC card drive number	*/

typedef struct {
	W	num;			/* Top drive number (0:next)	*/
	SDSpec	spec;			/* Device specification 	*/
	UB	devnm[L_DEVNM]; 	/* Device name			*/
	UW	name4;			/* Object name (4 characters)	*/
	UW	drvno;			/* Drive number			*/
	UW	iob;			/* IO base address		*/
	UW	misc;			/* Various parameters		*/
} SDInfo;

/*				iob			misc
 *	PC card		IO base address	--
 *	memory disk		memory top address	memory size
 */

/*
 *	System disk drive information
 */
#define	MAX_PART	4		/* Maximum number of partitions		*/
#define	MAX_SUNIT	(MAX_PART + 1)	/* Maximum number of subunits		*/
#define	IDENT_DTSZ	96		/* Identification information size	*/
#define	IDENT2_DTSZ	16		/* Identification information (2) size	*/

typedef	struct {			/* Subunit information		*/
	DiskSystemId SystemId;		/* System ID		*/
	UW	StartBlock;		/* Partition start block number	*/
	UW	EndBlock;		/* Partition end block number + 1	*/
	W	OpenCnt;		/* Open count		*/
} SUNIT;

typedef struct _drvtab {
	/* Configuration information    */
	SDSpec	Spec;			/* Disk specification		*/
	UB	DevName[L_DEVNM];	/* Device name			*/
	UW	DrvNo;			/* Drive number		*/
	UW	DrvBit;			/* Drive number bit		*/
	UW	IOB;			/* IO / memory base address */
	UW	MiscPar;		/* Interrupt vector/memory size */

	struct	_drvtab	*Next;		/* Next drive			*/
	struct	_drvtab	*Top;		/* Top drive		*/

	/* Half-fixed setting information */
	ID	MbfId;			/* Message buffer for event notification */
	GDI	Gdi;			/* Driver I/F handler	*/
	ID	DevId;			/* Device ID		*/
	ID	WaitTskId;		/* Wait-for-interrupt ID	*/

	/* Processing function in accordance with device type */
	void	(*Abort)(struct _drvtab *drv);
	INT	(*ReadWrite)(struct _drvtab *drv, W blk, W cnt, void *mptr,
								BOOL write);
	ER	(*Format)(struct _drvtab *drv, DiskFormat *fmt, W dcnt);
	ER	(*Identify)(struct _drvtab *drv, BOOL check);
	ER	(*DiskInit)(struct _drvtab *drv);
	ER	(*Misc)(struct _drvtab *drv, W cmd);

	void	(*IntHdr)(struct _drvtab *drv);

	/* Common flag */
	BOOL	DriveOK:1;		/* Absence or presence of drive (card)	*/
	BOOL	MediaOK:1;		/* Absence or presence of media		*/
	BOOL	Wprotect:1;		/* Write-protection media		*/
	BOOL	EnbInt:1;		/* Interrupt-enabled (valid)		*/
	BOOL	Suspended:1;		/* During "suspend"			*/
	BOOL	Aborted:1;		/* Abort request			*/
	BOOL	Reset:1;		/* Reset flag		*/
	BOOL	SuptMBR:1;		/* "DISKMBR" support		*/

	/* ATA I/F exclusive flag */
	BOOL	UseLBA:1;		/* Use LBA		*/
	BOOL	SetXCHS:1;		/* XCHS data is already set	*/

	BOOL	ResvFlags:22;		/* Reserve				*/

	/* Information for internal processing */
	FastLock ProcLock;		/* Lock of common processing		*/
	T_DEVREQ *CurReq;		/* Request the abort target   	*/
	TMO	ReqTmout;		/* Wait-for-request time-out		*/

	/* PCMCIA card-related information */
	union {
		struct {
			ID	CardId;		/* Card ID		*/
			ID	MapId;		/* Map ID		*/
			ID	MapId2;		/* Map ID 2	*/
			W	CardInf;	/* Individual information of card	*/
			UH	IOConf[2];	/* IO configuration information	*/
		} pcc;
		union {
			UW	uw[5];		/* General-purpose parameter	*/
			UH	uh[10];
			UB	ub[20];
		} g;
	} d;

	/* Disk information	*/
	DiskFormat DiskFmt;		/* Disk format		*/
	W	SecSize;		/* Disk sector size	*/
	W	SecBias;		/* Sector bias		*/

	W	CurSUnit;		/* Subunit in processing	*/
	W	OpenCnt;		/* Open count		*/

	UH	MultiCnt;		/* Multi count		*/
	UH	nSUnit;			/* The number of valid subunits		*/

	UH	nCyl;			/* The number of cylinders for internal		*/
	UH	nHead;			/* The number of heads for internal + other information	*/
	UH	nSec;			/* The number of sectors for internal /Track	*/
	UH	nXCyl;			/* The number of cylinders			*/
	UH	nXHead;			/* The number of heads			*/
	UH	nXSec;			/* The number of sectors/tracks		*/

	union {				/* Subunit information		*/
		SUNIT	SUnit[MAX_SUNIT];	/* [0] is the physical unit	*/
	} s;
	UB	Ident[IDENT_DTSZ];	/* Identification information			*/
	UB	Ident2[IDENT2_DTSZ];	/* Identification information (2)		*/

	union {				/* Individual working area		*/
		UW	uw[1];
		UH	uh[2];
		UB	ub[4];
	} wrk;
} DrvTab;

#define	ENB_INT(drv)	((drv)->EnbInt = TRUE)		/* Enable interrupt*/
#define	DIS_INT(drv)	((drv)->EnbInt = FALSE)		/* Disable interrupt*/

/* Command in "Misc" processing*/
#define	DC_OPEN		(1)
#define	DC_CLOSE	(2)
#define	DC_CLOSEALL	(3)
#define	DC_SUSPEND	(4)		/* "Spec.susres = 1" only	*/
#define	DC_RESUME	(5)		/* "Spec.susres = 1" only	*/
#define	DC_TSKINIT	(-1)		/* Task initialization processing		*/
#define	DC_TSKTMO	(-2)		/* Task time-out processing	*/
#define	DC_DRVCHG	(-3)		/* Drive change processing		*/

/*
 * Event time-out (msec)
 */
#define	TMO_EVENT	(2 * 1000)	/* Event time-out		*/

/*
 *	Card power-supply OFF
 */
#define	CP_POFF		CP_POFFREQ(5)	/* Card power supply is OFF in 5 seconds	*/

/*
 *	Error code
 */
#define	ERR_ABORT		(E_IO | 0)
#define	ERR_TMOUT		(E_IO | 1)
#define	ERR_MEDIA		(E_IO | 2)
#define	ERR_HARD		(E_IO | 3)
#define	ERR_DMA			(E_IO | 4)
#define	ERR_BLKSZ		(E_NOSPT)

#define	ERR_CMDBUSY		(E_IO | 0x10)
#define	ERR_DATABUSY		(E_IO | 0x11)
#define	ERR_NOTRDY		(E_IO | 0x12)

#define	ERR_ATA(c)		(E_IO | 0x8000 | (c))
#define	ERR_ATAMSK(er, m)	((er) & (0xffff8000 | (m)))

#define	ERR_ATASENSE		ERR_ATA(0xF0)

#define	ERR_NOBLK		(E_OBJ | 0)
#define	ERR_NOPORT		(E_OBJ | 1)
#define	ERR_NOMEDIA		E_NOMDA
#define	ERR_RONLY		E_RONLY
#define	ERR_NOTSAME		E_PAR

/*
 *	Device request accept pattern
 */
#define	DISWAI		16
#define	DISWAI_REQ	DEVREQ_ACPPTN(DISWAI)
#define	NORMAL_REQPTN	(DRP_NORMREQ | DISWAI_REQ)

/*
 *	Invalid device ID
 */
#define INVALID_DEVID	(-1)

/*
 *	Driver task information
 */
#ifdef	DEBUG
#define	PRINTF_STKSZ	(1536)
#else
#define	PRINTF_STKSZ	(0)
#endif
#define	DRVTSK_STKSZ	(3072 + PRINTF_STKSZ)	/* Stack size	*/

/*
 *	Function definition
 */
/* main.c */
IMPORT	ER	SysDiskDrv(INT ac, UB *av[]);

/* accept.c */
IMPORT	INT	CheckSize(W datacnt, W size);
IMPORT	ER	sdAbortFn(T_DEVREQ *req, GDI gdi);
IMPORT	INT	sdEventFn(INT evttyp, void *evtinf, GDI gdi);
IMPORT	ER	sdOpenFn(ID devid, UINT omode, GDI gdi);
IMPORT	ER	sdCloseFn(ID devid, UINT option, GDI gdi);
IMPORT	void	sdAcceptRequest(DrvTab *drv);

/* common.c */
IMPORT	void	sdSendEvt(DrvTab *drv, W kind);
IMPORT	void	sdSetUpAccFn(DrvTab *drv);
IMPORT	void	sdMakeDiskInfo(DrvTab *drv, DiskInfo *inf);
IMPORT	ER	sdRegistDevice(DrvTab *drv);
IMPORT	UW	sdGetMsec(void);
IMPORT	BOOL	sdChkTmout(UW *tm, UW tmo, W delay);

/* inthdr.c */
IMPORT	ER	sdDefIntHdr(DrvTab *drv, BOOL regist);
IMPORT	void	sdSetIntWait(DrvTab *drv);

/* pccard.c */
IMPORT	ER	sdPowerOn(DrvTab *drv, UW req);
IMPORT	void	sdPowerOff(DrvTab *drv, UW req);
IMPORT	INT	sdCardEvent(INT evttyp, CardReq *req, GDI gdi);
IMPORT	ER	sdInitCard(DrvTab *drv);

/* ata.c */
IMPORT	INT	ataReadWrite(DrvTab *drv, W blk, W cnt, void *mptr, BOOL write);
IMPORT	ER	ataFormat(DrvTab *drv, DiskFormat *fmt, W dcnt);
IMPORT	ER	ataIdentify(DrvTab *drv, BOOL check);
IMPORT	ER	ataDiskInit(DrvTab *drv);
IMPORT	ER	ataMisc(DrvTab *drv, W cmd);
IMPORT	W	ataSetXCHS(DrvTab *drv);
IMPORT	ER	ataSetupPart(DrvTab *drv);

/* atacmd.c */
IMPORT	void	ataSelDrv(DrvTab *drv);
IMPORT	void	ataIntHdr(DrvTab *drv);
IMPORT	void	ataAbort(DrvTab *drv);
IMPORT	ER	ataCmd(DrvTab *drv, W cmd, UW lba, W len, void *buf);
IMPORT	INT	atapiCmd(DrvTab *drv, W cmd, UW lba, W len, void *buf);

/* mem.c */
IMPORT	void	memAbort(DrvTab *drv);
IMPORT	INT	memReadWrite(DrvTab *drv, W blk, W cnt, void *mptr, BOOL write);
IMPORT	ER	memFormat(DrvTab *drv, DiskFormat *fmt, W dcnt);
IMPORT	ER	memIdentify(DrvTab *drv, BOOL check);
IMPORT	ER	memDiskInit(DrvTab *drv);
IMPORT	ER	memMisc(DrvTab *drv, W cmd);

/* mmc-common.c */
IMPORT	void	mmcAbort(DrvTab *drv);
IMPORT	INT	mmcReadWrite(DrvTab *drv, W blk, W cnt, void *mptr, BOOL write);
IMPORT	ER	mmcFormat(DrvTab *drv, DiskFormat *fmt, W dcnt);
IMPORT	ER	mmcIdentify(DrvTab *drv, BOOL check);
IMPORT	ER	mmcDiskInit(DrvTab *drv);
IMPORT	ER	mmcMisc(DrvTab *drv, W cmd);

/* ataio.c */
IMPORT	W	ataStatusIn(DrvTab *drv);
IMPORT	W	ataErrorIn(DrvTab *drv);
IMPORT	W	ataCylIn(DrvTab *drv);
IMPORT	ER	ataSetDrive(DrvTab *drv, W drvno);
IMPORT	void	ataEnbInt(DrvTab *drv);
IMPORT	ER	ataWaitData(DrvTab *drv);
IMPORT	ER	ataWaitReady(DrvTab *drv);
IMPORT	void	ataReset(DrvTab *drv);
IMPORT	void	atapiReset(DrvTab *drv);
IMPORT	BOOL	atapiTest(DrvTab *drv, W init);
IMPORT	ER	ataCommandOut(DrvTab *drv, W cmd, W cnt, W sec, W cyl, W head);
IMPORT	ER	atapiCommandOut(DrvTab *drv, UH *cmd, W cnt, W dma);
IMPORT	void	ataDataIn(DrvTab *drv, UH *buf, W cnt);
IMPORT	void	ataDataSkip(DrvTab *drv, W cnt);
IMPORT	void	ataDataOut(DrvTab *drv, UH *buf, W cnt);
