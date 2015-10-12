/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	@(#)netdrv.h
 *
 */

#include	<tk/tkernel.h>
#include	<tk/util.h>
#include	<libstr.h>
#include	<device/gdrvif.h>
#include	<device/netdrv.h>
#include	<device/pcmcia.h>
#include	<device/devconf.h>

#ifdef	DEBUG
#define DP(exp) 	printf exp
#define DBG(exp)	exp
#else
#define DP(exp) 	/**/
#define DBG(exp)	/**/
#endif

#define	INVALID_ID	(0)		/* Invalid ID			*/
#define	INVALID_DEVID	(-1)		/* Invalid Device ID		*/

/*
 *	Task information
 */
#define DEFAULT_PRI	(25)		/* Default priority		*/
#ifdef	DEBUG
#define TASK_STKSZ	(4096)		/* Stack size			*/
#else
#define TASK_STKSZ	(2048)		/* Stack size			*/
#endif

/*
 *	Other definitions
 */
#define	MAX_NETDEV	2		/* Max numbers of network devices */

#define	L_EADDR		(6)		/* Length of physical address	*/

#define	MAXPKTLEN	(1500+14+6)	/* Max packet length (16-byte units) */
#define	MINPKTLEN	(46+14)		/* Min packet length		*/
#define	MINRXBUFSZ	(512+20+44)	/* Min receive buffer length	*/

#define	N_RXBUF		(64)		/* Max number of receive buffer	*/

#define	IOSTEP	1

/*
 *	Network driver information
 */
typedef struct _netinf {
	W	netno;			/* Network number (0 -)		*/
	ID	devid;			/* Device ID			*/
	GDI	Gdi;			/* Generic driver I/F handle	*/
	ID	mbfid;			/* Event notification MBF ID	*/
	ID	tskid;			/* My task ID			*/

	NetDevInfo di;			/* Device information		*/
	W	pciadr;			/* PCI address			*/
	W	devix;			/* Device index			*/

	NetAddr	eaddr;			/* Ethernet address		*/
	NetStInfo stinf;		/* Statistical information	*/

	W	opencnt;		/* Open counts			*/
	W	tmout;			/* Timeout			*/

	BOOL	exist:1;		/* Exist status			*/
	BOOL	suspended:1;		/* Suspend status		*/
	BOOL	txbusy:1;		/* Transmit busy status		*/
	BOOL	poweroff:1;		/* Card power off status	*/
	BOOL	rsv1:12;
	BOOL	wrkflg1:1;		/* Individual flags		*/
	BOOL	wrkflg2:1;
	BOOL	wrkflg3:1;
	BOOL	wrkflg4:1;
	BOOL	rsv2:12;

	ID	cardid;			/* PC card ID			*/
	ID	mapid;			/* PC card map	ID		*/
	UH	ccraddr;		/* PC card CCR address		*/
	UB	ccr[2];			/* PC card CCR value		*/
	UW	orgbase;		/* PC card mapping address	*/

	NetRxBufSz bufsz;		/* Receive buffer size		*/
	UW	ip_rxbuf;		/* Receive buffer set position	*/
	UW	op_rxbuf;		/* Receive buffer get position	*/
	VP	rxbuf[N_RXBUF];		/* Receive buffer list		*/

	/* Individual functions */
	ER	(*initfn)(struct _netinf *inf, W init);
	void	(*inthdr)(struct _netinf *inf);
	INT	(*sendfn)(struct _netinf *inf, UB *buf, W len);
	void	(*reset)(struct _netinf *inf, BOOL start);
	ER	(*cardchk)(struct _netinf *inf);
	INT	(*misc)(struct _netinf *inf, W wrt, W par, VP buf, W len);
	void	(*tmofn)(struct _netinf *inf);
	UH	(*mii_read)(struct _netinf *inf, W adr, W reg);
	void	(*mii_write)(struct _netinf *inf, W adr, W reg, W dat);
	INT	(*mcast)(struct _netinf *inf, W wrt, W all, UB *buf, W len);

	union {			/* Individual working area	*/
		VP	vp[12];		/* Assume sizeof(VP) = sizeof(UW) */
		UW	uw[12];
		UH	uh[24];
		UB	ub[48];
	} wrk;
	VP	extwrk;			/* Pointer to extended working area */
	FastLock	lock;		/* Exclusive lock		*/
} NetInf;

#define	NetUnit(inf)	((inf)->netno + 'a')	/* for debug messages	*/

/*
	PCI Device ID definition table
*/
typedef	struct {
	UH	vendor;			/* Vendor ID			*/
	UH	devid;			/* Device ID			*/
	UH	idmask;			/* Device ID mask		*/
	UH	devix;			/* Device index			*/
} PciTab;

#define	PCI_NET_ETHER	0x0200		/* Ethernet device class	*/

/*
 *	Driver definition table
 */
typedef	struct	{
	UW	kind;			/* Hardware kind		*/
	PciTab	*pcitab;		/* PCI device definition	*/
	ER	(*initfn)(NetInf *inf, W init);	/* Initialize function	*/
} DrvDef;

/*
 *	Hardware kind
 */
#define	HK_MASK		(0x000007ff)
#define	HK_CHIPMASK	(0x000000ff)
#define	HK_ISA		(0x00000000)
#define	HK_PCI		(0x00000100)
#define	HK_PCCARD	(0x00000200)
#define	isPcCARD(inf)	(((inf)->di.kind & 0x00000700) == HK_PCCARD)
#define	isPCI(inf)	(((inf)->di.kind & 0x00000700) == HK_PCI)
#define	isISA(inf)	(((inf)->di.kind & 0x00000700) == HK_ISA)

#define	isFORCE_OPEN(inf)	( (inf)->di.kind & 0x00000800)
#define	IFC_TYPE(inf)		(((inf)->di.kind & 0x0000f000) >> 12)

#define	HK_FDX		(0x00010000)
#define	isFDX(inf)	((inf)->di.kind & HK_FDX)

/*
 *	I/O MACRO definitions
 */
#define	InB(ix)		in_b(IOB + ((ix) * IOSTEP))
#define	InH(ix)		in_h(IOB + ((ix) * IOSTEP))
#define	InW(ix)		in_w(IOB + ((ix) * IOSTEP))
#define	OutB(ix, dt)	out_b(IOB + ((ix) * IOSTEP), (dt))
#define	OutH(ix, dt)	out_h(IOB + ((ix) * IOSTEP), (dt))
#define	OutW(ix, dt)	out_w(IOB + ((ix) * IOSTEP), (dt))

/*
 *	External functions
 */

/* main.c */
IMPORT	PRI	TskPri;
IMPORT	ID	CreTask(FP entry, PRI pri, W name, UW par, W stksz);

/* accept.c */
IMPORT	ER	OpenProc(ID devid, UINT omode, GDI gdi);
IMPORT	ER	CloseProc(ID devid, UINT option, GDI gdi);
IMPORT	INT	EventProc(INT evttyp, VP evtinf, GDI gdi);
IMPORT	void	AcceptRequest(NetInf *inf);

/* card.c */
IMPORT	void	CardPowerOff(NetInf *inf, BOOL close);
IMPORT	ER	CardPowerOn(NetInf *inf, BOOL open);
IMPORT	ER	CardEvent(NetInf *inf, CardReq *req);

/* io.c */
IMPORT	void	OutMemB(UW iob, W ix, UB *buf, W cnt);
IMPORT	void	InMemB(UW iob, W ix, UB *buf, W cnt);
IMPORT	void	OutMemH(UW iob, W ix, UH *buf, W cnt);
IMPORT	void	InMemH(UW iob, W ix, UH *buf, W cnt);
IMPORT	void	OutMemW(UW iob, W ix, UW *buf, W cnt);
IMPORT	void	InMemW(UW iob, W ix, UW *buf, W cnt);

/* inthdr.c */
IMPORT	ER	DefIntHdr(NetInf *inf, BOOL regist);

/* pci.c */
IMPORT	ER	PciAllocMem(W sz, VP *p);
IMPORT	void	PciFreeMem(VP p);
IMPORT	ER	PciProbe(NetInf *inf, PciTab *tab);
IMPORT	void	PciEnable(W caddr, W iocmd, W latency);
IMPORT	void	SetACPI(NetInf *inf, W state);
IMPORT	INT	GetPCIAdr(VP addr, W len, VP *physical_addr);

/* misc.c */
IMPORT	ER	AllocMem(W sz, VP *p);
IMPORT	void	FreeMem(VP p);
IMPORT	ER	SetRxBuf(NetInf *inf, VP bp);
IMPORT	VP	GetRxBuf(NetInf *inf);
IMPORT	ER	SendMsg(NetInf *inf, VP pb, W len);
IMPORT	INT	MIIfind(NetInf *inf, W target);
IMPORT	INT	MIIinit(NetInf *inf, W target);
IMPORT	void	SetProdName(NetInf *inf, UB **nm1, UB **nm2);
IMPORT	UW	CalcCRC32(UB *data, W len);

/* smsc9118.c */
IMPORT	ER	InitSMSC9118(NetInf *inf, W init);

