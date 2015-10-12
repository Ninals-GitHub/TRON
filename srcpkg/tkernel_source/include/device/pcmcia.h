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
 *	pcmcia.h	PCMCIA card manager definition
 */

#ifndef	__DEVICE_PCMCIA_H__
#define	__DEVICE_PCMCIA_H__

#include <basic.h>
#include <tk/syscall.h>
#include <tk/syslib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Event type */
#define	CE_INSERT	1		/* Insert the card */
#define	CE_EJECT	2		/* Remove the card */
#define	CE_BATTERY	3		/* Card battery status change */

/* Response code */
#define	CR_NONE		0		/* Not an object card	*/
#define	CR_OWN		1		/* Object card:occupy */
#define	CR_SHARE	2		/* Object card:share  */

/* Card type:"FUNCID" tuple code */
#define	CK_MULTI	0		/* Multifunction */
#define	CK_MEMORY	1		/* Memory */
#define	CK_SERIAL	2		/* Serial port */
#define	CK_PARALLEL	3		/* Parallel port */
#define	CK_FIXED	4		/* Fixed disk */
#define	CK_VIDEO	5		/* Video */
#define	CK_NETWORK	6		/* Network 	*/
#define	CK_AIMS		7		/* AIMS */
#define	CK_SCSI		8		/* SCSI */
#define	CK_ANY		255		/* Any (illegal) */
#define	CK_NONE		(-1)		/* Non (deregistration)	*/

typedef	struct {			/* Card status */
	UW	kind:8;			/* Card type */
	UW	battery:2;		/* Battery status: memory card only */
	UW	wprotect:1;		/* Read-only: memory card only */
	UW	power:1;		/* Electric supply status */
	UW	client:1;		/* In-use status	*/
	UW	rsv:19;			/* Reserved */
} CardStat;

/* Map / Memory attribute */
#define	CA_IONOCHK	0x10000		/* There is no resource management of IO space	*/
#define	CA_IOMAP	0x8000		/* Map the IO space */
#define	CA_ATTRMEM	0x4000		/* Attribute memory specification	*/
#define	CA_WPROTECT	0x2000		/* Read-only */
#define	CA_16BIT	0x0800		/* 16 bit access specification */
#define	CA_IOCS16	0x0400		/* IOCS16 source */

#define	CA_WAIT0	0x0100		/* Additional WAIT 0 specification		*/
#define	CA_WAIT1	0x0101		/* Additional WAIT 1 specification		*/
#define	CA_WAIT2	0x0102		/* Additional WAIT 2 specification		*/
#define	CA_WAIT3	(CA_WAIT1 | CA_WAIT2)	/* Additional WAIT 3 specification	*/
#define	CA_ZWAIT	0x0104		/* Zero WAIT specification 		*/
#define	CA_SPEED	0x00FF		/* Access speed specification in 50 ns unit */

/* Power-supply control */
#define	CP_POWEROFF	0x00		/* Power-OFF			*/
#define	CP_POWERON	0x01		/* Power-ON			*/
#define	CP_SUSPEND	0x10		/* Suspend (power-OFF) */
#define	CP_RESUME	0x11		/* Resume (power-ON) */
#define	CP_SUSRIRES	0x12		/* Suspend (RI resume) */
#define	CP_POFFREQ(tm)	(((tm) << 16) + 0x99) /* Power-OFF after the specified time (second) */

/* Special card ID */
#define	TEST_CARDID	0x12345678	/* Special card ID for test */

/* Maximum size of tuple data */
#define	MAX_TUPLESZ	(255 + 2)

/* Card event */
typedef struct {
	ID		cardid;		/* Card ID	 */
	W		evtype;		/* Event type (CE_xxxx) */
	CardStat	cardstat;	/* Card status */
} CardReq;

/*
	Definition for interface library automatic creation (mkiflib)
*/
/*** DEFINE_IFLIB
[INCLUDE FILE]
<device/pcmcia.h>

[PREFIX]
PCM
***/

/*
	Card manager service function
*/
/* [BEGIN SYSCALLS] */
IMPORT	ER	pcRegClient(ID portid, W kind);
IMPORT	INT	pcGetTuple(ID cardid, W tuple, W order, UB* buf);
IMPORT	INT	pcMap(ID cardid, W offset, W len, UW attr, void **addr);
IMPORT	ER	pcReMap(ID cardid, ID mapid);
IMPORT	ER	pcUnMap(ID mapid);
IMPORT	ER	pcReadMem(ID cardid, W offset, W len, UW attr, void *buf);
IMPORT	ER	pcWriteMem(ID cardid, W offset, W len, UW attr, void *buf);
IMPORT	INT	pcGetStat(ID cardid);
IMPORT	INT	pcDefInt(ID cardid, T_DINT *dint, INTVEC vec, UW par);
IMPORT	ER	pcPowerCtl(ID cardid, UW power);
/* [END SYSCALLS] */

#ifdef __cplusplus
}
#endif
#endif	/* __DEVICE_PCMCIA_H__ */
