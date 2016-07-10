/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by T-Engine Forum at 2011/09/08.
 *
 *----------------------------------------------------------------------
 */

/*
        em1d512_spi.c  SPI access (EM1-D512)
 *
 */

#include "clkdrv.h"
#include <tk/util.h>

#ifdef	DEBUG
#define	DP(x)	printf x
#else
#define	DP(x)	/* do nothing */
#endif

#define	SPIMAX		3
//LOCAL	FastLock	SPILock[SPIMAX];
//LOCAL	ID		SPITskID[SPIMAX];
LOCAL	const UW	SPIBase[SPIMAX] = {0xc0120000, 0xc0130000, 0x40130000};
LOCAL	const UW	SPIVec[SPIMAX] = {IV_IRQ(24), IV_IRQ(25), IV_IRQ(42)};
LOCAL	const UW	SPIPol[SPIMAX] = {0x009a, 0x0000, 0x0003};
LOCAL	const UW	SPIMode[SPIMAX] = {0x2700, 0x0700, 0x2700};

#define	SPn_MODE(n)		(SPIBase[n] + 0x0000)
#define	SPn_POL(n)		(SPIBase[n] + 0x0004)
#define	SPn_CONTROL(n)		(SPIBase[n] + 0x0008)
#define	SPn_TX_DATA(n)		(SPIBase[n] + 0x0010)
#define	SPn_RX_DATA(n)		(SPIBase[n] + 0x0014)
#define	SPn_STATUS(n)		(SPIBase[n] + 0x0018)
#define	SPn_RAW_STATUS(n)	(SPIBase[n] + 0x001c)
#define	SPn_ENSET(n)		(SPIBase[n] + 0x0020)
#define	SPn_ENCLR(n)		(SPIBase[n] + 0x0024)
#define	SPn_FFCLR(n)		(SPIBase[n] + 0x0028)
#define	SPn_CONTROL2(n)		(SPIBase[n] + 0x0034)
#define	SPn_TIECS(n)		(SPIBase[n] + 0x0038)

#define	TIMEOUT			10	// msec

#if 0
/* interrupt handler */
LOCAL	void	spi_inthdr(INTVEC vec)
{
	return;
}

/* wait for interrupt */
LOCAL	ER	wait_int(void)
{
	return tk_slp_tsk(TIMEOUT);
}

/* SPI initialization */
LOCAL	void	spi_init(W ch)
{
	return;
}

/* control CS line */
LOCAL	void	spi_cs(W ch, W cs, BOOL enable)
{
	return;
}
#endif

/* SPI send/receive */
EXPORT	ER	SPIXfer(W ch_cs, UB *xmit, UB *recv, W len)
{
	return(E_OK);
}

/* SPI driver prolog ends */
EXPORT	ER	SPIup(W ch, BOOL start)
{
	return(E_OK);
}
