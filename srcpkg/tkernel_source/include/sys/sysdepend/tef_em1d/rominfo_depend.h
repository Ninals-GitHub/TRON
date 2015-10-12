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
 *	@(#)rominfo_depend.h (sys/EM1-D512)
 *
 *	ROM information
 */

#ifndef __SYS_ROMINFO_DEPEND_H__
#define __SYS_ROMINFO_DEPEND_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ROM information
 */
typedef struct {
	FP	kernel;		/* Kernel startup address */
	UB	*sysconf;	/* SYSCONF top */
	UB	*devconf;	/* DEVCONF top */
	void	*userarea;	/* RAM user area top */
	FP	userinit;	/* User initialization program address */
	FP	resetinit;	/* Reset initialization program address */
	VW	rsv[10];	/* Reserved (always 0) */
	UW	rd_type;	/* ROM disk type */
	UW	rd_blksz;	/* ROM disk block size */
	UW	rd_saddr;	/* ROM disk start address */
	UW	rd_eaddr;	/* ROM disk end address */
	VW	rsv2[12];	/* Reserved (always 0) */
} RomInfo;

/* Start address of ROMInfo */
#define ROMInfo	( (RomInfo*)0x70020000 )


/* common tag in DEVCONF[] --> config/src/sysdepend/<TARGET>/DEVCONF */
#define DCTAG_DEBUGMODE	((UB*)"DEBUGMODE")	/* Debug mode (1:debug, 0:normal) */

/* common tag in SYSCONF[] --> config/src/sysdepend/<TARGET>/SYSCONF */
/* Product information */
#define SCTAG_TSYSNAME	((UB*)"TSysName")	/* System name */

#define SCTAG_MAKER	((UB*)"Maker")		/* Maker Code */
#define SCTAG_PRODUCTID	((UB*)"ProductID")	/* Kernel Identifier */
#define SCTAG_SPECVER	((UB*)"SpecVer")	/* Specification Version */
#define SCTAG_PRODUCTVER ((UB*)"ProductVer")	/* Product Version */
#define SCTAG_PRODUCTNO	((UB*)"ProductNo")	/* Product Number [0]-[3] */


/* T-Kernel/OS */
#define SCTAG_TMAXTSKID	((UB*)"TMaxTskId")	/* Maximum task ID */
#define SCTAG_TMAXSEMID	((UB*)"TMaxSemId")	/* Maximum semaphore ID */
#define SCTAG_TMAXFLGID	((UB*)"TMaxFlgId")	/* Maximum event flag ID */
#define SCTAG_TMAXMBXID	((UB*)"TMaxMbxId")	/* Maximum mail box ID */
#define SCTAG_TMAXMTXID	((UB*)"TMaxMtxId")	/* Maximum mutex ID */
#define SCTAG_TMAXMBFID	((UB*)"TMaxMbfId")	/* Maximum message buffer ID */
#define SCTAG_TMAXPORID	((UB*)"TMaxPorId")	/* Maximum rendezvous port ID */
#define SCTAG_TMAXMPFID	((UB*)"TMaxMpfId")	/* Maximum fixed size memory pool ID */
#define SCTAG_TMAXMPLID	((UB*)"TMaxMplId")	/* Maximum variable size memory pool ID */
#define SCTAG_TMAXCYCID	((UB*)"TMaxCycId")	/* Maximum cyclic handler ID */
#define SCTAG_TMAXALMID	((UB*)"TMaxAlmId")	/* Maximum alarm handler ID */
#define SCTAG_TMAXRESID	((UB*)"TMaxResId")	/* Maximum resource group ID */
#define SCTAG_TMAXSSYID	((UB*)"TMaxSsyId")	/* Maximum sub system ID */
#define SCTAG_TMAXSSYPRI ((UB*)"TMaxSsyPri")	/* Maximum sub system priority */

#define SCTAG_TSYSSTKSZ	((UB*)"TSysStkSz")	/* Default system stack size (byte) */
#define SCTAG_TSVCLIMIT	((UB*)"TSVCLimit")	/* SVC protection level */
#define SCTAG_TTIMPERIOD ((UB*)"TTimPeriod")	/* Timer interval (msec) */


/* T-Kernel/SM */				/* Maximum number of ... */
#define SCTAG_TMAXREGDEV ((UB*)"TMaxRegDev")	/* devices registration */
#define SCTAG_TMAXOPNDEV ((UB*)"TMaxOpnDev")	/* devices open */
#define SCTAG_TMAXREQDEV ((UB*)"TMaxReqDev")	/* device requests */

#define SCTAG_TDEVTMBFSZ ((UB*)"TDEvtMbfSz")	/* Event notification message */
						/* buffer size (byte), */
						/* Maximum length of message (byte) */


/* Task Event(1-8) */				/* Message management */
#define SCTAG_TEV_MSGEVT ((UB*)"TEV_MsgEvt")	/* Receive message */
#define SCTAG_TEV_MSGBRK ((UB*)"TEV_MsgBrk")	/* Release of an message waiting state */

#define SCTAG_TEV_GDI	 ((UB*)"TEV_GDI")	/* GDI interface */
#define SCTAG_TEV_FFLOCK ((UB*)"TEV_FFLock")	/* Release of an FIFO lock waiting state */


/* Segment manager */
#define SCTAG_REALMEMEND ((UB*)"RealMemEnd")	/* RAM bottom address (logical address) */


/* Stack Size for ARM */
#define SCTAG_ABTSTKSZ	((UB*)"AbtStkSz")	/* Abort(MMU) */
#define SCTAG_UNDSTKSZ	((UB*)"UndStkSz")	/* Undefined instruction */
#define SCTAG_IRQSTKSZ	((UB*)"IrqStkSz")	/* IRQ interrupt */
#define SCTAG_FIQSTKSZ	((UB*)"FiqStkSz")	/* FIQ interrupt */


#ifdef __cplusplus
}
#endif
#endif /* __SYS_ROMINFO_DEPEND_H__ */
