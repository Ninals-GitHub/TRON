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
 *	kbpd.h
 *
 *       KB/PD driver
 */

#ifndef	__DEVICE_KBPD_H__
#define	__DEVICE_KBPD_H__

#include <basic.h>
#include <tk/devmgr.h>

#ifdef __cplusplus
extern "C" {
#endif

/* KBPD attribute data number */
typedef	enum {
	DN_KPEVENT	= TDN_EVENT,	/* message buffer for event notification
						data: ID		RW */
	DN_KPINPUT	= -100,		/* input mail box
						data: ID		R  */
	DN_KPSTAT	= -101,		/* KB/PD status
						data: KPstat		RW */
	DN_KEYMAP	= -102,		/* keymap
						data: KeyMap		R  */
	DN_KEYTAB	= -103,		/* keytable
						data: KeyTab		RW */
	DN_KEYMODE	= -104,		/* key mode
						data: KeyMode		RW */
	DN_PDMODE	= -105,		/* PD mode
						data: PdMode		RW */
	DN_PDRANGE	= -106,		/* PD range
						data: PdRange		RW */
	DN_PDSIM	= -107,		/* PD simulation
						data: W			RW */
	DN_PDSIMINH	= -108,		/* temporary prohibition of PD simulation
						data: BOOL		RW */
	DN_KEYID	= -109,		/* keyboard ID
						data: UW		RW */
	DN_KPMETABUT	= -110,		/* meta key / button status
						data: MetaBut[2]	 W */
	DN_KEYDEF_S	= -200,		/* keyboard definition 1 (-200 - -327) */
	DN_KEYDEF_E	= -327,		/*	data: KeyDef		RW */
	DN_KEYDEF2_S	= -400,		/* keyboard definition 2 (-400 - - 527) */
	DN_KEYDEF2_E	= -527		/*	data: KeyDef		RW */
} KPDataNo;

/* DN_KPSTAT:   KB/PD status (RW) */
typedef enum {
	HiraMode	= 0,	/* Japanese Hiragana */
	AlphaMode	= 1,	/* English (lower case) */
	KataMode	= 2,	/* Japanese katakana */
	CapsMode	= 3	/* English (upper case) */
} InputMode;

typedef enum {
	PdSim_Off	= 0,	/* PD simulation OFF */
	PdSim_Std	= 1,	/* standard PD simulation */
	PdSim_MainBut	= 2,	/* main button PD simulation */
	PdSim_TenKey	= 3	/* ten key PD simulation */
} PdSimMode;

typedef struct {
#if BIGENDIAN
	UW	rsv1:8;		/* reserved (0)                */
	UW	pdsim:2;	/* PD simulation (PdSimMode) */
	UW	nodsp:1;	/* hide pointer      */
	UW	rsv2:3;		/* reserved (0)                */
	UW	kbsel:1;	/* select keyboard     */
	UW	han:1;		/* hankaku mode               */

	UW	tcmd:1;		/* command temporary shift      */
	UW	text:1;		/* expansion temporary shift */
	UW	trsh:1;		/* shift right temporary shift    */
	UW	tlsh:1;		/* shift left temporary shift    */

	UW	lcmd:1;		/* command simple lock     */
	UW	lext:1;		/* expansion simple lock      */
	UW	lrsh:1;		/* shift right simple lock    */
	UW	llsh:1;		/* left shift simple lock */

	UW	cmd:1;		/* command shift                */
	UW	ext:1;		/* extended shift                */
	UW	rsh:1;		/* right shift          */
	UW	lsh:1;		/* left shift          */

	UW	mode:2;		/* key input mode (InputMode) */

	UW	sub:1;		/* subbutton                */
	UW	main:1;		/* main button                */
#else
	UW	main:1;		/* main button                */
	UW	sub:1;		/* subbutton                */

	UW	mode:2;		/* key input mode (InputMode) */

	UW	lsh:1;		/* left shift          */
	UW	rsh:1;		/* right shift          */
	UW	ext:1;		/* extended shift                */
	UW	cmd:1;		/* command shift                */

	UW	llsh:1;		/* left shift simple lock */
	UW	lrsh:1;		/* shift right simple lock    */
	UW	lext:1;		/* expansion simple lock      */
	UW	lcmd:1;		/* command simple lock     */

	UW	tlsh:1;		/* shift left temporary shift    */
	UW	trsh:1;		/* shift right temporary shift    */
	UW	text:1;		/* expansion temporary shift */
	UW	tcmd:1;		/* command temporary shift      */

	UW	han:1;		/* hankaku mode               */
	UW	kbsel:1;	/* select keyboard     */
	UW	rsv2:3;		/* reserved (0)                */
	UW	nodsp:1;	/* hide pointer      */
	UW	pdsim:2;	/* PD simulation (PdSimMode) */
	UW	rsv1:8;		/* reserved (0)                */
#endif
} MetaBut;

typedef struct {
	H	xpos;		/* x coordinate             */
	H	ypos;		/* y coordinate               */
	MetaBut	stat;		/* meta / button status    */
} KPStat;

/* DN_KEYMAP:  keymap (R) */
#ifndef __keymap__
#define __keymap__
#define	KEYMAX		256

typedef	UB	KeyMap[KEYMAX/8];
#endif /* __keymap__ */

/* DN_KEYTAB:   keytable (RW) */
#ifndef __keytab__
#define __keytab__
#define	KCTSEL		64
#define	KCTMAX		4000

typedef struct {
	W	keymax;		/* maximum number of keys      */
	W	kctmax;		/* real number of keymaps      */
	UH	kctsel[KCTSEL];	/* number of translation table             */
	UH	kct[KCTMAX];	/* translation table itself (variable length)    */
} KeyTab;
#endif /* __keytab__ */

/* DN_KEYDEF:   keyboard definition (RW) */
#define	DN_KEYDEF(kid)		( DN_KEYDEF_S - (kid) )
#define	DN_KEYDEF2(kid)		( DN_KEYDEF2_S - (kid) )
typedef struct {
	W	keytopofs;	/* offset value */
	KeyTab	keytab;		/* key table (variable length) */
} KeyDef;

/* keyboard ID (0x00 - 0x7F) */
#define	KID_unknown	0x00	/* undefined keyboard */
#define	KID_TRON_JP	0x01	/* Japanese TRON keyboard */
#define	KID_IBM_EG	0x40	/* IBM 101 (and friends) English keyboard */
#define	KID_IBM_JP	0x41	/* IBM 106 (and friends) Japanese keyboard */
 
/* DN_KEYMODE:  key mode (RW) */
typedef struct {
	MSEC	ontime;		/* ON effective interval     */
	MSEC	offtime;	/* OFF effective interval      */
	MSEC	invtime;	/* invalid interval          */
	MSEC	contime;	/* simultaneous key press interval        */
	MSEC	sclktime;	/* short click interval      */
	MSEC	dclktime;	/* double click interval      */
	BOOL	tslock;		/* temporary shift      */
} KeyMode;

#define	KB_MAXTIME	10000

/* DN_PDMODE:   PD mode (RW) */
typedef struct {
#if BIGENDIAN
	UW	rsv1:17;	/* reserved (0)                */
	UW	wheel:1;	/* wheel          */
	UW	qpress:1;	/* quick press      */
	UW	reverse:1;	/* left and right reversal          */
	UW	accel:3;	/* accelleration         */
	UW	absolute:1;	/* absolute / relative                */
	UW	rate:4;		/* scan speed                */
	UW	sense:4;	/* sensitivity                    */
#else
	UW	sense:4;	/* sensitivity                    */
	UW	rate:4;		/* scan speed                */
	UW	absolute:1;	/* absolute / relative                */
	UW	accel:3;	/* accelleration         */
	UW	reverse:1;	/* left and right reversal          */
	UW	qpress:1;	/* quick press      */
	UW	wheel:1;	/* wheel          */
	UW	rsv1:17;	/* reserved (0)                */
#endif
} PdAttr;

typedef struct {
	MSEC	ontime;		/* ON effective interval               */
	MSEC	offtime;	/* OFF effective interval      */
	MSEC	invtime;	/* invalid interval          */
	MSEC	timeout;	/* timeout period      */
	PdAttr	attr;		/* PD attribute          */
} PdMode;

#define	PD_MAXTIME	10000

/* DN_PDRANGE:  PD range (RW) */
typedef struct {
	H	xmax;		/* maximum x coordinate       */
	H	ymax;		/* maximum y coordinate        */
} PdRange;

/* event notification */
typedef struct {
	T_DEVEVT	h;	/* standard header               */
	UH		keytop;	/* key top code     */
	UH		code;	/* character code                */
	MetaBut		stat;	/* meta key status                */
} KeyEvt;

typedef struct {
	T_DEVEVT	h;	/* standard header               */
	KPStat		stat;	/* PD position / button status    */
} PdEvt;

typedef struct {
	T_DEVEVT	h;	/* standard header               */
	H		wheel;	/* wheel rotation amount      */
	H		rsv[3];	/* reserved (0)                */
} PdEvt2;

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_KBPD_H__ */
