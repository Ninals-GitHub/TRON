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
 *	int_em1d512.c
 *
 *	EM1-D512 interrupt controller management
 *
 *	intvec passed to the functions for interrupt controller must
 *      be within the valid ranges of IRQ and GPIO interrupt.
 *	If a value outside the valid range is passed, the subsequent
 *      correct behavior of the system is not guaranteed.
 */

#include <basic.h>
#include <tk/syslib.h>
#include <tk/sysdef.h>

/*
 * Interrupt controller
 *	register size W
 */
#define	AINT(n)		( 0xc0020000 + (n) )	/* ACPU Interrupt */
#define	SINT(n)		( 0xcc010000 + (n) )	/* ACPU Secure Interrupt */

#define	IT0_IEN0	AINT(0x0000)	/* RW ACPU enable interrupt 0 */
#define	IT0_IEN1	AINT(0x0004)	/* RW ACPU enable interrupt 1 */
#define	IT0_IEN2	AINT(0x0100)	/* RW ACPU enable interrupt 2 */
#define	IT0_IDS0	AINT(0x0008)	/* -W ACPU disable interrupt 0 */
#define	IT0_IDS1	AINT(0x000C)	/* -W ACPU disable interrupt 1 */
#define	IT0_IDS2	AINT(0x0104)	/* -W ACPU disable interrupt 2 */
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
#define	IT_LIIS		AINT(0x0320)	/* -W internal interrupt status set */
#define	IT_LIIR		AINT(0x0324)	/* -W internal interrupt status reset */

#define	IT0_IENS0	SINT(0xE200)	/* RW ACPU Secure enable interrupt 0 */
#define	IT0_IENS1	SINT(0xE204)	/* RW ACPU Secure enable interrupt 1 */
#define	IT0_IENS2	SINT(0xE208)	/* RW ACPU Secure enable interrupt 2 */
#define	IT0_IDSS0	SINT(0xE20C)	/* -W ACPU Secure disable interrupt 0 */
#define	IT0_IDSS1	SINT(0xE210)	/* -W ACPU Secure disable interrupt 1 */
#define	IT0_IDSS2	SINT(0xE214)	/* -W ACPU Secure disable interrupt 2 */

LOCAL const struct aint_reg {
	UINT	IT0_IEN;	/* RW ACPU enable interrupt state */
	UINT	IT0_IDS;	/* -W ACPU disable interrupt */
	UINT	IT0_RAW;	/* R- ACPU interrupt Raw status */
	UINT	IT0_MST;	/* R- ACPU interrupt mask / status */
	UINT	IT_PINV_SET;	/* RW enable inverted logic for interrupt input */
	UINT	IT_PINV_CLR;	/* -W disable inverted logic for interrupt input */
	UINT	IT0_IENS;	/* RW ACPU Secure enable interrupt state */
	UINT	IT0_IDSS;	/* -W ACPU Secure disable interrupt */
} AINT[3] = {
	{ IT0_IEN0, IT0_IDS0, IT0_RAW0, IT0_MST0, IT_PINV_SET0, IT_PINV_CLR0,
	  IT0_IENS0, IT0_IDSS0 },
	{ IT0_IEN1, IT0_IDS1, IT0_RAW1, IT0_MST1, IT_PINV_SET1, IT_PINV_CLR1,
	  IT0_IENS1, IT0_IDSS1 },
	{ IT0_IEN2, IT0_IDS2, IT0_RAW2, IT0_MST2, IT_PINV_SET2, IT_PINV_CLR2,
	  IT0_IENS2, IT0_IDSS2 }
};

/*
 * GIO Interrupt
 *	register size W
 */
#define	GIO(b, n)	( (b) + (n) )		/* General I/O */
#define	_L		( 0xc0050000 + 0x000 )
#define	_H		( 0xc0050000 + 0x040 )
#define	_HH		( 0xc0050000 + 0x080 )
#define	_HHH		( 0xc0050000 + 0x200 )

#define	GIO_IIA(b)	GIO(b,0x0014)	/* RW enable interrupt status */
#define	GIO_IEN(b)	GIO(b,0x0018)	/* -W enable interrupt */
#define	GIO_IDS(b)	GIO(b,0x001C)	/* -W disable interrupt */
#define	GIO_IIM(b)	GIO(b,0x001C)	/* R- enable interrupt state */
#define	GIO_RAW(b)	GIO(b,0x0020)	/* R- interrupt Raw status */
#define	GIO_MST(b)	GIO(b,0x0024)	/* R- interrupt mask / status */
#define	GIO_IIR(b)	GIO(b,0x0028)	/* -W reset the cause of interrupt */
#define	GIO_GSW(b)	GIO(b,0x003C)	/* RW connected to GIO_INT_FIQ pin */
#define	GIO_IDT(n,b)	GIO(b,0x0100+(n)*4) /* RW interrupt detection method 0-3 */
#define	GIO_RAWBL(b)	GIO(b,0x0110)	/* R- edge-triggered interrupt status L */
#define	GIO_RAWBH(b)	GIO(b,0x0114)	/* R- edge-triggered interrupt status H */
#define	GIO_IRBL(b)	GIO(b,0x0118)	/* -W clear the cause of edge-triggered interrupt L */
#define	GIO_IRBH(b)	GIO(b,0x011C)	/* -W clear the cause of edge-triggered interrupt H */

LOCAL const UINT	GIOB[4] = { _L, _H, _HH, _HHH };

#define	AINTNO(intvec)	( ((intvec) - IV_IRQ(0)) / 32 )		/* AINT number */
#define	GIONO(intvec)	( ((intvec) - IV_GPIO(0)) / 32 )	/* GIO number */
#define	BITNO(intvec)	( ((intvec) - IV_IRQ(0)) % 32 )		/* bit number */

/* ------------------------------------------------------------------------ */

/*
 * Disable / Enable interrupt
 */
Inline UW _disint( void )
{
	UW	imask;
	Asm("	mrs	%0, cpsr	\n"
	"	orr	ip, %0, %1	\n"
	"	msr	cpsr_xc, ip	"
		: "=r"(imask)
		: "i"(PSR_DI)
		: "ip" );
	return imask;
}
Inline void _enaint( UW imask )
{
	Asm("msr cpsr_xc, %0":: "r"(imask));
}
#define	_DI(imask)	( imask = _disint() )
#define	_EI(imask)	( _enaint(imask) )

/*
 * Set interrupt mode.
 *	Set the interrupt mode specified by `intvec' to the mode given
 *  	by `mode'. If an illegal mode is given, subsequent correct behavior
 *      of the system is not guaranteed.
 *
 *	The case of IRQ
 *	mode := IM_ENA | IM_INV
 *	or	IM_DIS
 *
 *	The case of GPIO
 *	mode := IM_ENA | IM_LEVEL | (IM_HI || IM_LOW) | IM_ASYN
 *	 or IM_ENA | IM_EDGE | (IM_HI || IM_LOW || IM_BOTH) | IM_ASYN
 *	 or IM_DIS
 *
 *	If IM_ENA is specified, the mode setting is done, and
 *      interrupt is disabled (DisableInt) and the interrupt pin is
 *      asserted.
 *      If IM_DIS is specified, interrupt pin is disasserted.
 *      disasserted interrupt pin doesn't generate interrupt even if
 *      it is enabled (EnabledInt).
 *      In the initial state, pins are dis-asserted (IM_DIS).
 *
 *	The initial status of the following interrupt controllers
 *	that manage GPIO interrupt is such that
 *      as if IM_ENA had been specified and interrupts are enabled
 * 	(EnableInt).
 *
 *		IRQ26	GIO6 Interrupt (GPIO port  96--111)
 *		IRQ27	GIO7 Interrupt (GPIO port 112--127)
 *		IRQ50	GIO0 Interrupt (GPIO port   0-- 15)
 *		IRQ51	GIO1 Interrupt (GPIO port  16-- 31)
 *		IRQ52	GIO2 Interrupt (GPIO port  32-- 47)
 *		IRQ53	GIO3 Interrupt (GPIO port  48-- 63)
 *		IRQ79	GIO4 Interrupt (GPIO port  64-- 79)
 *		IRQ80	GIO5 Interrupt (GPIO port  80-- 95)
 */
EXPORT void SetIntMode( INTVEC intvec, UINT mode )
{
	UW	m = 1 << BITNO(intvec);
	UINT	imask;

	if ( intvec < IV_GPIO(0) ) {
		/* IRQ */
		const struct aint_reg	*r = AINT + AINTNO(intvec);

		out_w(r->IT0_IDS,  m);	/* disable interrupt */
		out_w(r->IT0_IDSS, m);	/* disassert interrupt pin */
		if ( (mode & IM_ENA) == 0 ) return;

		if ( (mode & IM_INV) == 0 ) out_w(r->IT_PINV_CLR, m);
		else			    out_w(r->IT_PINV_SET, m);

		out_w(r->IT0_IENS, m);	/* assert interrupt pin */
	} else {
		/* GPIO */
		UINT	gb = GIOB[GIONO(intvec)];
		UINT	n;
		UW	mm, mv;

		_DI(imask);
		out_w(GIO_IDS(gb), m);			    /* disable interrupt */
		out_w(GIO_IIA(gb), in_w(GIO_IIA(gb)) & ~m); /* disassert interrupt pin */
		_EI(imask);
		if ( (mode & IM_ENA) == 0 ) return;

		n = (intvec - IV_GPIO(0)) % 8 * 4;
		mm = 0xf << n;
		mv = ((mode >> 8) & 0xf) << n;
		n = (intvec - IV_GPIO(0)) % 32 / 8;
		_DI(imask);
		out_w(GIO_IDT(n, gb), (in_w(GIO_IDT(n, gb)) & ~mm) | mv);
		out_w(GIO_IIA(gb), in_w(GIO_IIA(gb)) | m); /* assert interrupt pin */
		_EI(imask);
	}
}

/*
 * Enable interrupt
 *	enable interrupt specified by `intvec'
 */
EXPORT void EnableInt( INTVEC intvec )
{
	UW	m = 1 << BITNO(intvec);

	if ( intvec < IV_GPIO(0) ) {
		/* IRQ */
		const struct aint_reg	*r = AINT + AINTNO(intvec);
		out_w(r->IT0_IEN, m);
	} else {
		/* GPIO */
		UINT	gb = GIOB[GIONO(intvec)];
		out_w(GIO_IEN(gb), m);
	}
}

/*
 * Disable interrupt
 *	disable interrupt specified by `intvec'
 */
EXPORT void DisableInt( INTVEC intvec )
{
	UW	m = 1 << BITNO(intvec);

	if ( intvec < IV_GPIO(0) ) {
		/* IRQ */
		const struct aint_reg	*r = AINT + AINTNO(intvec);
		out_w(r->IT0_IDS, m);
	} else {
		/* GPIO */
		UINT	gb = GIOB[GIONO(intvec)];
		out_w(GIO_IDS(gb), m);
	}
}

/*
 * Clear the request for interrupt
 *	clear the request of interrupt specified by `intvec'.
 *	We need to clear the requests only in the case of edge-trigger interrupts.
 */
EXPORT void ClearInt( INTVEC intvec )
{
	UW	m = 1 << BITNO(intvec);

	if ( intvec < IV_IRQ(32 ) ) {
		/* IRQ 0-31  no relevant targets */

	} else if ( intvec < IV_IRQ(64) ) {
		/* IRQ 32--63 */
		out_w(IT0_IIR, m);

	} else if ( intvec < IV_GPIO(0) ) {
		/* IRQ 64-95 no relevant targets */

	} else {
		/* GPIO */
		UINT	gb = GIOB[GIONO(intvec)];
		out_w(GIO_IIR(gb), m);
	}
}

/*
 * Check the existence of interrupt request
 *	check the existence of request for interrupt specified by intvec
 *	If there is, TRUE (non-zero value) is returned
 *
 *	The existence of the request of interrupt is checked by
 *      'raw' status register.
 */
EXPORT BOOL CheckInt( INTVEC intvec )
{
	UW	sts;

	if ( intvec < IV_GPIO(0) ) {
		/* IRQ */
		const struct aint_reg	*r = AINT + AINTNO(intvec);
		sts = in_w(r->IT0_RAW);
	} else {
		/* GPIO */
		UINT	gb = GIOB[GIONO(intvec)];
		sts = in_w(GIO_RAW(gb));
	}

	return (sts >> BITNO(intvec)) & 1;
}
