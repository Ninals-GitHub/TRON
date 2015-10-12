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
 *	ns16550.c
 *
 *       serial port I/O
 */

#include <tmonitor.h>

/*
 * serial port hardware configuration definition
 */
typedef struct {
	UW	iob;		/* I/O base address */
} DEFSIO;

#if 0
#  define IOSTEP		/* I/O address separation */
#  define CLOCK			/* input clock (Hz) */

/* ----------------------------------------------------------------------- */
#elif _TEF_EM1D_
#  include <arm/em1d512.h>
   LOCAL const DEFSIO	DefSIO[3] = {
			{ UARTnBase(UART0) },
			{ UARTnBase(UART1) },
			{ UARTnBase(UART2) },
   };
#  define IOSTEP	4
#  define CLOCK		229376000

// Unlike ordinary 16550, all registers exist and are independently accessed.
// (No overlaid meaning/behavior per read or write, or switching of register sets is necessary.
// also, 16-bits read/write to data register while FIFO is enabled causes two character input/output.)
//
#define	UART(n)		( IOB + (n) * IOSTEP )
#define	regDATA		UART(0)		/* data register      (RW) */
#define	regINTE		UART(1)		/* interrupt enable register     (RW) */
#define	regINTS		UART(2)		/* interrupt selection register     (R ) */
#define	regFCTL		UART(3)		/* FIFO control register (RW) */
#define	regLCTL		UART(4)		/* line control register (RW) */
#define	regMCTL		UART(5)		/* model control register    (RW) */
#define	regLSTS		UART(6)		/* line status register     (R ) */
#define	regMSTS		UART(7)		/* modem status register    (R ) */
#define	regSCRA		UART(8)		/* scratch data register   (RW) */
#define	regDIVL		UART(9)		/* divisor lower bits   (RW) */
#define	regDIVH		UART(10)	/* divisor upper bits   (RW) */
/* ----------------------------------------------------------------------- */
#endif

#define	N_DEFSIO	( sizeof(DefSIO) / sizeof(DEFSIO) )

/* ------------------------------------------------------------------------ */

#define	IOB	( scb->info )		/* I/O base address */

#if !defined(IN)
#define IN(x)		in_b(x)
#define OUT(x, y)	out_b((x), (y))
#endif

/*
 * definition of 16550
 *       Every access is byte access.
 */
#if !defined(UART)
#define	UART(n)		( IOB + (n) * IOSTEP )
#define	regDATA		UART(0)		/* data register      (RW) */
#define	regINTE		UART(1)		/* interrupt enable register     (RW) */
#define	regINTS		UART(2)		/* interrupt selection register     (R ) */
#define regFCTL		UART(2)		/* FIFO control register ( W) */
#define	regLCTL		UART(3)		/* line control register   (RW) */
#define	regMCTL		UART(4)		/* modem control register   (RW) */
#define	regLSTS		UART(5)		/* line status register     (R ) */
#define	regMSTS		UART(6)		/* modem status register (R ) */
#define	regSCRA		UART(7)		/* scratch data register (RW) */
#define	regDIVL		UART(0)		/* divisor lower bits  (RW) */
#define	regDIVH		UART(1)		/* divisor upper bits  (RW) */
#endif

/* transmission speed -> divided counter value. */
#define	LC_LINE_SPEED(bps)	(CLOCK / 16 / (bps))

/* line control register */
#define	LC_DLAB		0x80		/* (divided) counter access */
#define	LC_SBRK		0x40		/* send BREAK */
#define	LC_SNDP		0x20		/* with parity */
#define	LC_EVNP		0x10		/* even parity */
#define	LC_ENAP		0x08		/* enable parity */
#define	LC_STOP		0x04		/* stop bit */
#define	LC_BLEN		0x03		/* number of bits in data */

/* default : 8 bits data, one bit stop, and no parity. */
#define	dtLC		(0x03)

/* line status register */
#define	LS_TSRE		0x40		/* transmission shift register empty */
#define	LS_THRE		0x20		/* transmission hold register empty */
#define	LS_BINT		0x10		/* BREAK received */
#define	LS_FERR		0x08		/* framing error */
#define	LS_PERR		0x04		/* parity error */
#define	LS_OERR		0x02		/* overrun error */
#define	LS_DRDY		0x01		/* received data ready */

#define	LS_RxERR	(LS_BINT|LS_FERR|LS_PERR|LS_OERR)

/* modem control register */
#define	MC_OUT2		0x08		/* auxiliary output #2 (enable interrupt) */
#define	MC_OUT1		0x04		/* auxiliary output #1 */
#define	MC_RTS		0x02		/* Request To Send */
#define	MC_DTR		0x01		/* Data Terminal Ready */

/* default : disable interrupt */
#define	dtMC		(MC_RTS | MC_DTR)

/* modem status register */
#define	MS_CD		0x80		/* Data Carrier Detect */
#define	MS_RI		0x40		/* Ring Indicate */
#define	MS_DR		0x20		/* Data Set Ready */
#define	MS_CS		0x10		/* Clear To Send */
#define	MS_D_CD		0x08		/* change of CD detected */
#define	MS_D_RI		0x04		/* change of RI detected */
#define	MS_D_DR		0x02		/* change of DR detected */
#define	MS_D_CS		0x01		/* change of CS detected */

/* interrupt enable register */
#define	IM_MSTS		0x08		/* modem state interrupt */
#define	IM_LSTS		0x04		/* receive line state interrupt */
#define	IM_SND		0x02		/* send ready interrupt  */
#define	IM_RCV		0x01		/* input data ready interrupt */

/* default : all interrupts are disabled */
#define	dtIM		(0x00)

/* interrupt selection register */
#define IS_PEND 	0x01		/* interrupt is pending */
#define IS_ID		0x0e		/* interrupt ID */
#define IS_CTMO 	0x0c		/* ID=6 timeout interrupt */
#define IS_LSTS 	0x06		/* ID=3 receive line state interrupt. */
#define IS_RCV		0x04		/* ID=2 input data ready interupt */
#define IS_SND		0x02		/* ID=1 send ready interrupt */
#define IS_MSTS 	0x00		/* ID=0 modem state interrupt */
#define IS_FIFO 	0xc0		/* FIFO is in use */

/* FIFO control register */
#define FC_TL01 	0x00		/* receive interrupt threshold ( 1 byte ) */
#define FC_TL04 	0x40		/* receive threshold 4 byte */
#define FC_TL08 	0x80		/* receive interrupt thereshold 8 byte */
#define FC_TL14 	0xc0		/* receive interrupt threshold 14 byte */
#define FC_TXCLR	0x04		/* clear trasnmission FIFO */
#define FC_RXCLR	0x02		/* receive FIFO clear. */
#if _TEF_EM1D_
#define FC_FIFO 	0x21		/* enable 64 bytes FIFO */
#define	FIFO_SIZE	64			/* FIFO size */
#else
#define FC_FIFO 	0x01		/* enable FIFO */
#define	FIFO_SIZE	16			/* FIFO size */
#endif

/* default : enable FIFO, 8 byte threshold */
#define	dtFC_CLR	(FC_FIFO | FC_TXCLR | FC_RXCLR)	/* clear */
#define	dtFC		(FC_FIFO | FC_TL08)		/* default */

/* ------------------------------------------------------------------------ */

/*
 * Power on RS-232C driver IC
 */
#define	RSDRV_PWON(siocb)		/* no operation */

/*
 * serial port I/O
 */
LOCAL void putSIO_16550( SIOCB *scb, UB c )
{
	RSDRV_PWON(scb);

        /* wait until transmission is ready. */
	while ((IN(regLSTS) & LS_THRE) == 0);

        /* write transmission data */
	OUT(regDATA, c);

        /* wait until the completion of transmission */
	while ((IN(regLSTS) & LS_THRE) == 0);
}

/*
 * serial port input
 *       tmo     timeout (milliseconds)
 *              You can not wait forever.
 *       return value       >= 0 : character code
 *                 -1 : timeout
 *       input data using buffer.
 *       receive error is ignored.
 */
LOCAL W getSIO_16550(SIOCB *scb, W tmo )
{
	W	sts, c = 0;

	RSDRV_PWON();

	tmo *= 1000/20;		/* convert tmo to 20 usec units */

        /* receive as much data as possible in the receive buffer */
	while (scb->iptr - scb->optr < SIO_RCVBUFSZ) {

                /* is there data in FIFO? */
		if ( !((sts = IN(regLSTS)) & (LS_DRDY | LS_RxERR))) {
			if (scb->iptr != scb->optr) break;  /* already received */
			if (tmo-- <= 0) break;		    /* timeout */
			waitUsec(20);
			continue;
		}

                /* receive data input */
		if (sts & LS_DRDY) c = IN(regDATA);

                /* error check */
		if (sts & LS_RxERR) continue;

                /* set data to rcvbuf */
		scb->rcvbuf[scb->iptr++ & SIO_PTRMSK] = c;
	}

        /* return the data in rcvbuf */
	return (scb->iptr == scb->optr)?
			-1 : scb->rcvbuf[scb->optr++ & SIO_PTRMSK];
}

/* ------------------------------------------------------------------------ */

/*
 * initialize serial port
 *       serial port that is supported by the initialization of CFGSIO
 *       speed   communication speed (bps)
 *       initialize the serial port according to the specified parameters and set SIOCB
 *       SIOCB is given in 0-cleared state initially.
 *       Subsequent I/O operations uses the SIOCB.
 *
 *       Only for PC/AT version
 *      if speed = 0, we use the value in biosp->siomode.
 *       But we use only the transmission speed and other settings are ignored.
 *       Efforts were made to be compatible B-right/V, but because of the ignorance of no-speed settings such as data length and stop bit length,
 *       we have reduced functionality.
 */
EXPORT ER initSIO_ns16550(SIOCB *scb, const CFGSIO *csio, W speed)
{
	UH	div;

	if ( (UW)csio->info >= N_DEFSIO ) return E_PAR;

        /* select the target port */
	scb->info = DefSIO[csio->info].iob;

        /* communicatin speed default value */
	div = LC_LINE_SPEED(speed);

        /* initialize serial controller */
	IN(regLSTS);			/* clear IS_LSTS */
	IN(regINTS);			/* clear IS_SND  */
	IN(regMSTS);			/* clear IS_MSTS */

	OUT(regLCTL, LC_DLAB);
	OUT(regDIVL, 0xff);		/* to keep the following from happening, Div = 0 */
	OUT(regDIVH, div >> 8);		/* communication speed */
	OUT(regDIVL, div & 0xff);
	OUT(regLCTL, dtLC);		/* line mode */
	OUT(regFCTL, dtFC_CLR);		/* clear FIFO */
	OUT(regFCTL, dtFC);		/* FIFO mode */
	OUT(regMCTL, dtMC);		/* modem mode */
	OUT(regINTE, dtIM);		/* interrupt mask */

        /* I/O function default */
	scb->put = putSIO_16550;
	scb->get = getSIO_16550;

	return E_OK;
}
