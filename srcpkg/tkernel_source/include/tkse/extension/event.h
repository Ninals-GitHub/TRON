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
 *	event.h (extension)
 *
 *	Event management
 */

#ifndef __EXTENSION_EVENT_H__
#define __EXTENSION_EVENT_H__

#include <basic.h>
#include "typedef.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Point
 */
typedef struct point {
	H	x;	/* Horizontal coordinate value	*/
	H	y;	/* Vertical coordinate value	*/
} PNT;

/*
 *	Event-specific data
 */
typedef union {
	struct {			/* EV_KEYUP,EV_KEYDWN,EV_AUTKEY	*/
		UH	keytop;		/* Key top code			*/
		TC	code;		/* Character code		*/
	} key;
	struct {			/* EV_DEVICE			*/
		H	kind;		/* Device event type (DE_xxx)	*/
		H	devno;		/* Device number		*/
	} dev;
	W	info;			/* Other event data		*/
} EVDATA;

/*
 *	Event
 */
typedef struct {
	W	type;			/* Event type				*/
	UW	time;			/* Event occurrence time		*/
	PNT	pos;			/* PD position at occurrence of event	*/
	EVDATA	data;			/* Event-specific data			*/
	UW	stat;			/* Metakey, PD button state		*/
	UB	exdat[16];		/* Extended Data 			*/
} EVENT;

/*
 *	Event type
 */
#define	EV_NULL		0		/* Null event 			*/
#define	EV_BUTDWN	1		/* Button down 			*/
#define	EV_BUTUP	2		/* Button up 			*/
#define	EV_KEYDWN	3		/* Key down			*/
#define	EV_KEYUP	4		/* Key up			*/
#define	EV_AUTKEY	5		/* Automatic key repeat		*/
#define	EV_DEVICE	6		/* Device event			*/
#define	EV_EXDEV	7		/* Extended Device event	*/
#define	EV_APPL1	8		/* Application event #1		*/
#define	EV_APPL2	9		/* Application event #2 	*/
#define	EV_APPL3	10		/* Application event #3 	*/
#define	EV_APPL4	11		/* Application event #4 	*/
#define	EV_APPL5	12		/* Application event #5 	*/
#define	EV_APPL6	13		/* Application event #6 	*/
#define	EV_APPL7	14		/* Application event #7 	*/
#define	EV_APPL8	15		/* Application event #8 	*/

/*
 *	Event mask
 */
#define	EM_NULL		0x0000
#define	EM_ALL		0x7fff
#define	EM_BUTDWN	0x0001
#define	EM_BUTUP	0x0002
#define	EM_KEYDWN	0x0004
#define	EM_KEYUP	0x0008
#define	EM_AUTKEY	0x0010
#define	EM_DEVICE	0x0020
#define	EM_EXDEV	0x0040
#define	EM_APPL1	0x0080
#define	EM_APPL2	0x0100
#define	EM_APPL3	0x0200
#define	EM_APPL4	0x0400
#define	EM_APPL5	0x0800
#define	EM_APPL6	0x1000
#define	EM_APPL7	0x2000
#define	EM_APPL8	0x4000

/*
 *	Metakey, PD button state
 */
#define	ES_BASICBTN	0x00000003U	/* PD basic button		*/
#define ES_META		0x001FFFFCU	/* Metakey			*/
#define	ES_NODSP	0x00200000U	/* PD type			*/
#define	ES_PDSIM	0x00C00000U	/* PD state 			*/
#define	ES_EXTBTN	0xFF000000U	/* PD extended button		*/

/*
 *	Device event type
 *	Defined based on T-Kernel device event type (TDEvtTyp)
 */
#define	DE_unknown	0x00U		/* Not defined				*/
#define	DE_MOUNT	0x01U		/* Insert media				*/
#define	DE_EJECT	0x02U		/* Eject media				*/
#define	DE_ILLMOUNT	0x03U		/* Insert media improperly		*/
#define	DE_ILLEJECT	0x04U		/* Eject media improperly		*/
#define	DE_REMOUNT	0x05U		/* Insert media again			*/
#define	DE_BATLOW	0x06U		/* Warning on remaining battery level	*/
#define	DE_BATFAIL	0x07U		/* Battery trouble			*/
#define	DE_REQEJECT	0x08U		/* Request media ejection		*/
#define	DE_PDBUT	0x11U		/* Change in PD button state		*/
#define	DE_PDMOVE	0x12U		/* Shift in PD position			*/
#define	DE_PDSTATE	0x13U		/* Change in PD state			*/
#define	DE_PDEXT	0x14U		/* PD extended event			*/
#define	DE_KEYDOWN	0x21U		/* Key down				*/
#define	DE_KEYUP	0x22U		/* Key up				*/
#define	DE_KEYMETA	0x23U		/* Change in metaky state		*/
#define	DE_POWEROFF	0x31U		/* Power switch off			*/
#define	DE_POWERLOW	0x32U		/* Warning on remaining power level	*/
#define	DE_POWERFAIL	0x33U		/* Power trouble			*/
#define	DE_POWERSUS	0x34U		/* Automatic suspend			*/
#define	DE_POWERUPTM	0x35U		/* Update clock				*/
#define	DE_CKPWON	0x41U		/* Notify automatic power on		*/

/*
 *	Parameter for put_evt()
 */
#define	EP_NONE		0x0000		/* Not change time, pos and stat		*/
#define	EP_POS		0x0001		/* Set current PD position to pos.		*/
#define	EP_STAT		0x0002		/* Set current metakey state to stat		*/
#define	EP_TIME		0x0004		/* Set current time to time			*/
#define	EP_ALL		0x0007		/* Set current values to pos,stat and time	*/

/*
 *	PD attribute
 */
#define	PD_WHEEL	0x4000		/* Wheel (0: disabled)		*/
#define	PD_QPRESS	0x2000		/* Quick press (0: disabled)	*/
#define	PD_REV		0x1000		/* Flip horizontally		*/
#define	PD_ACMSK	0x0e00		/* Acceleration (mask)		*/
#define	PD_ABS		0x0100		/* Absolute coordinate type	*/
#define	PD_REL		0x0000		/* Relative coordinate type	*/
#define	PD_SCMSK	0x00f0		/* Scanning speed (mask)	*/
#define	PD_SNMSK	0x000f		/* Sensitivity (mask)		*/

/*
 *	Keyboard ID
 */
typedef struct {
	UH	kind;			/* Keyboard type			*/
	UH	maker;			/* Manufacturer ID			*/
	UB	id[4];			/* Manufacturer-dependent keyboard ID	*/
} KBD_ID;

/*
 *	Key map
 */
#ifndef __keymap__
#define __keymap__
#define	KEYMAX		256

typedef UB	KeyMap[KEYMAX / 8];
#endif /* __keymap__ */

/*
 * Definitions for automatic generation of interface library (mkiflib)
 */
/*** DEFINE_IFLIB
[INCLUDE FILE]
<extension/event.h>

[PREFIX]
EM
***/

/*
 * Event management system call
 */
/* [BEGIN SYSCALLS] */
IMPORT WER	tkse_get_evt(W t_mask, EVENT *evt, W opt);
IMPORT ER	tkse_put_evt(EVENT *evt, W opt);
IMPORT ER	tkse_clr_evt(W t_mask, W last_mask);
IMPORT WER	tkse_get_pdp(PNT *pos);
/* RESERVE_NO */
IMPORT ER	tkse_get_etm(UW* time);
/* RESERVE_NO */
IMPORT WER	tkse_chg_emk(W mask);
IMPORT ER	tkse_set_krp(W offset, W interval);
IMPORT ER	tkse_get_krp(W* offset, W* interval);
IMPORT ER	tkse_set_krm(KeyMap keymap);
IMPORT ER	tkse_get_krm(KeyMap keymap);
/* RESERVE_NO */
/* RESERVE_NO */
/* RESERVE_NO */
/* RESERVE_NO */
/* RESERVE_NO */
/* RESERVE_NO */
IMPORT ER	tkse_req_evt(W t_mask);
IMPORT WER	tkse_las_evt(W t_mask);
/* [END SYSCALLS] */

#ifdef __cplusplus
}
#endif
#endif /* __EXTENSION_EVENT_H__ */
