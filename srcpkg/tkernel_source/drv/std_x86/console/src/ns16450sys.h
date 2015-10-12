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
 *	ns16450sys.h	Console/Low-level serial I/O driver
 *
 *	Serial line low level driver (for NS16450) EM1-D512 system-dependent definition
 */

/*
 *	"UART" clock
 */
#define	UART_CLK	(229376000)	/* Hz */

/*
 *	I/O input and output macro
 */
#define	OutB(ix, dt)	out_b((ix), (dt))
#define	InB(ix)		in_b((ix))

/*
 *	Post-interrupt processing
 */
Inline	void	end_inthdr(SC_DEFS *scdefs)
{
	/* Unnecessary to execute anything */
}

/*
 *	Release an interrupt handler
 */
Inline	void	delete_inthdr(SC_DEFS *scdefs, void *sio_inthdr)
{
	ER	err;

	/* Release an interrupt handler */
	err = def_inthdr(- scdefs->c.intvec, sio_inthdr);
	if (err > E_OK) {
		/* Disable interrupt */
		DisableInt(scdefs->c.intvec);
	}
}

/*
 *	Register the interrupt handler
 */
Inline	ER	regist_inthdr(SC_DEFS *scdefs, void *sio_inthdr)
{
	ER	err;

	/* Register the interrupt handler */
	err = def_inthdr(scdefs->c.intvec, sio_inthdr);
	if (err > E_OK) {
		/* Enable interrupt */
		SetIntMode(scdefs->c.intvec, IM_ENA);
		EnableInt(scdefs->c.intvec);
	}
	return err;
}

/*
 *	Suspend/Resume
 */
Inline	void	sio_susres(LINE_INFO *li, W resume)
{
	/* Execute nothing */
}
