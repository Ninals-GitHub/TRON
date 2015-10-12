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
 *	sio.c
 *
 *       serial port I/O
 */

#include "hwdepend.h"

EXPORT	W	ConPort;	/* console port number */
EXPORT	UW	ConPortBps;	/* console port commnication speed (bps) */

LOCAL	SIOCB	SIO;		/* serial port control block */

/*
 * initialize serial port
 *       port    console port number (0 - )
 *              when it is -1, it means there is no console.
 *       speed   communication speed (bps)
 */
EXPORT ER initSIO( W port, W speed )
{
	const CFGSIO	*cp;
	ER	err;

	if ( port >= N_ConfigSIO ) port = 0; /* invalid value is turned into a default value. */

	memset(&SIO, 0, sizeof(SIO));
	ConPort    = port;
	ConPortBps = speed;

	if ( port < 0 ) return E_OK; /* no console */

        /* initialize hardware */
	cp = &ConfigSIO[port];
	err = (*cp->initsio)(&SIO, cp, speed);
	if ( err < E_OK ) goto err_ret;

	return E_OK;

err_ret:
        /* if there was an error, treat it as no console */
	memset(&SIO, 0, sizeof(SIO));
	ConPort = -1;
	return err;
}

/*
 * serial port I/O
 */
EXPORT void putSIO( UB c )
{
	if ( SIO.put != NULL ) (*SIO.put)(&SIO, c);
}

/*
 * serial port input (with buffering)
 *       tmo     timeout (milliseconds)
 *              You can not wait forever.
 *       return value       >= 0 : character code
 *                 -1 : timeout
 *       receive error is ignored
 */
EXPORT W getSIO( W tmo )
{
	return ( SIO.get != NULL )? (*SIO.get)(&SIO, tmo): -1;
}
