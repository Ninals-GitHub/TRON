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
 *	ns16450.h	Console/Low-level serial I/ driver
 *
 *	General definition of serial controller (NS16450, NS16550)
 */

/* Individual definition is required for "_IOADR(n)" and "_CLOCK (Hz)" */

/* I/O port address */
#if 0
#elif _TEF_EM1D_
#define	SC_DATA		_IOADR(0)	/* Data register	R/W */
#define	SC_INTE		_IOADR(1)	/* Interrupt-enabled register	R/W */
#define	SC_INTS		_IOADR(2)	/* Interrupt identification register	R   */
#define	SC_FCTL		_IOADR(3)	/* FIFO control register	  W */
#define	SC_LCTL		_IOADR(4)	/* Line control register	R/W */
#define	SC_MCTL		_IOADR(5)	/* Modem control register	R/W */
#define	SC_LSTS		_IOADR(6)	/* Line status register	R   */
#define	SC_MSTS		_IOADR(7)	/* Modem status register	R   */
#define	SC_SCRA		_IOADR(8)	/* Temporary data register	R/W */
#define	SC_DIVL		_IOADR(9)	/* Frequency dividing count value  low level R/W */
#define	SC_DIVH		_IOADR(10)	/* Frequency dividing count value  upper level  R/W */
#else
#define	SC_DATA		_IOADR(0)	/* Data register	R/W */
#define	SC_INTE		_IOADR(1)	/* Interrupt-enabled register	R/W */
#define	SC_INTS		_IOADR(2)	/* Interrupt identification register	R   */
#define	SC_FCTL		_IOADR(2)	/* FIFO control register	  W */
#define	SC_LCTL		_IOADR(3)	/* Line control register	R/W */
#define	SC_MCTL		_IOADR(4)	/* Modem control register	R/W */
#define	SC_LSTS		_IOADR(5)	/* Line status register	R   */
#define	SC_MSTS		_IOADR(6)	/* Modem status register	R   */
#define	SC_SCRA		_IOADR(7)	/* Temporary data register	R/W */
#define	SC_DIVL		_IOADR(0)	/* Frequency dividing count value  low level R/W */
#define	SC_DIVH		_IOADR(1)	/* Frequency dividing count value  upper level  R/W */
#endif

/* Communication speed -> frequency dividing count value */
#define	SC_LINE_SPEED(bps)	( _CLOCK / 16 / (bps) )

/* Line control register */
#define	LC_DLAB		0x80		/* Frequency dividing counter access */
#define	LC_SBRK		0x40		/* Break sending */
#define	LC_STKP		0x20		/* Parity fix */
#define	LC_EVNP		0x10		/* Even parity */
#define	LC_ENAP		0x08		/* Enable parity */
#define	LC_STOP		0x04		/* Stop bit */
#define	LC_BLEN		0x03		/* The number of data bits */

#define	LC_NoParity	(0x00)
#define	LC_EvenParity	(LC_ENAP|LC_EVNP)
#define	LC_OddParity	(LC_ENAP)

/* Line status register */
#define	LS_RFER		0x80		/* There is error in the receive FIFO */
#define	LS_TSRE		0x40		/* Send shift register is empty */
#define	LS_THRE		0x20		/* Send hold register is empty*/
#define	LS_BINT		0x10		/* Receive break */
#define	LS_FERR		0x08		/* Framing error */
#define	LS_PERR		0x04		/* Parity error */
#define	LS_OERR		0x02		/* Overrun error */
#define	LS_DRDY		0x01		/* There is the receive data */
/* Additional definition for driver internal processing */
#define	LS_RXOV		0x80		/* Receive-buffer overflow */

#define	LS_RxERR	(LS_BINT|LS_FERR|LS_PERR|LS_OERR)

/* Modem control register */
#define	MC_LOOP		0x10		/* Loopback test mode */
#define	MC_OUT2		0x08		/* User-specified auxiliary output #2 */
#define	MC_OUT1		0x04		/* User-specified auxiliary output #1 */
#define	MC_RTS		0x02		/* Request To Send */
#define	MC_DTR		0x01		/* Data Terminal Ready */

/* Modem status register*/
#define	MS_CD		0x80		/* Data Carrier Detect */
#define	MS_RI		0x40		/* Ring Indicate */
#define	MS_DR		0x20		/* Data Set Ready */
#define	MS_CS		0x10		/* Clear To Send */
#define	MS_D_CD		0x08		/* Change of CD */
#define	MS_D_RI		0x04		/* Change of RI */
#define	MS_D_DR		0x02		/* Change of DR */
#define	MS_D_CS		0x01		/* Change of CS */

/* Interrupt-enabled register  */
#define	IM_MSTS		0x08		/* Modem status interrupt */
#define	IM_LSTS		0x04		/* Receive line status interrupt */
#define	IM_SND		0x02		/* Send-enabled interrupt */
#define	IM_RCV		0x01		/* Receive-data-enabled interrupt */

#define	IM_NORM		(IM_LSTS|IM_SND|IM_RCV|IM_MSTS)

/* Interrupt identification register */
#define	IS_PEND		0x01		/* Interrupt pending */
#define	IS_ID		0x0e		/* Interrupt ID */
#define	IS_CTMO		0x0c		/* ID=6 character time-out interrupt */
#define	IS_LSTS		0x06		/* ID=3 receive-line status interrupt */
#define	IS_RCV		0x04		/* ID=2 receive-data-enabled interrupt */
#define	IS_SND		0x02		/* ID=1 send-enabled interrupt  */
#define	IS_MSTS		0x00		/* ID=0 modem status interrupt */
#define	IS_FIFO		0xc0		/* FIFO during use */

/* FIFO control register */
#define	FC_TL01		0x00		/* Receive-interrupt unit   1 byte */
#define	FC_TL04		0x40		/* Receive-interrupt unit   4 byte */
#define	FC_TL08		0x80		/* Receive-interrupt unit   8 byte */
#define	FC_TL14		0xc0		/* Receive-interrupt unit  14 byte */
#define	FC_TXCLR	0x04		/* Send FIFO clear  */
#define	FC_RXCLR	0x02		/* Receive FIFO clear*/

#if _TEF_EM1D_
#define	FC_FIFO		0x21		/* Enable 64 bytes FIFO */
#define	FIFO_SIZE	64			/* FIFO size */
#else
#define	FC_FIFO		0x01		/* Enable FIFO */
#define	FIFO_SIZE	16			/* FIFO size */
#endif

