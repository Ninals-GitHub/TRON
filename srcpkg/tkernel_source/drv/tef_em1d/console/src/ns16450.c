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
 *	ns16450.c	Console/Low-level serial I/O driver
 *
 *	Serial line low-level driver (for NS16450)
 */

#include "line_drv.h"
#include <sys/sysinfo.h>
#include <device/share.h>

/*
 *	Define the serial controller operation function groups
 */
LOCAL ER	ns16450_in(LINE_INFO *li, UB *buf, W len, W *alen, W tmout);
LOCAL ER	ns16450_out(LINE_INFO *li, UB *buf, W len, W *alen, W tmout);
LOCAL ER	ns16450_ctl(LINE_INFO *li, W kind, UW *arg);
LOCAL void	ns16450_up(LINE_INFO *li);
LOCAL void	ns16450_down(LINE_INFO *li);

EXPORT	SC_FUNC	ScFuncNS16450 = {
	ns16450_in, ns16450_out, ns16450_ctl, ns16450_up, ns16450_down,
};

/*
 *	Define the serial controller ("NS16450" and "NS16550")
 */
#include "ns16450.h"		/* Common definition */
#include "ns16450sys.h"		/* System-dependent definition */

/* Clock */
#define	_CLOCK		UART_CLK

/* I/O port address */
#define	_IOADR(n)	(scdefs->c.iobase + (n) * scdefs->c.iostep)

/*
 *	Bit-unit access to serial controller register
 */
Inline void clr_screg( W port, UB m )
{
	OutB(port, InB(port) & ~m);
}
Inline void set_screg( W port, UB m )
{
	OutB(port, InB(port) | m);
}
Inline void set_screg2( W port, UB m, UB v )
{
	OutB(port, (InB(port) & ~m) | v);
}

/*
 *	"RTS" and "Break" signal operations
 */
#define ON_RTS		set_screg(SC_MCTL, MC_RTS|MC_OUT2)	 /* Turn on RTS */
#define OFF_RTS		set_screg2(SC_MCTL, MC_RTS, MC_OUT2)	/* Turn off RTS */
#define	ON_BREAK	set_screg(SC_LCTL, LC_SBRK)	/* Break send */
#define	OFF_BREAK	clr_screg(SC_LCTL, LC_SBRK)	/* Break end */

/*
 *	Send/Receive interrupt control
 */

#define	ENB_RXINT(li)	OutB(SC_INTE, (li->enbint == 0) ? \
					0x00 : (IM_LSTS|IM_RCV|IM_MSTS))
#define	ENB_TXRXINT(li)	OutB(SC_INTE, (li->enbint == 0) ? \
					0x00 : (IM_LSTS|IM_SND|IM_RCV|IM_MSTS))
#define	DIS_TXRXINT(li)	OutB(SC_INTE, 0x00)

/*
 *	Forcible setting of the control line
 */
LOCAL	ER	line_ctl( LINE_INFO *li, UW cmd )
{
	SC_DEFS	*scdefs = &li->scdefs;
	UW	line;
	ER	err = E_OK;

	line = cmd & (MC_RTS|MC_DTR);

	switch ( cmd & 0xfffffffc ) {
	  case RSCTL_ON:
		set_screg(SC_MCTL, line | MC_OUT2);
		break;
	  case RSCTL_OFF:
		set_screg2(SC_MCTL, line, MC_OUT2);
		break;
	  case RSCTL_SET:
		set_screg2(SC_MCTL, MC_RTS|MC_DTR, line | MC_OUT2);
		break;
	  default:
		err = E_PAR;
	}
	return err;
}

/*
 *	Initialize the serial controller (mode setting)
 */
LOCAL	ER	init_sio( LINE_INFO *li, RsMode mode )
{
	SC_DEFS	*scdefs = &li->scdefs;
	UH	divcnt;
	UB	lctl, fctl, mctl;
	W	i;
	ER	err = E_OK;

	/* Parameter check */
	if ( mode.parity == 3
	  || mode.stopbits == 3
	  || mode.baud < 50 || mode.baud > 115200 ) return E_PAR;

	/* Communication speed setting value */
	divcnt = SC_LINE_SPEED(mode.baud);

	/* Line mode */
	lctl = 0x00;
	if ( mode.datalen == 0 ) {
		/* 5 data bits */
		if ( mode.stopbits == 2 ) return E_NOSPT;   /* 2 stop bits */
		if ( mode.stopbits == 1 ) lctl |= LC_STOP;  /* 1.5 stop bits */
	} else {
		/* 6,7,8 data bits */
		if ( mode.stopbits == 1 ) return E_NOSPT;   /* 1.5 stop bits */
		if ( mode.stopbits == 2 ) lctl |= LC_STOP;  /* 2 stop bits */
		lctl |= mode.datalen;
	}
	switch ( mode.parity ) {
	  case 1: lctl |= LC_OddParity;  break;	/* Odd parity */
	  case 2: lctl |= LC_EvenParity; break;	/* Even parity */
	}

	/* FIFO mode*/
	fctl = FC_FIFO;
	fctl |= ( mode.baud < 9600 )? FC_TL01: FC_TL08;

	/* Modem mode*/
	mctl = ( li->suspend == 0 )? MC_RTS|MC_DTR|MC_OUT2: MC_OUT2;

	/* Disable the interrupt */
	li->enbint = 0;
	DIS_TXRXINT(li);

	/* Wait for the send buffer to become empty for confirmation */
	for ( i = 0; i < FIFO_SIZE; ++i ) {
		if ( (InB(SC_LSTS) & LS_TSRE) != 0 ) break;
		tk_dly_tsk(100);
	}

	/* Clear the pending interrupt */
	OutB(SC_LCTL, 0x00);
	InB(SC_LSTS);		/* Clear the "IS_LSTS" */
	InB(SC_INTS);		/* Clear the "IS_SND" */
	InB(SC_MSTS);		/* Clear the "IS_MSTS" */

	/* Read and discard the data remaining in the receive-buffer */
	for ( i = 0; i < FIFO_SIZE; ++i ) {
		InB(SC_DATA);	/* Clear "IS_RCV IS_CTMO" */
	}

	if ( fctl != 0 ) {
		/* Check the presence and absence of FIFO */
		OutB(SC_FCTL, fctl);
		if ( (InB(SC_INTS) & IS_FIFO) == 0 ) {
			fctl = 0;  /* There is no FIFO */
		}
	}
	scdefs->fctl = fctl;

	/* Communication speed over 19200 bps is disabled without FIFO function */
	if ( fctl == 0 && mode.baud > 19200 ) err = E_NOSPT;

	/* Return to RS flow status */
	if ( li->flowsts.rsoff ) mctl &= ~MC_RTS;  /* RTS OFF */

	/* Initialize the serial controller */
	OutB(SC_LCTL, LC_DLAB);
	OutB(SC_DIVH, divcnt >> 8);	/* Communication speed */
	OutB(SC_DIVL, divcnt & 0xff);
	OutB(SC_LCTL, lctl);		/* Line mode */
	OutB(SC_FCTL, (fctl == 0) ? 0 : (fctl|FC_TXCLR|FC_RXCLR));
					/* Set/Disable the FIFO mode */
	li->msts = InB(SC_MSTS);	/* Initilize the modem status */

	OutB(SC_MCTL, mctl);		/* Modem mode */

	if ( err == E_OK && li->suspend == 0 ) {
		li->enbint = 1;
		ENB_RXINT(li);		/* Enable the receive interrupt only */
	}

	return err;
}

/*
 *	Interrupt handler
 */
EXPORT	void	sio_inthdr( UINT dintno )
{
	LINE_INFO	*li;
	SC_DEFS		*scdefs;
	UB		lsts, fctl, c;
	UW		ptr, nptr;
	W		i, n, cnt;

	/* Search the port informartion */
	for (li = &LineInfo[0], i = nPorts; ; li++) {
		if (--i < 0) return;
		if (li->scdefs.c.intvec == dintno) {
			if (li->scdefs.fn != &ScFuncNS16450) return;
			break;
		}
	}
	scdefs = &li->scdefs;
	fctl = scdefs->fctl;		/* FIFO configuration information */

   /* Execute the processing while the interrupt cause exists */

   while( (InB(SC_INTS) & IS_PEND) == 0 ) {

	/* Receive processing : repeat until there is no receive data :
		Limit the maximum number to prevent the infinite loop when card is pulled out */

	for ( cnt = 0; cnt < FIFO_SIZE; cnt++ ) {
		lsts = InB(SC_LSTS);

		if ( (lsts & LS_RxERR) != 0 ) {
			UB	s;

			/* Receive error*/
			s = lsts & LS_RxERR;
			li->lsts |= s;		/* Record the error status */
			li->lstshist |= s;

			/* Generate the event when the receive-buffer is empty	*/
			if ( li->in_wptr == li->in_rptr ) {
				tk_set_flg(li->flg, FLG_IN_NORM);
			}

			if ( fctl != 0 && (lsts & LS_OERR) != 0 ) {
				/* If overrun error occurs when using FIFO,
			the all receive FIFO shall be cleared.  */
				OutB(SC_FCTL, fctl|FC_RXCLR);
			}

			/* Read and discard the receive data */
			InB(SC_DATA);
			continue;
		}

		if ( (lsts & LS_DRDY) == 0 ) break;  /* There is no receive data */

		/* Fetch the receive data */
		c = InB(SC_DATA);

		/* Call the external function */
		if ( li->extfn != NULL ) {
			if ( (*li->extfn)(li->extpar, c) ) continue;
		}

		/* Send "XON/XOFF" flow control  */
		if ( li->flow.sxflow ) {
			if ( c == XOFF ) {
				li->flow.rcvxoff = 1;
				continue;
			}
			if ( c == XON
			  || (li->flow.rcvxoff && li->flow.xonany) ) {
				li->flow.rcvxoff = 0;
				continue;
			}
		}

		/* Record and discard the error status when the receive buffer is full  */
		ptr = li->in_wptr;
		nptr = PTRMASK(li, ptr + 1);
		if ( nptr == li->in_rptr ) {
			li->lsts |= LS_RXOV;	/* Record the error status */
			li->lstshist |= LS_RXOV;
			continue;
		}

		/* Store the receive data to the receive-buffer */
		li->in_buf[ptr] = c;
		li->in_wptr = nptr;

		/* Generate the event when a receive-buffer is empty */
		if ( ptr == li->in_rptr ) tk_set_flg(li->flg, FLG_IN_NORM);

		/* Receive error control */
		if ( (li->flow.rsflow || li->flow.rxflow)
		  && IN_BUF_REMAIN(li) < XOFF_MARGIN ) {
			/* RS flow control */
			if ( li->flow.rsflow && li->flowsts.rsoff == 0 ) {
				li->flowsts.rsoff = 1;
				OFF_RTS;
			}
			/* "XON/XOFF" flow control */
			if ( li->flow.rxflow && li->flowsts.sndxoff == 0 ) {
				li->flowsts.sndxoff = 1;
				li->flowsts.reqchar = XOFF;
			}
		}
	}

	/* Send processing*/
	li->msts = InB(SC_MSTS);

	if ( (lsts & LS_THRE) != 0				/* Tx Ready */
	  && li->flow.rcvxoff == 0				/* XOFF */
	  && (li->flow.csflow == 0 || (li->msts & MS_CS) != 0)	/* CS ON */
	) {
		cnt = n = li->ou_wptr - li->ou_rptr;	/* The number of characters to send */
		if ( li->flowsts.reqchar != 0 ) n++;	/* Send the "XON/XOFF"  */
		if ( fctl != 0 ) {
			if ( n > FIFO_SIZE ) n = FIFO_SIZE;	/* Limit to the FIFO buffer size */
		} else {
			if ( n >  1 ) n = 1;	/* There is no FIFO */
		}

		if ( n > 0 ) {			/* There is the send data */
			if ( li->flowsts.reqchar != 0 ) {
				/* Send the "XON / XOFF" */
				OutB(SC_DATA, li->flowsts.reqchar);
				li->flowsts.reqchar = 0;
				n--;
			}
			cnt -= n;		/* The remainig number of characters to send */

			ENB_TXRXINT(li);		/* Enable the send/receive interrupts */

			/* Send the data */
			while ( --n >= 0 ) {
				OutB(SC_DATA, li->ou_buf[
					OU_PTRMASK(li, li->ou_rptr++)]);
			}
		} else {
			/* Completion of sending or No sending */
			ENB_RXINT(li);		/* Enable the receive interrupt handler only */
		}
		/* Send (completion)-notification event occurs */
		if (cnt < li->ou_cnt) {
			tk_set_flg(li->flg, FLG_OU_NORM);
			li->ou_cnt = 0;
		}
	} else {
		/* Disable the send interrupt when the send is unavailable in "Tx Ready" status */
		if ((lsts & LS_THRE) != 0) ENB_RXINT(li);
	}
    }

	/* Interrupt post-processing */
	end_inthdr(scdefs);
}

/*
 *	Set the hardware configuration
 */
LOCAL	ER	setHwConf( LINE_INFO *li, RsHwConf_16450 *conf,
						RsMode mode, BOOL startup )
{
	SC_DEFS	*scdefs = &li->scdefs;
	ER	err;

	/* Cancel the current setting when it is already set */
	if ( scdefs->c.iostep > 0 ) {
		/* Disable the interrupt to the target port */
		li->enbint = 0;
		DIS_TXRXINT(li);

		/* Release the interrupt handler */
		delete_inthdr(scdefs, sio_inthdr);
	}

	/* Save the new configuration */
	scdefs->c = *conf;
	li->mode = mode;

	if ( scdefs->c.iostep > 0 ) {

		/* Disable the interrupt to the target port */
		li->enbint = 0;
		DIS_TXRXINT(li);

		/* Register the interrupt handler */
		err = regist_inthdr(scdefs, sio_inthdr);
		if (err < E_OK) return err;

		/* Initialize the line by the specified communication mode */
		err = init_sio(li, mode);
		if ( err < E_OK ) {
			/* Initialize by the default line mode when error occurs */
			li->mode = DefMode;
			err = init_sio(li, DefMode);
			if ( err < E_OK ) return err;
		}
		/* Interrupt is enabled by the line initialization (when "suspend = 0")  */
	}

	return E_OK;
}

/*
 *	Set the line status
 */
LOCAL	RsStat	make_line_stat( LINE_INFO *li, UB lsts )
{
	RsStat	stat;
static	RsStat	stat0 = {0};

	stat = stat0;
	if ( (lsts & LS_RXOV) != 0 )	stat.BE = 1;
	if ( (lsts & LS_FERR) != 0 )	stat.FE = 1;
	if ( (lsts & LS_OERR) != 0 )	stat.OE = 1;
	if ( (lsts & LS_PERR) != 0 )	stat.PE = 1;
	stat.XF = li->flow.rcvxoff;
	if ( (lsts & LS_BINT) != 0 )	stat.BD = 1;
	if ( (li->msts & MS_DR) != 0 )	stat.DR = 1;
	if ( (li->msts & MS_CD) != 0 )	stat.CD = 1;
	if ( (li->msts & MS_CS) != 0 )	stat.CS = 1;
	if ( (li->msts & MS_RI) != 0 )	stat.CI = 1;
	return stat;
}

/*
 *	Send the initial one character.
 *	It shall be called in the interrupt-disabled status."True" shall be returned when it can be sent.
 *	It shall be sent from the send buffer when "c < 0".
 *	In addition, it shall be sent to FIFO buffer size when using FIFO.
 */
LOCAL	BOOL	send_1st_char( LINE_INFO *li, W c )
{
	SC_DEFS		*scdefs = &li->scdefs;
	UB		lsts, s;
	W		n;

	/* Unable to send during a "XOFF" */
	if ( li->flow.rcvxoff != 0 ) return FALSE;

	/* Get the line status and the modem status */
	lsts = InB(SC_LSTS);
	li->msts = InB(SC_MSTS);

	/* Record the line error */
	if ( (s = lsts & LS_RxERR) != 0 ) {
		li->lsts |= s;
		li->lstshist |= s;
	}

	/* Unable to send when it is "Tx Not Ready" status or "CS OFF" status */
	if ( (lsts & LS_THRE) == 0
	  || (li->flow.csflow != 0 && (li->msts & MS_CS) == 0) ) return FALSE;

	if ( c < 0 ) {
		n = li->ou_wptr - li->ou_rptr;	/* The number of characters to send */
		if ( n <= 0 ) return FALSE;	/* There is no data to send */
		if ( scdefs->fctl != 0 ) {
			/* When FIFO is used */
			if ( n > FIFO_SIZE ) n = FIFO_SIZE;	/* Limit to FIFO buffer size */
		} else {
			n = 1;	/* There is no FIFO */
		}
		c = li->ou_buf[OU_PTRMASK(li, li->ou_rptr++)];
	} else {
		n = 1;
	}

	/* Send */
	ENB_TXRXINT(li);		/* Enable the send/receive interrupts */
	OutB(SC_DATA, c);
	while ( --n > 0 )
		OutB(SC_DATA, li->ou_buf[OU_PTRMASK(li, li->ou_rptr++)]);
	return TRUE;
}

/*
 *	One character input
 */
LOCAL	WRTN	recv_one_char( LINE_INFO *li, W tmout )
{
	SC_DEFS	*scdefs = &li->scdefs;
	UB	c;
	UINT	flgptn;
	UW	imask;
	ER	er;

	/* Wait for receiving while the receive buffer is empty */
	while ( li->in_wptr == li->in_rptr ) {

		if ( tmout == 0 ) return RTN_NONE; /* There is no wait */

		/* Wait for receiving or line status change */
		er = tk_wai_flg(li->flg, FLG_IN_WAIPTN,
				TWF_ORW | TWF_BITCLR, &flgptn, tmout);
		if ( er == E_TMOUT ) return RTN_TMOUT;
		if ( er < E_OK || (flgptn & FLG_IN_ABORT) ) return RTN_ABORT;
		if ( li->lsts != 0 ) return RTN_ERR; /* Receive error*/
	}

	/* Fetch the receive data */
	c = li->in_buf[li->in_rptr];
	li->in_rptr = PTRMASK(li, li->in_rptr + 1);

	/* Receive flow control */
	if ( (li->flowsts.rsoff || li->flowsts.sndxoff)
	   && IN_BUF_REMAIN(li) > XON_MARGIN ) {
		/* It is a receive-stop status. However, the receive-buffer has some empty space  */

		if ( li->flowsts.rsoff ) {
			/* "RS" flow control */
			DI(imask);
			ON_RTS;
			li->flowsts.rsoff = 0;
			EI(imask);
		}
		if ( li->flowsts.sndxoff ) {
			/* "XOFF" flow control */
			DI(imask);
			if ( !send_1st_char(li, XON) ) {
				li->flowsts.reqchar = XON;  /* "XON" reservation for sending */
			}
			li->flowsts.sndxoff = 0;
			EI(imask);
		}
	}

	return c;
}

/*
 *	Input from serial port
 */
LOCAL	ER	ns16450_in( LINE_INFO *li, UB *buf, W len, W *alen, W tmout )
{
	SC_DEFS		*scdefs = &li->scdefs;
	W		c;
	UW		rsz = 0;
	RsErr		err;

	if ( scdefs->c.iostep <= 0 ) return E_NOMDA;  /* Hardware configuration is not set yet */

	/* Execute the special processing to avoid deadlock at the time of polling */
	if (tmout == 0 && li->in_wptr == li->in_rptr) {
		err.w = E_BUSY;
		goto EEXIT;
	}

	/* Console input processing lock */
	if ( consMLock(&li->lock, IN_LOCK) < E_OK ) {
		err.s = make_line_stat(li, 0);
		err.c.ErrorClass = MERCD(E_IO);
		err.c.Aborted = 1;
		goto EEXIT;
	}

	err.w = E_OK;

	if ( len <= 0 ) {
		/* Check the number of the received bytes */
		rsz = IN_BUF_SIZE(li);
	} else {
		li->lsts = 0;  /* Clear the error status */

		/* Receive the data */
		for ( rsz = 0; rsz < len; rsz++ ) {

			c = recv_one_char(li, tmout);
			if ( c < RTN_OK ) {
				if ( c == RTN_NONE ) {
					err.w = ( rsz > 0 )? E_OK: E_BUSY;
				} else {
					err.s = make_line_stat(li, li->lsts);
					err.c.ErrorClass = MERCD(E_IO);
					if ( c == RTN_TMOUT ) err.c.Timout = 1;
					if ( c == RTN_ABORT ) err.c.Aborted= 1;
				}
				break;
			}
			buf[rsz] = c;
		}
	}

	/* Clear the "ABORT" flag */
	tk_clr_flg(li->flg, ~(FLG_IN_ABORT | FLG_OU_ABORT));

	/* Release the lock */
	consMUnlock(&li->lock, IN_LOCK);

EEXIT:

	*alen = rsz;  /* The number of the actually received bytes */
	return err.w;
}

/*
 *	Output to the serial port
 */
LOCAL	ER	ns16450_out( LINE_INFO *li, UB *buf, W len, W *alen, W tmout )
{
	SC_DEFS		*scdefs = &li->scdefs;
	RsErr		err;
	W		sz, n;
	RTN		r;
	UW		wptr, ptr, imask;
	UINT		flgptn;
	ER		er;

	if ( scdefs->c.iostep <= 0 ) return E_NOMDA;  /* Hardware configuration is not set yet */

	*alen = 0;

	/* Console output processing lock */
	if ( consMLock(&li->lock, OU_LOCK) < E_OK ) {
		err.s = make_line_stat(li, 0);
		err.c.ErrorClass = MERCD(E_IO);
		err.c.Aborted = 1;
		goto EEXIT;
	}

	err.w = E_OK;

	/*
	 * An output handler is read by the interrupt handler, so the page-out shall not be executed.
	 * Therefore, the resident memory shall be used as the output buffer
	 * instead of using the user-specified buffer "buffer", and the output data shall be copied.
	 */

	for (wptr = li->ou_wptr; len > 0; ) {

		/* Copy the send data to the output buffer */
		sz = li->ou_rptr + OUBUFSZ - li->ou_wptr;
		if (sz > len) sz = len;
		n = OUBUFSZ - (ptr = OU_PTRMASK(li, li->ou_wptr));
		if (n > sz) n = sz;
		if (n > 0)  memcpy(&li->ou_buf[ptr], buf, n);
		if (sz > n) memcpy(&li->ou_buf[0], &buf[n], sz - n);
		buf += sz;
		len -= sz;

		DI(imask);
		if (len <= 0) {	/* All the send data is copied to the output buffer.
			Therefore, notification shall be received when it is completed */
			n = 1;
			if (tmout == 0)	n = 0;	/* Do not wait for the completion of sending */

		} else {	/* The all send-data can not be copied to the output buffer,
				so notification shall be received when an empty space is available */
			if ((n = OUBUFSZ - len) < 20) n = 20;
		}
		li->ou_cnt = n;
		li->ou_wptr += sz;

		/* While no sending is executed,
		the one character shall be first sent to cause the send interrupt  */
		if ((InB(SC_INTE) & IM_SND) == 0) send_1st_char(li, -1);

		EI(imask);

		/* Wait-for-completion of sending */
		for (r = RTN_OK; li->ou_cnt != 0; ) {
			ptr = li->ou_rptr;
			er = tk_wai_flg(li->flg, FLG_OU_WAIPTN,
					TWF_ORW | TWF_BITCLR, &flgptn, tmout);
			if ( er == E_TMOUT ) {
				/* Time-out error shall occur
			only when even one character has not been sent from the previous time.  */
				if ( ptr == li->ou_rptr ) {
					r = RTN_TMOUT;
					break;
				}
			} else if ( er < E_OK || (flgptn & FLG_OU_ABORT) ) {
				r = RTN_ABORT;
				break;
			}
		}
		if ( r < RTN_OK ) {
			err.s = make_line_stat(li, 0);
			err.c.ErrorClass = MERCD(E_IO);
			if ( r == RTN_TMOUT ) err.c.Timout  = 1;
			if ( r == RTN_ABORT ) err.c.Aborted = 1;
			DI(imask);
			li->ou_wptr = li->ou_rptr;	/* Stop sending */
			EI(imask);
			break;
		}
	}

	*alen = li->ou_wptr - wptr;	/* Actual already sent byte */
	li->ou_cnt = 0;

	/* Clear the "ABORT" flag */
	tk_clr_flg(li->flg, ~(FLG_IN_ABORT | FLG_OU_ABORT));

	/* Release the lock */
	consMUnlock(&li->lock, OU_LOCK);

EEXIT:

	return err.w;
}

/*
 *	Control the serial port
 */
LOCAL	ER	ns16450_ctl( LINE_INFO *li, W kind, UW *arg )
{
	SC_DEFS		*scdefs = &li->scdefs;
	ER		err = E_OK;
	UW		imask;
	W		n;

	if ( !(kind == DN_RS16450 || kind == -DN_RS16450)
		&& scdefs->c.iostep <= 0 ) return E_NOMDA;  /* Hardware configuration is not set yet */

	/* Lock is unnecessary */

	switch ( kind ) {
	  case RS_ABORT:	/* Abort (release wait) */
		tk_set_flg(li->flg, FLG_IN_ABORT | FLG_OU_ABORT); /* Release */
		break;

	  case RS_SUSPEND:	/* Transit to the suspend status  */
	  case RS_RESUME:	/* Return from the suspend status */
		/* Debug port shall not transit to the "suspend" status in debug mode */
		if ( !(isDebugPort(li) && _isDebugMode()) ) {
			if (kind == RS_RESUME) sio_susres(li, 1);

			/* Reset controller */
			li->suspend = (kind == RS_SUSPEND) ? 1 : 0;
			err = setHwConf(li, &li->scdefs.c, li->mode, FALSE);

			if (kind == RS_SUSPEND) sio_susres(li, 0);
		}
		break;

	  case DN_RSMODE:	/* Set the communication mode */
		err = init_sio(li, *(RsMode*)arg);
		if ( err == E_OK ) {
			/* Initialize the serial line management information */
			DI(imask);
			li->in_rptr = li->in_wptr = 0;
			li->ou_rptr = li->ou_wptr = 0;
			li->ou_cnt = li->ou_len = 0;
			li->flow =  RsFlow0;
			li->flowsts = FlowState0;
			li->lsts = li->lstshist = 0;
			li->mode = *(RsMode*)arg;
			EI(imask);
		}
		break;

	  case -DN_RSMODE:	/* Get the communication mode */
		*(RsMode*)arg = li->mode;
		break;

	  case DN_RSFLOW:	/* Set the flow control */
		li->flow = *(RsFlow*)arg;
		break;

	  case -DN_RSFLOW:	/* Set and get the flow control */
		*(RsFlow*)arg = li->flow;
		break;

	  case -DN_RSSTAT:	/* Get the line status */
		*(RsStat*)arg = make_line_stat(li, li->lstshist);
		li->lstshist = 0;
		break;

	  case DN_RSBREAK:	/* Send break signal */
		if ( (n = *arg) > 0 ) {
			DI(imask);  ON_BREAK;  EI(imask);
			tk_dly_tsk(n);	/* Wait */
			DI(imask);  OFF_BREAK; EI(imask);
		}
		break;

	  case RS_RCVBUFSZ:	/* Set the receive-buffer */
		if ( (n = *arg) < MIN_INBUFSZ ) {
			err = E_PAR;
			break;
		}
		if ( n != li->in_bufsz ) {
			UB *new, *old;
			new = Malloc(n);
			if ( new != NULL ) {
				DI(imask);
				old = li->in_buf;
				li->in_buf = new;
				li->in_rptr = li->in_wptr = 0;
				li->in_bufsz = n;
				EI(imask);
				Free(old);
			} else {
				err = E_NOMEM;
			}
		}
		break;

	  case -RS_RCVBUFSZ:	/* Get the receive-buffer size */
		*arg = li->in_bufsz;
		break;

	  case RS_LINECTL:	/* Set the "ON/OFF" of control line */
		err = line_ctl(li, *arg);
		break;

	  case RS_EXTFUNC:	/* Set the external processing function */
		if ( li->extfn != NULL && (FUNCP)arg[0] != NULL ) {
			err = E_OBJ;  /* Already set */
		} else {
			DI(imask);
			li->extfn  = (FUNCP)arg[0];
			li->extpar = arg[1];
			EI(imask);
		}
		break;

	  case DN_RS16450:	/* Set the hardware configuration (for 16450) */
		err = setHwConf(li, (RsHwConf_16450*)arg, li->mode, FALSE);
		break;

	  case -DN_RS16450:	/* Get the hardware configuration (for 16450)  */
		*(RsHwConf_16450*)arg = scdefs->c;
		break;

	  default:
		err = E_PAR;
	}

	return	err;
}

/*
 *	Exit the serial port
 */
LOCAL	void	ns16450_down( LINE_INFO *li )
{
	SC_DEFS		*scdefs = &li->scdefs;

	if ( scdefs->c.iostep > 0 ) {
		li->enbint = 0;
		DIS_TXRXINT(li);
		OutB(SC_MCTL, 0x00);
	}
}

/*
 *	Start up the serial port
 */
LOCAL	void	ns16450_up( LINE_INFO *li )
{
	li->suspend = 0;
	setHwConf(li, &li->scdefs.c, DefMode, TRUE);
}
