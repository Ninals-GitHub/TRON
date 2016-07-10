/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */

/*
 *	ptimer.c
 *
 *	Physical timer
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <tk/ptimer.h>

/*
 * List of timers
 */
#if 0
LOCAL struct timerlist {
	INT	base;		/* base address of timer registers  */
	INTVEC	irq;		/* interrupt number */
	UINT	mode;		/* operation mode */
	FP	hdr;		/* user-defined interrupt hander */
	void*	exinf;		/* user-defined exntended information */
} TimerList[] = {
	{ 0xc0000100,	IV_IRQ(55) },	/* TI1 */
	{ 0xc0000200,	IV_IRQ(56) },	/* TI2 */
	{ 0xc0000300,	IV_IRQ(57) },	/* TI3 */
	{ 0xc0002000,	IV_IRQ(34) },	/* TG0 */
	{ 0xc0002100,	IV_IRQ(35) },	/* TG1 */
	{ 0xc0002200,	IV_IRQ(36) },	/* TG2 */
	{ 0xc0002300,	IV_IRQ(44) },	/* TG3 */
	{ 0xc0002400,	IV_IRQ(45) },	/* TG4 */
	{ 0xc0002500,	IV_IRQ(46) },	/* TG5 */
	{ 0xc0001000,	IV_IRQ(58) },	/* TW0 */
	{ 0xc0001100,	IV_IRQ(59) },	/* TW1 */
	{ 0xc0001200,	IV_IRQ(60) },	/* TW2 */
	{ 0xc0001300,	IV_IRQ(61) }	/* TW3 */
};
#endif

#define	N_TIMER		( sizeof(TimerList) / sizeof(TimerList[0]) )

/*
 * Timer
 *	register size W
 */
#define	TI(pn, ofs)	( TimerList[pn].base + (ofs) )

#define	TI_OP(pn)	TI(pn, 0x00)	/* RW timer operation */
#define	TI_CLR(pn)	TI(pn, 0x04)	/* -W clear timer */
#define	TI_SET(pn)	TI(pn, 0x08)	/* RW set timer value */
#define	TI_RCR(pn)	TI(pn, 0x0c)	/* R- current counter */
#define	TI_SCLR(pn)	TI(pn, 0x14)	/* RW watch out for setting timer value */

#define	TO_EN		0x0004		/* enable TOUT */
#define	TSTART		0x0002		/* start counting */
#define	TM_EN		0x0001		/* enable timer */

#define	TCR_CLR		0x0002		/* clear counter */

#define	TM_SCLR		0x0001		/* warch out for setting timer value */

/*
 * Supply clock
 *	register size W
 */
#define	ASMU(n)		( 0xc0110000 + (n) )

#define	TI0TIN_SEL	ASMU(0x0138)	/* set TI0/TW0 TIN */
#define	TI1TIN_SEL	ASMU(0x013c)	/* set TI1/TW1 TIN */
#define	TI2TIN_SEL	ASMU(0x0140)	/* set TI2/TW2 TIN */
#define	TI3TIN_SEL	ASMU(0x0144)	/* set TI3/TW3 TIN */
#define	TGnTIN_SEL	ASMU(0x0148)	/* set TG0-5   TIN */

#define	DIVTIMTIN	ASMU(0x014c)	/* set timer clock divisor */
#define	GCLKCTRL3	ASMU(0x01cc)	/* set clock gate */
#define	GCLKCTRL3ENA	ASMU(0x01d0)	/* enable write */

#define	TITIN_PLL3	0		/* PLL3 divided by DIVTIMTIN */
#define	TITIN_32768	1		/* 32.768 KHz */
#define	TITIN_32K	2		/* 32 KHz */

#define	TIN_GCK(pn)	( 2 << (pn) )	/* TIN gate */

/*
 * Input clock
 *	PLL3 divided by DIVTIMTIN
 */
#define	PLL3_CLK	229376000	/* Hz */

#define	D0(d)		( (d) & 0x7 )		/* DIV0TIMTIN */
#define	D1(d)		( ((d) >> 4) & 0xf )	/* DIV1TIMTIN */

#define	TIN_CLK(d)	( PLL3_CLK / ((1 << D0(d)) * (D1(d) + 1)) )

/* ------------------------------------------------------------------------ */

/*
 * Initialize timer
 */
#if 0
LOCAL void initialize_timer( void )
{
	return;
}
#endif

/*
 * Check the physical timer number
 */
#if 0
LOCAL INT CHK_PTMRNO( UINT ptmrno )
{
	return 0;
}
#endif

/* ------------------------------------------------------------------------ */

/*
 * Timer interrupt handler
 */
#if 0
LOCAL void timer_inthdr( UINT dintno, void *sp )
{
	return;
}
#endif

/*
 * Start physical timer
 */
EXPORT ER StartPhysicalTimer( UINT ptmrno, UW limit, UINT mode )
{
	return E_OK;
}

/*
 * Stop physical timer
 */
EXPORT ER StopPhysicalTimer( UINT ptmrno )
{
	return E_OK;
}

/*
 * Obtain the count of the physical timer
 */
EXPORT ER GetPhysicalTimerCount( UINT ptmrno, UW *p_count )
{
	return E_OK;
}

/*
 * Definition of physical timer hander
 */
EXPORT ER DefinePhysicalTimerHandler( UINT ptmrno, CONST T_DPTMR *pk_dptmr )
{
	return E_OK;
}

/*
 * Obtain the configuration information of physical timer
 */
EXPORT ER GetPhysicalTimerConfig( UINT ptmrno, T_RPTMR *pk_rptmr )
{
	return E_OK;
}
