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
 *	portdef.h	Console/Low-level serial I/O driver
 *
 *	Serial line low-level driver system-dependent port definition (EM1-D512)
 */

/* Port definition */
IMPORT	SC_FUNC	ScFuncNS16450;

#define	N_PORTS		1

LOCAL  SC_DEFS	PortDefs[N_PORTS] = {
	{	&ScFuncNS16450, 		/* #0: UART0 */
		{0x50000000, 4, IV_IRQ(9)},
		0, {0,0,0}
	}
};

#define	INIT_AUXPORT(sup)	/* Nothing */
#define	START_AUXPORT(sup)	/* Nothing */
