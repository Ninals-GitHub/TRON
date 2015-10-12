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
 *	console_drv.c	Console/Low-level serial I/O driver
 *
 *	Console driver : System-independent
 */

#include <basic.h>
#include <sys/consio.h>
#include <tk/tkernel.h>
#include <tk/util.h>
#include <device/serialio.h>
#include <device/rs.h>
#include <sys/imalloc.h>
#include <sys/util.h>
#include <libstr.h>
#include <sys/queue.h>
#include <sys/debug.h>
#include <tm/tmonitor.h>
#include <sys/rominfo.h>

#include <sys/svc/ifconsio.h>

/* Default console port number */
#ifndef DEFAULT_CONSOLE_PORT
#define	DEFAULT_CONSOLE_PORT	CONF_SERIAL_0
#endif

EXPORT	W	DebugPort;	/* Serial port number for debugging */

IMPORT	ER	con_def_subsys(W svc, W pri, void *svcent, void *brkent);
IMPORT	ER	devshare_entry(void *para, W fn, void *gp);

IMPORT	ER	consMLock(FastMLock *lock, INT no);
IMPORT	ER	consMUnlock(FastMLock *lock, INT no);
IMPORT	ER	consCreateMLock(FastMLock *lock, UB *name);
IMPORT	ER	consDeleteMLock(FastMLock *lock);

/*----------------------------------------------------------------------
	Console port processing
----------------------------------------------------------------------*/
/* Console port management block*/
typedef struct {
	QUEUE	q;		/* Queue			*/
	W	port;		/* Port number			*/
	W	in_use;		/* In-use count			*/

	W	conf;		/* Configuration		*/
	FastMLock lock;		/* Lock for input-output	*/
	ID	flg;		/* Event flag for input-output	*/

	UB	*in_buf;	/* Input buffer			*/
	UW	in_bufsz;	/* Input buffer size		*/
	UW	in_rptr;	/* Input buffer read-in counter	*/
	UW	in_wptr;	/* Input buffer write-counter	*/

	UB	*ou_buf;	/* Output buffer		*/
	UW	ou_bufsz;	/* Output buffer size		*/
	UW	ou_rptr;	/* Output buffer read-in pointer*/
	UW	ou_wptr;	/* Output buffer write pointer	*/

	UB	*h_buf;		/* History buffer		*/

	UW	rsv:11;
	UW	rcv_xoff:1;	/* "XOFF" receive status		*/
	UW	disable:2;	/* Send-disabled and Receive-disabled status		*/
	UW	echo:1;		/* The presence or absence of echo	*/
	UW	newline:1;	/* CR / NL				*/
	UW	input:4;	/* Input mode				*/
	UW	flowc:4;	/* Flow control				*/
	UW	wup_char:8;	/* Character to execute "wakeup"	*/

	ID	wup_tskid;	/* Task ID to execute "wakeup"		*/

	W	snd_tmout;	/* Send time out			*/
	W	rcv_tmout;	/* Receive time out			*/
} CONSCB;

LOCAL	QUEUE	ConsPort;		/* Console port	        	*/
LOCAL	UH	last_port = 0;		/* Last port number		*/

LOCAL	FastLock	ConsLock;	/* Lock the overall console	*/

#define	DEF_INBUFSZ	256		/* Input buffer size		*/
#define	DEF_OUBUFSZ	1024		/* Output buffer size		*/
#define	MIN_BUFSZ	128		/* Minimum buffer size		*/
#define	HIST_BUFSZ	2048		/* History buffer size  	*/

#define	INPTRMASK(p, ptr)	((ptr) % ((p)->in_bufsz))
					/* Input buffer pointer mask	*/
#define	OUPTRMASK(p, ptr)	((ptr) % ((p)->ou_bufsz))
					/* Output buffer pointer mask	*/

#define	XOFF_MARGIN	64		/* Remaining size to send "XOFF"*/
#define	XON_MARGIN	128		/* Remaining size to send "XON"	*/

/* Control character code       */
#define	BS	('H'-'@')
#define	CAN	('X'-'@')
#define	CTLC	('C'-'@')
#define	DEL	(0x7f)
#define	CR	(0x0d)
#define	LF	(0x0a)
#define	ERASE	('K'-'@')
#define	CAN2	('U'-'@')
#define	TAB	('I'-'@')
#define	ESC	('['-'@')
#define	CUR_UP	('P'-'@')		/* or	ESC [ A 		*/
#define	CUR_DWN	('N'-'@')		/* or	ESC [ B 		*/
#define	CUR_FWD	('F'-'@')		/* or	ESC [ C 		*/
#define	CUR_BWD	('B'-'@')		/* or	ESC [ D 		*/
#define	XOFF	('S'-'@')
#define	XON	('Q'-'@')

#define	OU_LOCK		17
#define	IN_LOCK		16
#define	FLG_OU_EVT	(1 << 1)
#define	FLG_IN_EVT	(1 << 0)

/*
 *	Lock
 */
#define	CreLock(lock, name)	consCreateMLock(lock, name)
#define	DelLock(lock)		consDeleteMLock(lock)
#define	LockIn(lock, no)	consMLock(lock, no)
#define	LockOut(lock, no)	consMUnlock(lock, no)

/*
 *	Get/Release the memory
 */
#define	Malloc(len)	(void*)Imalloc(len)
#define	Free(ptr)	Ifree((void*)(ptr))

/*
 *	Check the port number
 */
LOCAL	CONSCB	*check_port(W port)
{
	QUEUE	*q;

	if (port <= 0) return NULL;
	q = QueSearch(&ConsPort, &ConsPort, port, offsetof(CONSCB, port));
	return (q == &ConsPort) ? NULL : (CONSCB*)q;
}
/*
 *	Check & Use the port number
 */
LOCAL	CONSCB	*get_port(W port)
{
	CONSCB	*p;

	/* Search the port*/
	Lock(&ConsLock);
	if ((p = check_port(port)) != NULL) p->in_use++; /* Use-count ++*/
	Unlock(&ConsLock);
	return	p;
}
/*
 *	Special character processing
 */
LOCAL	W	special_char_proc(CONSCB *p, B c)
{
	if (p->input != RAW && c == p->wup_char) {	/* Task "wakeup"*/
		tk_rel_wai(p->wup_tskid);
		p->wup_tskid = 0;
	}
	return	0;
}
/*
 *	Write into the console input buffer
 */
LOCAL	W	put_consbuf(CONSCB *p, W c, W tmout)
{
	UW	ptr, nptr;
	UINT	flgptn;

	/* Special character processing */
	special_char_proc(p, c);

	/* Send flow control */
	if (p->flowc & IXON) {
		if (c == XOFF) {
			p->rcv_xoff = 1;
			return 0;
		}
		if (c == XON || (p->rcv_xoff && (p->flowc & IXANY))) {
			if (p->rcv_xoff) {	/* Event occurs */
				if (p->flg) tk_set_flg(p->flg, FLG_OU_EVT);
				p->rcv_xoff = 0;
			}
			return 0;
		}
	}

	ptr = p->in_wptr;
	nptr = INPTRMASK(p, ptr + 1);

	/* Input buffer full : wait for the event */
	while (nptr == p->in_rptr) {
		if (tk_wai_flg(p->flg, FLG_IN_EVT, TWF_ORW | TWF_BITCLR,
			       &flgptn, tmout)) return -1;
	}

	/* Store the data in the input buffer */
	p->in_buf[ptr] = c;
	p->in_wptr = nptr;

	/* Generate the event when the input-buffer is not empty */
	if (ptr == p->in_rptr) tk_set_flg(p->flg, FLG_IN_EVT);

	/* Receive flow control is unnecessary */
	return 0;
}
/*
 *	Read-in from the console input buffer
 */
LOCAL	W	read_consbuf(CONSCB *p)
{
	W	c;
	UINT	flgptn;
	UW	ptr;

	ptr = p->in_rptr;

	/* Input buffer is empty : wait for event */
	while (ptr == p->in_wptr) {
		if (tk_wai_flg(p->flg, FLG_IN_EVT, TWF_ORW | TWF_BITCLR,
			       &flgptn, p->rcv_tmout)) return -1;
	}

	/* Retrieve the data */
	c = p->in_buf[ptr];
	p->in_rptr = INPTRMASK(p, ptr + 1);

	/* Event occurs when the input buffer is not full  */
	if (ptr == INPTRMASK(p, p->in_wptr + 1)) {
		tk_set_flg(p->flg, FLG_IN_EVT);
	}

	/* Receive flow control is unnecessary */
	return c;
}
/*
 *	Write into the console output buffer
 */
LOCAL	W	write_consbuf(CONSCB *p, W c)
{
	UW	ptr, nptr;
	UINT	flgptn;

	ptr = p->ou_wptr;
	nptr = OUPTRMASK(p, ptr + 1);

	/* Wait for the event during receiving "XOFF" or when the output buffer is full */
	while (p->rcv_xoff || nptr == p->ou_rptr) {
		if (tk_wai_flg(p->flg, FLG_OU_EVT, TWF_ORW | TWF_BITCLR,
			       &flgptn, p->snd_tmout)) return -1;
	}
	p->ou_buf[ptr] = c;
	p->ou_wptr = nptr;

	/* Generate the event when the output buffer is not empty */
	if (ptr == p->ou_rptr) tk_set_flg(p->flg, FLG_OU_EVT);

	return 0;
}
/*
 *	Read-in from the console output buffer
 */
LOCAL	W	get_consbuf(CONSCB *p, W tmout)
{
	W	c;
	UW	ptr;
	UINT	flgptn;

	ptr = p->ou_rptr;

	/* Output buffer is empty : wait for event */
	while (ptr == p->ou_wptr) {
		if (tk_wai_flg(p->flg, FLG_OU_EVT, TWF_ORW | TWF_BITCLR,
			       &flgptn, tmout)) return -1;
	}

	/* Fetch the data */
	c = p->ou_buf[ptr];
	p->ou_rptr = OUPTRMASK(p, ptr + 1);

	/* Event occurs when the buffer is not full */
	if (ptr == OUPTRMASK(p, p->ou_wptr + 1)) {
		tk_set_flg(p->flg, FLG_OU_EVT);
	}

	return c;
}
/*
 *	One character input
 */
LOCAL	W	cons_getch(CONSCB *p)
{
	UB	c;
	W	alen;

	if (p->conf == CONF_BUFIO)
		return read_consbuf(p);	/* Input from the input-buffer */

	/* Input the serial port */
	return (serial_in(p->conf, &c, 1, &alen, p->rcv_tmout) < 0)? -1: c;
}
/*
 *	Output the one character
 */
LOCAL	W	cons_putch(CONSCB *p, B c)
{
	W	alen;

	if ((p->disable & 0x1) != 0) return 0;	/* Stop output   */

	if (p->conf == CONF_BUFIO)	/* Output to the output-buffer */
		return write_consbuf(p, c);

	/* Output the serial port */
	return (serial_out(p->conf, &c, 1, &alen, p->snd_tmout) < 0)? -1: 0;
}
/*
 *	Edit line input
 */
LOCAL	W	edit_input(CONSCB *p, B *buf, W max)
{
	W	i, c, c1;
	W	cp, ep, hp, esc;
	W	len;
	B	*hist;

	cp = ep = esc = 0;
	c1 = 0;
	hp = -1;
	hist = (B*)p->h_buf;

	for (;;) {
		/* Input one character */
		if ((c = cons_getch(p)) < 0) break;
		len = 1;
		if (c & 0x80) {
			if (c1 == 0) {c1 = c; continue;}
			c |= c1 << 8;
			c1 = 0;
			len = 2;
		}
		if (c == ESC) {esc = 1; continue;}
		if (esc) {	/* "ESC" sequence */
			if (esc == 1) {esc = (c == '[') ? 2 : 0; continue;}
			esc = 0;
			if (c == 'A')		c = CUR_UP;
			else if (c == 'B')	c = CUR_DWN;
			else if (c == 'C')	c = CUR_FWD;
			else if (c == 'D')	c = CUR_BWD;
			else			continue;
		}
		if (c == CUR_FWD) {
			if (cp < ep) {
				if (buf[cp] & 0x80) cons_putch(p, buf[cp++]);
				cons_putch(p, buf[cp++]);
			}
			continue;
		}
		if (c == CUR_BWD) {
			if (cp > 0) {
				if (buf[--cp] & 0x80){cons_putch(p, BS); cp--;}
				cons_putch(p, BS);
			}
			continue;
		}
		if (c == CUR_UP || c == CUR_DWN) {	/* Call history */
			if (! hist) continue;

			if (c == CUR_DWN) {
				if (hp <= 0) continue;
				for (hp--; (--hp) > 0 && hist[hp];);
				if (hp) hp++;
			} else {
				i = hp < 0 ? 0 : (strlen(&hist[hp]) + hp + 1);
				if (hist[i] == 0) continue;
				hp = i;
			}
			for (; cp > 0; cp--) cons_putch(p, BS);
			i = strlen(&hist[hp]);
			if (i > max) i = max;
			for (; cp < i; cp++)
				cons_putch(p, buf[cp] = hist[hp + cp]);
			c = ERASE;
		}
		if (c == BS || c == DEL) {
			if (cp <= 0) continue;
			len = (buf[cp - 1] & 0x80) ? 2 : 1;
			if (cp < ep) memcpy(&buf[cp-len], &buf[cp], ep - cp);
			for (i = 0; i < len; i++) {
				buf[--ep] = ' ';
				cons_putch(p, BS);
			}
			cp -= len;
			for (i = cp; i < ep + len; i++) cons_putch(p, buf[i]);
			for (; i > cp; i--) cons_putch(p, BS);
			continue;
		}
		if (c == CAN || c == CAN2) {
			for (; cp > 0; cp--) cons_putch(p, BS);
			c = ERASE;
		}
		if (c == ERASE) {
			for (i = cp; i < ep; i++) cons_putch(p, ' ');
			for (; i > cp; i--) cons_putch(p, BS);
			ep = cp;
			continue;
		}
		if (c == CR || c == LF) {c = LF; break;}

		if (c == CTLC) {cons_putch(p, c); break;}

		if (c < ' ' && c != TAB) continue;

		if (ep + len > max) continue;  /* Ignore the excess number of characters */

		if (cp < ep) memmove(&buf[cp+len], &buf[cp], ep - cp);
		if (len == 2) {
			buf[cp+1] = c & 0xff;
			c = (c >> 8) & 0xff;
		}
		buf[cp] = c;
		for (ep += len, i = cp; i < ep; i++) cons_putch(p, buf[i]);
		for (cp += len; i > cp; i--) cons_putch(p, BS);
	}
	if (c == LF) {
		cons_putch(p, CR);	/* Echo back */
		cons_putch(p, LF);
	}
	i = ep; /* Valid character-string end */
	if (ep < max && c >= 0) buf[ep++] = c;
	if (ep < max) buf[ep] = 0;
	if (i > 0 && c != CTLC) {		/* Add to the history buffer */
		if (hist) {
			memmove(&hist[i+1], hist, HIST_BUFSZ - 1 - i);
			memcpy(hist, buf, i);
			hist[i] = 0;
			hist[HIST_BUFSZ - 2] = 0;
			hist[HIST_BUFSZ - 1] = 0;
		}
	}
	return (c == CTLC) ? -1 : ep;
}
/*
 *	Output to the console port :"return": the number of the characters actually output
 */
LOCAL	W	_console_out(W port, B *buf, UW len)
{
	CONSCB	*p;
	UB	c;
	W	alen = 0;
	ER	er;

	/* Check the address */
	er = ChkSpaceR((void*)buf, len);
	if (er < E_OK && er != E_RSFN) return 0;

	/* Check the port number */
	if ((p = get_port(port)) != NULL) {

		/* Lock */
		if (LockIn(&p->lock, OU_LOCK) < 0) goto EEXIT;

		for (; alen < len; alen++) {
			if ((c = *buf++) == LF && p->newline) {
				if (cons_putch(p, CR)) break;
			}
			if (cons_putch(p, c)) break;
		}
		/* Unlock */
		LockOut(&p->lock, OU_LOCK);
EEXIT:
		p->in_use--;		/* Use-count - -*/
	}
	return	alen;
}
/*
 *	Input from the console port :"return": the number of the characters actually injput
 */
LOCAL	W	_console_in(W port, B *buf, UW len)
{
	CONSCB	*p;
	W	c;
	W	alen = 0;

	/* Check the address */
	if (ChkSpaceRW((void*)buf, len)) return 0;

	/* Check the port number */
	if ((p = get_port(port)) != NULL) {
		/* Lock */
		if (LockIn(&p->lock, IN_LOCK) < 0) goto EEXIT;

		/* Input the data */
		if (p->input == EDIT && len > 1) {
			alen = edit_input(p, buf, len);
		} else {
			while (alen < len) {
				if ((c = cons_getch(p)) < 0) break;
				if (c == CR && p->input != RAW) c = LF;
				if (p->echo) {		/* Echo*/
					if (c == LF && p->input != RAW)
						cons_putch(p, CR);
					cons_putch(p, c);
				}
				buf[alen++] = c;
				if (p->input == RAW || c == LF || c == CTLC)
							break;
			}
		}
		/* Unlock */
		LockOut(&p->lock, IN_LOCK);
EEXIT:
		p->in_use--;		/* Use-count - -*/
	}
	return	alen;
}
/*
 *	Control the serial port:"return" < 0 :error
 */
LOCAL	W	_console_ctl(W port, W req, W arg)
{
	CONSCB	*p;
	W	n;
	W	rtn = -1;

	/* Check the port number */
	if ((p = get_port(port)) != NULL) {

		/* Do not lock */
		switch (req) {
		case ECHO|GETCTL:
			rtn = p->echo;				break;
		case ECHO:
			rtn = 0;	p->echo = arg;		break;
		case INPUT|GETCTL:
			rtn = p->input;				break;
		case INPUT:
			rtn = 0;
			if ((p->input = arg) == EDIT) {
				if (!p->h_buf) {
					if (!(p->h_buf = Malloc(HIST_BUFSZ)))
							rtn = -1;
				}
				if (rtn == 0) p->h_buf[0] = 0;
			} else if (p->h_buf) {
				Free(p->h_buf);
				p->h_buf = NULL;
			}
			break;
		case NEWLINE|GETCTL:
			rtn = p->newline;			break;
		case NEWLINE:
			rtn = 0;	p->newline = arg;	break;
		case FLOWC|GETCTL:
			rtn = p->flowc;				break;
		case FLOWC:
			rtn = 0;
			p->flowc = arg;
			if (p->conf >= 0) {	/* Serial port */
				RsFlow flow;
				flow.rsv     = 0;
				flow.rcvxoff = 0;
				flow.csflow  = 0;
				flow.rsflow  = 0;
				flow.xonany  = (arg & IXANY) != 0;
				flow.sxflow  = (arg & IXON ) != 0;
				flow.rxflow  = (arg & IXOFF) != 0;
				serial_ctl(p->conf, DN_RSFLOW, (void*)&flow);
			}
			p->rcv_xoff = 0;
			break;
		case SNDTMO|GETCTL:
			rtn = p->snd_tmout;			break;
		case SNDTMO:
			rtn = 0;
			p->snd_tmout = (arg < 0) ? -1 : arg;	break;
		case RCVTMO|GETCTL:
			rtn = p->rcv_tmout;			break;
		case RCVTMO:
			rtn = 0;
			p->rcv_tmout = (arg < 0) ? -1 : arg;	break;
		case RCVBUFSZ|GETCTL:
			rtn = p->in_bufsz;			break;
		case SNDBUFSZ|GETCTL:
			rtn = p->ou_bufsz;			break;
		case 0x8f:
			rtn = 0;
			p->wup_char = arg;
			if ((n = p->wup_tskid) > 0) tk_wup_tsk(n);
			p->wup_tskid = tk_get_tid();
			break;
		case 0x8e:
			rtn = 0;	p->disable = arg;	break;
		}
		p->in_use--;		/* Use-count - -*/
	}
	return	rtn;
}
/*
 *	Write the input data into the console : "return" : the number of the characters actually written
 */
LOCAL	W	_console_put(W port, B *buf, UW len, W tmout)
{
	CONSCB	*p;
	W	alen = 0;

	/* Check the address */
	if (ChkSpaceR((void*)buf, len)) return 0;

	/* Check the port number */
	if ((p = get_port(port)) != NULL) {
		/* Do not lock */
		if (p->conf == CONF_BUFIO && (p->disable & 0x2) == 0) {
			for (; alen < len; alen++) {
				if (put_consbuf(p, (W)*buf++, tmout)) break;
			}
		}
		p->in_use--;		/* Use-count - -*/
	}
	return alen;
}
/*
 *	Read the output data from the console : "return" : the number of the characters actually read
 */
LOCAL	W	_console_get(W port, B *buf, UW len, W tmout)
{
	CONSCB	*p;
	W	c;
	W	alen = 0;

	/* Check the address */
	if (ChkSpaceRW((void*)buf, len)) return 0;

	/* Check the port number */
	if ((p = get_port(port)) != NULL) {
		/* Do not lock */
		if (p->conf == CONF_BUFIO) {
			for (; alen < len; alen++) {
				if ((c = get_consbuf(p, tmout)) < 0) break;
				*buf++ = c;
			}
		}
		p->in_use--;		/* Use-count - - */
	}
	return alen;
}
/*
 *	Delete the console
 */
LOCAL	W	delete_cons(CONSCB *p)
{
	UW	par[2];

	/* Delete from the queue of port */
	QueRemove(&p->q);

	/* Lock and delete event flag:Forcibly release the wait */
	if (p->flg) DelLock(&p->lock);

	/* Wait until the port is unused */
	while (p->in_use > 0) {
		if (p->conf >= 0)
			serial_ctl(p->conf, RS_ABORT, NULL);	/* Abort */
		tk_dly_tsk(10);
	}

	/* Release the special setting of the port */
	if (p->conf >= 0) {	/* Serial line port */
		par[0] = par[1] = 0;
		serial_ctl(p->conf, RS_EXTFUNC, par);
	}

	/* Release the buffer */
	if (p->in_buf) Free(p->in_buf);
	if (p->ou_buf) Free(p->ou_buf);
	if (p->h_buf)  Free(p->h_buf);

	/* Delete the port itself */
	Free(p);

	return	0;
}
/*
 *	Create the console
 */
LOCAL	W	make_cons(W new, W *arg)
{
static	union objname	name = {{ "con0" }};
	CONSCB		*p, *old;
	UH		port;
	W		conf, sz, dmy;
	UW		par[2];

	/* Check the "conf" */
	conf = arg[1];
	if (conf != CONF_BUFIO) {
		/* Check the serial line port */
		if (conf < 0 || serial_ctl(conf, -DN_RSMODE, &dmy)) return -1;
	}

	if (new) {	/* New creation */
		/* Find the unused port number */
		for (port = ++last_port; QueSearch(&ConsPort, &ConsPort,
			(W)port, offsetof(CONSCB, port)) != &ConsPort; port++);
		arg[0] = (W)(last_port = port);
		old = NULL;
	} else {	/* Existent :check the port number */
		port = arg[0];
		if (!(old = check_port(port))) return -1;
	}

	/* Create the new port */
	if (!(p = (CONSCB*)Malloc(sizeof(CONSCB)))) return -1;
	bzero((void*)p, sizeof(CONSCB));

	/* Object name */
	name.s[3] = '0' + port;

	/* Create the event flag/the lock */
	if ((p->flg = CreLock(&p->lock, name.s)) < 0) goto EEXIT;

	/* Create the input buffer */
	if (conf == CONF_BUFIO) {
		if ((sz = arg[2]) < MIN_BUFSZ)
				sz = sz ? MIN_BUFSZ : DEF_INBUFSZ;
		if (!(p->in_buf = Malloc(sz)))	goto EEXIT;
		p->in_bufsz = sz;
	}
	/* Create the output buffer */
	if (conf == CONF_BUFIO) {
		if ((sz = arg[3]) < MIN_BUFSZ)
				sz = sz ? MIN_BUFSZ : DEF_OUBUFSZ;
		if (!(p->ou_buf = Malloc(sz)))	goto EEXIT;
		p->ou_bufsz = sz;
	}

	/* Delete the current port when it is not newly created */
	if (new == 0) delete_cons(old);

	/* Initialization (except for 0) */
	p->port = port;
	p->conf = conf;
	p->input = CANONICAL;
	p->snd_tmout = p->rcv_tmout = -1;

	/* Special setting of port */
	if (conf >= 0) {	/* Serial line port */
		par[0] = (UW)special_char_proc;
		par[1] = (UW)p;
		serial_ctl(conf, RS_EXTFUNC, par);
	}

	/* Register to the queue */
	QueInsert(&p->q, &ConsPort);
	return	0;

EEXIT:	/* Error processing */
	if (p->in_buf) Free(p->in_buf);
	if (p->ou_buf) Free(p->ou_buf);
	if (p->flg > 0) DelLock(&p->lock);
	Free(p);
	return -1;
}

/*
 *	Configuration operation of console
 */
LOCAL	W	_console_conf(W req, UW* arg)
{
	CONSCB	*p;
	W	n;

	/* Skip the address checking            */

	/* Lock                                 */
	Lock(&ConsLock);

	switch(req) {
	case CS_CREATE:		/* Create the console port		*/
		n = make_cons(1, (W*)arg);
		break;

	case CS_DELETE:		/* Delete the console port		*/
		if (arg[0] > 2 && (p = check_port(arg[0]))) n = delete_cons(p);
		else	n = -1;
		break;

	case CS_GETCONF:	/* Fetch the console configuration	*/
		if ((p = check_port(arg[0])) != NULL) {
			arg[1] = p->conf;
			arg[2] = p->in_bufsz;
			arg[3] = p->ou_bufsz;
			n = 0;
		} else	n = -1;
		break;

	case CS_SRCHPORT:	/* Search the console port              */
		n = 0;
		p = (CONSCB*)&ConsPort;
		while ((p = (CONSCB*)QueSearch((QUEUE*)p, &ConsPort, arg[1],
			offsetof(CONSCB, conf))) != (CONSCB*)&ConsPort) {
			if (p->port > arg[0]) {
				arg[0] = n = p->port;
				break;
			}
		}
		break;

	case CS_SETCONF:	/* Set the console configuration	*/
		n = make_cons(0, (W*)arg);
		break;

	case CS_GETPORT:	/* Fetch the standard console port	*/
		n = 1;		/* Default #1                           */
		arg[0] = check_port(n) ? n : 1;
		n = 0;
		break;

	case CS_SETPORT:	/* Set the standard console port	*/
		n = -1;
		break;

	default:
		n = -1;
	}
	/* Unlock                               */
	Unlock(&ConsLock);
	return n;
}
/*
 *	Console I/O extended SVC
 */
LOCAL	ER	console_io_entry(void *para, W fn, void *gp)
{
	switch ( fn ) {
	  /* Input-output between console and buffer */
	  case CONSIO_CONSOLE_GET_FN:
		{ CONSIO_CONSOLE_GET_PARA *p = para;
		return _console_get(p->port, p->buf, p->len, p->tmout); }
	  case CONSIO_CONSOLE_PUT_FN:
		{ CONSIO_CONSOLE_PUT_PARA *p = para;
		return _console_put(p->port, p->buf, p->len, p->tmout); }
	  case CONSIO_CONSOLE_CONF_FN:
		{ CONSIO_CONSOLE_CONF_PARA *p = para;
		return _console_conf(p->req, p->arg); }

	  /* Input-output between application and buffer */
	  case CONSIO_CONSOLE_IN_FN:
		{ CONSIO_CONSOLE_IN_PARA *p = para;
		return _console_in(p->port, p->buf, p->len); }
	  case CONSIO_CONSOLE_OUT_FN:
		{ CONSIO_CONSOLE_OUT_PARA *p = para;
		return _console_out(p->port, p->buf, p->len); }
	  case CONSIO_CONSOLE_CTL_FN:
		{ CONSIO_CONSOLE_CTL_PARA *p = para;
		return _console_ctl(p->port, p->req, p->arg); }

	}

	/* Device common manager processing    */
	return	devshare_entry(para, fn, gp);
}
/*
 *	Console I/O break function
 */
LOCAL	void	console_break(ID tskid)
{
	tk_dis_wai(tskid, TTW_FLG|TTX_SVC);
}
/*
 *	Start-up/end of console
 */
EXPORT	ER	console_startup( BOOL StartUp )
{
	W	n, arg[4];

	if ( !StartUp ) {	/* End processing               */

		/* Deregister the subsystem     */
		con_def_subsys(CONSIO_SVC, CONSIO_PRI, NULL, NULL);

		/* Delete the all consoles      */
		while (!isQueEmpty(&ConsPort)) {
			delete_cons((CONSCB*)ConsPort.next);
		}

		/* Delete the exclusive lock    */
		DeleteLock(&ConsLock);

		return E_OK;
	}

	/* Initialization               */
	QueInit(&ConsPort);

	/* Create the exclusive lock    */
	CreateLock(&ConsLock, "cons");

	/* Wait for a while             */
	tk_dly_tsk(50);

	/* Fetch the debug port number from T-Monitor  */
	DebugPort = DEFAULT_CONSOLE_PORT;
	if ((n = tm_extsvc(0x04, 0, 0, 0)) >= 0) DebugPort = n;

	/* Create the standard console port (#1 : debug console) */
	arg[1] = _isDebugMode() ? DebugPort : CONF_BUFIO;
	arg[2] = arg[3] = 0;
	if (_console_conf(CS_CREATE, (UW*)arg) < E_OK && _isDebugMode()) {
		/* The debug console is unusable.
	Therefore, It shall be switched to the buffer I/O to stop send/receive */
		last_port = 0;
		arg[1] = CONF_BUFIO;
		_console_conf(CS_CREATE, (UW*)arg);
		_console_ctl(1, 0x8e, 0x3);	/* Stop sending/receiving */
	}

	/* Create the standard console port (#2 : Serial#1) */
	arg[1] = DebugPort;
	_console_conf(CS_CREATE, (UW*)arg);

	/* Initialize the standard console port  */
	_console_ctl(1, ECHO, 1);
	_console_ctl(1, INPUT, EDIT);
	_console_ctl(1, NEWLINE, 1);
	_console_ctl(1, FLOWC, IXON | IXOFF);
	if (!_isDebugMode()) _console_ctl(1, 0x8e, 0x3);  /* Stop sending/receiving      */

	/* Register the subsystem :
	Overwrite the registration in device common manager */
	return con_def_subsys(CONSIO_SVC, CONSIO_PRI,
					console_io_entry, console_break);
}
