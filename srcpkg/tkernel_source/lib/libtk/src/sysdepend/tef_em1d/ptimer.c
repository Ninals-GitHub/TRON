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
LOCAL void initialize_timer( void )
{
	UW	m;

	/* select clock */
	out_w(TI0TIN_SEL, in_w(TI0TIN_SEL) & ~0x00030000);
	out_w(TI1TIN_SEL, 0);
	out_w(TI2TIN_SEL, 0);
	out_w(TI3TIN_SEL, 0);
	out_w(TGnTIN_SEL, 0);

	/* close clock gate */
	m = 0x00003ffe;
	out_w(GCLKCTRL3ENA, in_w(GCLKCTRL3ENA) | m);
	out_w(GCLKCTRL3, in_w(GCLKCTRL3) & ~m);
}

/*
 * Check the physical timer number
 */
LOCAL INT CHK_PTMRNO( UINT ptmrno )
{
	static BOOL	init = FALSE;
	UINT	imask;

	if ( (ptmrno < 1) || (ptmrno > N_TIMER) ) return E_PAR;

	if ( !init ) {
		DI(imask);

		/* Initialize the timer on the first visit */
		if ( !init ) initialize_timer();
		init = TRUE;

		EI(imask);
	}

	return ptmrno - 1;
}

/* ------------------------------------------------------------------------ */

/*
 * Timer interrupt handler
 */
#if 0
LOCAL void timer_inthdr( UINT dintno, void *sp )
{
	struct timerlist *tp;
	INT	pn;

	for ( pn = 0; pn < N_TIMER; ++pn ) {
		tp = &TimerList[pn];
		if ( DINTNO(tp->irq) == dintno ) break;
	}
	if ( pn >= N_TIMER ) return;

	ClearInt(tp->irq);

	if ( tp->mode == TA_ALM_PTMR ) {
		/* stop counting */
		out_w(TI_OP(pn), 0);

		/* stop clock */
		out_w(GCLKCTRL3, in_w(GCLKCTRL3) & ~TIN_GCK(pn));
	}

	if ( tp->hdr != NULL ) {
		/* call user-defined interrupt handler */
		(*tp->hdr)(tp->exinf);
	}
}
#endif

/*
 * Start physical timer
 */
EXPORT ER StartPhysicalTimer( UINT ptmrno, UW limit, UINT mode )
{
	INT	pn;
	UINT	imask;
	T_DINT	dint;
	ER	err;

	if ( (mode != TA_ALM_PTMR) && (mode != TA_CYC_PTMR) ) return E_PAR;

	/* setting below 2 is impossible ??? */
	if ( limit <= 2 ) return E_PAR;

	pn = CHK_PTMRNO(ptmrno);
	if ( pn < E_OK ) return pn;

	/* stop timer */
	out_w(TI_OP(pn), 0);

	TimerList[pn].mode = mode;

	/* set up timer interrupt handler */
	dint.intatr = TA_HLNG;
	dint.inthdr = timer_inthdr;
	err = tk_def_int(DINTNO(TimerList[pn].irq), &dint);
	if ( err < E_OK ) return err;

	SetIntMode(TimerList[pn].irq, IM_ENA);
	ClearInt(TimerList[pn].irq);

	DI(imask);

	/* start feeding clock */
	out_w(GCLKCTRL3, in_w(GCLKCTRL3) | TIN_GCK(pn));

	/* enable timer */
	out_w(TI_OP(pn), TM_EN);
	while ( (in_w(TI_SCLR(pn)) & TM_SCLR) != 0 );
	WaitUsec(1);

	/* set counter */
	out_w(TI_SET(pn), limit);

	/* start counting */
	out_w(TI_OP(pn), TO_EN|TSTART|TM_EN);

	EI(imask);

	/* enable timer interrupt */
	EnableInt(TimerList[pn].irq);

	return E_OK;
}

/*
 * Stop physical timer
 */
EXPORT ER StopPhysicalTimer( UINT ptmrno )
{
	INT	pn;
	UINT	imask;

	pn = CHK_PTMRNO(ptmrno);
	if ( pn < E_OK ) return pn;

	/* Stop timer interrupt */
	DisableInt(TimerList[pn].irq);

	DI(imask);

	/* Stop counting */
	out_w(TI_OP(pn), in_w(TI_OP(pn)) & TM_EN);

	/* Stop clock */
	out_w(GCLKCTRL3, in_w(GCLKCTRL3) & ~TIN_GCK(pn));

	EI(imask);

	return E_OK;
}

/*
 * Obtain the count of the physical timer
 */
EXPORT ER GetPhysicalTimerCount( UINT ptmrno, UW *p_count )
{
	INT	pn;

	pn = CHK_PTMRNO(ptmrno);
	if ( pn < E_OK ) return pn;

	if ( (in_w(TI_OP(pn)) & TM_EN) == 0 ) {
		*p_count = 0;
	} else {
		*p_count = in_w(TI_RCR(pn));
	}

	return E_OK;
}

/*
 * Definition of physical timer hander
 */
EXPORT ER DefinePhysicalTimerHandler( UINT ptmrno, CONST T_DPTMR *pk_dptmr )
{
	INT	pn;
	UINT	imask;

	pn = CHK_PTMRNO(ptmrno);
	if ( pn < E_OK ) return pn;

	if ( pk_dptmr != NULL ) {
		if ( (pk_dptmr->ptmratr & ~(TA_ASM|TA_HLNG)) != 0 )
						return E_PAR;

		DI(imask);
		TimerList[pn].exinf = pk_dptmr->exinf;
		TimerList[pn].hdr   = pk_dptmr->ptmrhdr;
		EI(imask);
	} else {
		TimerList[pn].hdr   = NULL;
	}

	return E_OK;
}

/*
 * Obtain the configuration information of physical timer
 */
EXPORT ER GetPhysicalTimerConfig( UINT ptmrno, T_RPTMR *pk_rptmr )
{
	INT	pn;
	UW	d;

	pn = CHK_PTMRNO(ptmrno);
	if ( pn < E_OK ) return pn;

	d = in_w(DIVTIMTIN);

	pk_rptmr->ptmrclk  = TIN_CLK(d);
	pk_rptmr->maxcount = 0xffffffff;
	pk_rptmr->defhdr   = TRUE;

	return E_OK;
}
