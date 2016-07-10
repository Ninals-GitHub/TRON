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
        hwkbpd.c        lowkbpd machine-depdendent part
 *
 */
#include "kbpd.h"
#include <tk/syslib.h>
#include <device/devconf.h>
#include <device/em1d512_iic.h>

/* Get "DEVCONF" entry */
IMPORT	W	GetDevConf(UB *name, W *val);

LOCAL	ID	FlgID;			/* flag for event notification */

#define	TpFlg	(1 << 0)
#define	SwFlg	(1 << 1)

// ---------------------------------------------------------------------------
/* tablet parameter (coordinate normalizaton parameter, etc.) */
typedef	struct {
	W	x_bias;
	W	x_span;
	W	y_bias;
	W	y_span;
	W	nodsp;
	W	rate_off;
	W	rate_on;
} TABPAR;

LOCAL	TABPAR	TabPar = {944, -880, 912, -832, 0, 50, 50};

#define	ACC_DELTA_X	(8)	 	/* avoid pen jitter (X) */
#define	ACC_DELTA_Y	(8)	 	/* avoid pen jitter(Y) */
#define	INV_DELTA_X	(0)	 	/* ignore small displacement (X) */
#define	INV_DELTA_Y	(0)		/* ignore small displacement (Y) */

LOCAL	W	px, py, pb, ax, ay;	/* previous TP status */
LOCAL	RawEvt	PrevTpEvt;		/* area to save previous TP event */
LOCAL	UW	PrevTpSts = 0;		/* area to save previous TP status */
LOCAL	W	CurrTpSw = 0;		/* area to save the current TP touch status */
LOCAL	W	TpActive = 1;		/* TP active or inactive status */
LOCAL	FastLock	HwLock_tp;	/* for exclusive access control purposes */
LOCAL	ID	TpTaskID;		/* TP processing task ID */

#define	TpVec	IV_GPIO(0)
#define	RETRY	4

/* read DA9052 register */
LOCAL	W	ReadDA9052(W reg)
{
	W	er, i;
	UB	cmd[2];

	for (i = RETRY; i > 0; i--) {
		cmd[0] = (reg << 1) | 1;
		cmd[1] = ~0;
		er = em1d512_spixfer(0, cmd, cmd, sizeof(cmd));
		if (er >= E_OK) return cmd[1];
	}

	DP(("ReadDA9052(R%d) [%#x]\n", reg, er));
	return er;
}

/* write DA9052 register */
LOCAL	W	WriteDA9052(W reg, W dat)
{
	W	er, i;
	UB	cmd[2];

	for (i = RETRY; i > 0; i--) {
		cmd[0] = reg << 1;
		cmd[1] = dat;
		er = em1d512_spixfer(0, cmd, cmd, sizeof(cmd));
		if (er >= E_OK) return E_OK;
	}

	DP(("WriteDA9052(R%d, %#x) [%#x]\n", reg, dat, er));
	return er;
}

/* touch panel data processing */
LOCAL	void	tpproc(InMsg *msg)
{
	W	x, y, sw, dx, dy;
	RawEvt	evt;
	union {
		PdInStat	stat;
		UW		uw;
	} u;

	sw = msg->hw.dt[0];
	x  = msg->hw.dt[2] | (msg->hw.dt[3] << 8);
	y  = msg->hw.dt[4] | (msg->hw.dt[5] << 8);

	if (!sw) {
		if (!pb) goto fin;
		x = px;
		y = py;
	} else {	/* if pen on */
		if (pb) {
			/* avoid pointer jitter */
			dx = x - px;
			dy = y - py;
			if (dx < 0 && (dx += INV_DELTA_X) > 0) dx = 0;
			if (dx > 0 && (dx -= INV_DELTA_X) < 0) dx = 0;
			if (dy < 0 && (dy += INV_DELTA_Y) > 0) dy = 0;
			if (dy > 0 && (dy -= INV_DELTA_Y) < 0) dy = 0;
			ax += dx;
			ay += dy;
			if (ax > -ACC_DELTA_X && ax < ACC_DELTA_X &&
			    ay > -ACC_DELTA_Y && ay < ACC_DELTA_Y) goto fin;
		}
	}
	ax = ay = 0;
	px = x;
	py = y;

        /* set event status */
	u.uw = 0;
	evt.p.stat = u.stat;
	evt.p.stat.cmd   = INP_PD;
	evt.p.stat.main  = pb = sw ? 1 : 0;
	evt.p.stat.abs   = 1;	/* absolute movement only, relative motion unsupported */
	evt.p.stat.norel = 1;
	evt.p.stat.nodsp = TabPar.nodsp;

        /* normalize X/Y coordinates */
	x = (x - TabPar.x_bias) * PDIN_XMAX / TabPar.x_span;
	y = (y - TabPar.y_bias) * PDIN_YMAX / TabPar.y_span;

	evt.p.xpos = (x < 0) ? 0 : ((x > PDIN_XMAX) ? PDIN_XMAX : x);
	evt.p.ypos = (y < 0) ? 0 : ((y > PDIN_YMAX) ? PDIN_YMAX : y);

        /* send event */
	kpSendPdEvt(&evt, &PrevTpSts, &PrevTpEvt);

 fin:
	return;
}

/* enable TP interrupt */
LOCAL	void	tpIntEnable(void)
{
        /* interrupts can be missed using edge-triggermode,
         * so we use level-trigger mode (register handling of DA9052 is done via SPI:
         * pay attention to the delay caused by SPI communication) */
	SetIntMode(TpVec, IM_ENA | IM_LEVEL | IM_LOW);
	EnableInt(TpVec);

	return;
}

/* disable TP interrupt */
LOCAL	void	tpIntDisable(void)
{
	DisableInt(TpVec);
	SetIntMode(TpVec, IM_DIS);

	return;
}

/* TP interrupt handler */
LOCAL	void	tp_inthdr(INTVEC vec)
{
        /* wakeup a waiting task */
	tk_set_flg(FlgID, TpFlg);

        /* disable interrupt */
	DisableInt(vec);

	return;
}

/* read TP */
LOCAL	ER	tpread(UW *x, UW *y, UW *sw)
{
	W	sts, xmsb, ymsb, xylsb;

        /* obtain status */
	if ((sts = ReadDA9052(6)) < 0 ||
	    (xmsb = ReadDA9052(107)) < 0 ||
	    (ymsb = ReadDA9052(108)) < 0 ||
	    (xylsb = ReadDA9052(109)) < 0) return E_IO;

        /* x-axis <- y-measure, y-axis <- x-measure */
	*x = (ymsb << 2) | ((xylsb & 0x0c) >> 2);
	*y = (xmsb << 2) | ((xylsb & 0x03) >> 0);
	*sw = xylsb & 0x40;

        /* error if pen down and A/D conversion is not finished */
	return (*sw && !(sts & 0x80)) ? E_BUSY : E_OK;
}

/* TP processing */
LOCAL	void	tpscan(void)
{
	ER	er;
	UW	x, y, sw;
	InMsg	msg;

        /* read TP status */
	er = tpread(&x, &y, &sw);
	if (er < E_OK) goto fin0;

        /* if already in untouch state, do not send untouch message. */
	if(!sw && !CurrTpSw) goto fin0;

        /* change touch state */
	CurrTpSw = sw;

	msg.hw.id = HWPD;
	msg.hw.dt[0] = sw;
	msg.hw.dt[2] = x & 0xff;
	msg.hw.dt[3] = (x >> 8) & 0xff;
	msg.hw.dt[4] = y & 0xff;
	msg.hw.dt[5] = (y >> 8) & 0xff;
	tk_snd_mbf(InpMbf, &msg, sizeof(msg), TMO_POL);
fin0:
	return;
}

/* TP processing task */
LOCAL	void	tp_task(void)
{
	ER	er;
	W	i;
	UINT	flg;
	LOCAL	const UB	tpsetup[] = {
		 10, 0xff,	// IRQ_MASK_A (mask all)
		 11, 0xbf,	// IRQ_MASK_B M_PEN_DOWN
		 12, 0xff,	// IRQ_MASK_C (mask all)
		 13, 0xff,	// IRQ_MASK_D (mask all)
		  5, 0xff,	// EVENT_A (clear all)
		  6, 0xff,	// EVENT_B (clear all)
		  7, 0xff,	// EVENT_C (clear all)
		  8, 0xff,	// EVENT_D (clear all)
		 22, 0x09,	// GPIO3:TSIYN,  GPIO2:GPI,Active low
		 23, 0x00,	// GPIO5:TSIXN,  GPIO4:TSIYP
		 24, 0x00,	// GPIO7:TSIREF, GPIO6:TSIXP
		105, 0xc3,	// TSI_CONT_A 4slot, PEN_DET_EN, AUTO_TSI_EN
		106, 0x80,	// TSI_CONT_B X+/X-,Y+/Y-,Y+/X-
		 82, 0x40,	// 1ms interval
		  0, 0x00,	// (terminate)
	};
	
        /* DA9052 TSI initialization */
	for (i = 0; tpsetup[i]; i += 2) {
		er = WriteDA9052(tpsetup[i], tpsetup[i + 1]);
		if (er < E_OK) goto fin0;
	}

        /* enable interrupt */
	tpIntEnable();

	while (1) {
                /* wait for TP interrupt */
		tk_wai_flg(FlgID, TpFlg, TWF_ANDW | TWF_BITCLR, &flg, TMO_FEVR);

                // interrupt is prohibited inside the TP intterupt handler

                /* polling is done while pen touch state */
		while (1) {
			Lock(&HwLock_tp);
			tpscan();
			Unlock(&HwLock_tp);

			if (CurrTpSw) {
				tk_dly_tsk(TabPar.rate_on);
				WriteDA9052(6, 0xc0);
			} else {
                                /* if untouch state is entered, generate an interrupt
                                 * even if we clear status flag,
                                 * the interrupt is continuously generated for a while, so
                                 * we wait for a while after clearing the flag */
				WriteDA9052(6, 0xc0);
				tk_dly_tsk(TabPar.rate_off);
				break;
			}
		}

                /* re-enabling TP interrupt */
		EnableInt(TpVec);
	}

fin0:
	tpIntDisable();
	tk_exd_tsk();
}

/* start processing of TP processing task ID */
LOCAL	ER	tpstart(BOOL start)
{
	ER	er;
	W	n, par[L_DEVCONF_VAL];
	T_DINT	dint;
	T_CTSK	ctsk;

        /* termination */
	if (!start) {
		er = E_OK;
		if (!TpActive) goto fin0;
		else goto fin4;
	}

        /* enable or disable tablet */
	n = GetDevConf("TEngUseTablet", par);
	if (n > 0) TpActive = par[0] ? 1 : 0;
	if (!TpActive) goto fin0;

        /* set tablet parameters */
	n = GetDevConf("TEngTabletPar", par);
	if (n >= 6 && par[1] != 0 && par[3] != 0) {
		TabPar.x_bias   = par[0];
		TabPar.x_span   = par[1];
		TabPar.y_bias   = par[2];
		TabPar.y_span   = par[3];
		TabPar.nodsp    = par[4] ? 1 : 0;
		TabPar.rate_off = par[5];
		TabPar.rate_on  = (n >= 7) ? par[6] : par[5];
	}

        /* create a lock */
	er = CreateLock(&HwLock_tp, "lkbY");
	if (er < E_OK) goto fin0;

        /* set up interrupt handler */
	dint.inthdr = tp_inthdr;
	dint.intatr = TA_HLNG;
	er = tk_def_int(TpVec, &dint);
	if (er < E_OK) goto fin1;

        /* define TP processing task */
	SetOBJNAME(ctsk.exinf, "lkbY");
	ctsk.task = tp_task;
	ctsk.itskpri = DEF_PRIORITY;
	ctsk.stksz = TASK_STKSZ;
	ctsk.tskatr = TA_HLNG | TA_RNG0;
	ctsk.dsname[0] = 't';
	ctsk.dsname[1] = 'p';
	ctsk.dsname[2] = 'p';
	ctsk.dsname[3] = 's';
	ctsk.dsname[4] = '\0';
	er = tk_cre_tsk(&ctsk);
	if (er < E_OK) goto fin2;
	TpTaskID = er;

        /* start TP processing task */
	er = tk_sta_tsk(TpTaskID, 0);
	if (er < E_OK) goto fin3;

	er = E_OK;	
	goto fin0;
fin4:
	tpIntDisable();
	tk_ter_tsk(TpTaskID);
fin3:
	tk_del_tsk(TpTaskID);
fin2:
	tk_def_int(TpVec, NULL);
fin1:
	DeleteLock(&HwLock_tp);
fin0:
	return er;
}

// ---------------------------------------------------------------------------
LOCAL	UB	PrevSwMsg = 0;		/* area to store previous SW message */
LOCAL	UB	PrevSwSts = 0;		/* area to store previous SW readout */
LOCAL	FastLock	HwLock_sw;	/* for exclusive access control purposes */
LOCAL	ID	SwTaskID;		/* SW processing task ID */

LOCAL	const INTVEC	SwVec[] = {IV_GPIO(4), IV_GPIO(6), IV_GPIO(7)};
LOCAL	const UB	KeyCode[] = {
/*	P4/SW4 P5/--- P6/SW3 P7/SW2 P8/SW1 */
	0x6f,  0x00,  0x6d,  0x6e,  0x00,
};

#define	GIOBase(x)	(0xc0050000 + 0x00000040 * (x))
#define	GIO_L		0x00
#define	GIO_I(x)	(GIOBase(x) + 0x0010)

/* SW data processing */
LOCAL	void	swproc(InMsg *msg)
{
	INT	i, mask, kchg, kcode;

        /* send the difference from the previous data */
	kchg = PrevSwMsg ^ msg->hw.dt[0];
	for (i = 0, mask = 1; i < sizeof(KeyCode); i++, mask <<= 1) {
		if (!(mask & kchg)) continue;

		kcode = KeyCode[i];
		if (kcode) kpSendKeyEvt((msg->hw.dt[0] & mask) ?
					0x01 : 0x00, kcode);
	}

        /* save key status */
	PrevSwMsg = msg->hw.dt[0];

	return;
}

/* enable SW interrupt */
LOCAL	void	swIntEnable(void)
{
	W	i;

	for (i = 0; i < sizeof(SwVec) / sizeof(INTVEC); i++) {
		SetIntMode(SwVec[i], IM_ENA | IM_EDGE | IM_BOTH);
		ClearInt(SwVec[i]);
		EnableInt(SwVec[i]);
	}

	return;
}

/* disable SW interrupt */
LOCAL	void	swIntDisable(void)
{
	W	i;

	for (i = 0; i < sizeof(SwVec) / sizeof(INTVEC); i++) {
		DisableInt(SwVec[i]);
		SetIntMode(SwVec[i], IM_DIS);
		ClearInt(SwVec[i]);
	}

	return;
}

/* SW interrupt handler */
LOCAL	void	sw_inthdr(INTVEC vec)
{
        /* wakeup a waiting task */
	tk_set_flg(FlgID, SwFlg);

        /* clear interrupt flag */
	ClearInt(vec);

	return;
}

/* SW processing */
LOCAL	void	swscan(void)
{
	InMsg	msg;
	UW	sw = 0;

        /* read SW status */
//	sw = in_w(GIO_I(GIO_L));

        /* mask and shift unnecessary bits */
	sw &= 0x000000d0;	// P7,P6,P4
	sw >>= 4;		// bit0: P4

        /* if the state is different from the previously read state, send a message */
	if (sw != PrevSwSts) {
		msg.hw.id = HWKB;
		msg.hw.dt[0] = sw;
		tk_snd_mbf(InpMbf, &msg, sizeof(msg), TMO_POL);
		PrevSwSts = sw;
	}
	return;
}

/* SW processing task */
LOCAL	void	sw_task(void)
{
	UINT	flg;
	
        /* enable interrupt */
	swIntEnable();

	while (1) {
                /* wait for SW interrupt */
		tk_clr_flg(FlgID, ~SwFlg);
		tk_wai_flg(FlgID, SwFlg, TWF_ANDW, &flg, TMO_FEVR);

                /* wait for a while to suprress chattering */
		tk_dly_tsk(10);

                /* SW processing */
		Lock(&HwLock_sw);
		swscan();
		Unlock(&HwLock_sw);
	}

        /* emergency (usually control does not come here) */
	swIntDisable();
	tk_exd_tsk();
}

/* start processing of SW processing task */
LOCAL	ER	swstart(BOOL start)
{
	ER	er;
	W	i;
	T_DINT	dint;
	T_CTSK	ctsk;

        /* termination */
	if (!start) {
		er = E_OK;
		goto fin4;
	}

        /* create a lock */
	er = CreateLock(&HwLock_sw, "lkbX");
	if (er < E_OK) goto fin0;

        /* set up interrupt handler */
	dint.inthdr = sw_inthdr;
	dint.intatr = TA_HLNG;
	for (i = 0; i < sizeof(SwVec) / sizeof(INTVEC); i++) {
		er = tk_def_int(SwVec[i], &dint);
		if (er < E_OK) goto fin2;
	}

        /* define SW processing task */
	SetOBJNAME(ctsk.exinf, "lkbX");
	ctsk.task = sw_task;
	ctsk.itskpri = DEF_PRIORITY;
	ctsk.stksz = TASK_STKSZ;
	ctsk.tskatr = TA_HLNG | TA_RNG0;
	ctsk.dsname[0] = 's';
	ctsk.dsname[1] = 'w';
	ctsk.dsname[2] = '\0';
	er = tk_cre_tsk(&ctsk);
	if (er < E_OK) goto fin2;
	SwTaskID = er;

        /* start SW processing task */
	er = tk_sta_tsk(SwTaskID, 0);
	if (er < E_OK) goto fin3;

	er = E_OK;
	goto fin0;
fin4:
	swIntDisable();
	tk_ter_tsk(SwTaskID);
fin3:
	tk_del_tsk(SwTaskID);
fin2:
	for (i = 0; i < sizeof(SwVec) / sizeof(INTVEC); i++) {
		tk_def_int(SwVec[i], NULL);
	}
/*fin1:*/
	DeleteLock(&HwLock_sw);
fin0:
	return er;
}

// ---------------------------------------------------------------------------
/* process received data */
EXPORT	void	hwProc(InMsg *msg)
{
	switch (msg->hw.id) {
	case	HWKB:	swproc(msg);			break;
	case	HWPD:	if (TpActive) tpproc(msg);	break;
	default:					break;
	}

	return;
}

/* set input mode */
EXPORT	void	hwImode(W inpmd)
{
	return;		/* no configurable item : do nothing */
}

/* device initialization processing(cmd: DC_OPEN/DC_SUSPEND/DC_RESUME) */
EXPORT	ER	hwInit(W cmd)
{
	ER	er = E_OK;
	T_CFLG	cflg;

	switch (cmd) {
	default:			/* not applicable */
		return E_OK;

	case	DC_SUSPEND:		/* suspend */
		Lock(&HwLock_sw);
		if (TpActive) Lock(&HwLock_tp);
		return E_OK;

	case	DC_RESUME:		/* resume */
		if (TpActive) Unlock(&HwLock_tp);
		Unlock(&HwLock_sw);
		return E_OK;

	case	DC_OPEN:		/* initial reset */
		break;
	}

	SetOBJNAME(cflg.exinf, "lkbZ");
	cflg.flgatr = TA_TFIFO | TA_WMUL;
	cflg.iflgptn = 0;
	er = tk_cre_flg(&cflg);
	if (er < E_OK) goto fin0;
	FlgID = er;

	er = swstart(TRUE);
	if (er < E_OK) goto fin1;

	er = tpstart(TRUE);
	if (er < E_OK) goto fin2;

	er = E_OK;
	goto fin0;

/*fin3:*/
	tpstart(FALSE);
fin2:
	swstart(FALSE);
fin1:
	tk_del_flg(FlgID);
fin0:
	return er;
}
