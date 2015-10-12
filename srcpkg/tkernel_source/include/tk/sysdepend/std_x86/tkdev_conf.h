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
 *	tkdev_conf.h (EM1-D512)
 *	Target System Configuration
 */

#ifndef _TKDEV_CONF_
#define _TKDEV_CONF_
/* Also included from assembler source */

/*
 * Timer
 *	register size  W
 */
#define	TI(n)		( 0xc0000000 + (n) )	/* TI0 */

#define	TI_OP		TI(0x00)	/* RW timer operation */
#define	TI_CLR		TI(0x04)	/* -W clear timer */
#define	TI_SET		TI(0x08)	/* RW set timer value */
#define	TI_RCR		TI(0x0c)	/* R- current counter */
#define	TI_SCLR		TI(0x14)	/* RW watch out for setting timer value */

#define	TO_EN		0x0004		/* enable TOUT */
#define	TSTART		0x0002		/* start counting */
#define	TM_EN		0x0001		/* enable timer */
 
#define	TCR_CLR		0x0002		/* clear counter */

#define	TM_SCLR		0x0001		/* watch out for setting timer value */

/*
 * Supply clock
 *	register size W
 */
#define	ASMU(n)		( 0xc0110000 + (n) )

#define	TI0TIN_SEL	ASMU(0x0138)	/* set TI0/TW0 TIN */
#define	DIVTIMTIN	ASMU(0x014c)	/* set timer clock divisor */
#define	GCLKCTRL3	ASMU(0x01cc)	/* set clock gate */
#define	GCLKCTRL3ENA	ASMU(0x01d0)	/* enable write */

#define	TITIN_PLL3	0		/* PLL3 divided by DIVTIMTIN */
#define	TITIN_32768	1		/* 32.768 KHz */
#define	TITIN_32K	2		/* 32 KHz */

#define	TI0_TIN_GCK	0x00000001	/* TI0 TIN gate */

/*
 * Input clock
 *	PLL3 divided by DIVTIMTIN
 */
#define	PLL3_CLK	229376000	/* Hz */

#define	D0(d)		( (d) & 0x7 )		/* DIV0TIMTIN */
#define	D1(d)		( ((d) >> 4) & 0xf )	/* DIV1TIMTIN */

#define	TIN_CLK(d)	( PLL3_CLK / ((1 << D0(d)) * (D1(d) + 1)) )

/*
 * TI0 timer interrupt
 */
#define	IRQ_TIMER	54			/* IRQ number */
#define	VECNO_TIMER	( EIT_IRQ(IRQ_TIMER) )	/* interrupt vector number */

#define	IRQM(irq)	( 1 << ((irq) % 32) )	/* IRQ bit mask */

/*
 * Interrupt controller
 *	register size W
 */
#define	AINT(n)		( 0xc0020000 + (n) )	/* ACPU Interrupt */
#define	SINT(n)		( 0xcc010000 + (n) )	/* ACPU Secure Interrupt */

#define	IT0_IEN0	AINT(0x0000)	/* RW ACPU interrupt enable  0 */
#define	IT0_IEN1	AINT(0x0004)	/* RW ACPU interrupt enable 1 */
#define	IT0_IEN2	AINT(0x0100)	/* RW ACPU interrupt enable 2 */
#define	IT0_IDS0	AINT(0x0008)	/* -W ACPU interrupt disable 0 */
#define	IT0_IDS1	AINT(0x000C)	/* -W ACPU interrupt disable 1 */
#define	IT0_IDS2	AINT(0x0104)	/* -W ACPU interrupt disable 2 */
#define	IT0_RAW0	AINT(0x0010)	/* R- ACPU interrupt Raw status 0 */
#define	IT0_RAW1	AINT(0x0014)	/* R- ACPU interrupt Raw status 1 */
#define	IT0_RAW2	AINT(0x0108)	/* R- ACPU interrupt Raw status 2 */
#define	IT0_MST0	AINT(0x0018)	/* R- ACPU interrupt mask / status 0 */
#define	IT0_MST1	AINT(0x001C)	/* R- ACPU interrupt mask / status 1 */
#define	IT0_MST2	AINT(0x010C)	/* R- ACPU interrupt mask / status 2 */
#define	IT0_IIR		AINT(0x0024)	/* -W ACPU interrupt status / reset */
#define	IT0_FIE		AINT(0x0080)	/* RW ACPU FIQ enable interrupt */
#define	IT0_FID		AINT(0x0084)	/* -W ACPU FIQ disable interrupt */
#define	IT_PINV_SET0	AINT(0x0300)	/* RW enable inverted logic for interrupt input 0 */
#define	IT_PINV_SET1	AINT(0x0304)	/* RW enable inverted logic for interrupt input 1 */
#define	IT_PINV_SET2	AINT(0x0308)	/* RW enable inverted logic for interrupt input 2 */
#define	IT_PINV_CLR0	AINT(0x0310)	/* -W disable inverted logic for interrupt input 0 */
#define	IT_PINV_CLR1	AINT(0x0314)	/* -W disable inverted logic for interrupt input 1 */
#define	IT_PINV_CLR2	AINT(0x0318)	/* -W disable inverted logic for interrupt input 2 */
#define	IT_LIIS		AINT(0x0320)	/* -W internal interrupt status / set */
#define	IT_LIIR		AINT(0x0324)	/* -W internal interrupt status / reset */

#define	IT0_IENS0	SINT(0xE200)	/* RW ACPU Secure enable interrupt 0 */
#define	IT0_IENS1	SINT(0xE204)	/* RW ACPU Secure enable interrupt 1 */
#define	IT0_IENS2	SINT(0xE208)	/* RW ACPU Secure enable interrupt 2 */
#define	IT0_IDSS0	SINT(0xE20C)	/* -W ACPU Secure disable interrupt 0 */
#define	IT0_IDSS1	SINT(0xE210)	/* -W ACPU Secure disable interrupt 1 */
#define	IT0_IDSS2	SINT(0xE214)	/* -W ACPU Secure disable interrupt 2 */

/*
 * GIO interrupt
 *  register size  W
 */
#define	GIO(b, n)	( (b) + (n) )		/* General I/O */
#define	_L		( 0xc0050000 + 0x000 )
#define	_H		( 0xc0050000 + 0x040 )
#define	_HH		( 0xc0050000 + 0x080 )
#define	_HHH		( 0xc0050000 + 0x200 )

#define	GIO_IIA(b)	GIO(b,0x0014)	/* RW enable interrupt ??? */
#define	GIO_IEN(b)	GIO(b,0x0018)	/* -W enable interrupt */
#define	GIO_IDS(b)	GIO(b,0x001C)	/* -W disable interrupt */
#define	GIO_IIM(b)	GIO(b,0x001C)	/* R- enable interrupt ??? */
#define	GIO_RAW(b)	GIO(b,0x0020)	/* R- interrupt Raw status */
#define	GIO_MST(b)	GIO(b,0x0024)	/* R- interrupt mask / status */
#define	GIO_IIR(b)	GIO(b,0x0028)	/* -W reset the cause of interrupt */
#define	GIO_GSW(b)	GIO(b,0x003C)	/* RW connected to GIO_INT_FIQ pin */
#define	GIO_IDT(n,b)	GIO(b,0x0100+(n)*4) /* RW interrupt detection method  0-3 */
#define	GIO_RAWBL(b)	GIO(b,0x0110)	/* R- edge-triggered interrupt status L */
#define	GIO_RAWBH(b)	GIO(b,0x0114)	/* R- edge-triggered interrupt status H */
#define	GIO_IRBL(b)	GIO(b,0x0118)	/* -W clear the cause of edge-triggered interrupt L */
#define	GIO_IRBH(b)	GIO(b,0x011C)	/* -W clear the cause of edge-tirggered interrupt H */

/*
 * Timer interrupt level
 *	No meaning since EM1-D512 does not have a mechanism of prioritized interrupt.
 */
#define TIMER_INTLEVEL		0

#endif /* _TKDEV_CONF_ */
