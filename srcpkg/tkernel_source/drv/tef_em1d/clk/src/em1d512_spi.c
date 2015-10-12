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
LOCAL	FastLock	SPILock[SPIMAX];
LOCAL	ID		SPITskID[SPIMAX];
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

/* interrupt handler */
LOCAL	void	spi_inthdr(INTVEC vec)
{
	W	i;

	for (i = 0; i < SPIMAX; i++) {
		if (vec == SPIVec[i]) {
			out_w(SPn_FFCLR(i), ~0);
			tk_wup_tsk(SPITskID[i]);
			break;
		}
	}

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
	out_w(SPn_MODE(ch), SPIMode[ch]);
	out_w(SPn_TIECS(ch), 0x000f);		// CS: control by SPn_POL
	out_w(SPn_POL(ch), SPIPol[ch]);
	out_w(SPn_ENCLR(ch), ~0);		// interrupt disable

	out_w(SPn_CONTROL(ch), 0x0100);		// starting reset
	WaitUsec(10);
	out_w(SPn_CONTROL(ch), 0x0000);		// releasing reset
	out_w(SPn_CONTROL2(ch), 0x0000);

	out_w(SPn_FFCLR(ch), ~0);
	out_w(SPn_ENSET(ch), 0x0004);		// interrupt enable

	return;
}

/* control CS line */
LOCAL	void	spi_cs(W ch, W cs, BOOL enable)
{
	WaitNsec(200);
	out_w(SPn_POL(ch), SPIPol[ch] ^ (enable ? (1 << (cs * 3)) : 0));
	WaitNsec(200);

	return;
}

/* SPI send/receive */
EXPORT	ER	SPIXfer(W ch_cs, UB *xmit, UB *recv, W len)
{
	ER	er;
	W	i, ch, cs;

        /* parameter check */
	ch = (ch_cs >> 8) & 0xff;
	cs = (ch_cs >> 0) & 0xff;
	if (ch > 2 || cs > 3) {
		er = E_PAR;
		goto fin0;
	}

	Lock(&SPILock[ch]);
	SPITskID[ch] = tk_get_tid();
	tk_can_wup(TSK_SELF);

	spi_cs(ch, cs, TRUE);

        /* send / receive */
	for (i = 0; i < len; i++) {
		if (xmit != NULL) {
			out_w(SPn_TX_DATA(ch), *xmit++);
		} else {
			out_w(SPn_TX_DATA(ch), ~0);
		}

		out_w(SPn_CONTROL(ch), 0x000d);
		er = wait_int();
		if (er < E_OK) {
			DP(("spi_txrx: wait_int %d\n", er));
			spi_init(ch);
			goto fin1;
		}

		if (recv != NULL) {
			*recv++ = in_w(SPn_RX_DATA(ch));
		} else {
			in_w(SPn_RX_DATA(ch));
		}
	}

	er = E_OK;

fin1:
	spi_cs(ch, cs, FALSE);
	Unlock(&SPILock[ch]);
fin0:
	return er;
}

/* SPI driver prolog ends */
EXPORT	ER	SPIup(W ch, BOOL start)
{
#define	SPITag	"SPI_"

	ER	er;
	T_DINT	dint;

        /* check channel number */
	if (ch < 0 || ch > 2) {
		er = E_PAR;
		goto fin0;
	}

        /* epilog processing */
	if (!start) {
		er = E_OK;
		goto fin2;
	}

        /* creating a lock for exclusive control */
	er = CreateLock(&SPILock[ch], SPITag);
	if (er < E_OK) {
		DP(("SPIup: CreateLock %d\n", er));
		goto fin0;
	}

        /* register interrupt handler */
	dint.intatr = TA_HLNG;
	dint.inthdr = spi_inthdr;
	er = tk_def_int(SPIVec[ch], &dint);
	if (er < E_OK) {
		DP(("SPIup: tk_def_int %d\n", er));
		goto fin1;
	}

        /* clear interrupt and permit it again */
	spi_init(ch);
	SetIntMode(SPIVec[ch], IM_ENA);
	EnableInt(SPIVec[ch]);

	er = E_OK;
	goto fin0;

fin2:
	DisableInt(SPIVec[ch]);
	SetIntMode(SPIVec[ch], IM_DIS);
	tk_def_int(SPIVec[ch], NULL);
fin1:
	DeleteLock(&SPILock[ch]);
fin0:
	return er;
}
