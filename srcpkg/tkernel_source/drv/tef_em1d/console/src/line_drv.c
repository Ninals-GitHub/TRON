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
 *	line_drv.c	Console/Low-level serial I/O driver
 *
 *	Serial line low-level driver common part  : system-independent
 */

#include "line_drv.h"
#include "svc/ifserialio.h"
#include <tm/tmonitor.h>

/* Register/Deregister subsystem */
IMPORT	ER	con_def_subsys(W svc, W pri, void *svcent, void *brkent);

/* Port definition (system-dependent)*/
#include "portdef.h"

/* Default line mode */
EXPORT	RsMode	DefMode = {
	0,	/* no parity */
	3,	/* 8 data bits */
	0,	/* 1 stop bit */
	0,	/* reserved */
	38400	/* bps */
};

/* Flow status initialization constant number */
EXPORT const	RsFlow		RsFlow0    = {0, 0, 1, 1, 0, 0, 0};
EXPORT const	FlowState	FlowState0 = {0, 0, 0};

/* Serial line management information */
EXPORT	LINE_INFO	*LineInfo;
EXPORT	W		nPorts = N_PORTS;

/*
 *	Serial input
 */
LOCAL ER	_serial_in(SERIAL_SERIAL_IN_PARA *p)
{
	/* Parameter check */
	if (p->port < 0 || p->port >= nPorts) return E_PAR;

	return (*(LineInfo[p->port].scdefs.fn->in))(
		&LineInfo[p->port], (UB*)p->buf, p->len, p->alen,
		(p->tmout < 0) ? TMO_FEVR : p->tmout);
}

/*
 *	Serial port Output
 */
LOCAL ER	_serial_out(SERIAL_SERIAL_OUT_PARA *p)
{
	/* Parameter check */
	if (p->port < 0 || p->port >= nPorts) return E_PAR;
	if (p->len <= 0) return E_OK;

	return (*(LineInfo[p->port].scdefs.fn->out))(
		&LineInfo[p->port], (UB*)p->buf, p->len, p->alen,
		(p->tmout < 0) ? TMO_FEVR : p->tmout);
}

/*
 *	Serial port control
 */
LOCAL ER	_serial_ctl(SERIAL_SERIAL_CTL_PARA *p)
{
	/* Parameter check */
	if (p->port < 0 || p->port >= nPorts) return E_PAR;

	return (*(LineInfo[p->port].scdefs.fn->ctl))
		(&LineInfo[p->port], p->kind, p->arg);
}

/*
 *	Serial I/O extended SVC entry
 */
LOCAL ER	serial_io_entry(void *para, W fn)
{
	switch(fn) {
	case SERIAL_SERIAL_IN_FN:	return _serial_in(para);
	case SERIAL_SERIAL_OUT_FN:	return _serial_out(para);
	case SERIAL_SERIAL_CTL_FN:	return _serial_ctl(para);
	}
	return E_RSFN;
}

/*
 *	Serial I/O break function
 */
LOCAL void	serial_io_break(ID tskid)
{
	tk_dis_wai(tskid, TTW_FLG);
}

/*
 *	Start up the serial interface driver
 */
EXPORT ER	startup_serial_io(BOOL StartUp)
{
	LINE_INFO	*li;
	W		n, port;
static	union objname	name = {{ "sio0" }};

	/* Fetch and set the communication speed from "tmonitor" */
	if ((n = tm_extsvc(0x00, 0, 0, 0)) >= 19200) DefMode.baud = n;

	/* Initialize the auxiliary port */
	INIT_AUXPORT(StartUp);

	if (!StartUp) {
		/* Deregister the subsystem */
		con_def_subsys(SERIAL_SVC, SERIAL_PRI, NULL, NULL);

		/* Stop the auxiliary port */
		START_AUXPORT(FALSE);

		/* Stop the all serial ports */
		for (port = 0; port < nPorts; port++) {
			li = &LineInfo[port];
			li->scdefs = PortDefs[port];

			/* Stop the serial port */
			(*(li->scdefs.fn->down))(li);

			/* Release the receive buffer */
			if (li->in_buf) Free(li->in_buf);

			/* Delete the event flag for notification of the lock and the interrupt  */
			if (li->flg > 0) consDeleteMLock(&li->lock);
		}

		/* Release the serial line management information */
		if (LineInfo) Free(LineInfo);

		return E_OK;
	}

	/* Start up the auxiliary port */
	START_AUXPORT(TRUE);

	/* Initialize the serial line management information */
	LineInfo = (LINE_INFO *)Malloc(sizeof(LINE_INFO) * nPorts);
	if (! LineInfo) {nPorts = 0; return E_NOMEM;}
	memset(LineInfo, 0, sizeof(LINE_INFO) * nPorts);

	/* Start up the serial port */
	for (port = 0; port < nPorts; port++) {
		li = &LineInfo[port];
		li->scdefs = PortDefs[port];
		name.s[3] = '0' + port;

		/* Allocate the receive buffer */
		li->in_buf = Malloc(li->in_bufsz = DEF_INBUFSZ);
		if (li->in_buf == NULL) return E_NOMEM;

		/* Create the event flag for notification of the lock and the interrupt */
		li->flg = consCreateMLock(&li->lock, name.s);

		/* RTS/CTS flow control - ON */
		li->flow.csflow = 1;
		li->flow.rsflow = 1;

		/* Set various hardware */
		(*(li->scdefs.fn->up))(li);
	}

	/* Register the subsystem */
	return con_def_subsys(SERIAL_SVC, SERIAL_PRI,
				&serial_io_entry, &serial_io_break);
}
