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
 *	em1d512_iicspi_svc.c	(clkdrv)
        IIC/SPI driver : extended SVC
 *
 */

#include "clkdrv.h"
#include "svc/ifem1d512.h"

/* IIC driver */
IMPORT	ER	IICup(W ch, BOOL start);
IMPORT	ER	IICXfer(W ch, UH *cmddata, W words);

/* SPI driver */
IMPORT	ER	SPIup(W ch, BOOL start);
IMPORT	ER	SPIXfer(W cs, UB *xmit, UB *recv, W len);

/* SPI/IIC driver / SVC entries */
LOCAL	INT	em1d512_iicspi_svcentry(void *par, W fn)
{
	H8IO_EM1D512_IICXFER_PARA	*iic;
	H8IO_EM1D512_SPIXFER_PARA	*spi;
	INT	er;

	switch (fn) {
	case	H8IO_EM1D512_IICXFER_FN:
		iic = par;
		er = IICXfer(iic->ch, iic->cmddat, iic->words);
		break;
	case	H8IO_EM1D512_SPIXFER_FN:
		spi = par;
		er = SPIXfer(spi->cs, spi->xmit, spi->recv, spi->len);
		break;
	default:
		er = E_ILUSE;
		break;
	}

	return er;
}

/* SPI/IIC driver prolog ends */
EXPORT	ER	em1d512_iicspi_svc(BOOL start)
{
	ER	er;
	T_DSSY	dssy;

	if (!start) {
		er = E_OK;
		goto fin3;
	}

	er  = IICup(0, TRUE);
	er |= IICup(1, TRUE);
	if (er < E_OK) goto fin1;

	er  = SPIup(0, TRUE);
	er |= SPIup(1, TRUE);
	er |= SPIup(2, TRUE);
	if (er < E_OK) goto fin2;

        /* register extended SVC */
	dssy.ssyatr    = TA_NULL;
	dssy.ssypri    = H8IO_PRI;
	dssy.svchdr    = (FP)em1d512_iicspi_svcentry;
	dssy.breakfn   = NULL;
	dssy.startupfn = NULL;
	dssy.cleanupfn = NULL;
	dssy.eventfn   = NULL;
	dssy.resblksz  = 0;
	er = tk_def_ssy(H8IO_SVC, &dssy);
	if (er < E_OK) goto fin1;

	er = E_OK;	
	goto fin0;

fin3:
	tk_def_ssy(H8IO_SVC, NULL);
fin2:
	SPIup(2, FALSE);
	SPIup(1, FALSE);
	SPIup(0, FALSE);
fin1:
	IICup(1, FALSE);
	IICup(0, FALSE);
fin0:
	return er;
}
