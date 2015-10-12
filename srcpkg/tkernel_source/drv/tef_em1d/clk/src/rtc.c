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
 *	rtc.c	(clkdrv)
        clock driver RTC handler
 *
 */

#include "clkdrv.h"
#include <stdlib.h>
#include "em1d512_iicspi_svc.h"
#include <device/em1d512_iic.h>

/* RX-4581NB RTC definitions */
#define	rxSEC		0x0
#define	rxMIN		0x1
#define	rxHOUR		0x2
#define	rxWEEK		0x3
#define	rxDAY		0x4
#define	rxMONTH		0x5
#define	rxYEAR		0x6
#define	rxEXT		0xd
#define	rxFLAG		0xe
#define	rxCTRL		0xf

/* number of retries */
#define	RETRY		4

/* SPI channel used by RTC */
#define	SPICH_RTC	2	// SP0:CS2

/* binary <-> BCD conversion */
#define	BCDtoBIN(v)	(((v) >> 4) * 10 + ((v) & 0xF))
#define	BINtoBCD(v)	((((v) / 10) << 4) + ((v) % 10))


/* RTC access interface */
LOCAL	W	rtcRead(W reg)
{
	W	i, er;
	UB	xmit[2], recv[2];

	xmit[0] = recv[0] = (reg & 0x0f) | 0x80;
	xmit[1] = recv[1] = ~0;

	for (i = 0; i < RETRY; i++) {
		er = em1d512_spixfer(SPICH_RTC, xmit, recv, sizeof(xmit));
		if (er >= E_OK) break;
	}

	return recv[1];
}

LOCAL	void	rtcWrite(W reg, W dat)
{
	W	i, er;
	UB	xmit[2];

	xmit[0] = reg & 0x0f;
	xmit[1] = dat;

	for (i = 0; i < RETRY; i++) {
		er = em1d512_spixfer(SPICH_RTC, xmit, NULL, sizeof(xmit));
		if (er >= E_OK) break;
	}

	return;
}

/* set current date time */
EXPORT	ER	cdSetDateTime(void *date_tim)
{
	DATE_TIM	dt;
	div_t		year;

        /* read calendar date time */
	dt = *((DATE_TIM *)date_tim);

        /* BIN -> BCD conversion */
	year = div(dt.d_year + 1900, 100);
	dt.d_year = BINtoBCD(year.rem);
	dt.d_month = BINtoBCD(dt.d_month);
	dt.d_wday = BINtoBCD(dt.d_wday);
	dt.d_day = BINtoBCD(dt.d_day);
	dt.d_hour = BINtoBCD(dt.d_hour);
	dt.d_min = BINtoBCD(dt.d_min);
	dt.d_sec = BINtoBCD(dt.d_sec);

        /* stop counter temporarily */
	rtcWrite(rxCTRL, 0x02);

        /* set current date time */
	rtcWrite(rxSEC, dt.d_sec);
	rtcWrite(rxMIN, dt.d_min);
	rtcWrite(rxHOUR,  dt.d_hour);
	rtcWrite(rxWEEK, 0x01);		// we do not use day of the week
	rtcWrite(rxDAY, dt.d_day);
	rtcWrite(rxMONTH,  dt.d_month);
	rtcWrite(rxYEAR, dt.d_year);

        /* resume counter */
	rtcWrite(rxCTRL, 0x00);

	return E_OK;
}

/* get current time */
EXPORT	ER	cdGetDateTime(void *date_tim)
{
	DATE_TIM	dt;
	INT	sec;

        /* get current time */
	do {
		dt.d_sec   = rtcRead(rxSEC);
		dt.d_min   = rtcRead(rxMIN);
		dt.d_hour  = rtcRead(rxHOUR);
		dt.d_day   = rtcRead(rxDAY);
		dt.d_month = rtcRead(rxMONTH);
		dt.d_year  = rtcRead(rxYEAR);
		sec        = rtcRead(rxSEC);
	} while (sec != dt.d_sec);	/* make sure data is read consistently in a whole second */

	dt.d_wday = 0;	// not supported (0 - pretend it is sunday)
	dt.d_days = 0;	// not used
	dt.d_week = 0;	// not used

	dt.d_year  = BCDtoBIN(dt.d_year) + 100;	/* 00-99 -> 2000-2099 */
	dt.d_month = BCDtoBIN(dt.d_month);
	dt.d_wday  = BCDtoBIN(dt.d_wday);
	dt.d_day   = BCDtoBIN(dt.d_day);
	dt.d_hour  = BCDtoBIN(dt.d_hour);
	dt.d_min   = BCDtoBIN(dt.d_min);
	dt.d_sec   = BCDtoBIN(dt.d_sec);

	*((DATE_TIM *)date_tim) = dt;

	return E_OK;
}

/* set / get automatic power-on time (not supported) */
EXPORT	ER	cdSetAutoPwOn(void *date_tim)	{return E_NOSPT;}
EXPORT	ER	cdGetAutoPwOn(void *date_tim)	{return E_NOSPT;}

/* read / write non-volatile register */
EXPORT	INT	cdSetRegister(void *buf, INT size)	{return E_NOSPT;}
EXPORT	INT	cdGetRegister(void *buf, INT size)	{return E_NOSPT;}

/* hardware initialization */
EXPORT	ER	cdInitHardware(void)
{
	ER	er;

	er = em1d512_iicspi_svc(TRUE);

	if (er >= E_OK) {
		rtcWrite(rxEXT, 0x00);
		rtcWrite(rxFLAG, 0x00);
		rtcWrite(rxCTRL, 0x00);
	}

	return er;
}

/* hardware stop processing */
EXPORT	ER	cdFinishHardware(void)
{
	em1d512_iicspi_svc(FALSE);
	return E_OK;
}
