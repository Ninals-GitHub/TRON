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
        scancode.h     KB/PD real I/O driver: keyscan code definitions
 *
 */
/*
        one byte scan code (partial)
*/
#define	L_Alt		0x38
#define	NumLock		0x45
#define	NumLockLo	0x47
#define	NumLockHi	0x53

/*
        special keyscan code (two bytes code is mapped to one byte code)
*/
#define	PrScr		0x54
#define	R_Alt		0x80
#define	R_Ctl		0x81
#define	Pause		0x82
#define	Ins		0x83
#define	Del		0x84
#define	Home		0x85
#define	End		0x86
#define	PgUp		0x87
#define	PgDn		0x88
#define	U_Arw		0x89
#define	D_Arw		0x8a
#define	L_Arw		0x8b
#define	R_Arw		0x8c
#define	G_Div		0x8d
#define	G_Entr		0x8e
#define	T_7		0x8f
#define	T_8		0x90
#define	T_9		0x91
#define	T_MINUS		0x92
#define	T_4		0x93
#define	T_5		0x94
#define	T_6		0x95
#define	T_PLUS		0x96
#define	T_1		0x97
#define	T_2		0x98
#define	T_3		0x99
#define	T_0		0x9a
#define	T_PERIOD	0x9b
#define	L_Win		0x9c
#define	R_Win		0x9d
#define	W_Menu		0x9e
