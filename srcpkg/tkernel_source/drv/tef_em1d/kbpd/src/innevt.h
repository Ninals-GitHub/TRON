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
 *	innevt.h
 *
 *       KB/PD device manager
 *       definitions of additional events used only inside manager
 */

/*
 * internal event type
 */
typedef enum {
	IE_NULL		= 0x0000,	/* no event */
	IE_KEYDOWN	= 0x0001,	/* key down */
	IE_KEYUP	= 0x0002,	/* key up */
	IE_S_PRESS	= 0x0010,	/* shift press */
	IE_S_REL	= 0x0020,	/* shift release */
	IE_S_SCLK	= 0x0030,	/* shift single click */
	IE_S_DCLK	= 0x0040,	/* shift double click */
	IE_KEYERR	= 0x0009,	/* key input error */
				/* following events can be generated simultaneously */
	IE_MBUTDOWN	= 0x0100,	/* PD main button down */
	IE_MBUTUP	= 0x0200,	/* PD main button up */
	IE_SBUTDOWN	= 0x0400,	/* PD subbutton down */
	IE_SBUTUP	= 0x0800,	/* PD subbutton up */
	IE_PDMOVE	= 0x1000,	/* move PD */

	IE_MBUT		= IE_MBUTDOWN | IE_MBUTUP,
	IE_SBUT		= IE_SBUTDOWN | IE_SBUTUP,
	IE_BUTDOWN	= IE_MBUTDOWN | IE_SBUTDOWN,
	IE_BUTUP	= IE_MBUTUP   | IE_SBUTUP,
	IE_PDBUT	= IE_MBUT | IE_SBUT
} InnEvtType;

/*
 * internal event structure
 */
typedef struct {
	InnEvtType	type;
	union {
		struct {
			KeyTop		keytop;	/* key top information */
			KpMetaBut	meta;	/* meta key status */
		} key;
		struct {
			ShiftKeyKind	kind;	/* type of shift key */
		} sft;
		struct {
			KpPdInStat	stat;	/* PD status */
			H		x;	/* PD postion / displacement */
			H		y;
		} pd;
	} i;
} InnerEvent;
