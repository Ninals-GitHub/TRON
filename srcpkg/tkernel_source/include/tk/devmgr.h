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
 *	@(#)devmgr.h (T-Kernel)
 *
 *	T-Kernel Device Management
 */

#ifndef __TK_DEVMGR_H__
#define __TK_DEVMGR_H__

#include <basic.h>
#include <tk/typedef.h>
#include <tk/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

#define L_DEVNM		8	/* Device name length */

/*
 * Device attribute (ATR)
 *
 *	IIII IIII IIII IIII PRxx xxxx KKKK KKKK
 *
 *	The first 16-bit is the device-dependent attribute and
 *	defined by each device.
 *	The last 16-bit is the standard attribute and defined
 *	like as followings.
 */
#define TD_PROTECT	0x8000U		/* P: Write protected */
#define TD_REMOVABLE	0x4000U		/* R: Media remove enabled */

#define TD_DEVKIND	0x00ffU		/* K: Device/media type */
#define TD_DEVTYPE	0x00f0U		/*    Device type */

/* Device type */
#define TDK_UNDEF	0x0000U		/* Undefined/Unknown */
#define TDK_DISK	0x0010U		/* Disk device */

/* Disk type */
#define TDK_DISK_UNDEF	0x0010U		/* Other disks */
#define TDK_DISK_RAM	0x0011U		/* RAM disk (Use main memory) */
#define TDK_DISK_ROM	0x0012U		/* ROM disk (Use main memory) */
#define TDK_DISK_FLA	0x0013U		/* Flash ROM, other silicon disks */
#define TDK_DISK_FD	0x0014U		/* Floppy disk */
#define TDK_DISK_HD	0x0015U		/* Hard disk */
#define TDK_DISK_CDROM	0x0016U		/* CD-ROM */

/*
 * Device open mode
 */
#define TD_READ		0x0001U		/* Read only */
#define TD_WRITE	0x0002U		/* Write only */
#define TD_UPDATE	0x0003U		/* Read and write */
#define TD_EXCL		0x0100U		/* Exclusive */
#define TD_WEXCL	0x0200U		/* Exclusive write */
#define TD_REXCL	0x0400U		/* Exclusive read */
#define TD_NOLOCK	0x1000U		/* Lock (resident) non-required */

/*
 * Device close option
 */
#define TD_EJECT	0x0001U		/* Media eject */

/*
 * Suspend mode
 */
#define TD_SUSPEND	0x0001U		/* Suspend */
#define TD_DISSUS	0x0002U		/* Disable suspend */
#define TD_ENASUS	0x0003U		/* Enable suspend */
#define TD_CHECK	0x0004U		/* Get suspend disable request count */
#define TD_FORCE	0x8000U		/* Specify forced suspend */

/*
 * Device information
 */
typedef struct t_rdev {
	ATR	devatr;		/* Device attribute */
	INT	blksz;		/* Specific data block size (-1: Unknown) */
	INT	nsub;		/* Number of subunits */
	INT	subno;		/* 0: Physical device,
				   1 - nsub: Subunit number +1 */
} T_RDEV;

/*
 * Registration device information
 */
typedef struct t_ldev {
	ATR	devatr;		/* Device attribute */
	INT	blksz;		/* Specific data block size (-1: Unknown) */
	INT	nsub;		/* Number of subunits */
	UB	devnm[L_DEVNM];	/* Physical device name */
} T_LDEV;

/*
 * Common attribute data number
 *	RW: Readable (tk_rea_dev)/writable (tk_wri_dev)
 *	R-: Readable (tk_rea_dev) only
 */
#define TDN_EVENT	(-1)	/* RW:Message buffer ID
				      for event notification */
#define TDN_DISKINFO	(-2)	/* R-:Disk information */
#define TDN_DISPSPEC	(-3)	/* R-:Display device specification */
#define TDN_PCMCIAINFO	(-4)	/* R-:PC card information */
#define TDN_DISKINFO_D	(-5)	/* R-: Disk information (64 bits device) */

/*
 * Device event type
 */
typedef	enum tdevttyp {
	TDE_unknown	= 0,		/* Undefined */
	TDE_MOUNT	= 0x01,		/* Media insert */
	TDE_EJECT	= 0x02,		/* Media eject */
	TDE_ILLMOUNT	= 0x03,		/* Media incorrect insert */
	TDE_ILLEJECT	= 0x04,		/* Media incorrect eject */
	TDE_REMOUNT	= 0x05,		/* Media re-insert */
	TDE_CARDBATLOW	= 0x06,		/* Card battery low */
	TDE_CARDBATFAIL	= 0x07,		/* Card battery abnormal */
	TDE_REQEJECT	= 0x08,		/* Media eject request */
	TDE_PDBUT	= 0x11,		/* PD button state change */
	TDE_PDMOVE	= 0x12,		/* PD position move */
	TDE_PDSTATE	= 0x13,		/* PD state change */
	TDE_PDEXT	= 0x14,		/* PD extended event */
	TDE_KEYDOWN	= 0x21,		/* Key down */
	TDE_KEYUP	= 0x22,		/* Key up */
	TDE_KEYMETA	= 0x23,		/* Meta key state change */
	TDE_POWEROFF	= 0x31,		/* Power switch off */
	TDE_POWERLOW	= 0x32,		/* Power low */
	TDE_POWERFAIL	= 0x33,		/* Power abnormal */
	TDE_POWERSUS	= 0x34,		/* Automatic suspend */
	TDE_POWERUPTM	= 0x35,		/* Clock update */
	TDE_CKPWON	= 0x41		/* Automatic power on notification */
} TDEvtTyp;

/*
 * Device event message format
 */
typedef struct t_devevt {
	TDEvtTyp	evttyp;		/* Event type */
	/* Information by each event type is added below */
} T_DEVEVT;

/*
 * Device event message format with device ID
 */
typedef struct t_devevt_id {
	TDEvtTyp	evttyp;		/* Event type */
	ID		devid;		/* Device ID */
	/* Information by each event type is added below */
} T_DEVEVT_ID;

/* ------------------------------------------------------------------------ */

/*
 * Device registration information
 */
typedef struct t_ddev {
	void	*exinf;		/* Extended information */
	ATR	drvatr;		/* Driver attribute */
	ATR	devatr;		/* Device attribute */
	INT	nsub;		/* Number of subunits */
	INT	blksz;		/* Specific data block size (-1: Unknown) */
	FP	openfn;		/* Open function */
	FP	closefn;	/* Close function */
	FP	execfn;		/* Execute function */
	FP	waitfn;		/* Completion wait function */
	FP	abortfn;	/* Abort function */
	FP	eventfn;	/* Event function */
#if TA_GP
	void	*gp;		/* Global pointer (gp) */
#endif
} T_DDEV;

/*
 * Open function:
 *	ER  openfn( ID devid, UINT omode, void *exinf )
 * Close function:
 *	ER  closefn( ID devid, UINT option, void *exinf )
 * Execute function:
 *	ER  execfn( T_DEVREQ *devreq, TMO tmout, void *exinf )
 * Completion wait function:
 *	INT waitfn( T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf )
 * Abort function:
 *	ER  abortfn( ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
 * Event function:
 *	INT eventfn( INT evttyp, void *evtinf, void *exinf )
 */

/*
 * Driver attribute
 */
#define TDA_OPENREQ	0x0001U	/* Every time open/close */
#define TDA_TMO_U	0x0002U	/* Timeout in microseconds */
#define TDA_DEV_D	0x0004U	/* 64 bits device */
 
/*
 * Device initial setting information
 */
typedef struct t_idev {
	ID	evtmbfid;	/* Message buffer ID for event notification */
} T_IDEV;

/*
 * Device request packet
 *	 I: Input parameter
 *	 O: Output parameter
 */
typedef struct t_devreq {
	struct t_devreq	*next;	/* I:Link to request packet (NULL:End) */
	void	*exinf;		/* X:Extended information */
	ID	devid;		/* I:Target device ID */
	INT	cmd:4;		/* I:Request command */
	BOOL	abort:1;	/* I:When executing abort request, TRUE */
	BOOL	nolock:1;	/* I:When lock (resident) is not required, TRUE */
	INT	rsv:26;		/* I:Reserved (always 0) */
	T_TSKSPC tskspc;	/* I:Task space of request task */
	W	start;		/* I:Start data number */
	W	size;		/* I:Request size */
	void	*buf;		/* I:Input/output buffer address */
	W	asize;		/* O:Result size */
	ER	error;		/* O:Result error */
} T_DEVREQ;
typedef struct t_devreq_d {
	struct t_devreq_d *next; /* I:Link to request packet (NULL:End) */
	void	*exinf;		/* X:Extended information */
	ID	devid;		/* I:Target device ID */
	INT	cmd:4;		/* I:Request command */
	BOOL	abort:1;	/* I:When executing abort request, TRUE */
	BOOL	nolock:1;	/* I:When lock (resident) is not required, TRUE */
	INT	rsv:26;		/* I:Reserved (always 0) */
	T_TSKSPC tskspc;	/* I:Task space of request task */
	D	start_d;	/* I:Start data number */
	W	size;		/* I:Request size */
	void	*buf;		/* I:Input/output buffer address */
	W	asize;		/* O:Result size */
	ER	error;		/* O:Result error */
} T_DEVREQ_D;

/*
 * Request command
 */
#define TDC_READ	1	/* Read request */
#define TDC_WRITE	2	/* Write request */

/*
 * Driver request event
 */
#define TDV_SUSPEND	(-1)	/* Suspend */
#define TDV_RESUME	(-2)	/* Resume */
#define TDV_CARDEVT	1	/* PC card event (Refer card manager) */
#define TDV_USBEVT	2	/* USB event     (Refer USB manager) */

/* ------------------------------------------------------------------------ */

/*
 * Definition for interface library automatic generation (mkiflib)
 */
/*** DEFINE_IFLIB
[INCLUDE FILE]
<tk/devmgr.h>

[PREFIX]
DEVICE
***/

/* [BEGIN SYSCALLS] */
/* Application interface */
IMPORT ID tk_opn_dev( CONST UB *devnm, UINT omode );
IMPORT ER tk_cls_dev( ID dd, UINT option );
IMPORT ID tk_rea_dev( ID dd, W start, void *buf, W size, TMO tmout );
IMPORT ER tk_srea_dev( ID dd, W start, void *buf, W size, W *asize );
IMPORT ID tk_wri_dev( ID dd, W start, CONST void *buf, W size, TMO tmout );
IMPORT ER tk_swri_dev( ID dd, W start, CONST void *buf, W size, W *asize );
IMPORT ID tk_wai_dev( ID dd, ID reqid, W *asize, ER *ioer, TMO tmout );
IMPORT INT tk_sus_dev( UINT mode );
IMPORT ID tk_get_dev( ID devid, UB *devnm );
IMPORT ID tk_ref_dev( CONST UB *devnm, T_RDEV *rdev );
IMPORT ID tk_oref_dev( ID dd, T_RDEV *rdev );
IMPORT INT tk_lst_dev( T_LDEV *ldev, INT start, INT ndev );
IMPORT INT tk_evt_dev( ID devid, INT evttyp, void *evtinf );

/* T-Kernel 2.0 */
IMPORT ID tk_rea_dev_du( ID dd, D start_d, void *buf, W size, TMO_U tmout_u );
IMPORT ER tk_srea_dev_d( ID dd, D start_d, void *buf, W size, W *asize );
IMPORT ID tk_wri_dev_du( ID dd, D start_d, CONST void *buf, W size, TMO_U tmout_u );
IMPORT ER tk_swri_dev_d( ID dd, D start_d, CONST void *buf, W size, W *asize );
IMPORT ID tk_wai_dev_u( ID dd, ID reqid, W *asize, ER *ioer, TMO_U tmout_u );

/* Device registration */
/* ALIGN_NO 0x0100 */
IMPORT ID tk_def_dev( CONST UB *devnm, CONST T_DDEV *ddev, T_IDEV *idev );
IMPORT ER tk_ref_idv( T_IDEV *idev );
/* [END SYSCALLS] */

#ifdef __cplusplus
}
#endif
#endif /* __TK_DEVMGR_H__ */
