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
        common.c        KB/PD real I/O drvier common part
 *
 */
#include	"kbpd.h"
#include	"scancode.h"

/*
        event buffer
*/
#define	MAXRAWEVT	32
LOCAL	RawEvt	evtbuf[MAXRAWEVT + 1];		/* the last entry is reserved */
LOCAL	W	evtbuf_init = 0;

/*
        PD scaling data
*/
#define	RATIO_ONE	(24)
#define	ACC_RATE	(3)
#define PDMOV_XMAX      512 /*640*/
LOCAL	UB	sensetbl[] =
	{6, 8, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168};
LOCAL	UB	threshold[] = {0, 3, 7, 11, 15, 23, 39, 63};

/*
        scan code conversion table when NumLock is OFF
*/
LOCAL	UB	numtbl[] = {
  /* 47-4A */	T_7,	T_8,	T_9,	0x4a,	/* 7  8  9  -	*/
  /* 4B-4E */	T_4,	T_5,	T_6,	0x4e,	/* 4  5  6  +	*/
  /* 4F-51 */	T_1,	T_2,	T_3,		/* 1  2  3	*/
  /* 52-53 */	T_0,	T_PERIOD		/* 0  .		*/
};

/*
        memory allocation / release
*/
EXPORT	void*	MemAlloc(W size)
{
static	W	blksz = 0;
	T_RSMB	msts;
	void*	ptr;

	if (blksz == 0) {
		if (tk_ref_smb(&msts) < E_OK) return NULL;
		blksz = msts.blksz;
	}
	if (tk_get_smb(&ptr, (size + blksz - 1) / blksz,
				TA_RNG0) >= E_OK) return ptr;
	return NULL;
}
EXPORT	void	MemFree(void *ptr)
{
	tk_rel_smb(ptr);
}
/*
        event notification to upper KB/PD driver
*/
EXPORT	ER	kpSendMsg(RawEvt *msg)
{
	ER	er;
	W	i;

	if (evtbuf_init == 0) {	/* initialization */
		memset(evtbuf, 0, sizeof(evtbuf));
		for (i = 0; i < MAXRAWEVT + 1; i++) evtbuf[i].p.stat.read = 1;
		evtbuf_init++;
	}

        /* look for message buffer free space: the last one is reserved. */
	for (i = 0; i < MAXRAWEVT && evtbuf[i].p.stat.read == 0; i++);

        /* if we can not even use the reserved slot in event buffer, then error */
	if (i == MAXRAWEVT && evtbuf[i].p.stat.read == 0) {
		DP(("kpSendMsg: evtbuf overflow\n"));
		er = E_TMOUT;
		goto fin0;
	}		

        /* set up event */
	evtbuf[i] = *msg;

        /* set OVRRUN if we use the reserved slot */
	if (i == MAXRAWEVT) {
		DP(("kpSendMsg: DEV_OVRRUN\n"));
		evtbuf[i].p.stat.err = DEV_OVRRUN;
	}

        /* send event */
	//er = tk_snd_mbx(EvtMbx, (T_MSG*)&evtbuf[i]);
	if (er < E_OK) {
		evtbuf[i].p.stat.read = 1;
		DP(("kpSendMsg: err=%#x\n", er));
	}
 fin0:
	return er;
}
/*
        Scaling based on PD sensitivity (for relative coordinate device)
*/
EXPORT	void	kpScalingPos(RawEvt *evt, W x, W y, PNT *fract, W x_max)
{
	W	acc, sns, ax, ay, max_xy;
	W	ratio, bias;
	W	base = RATIO_ONE;

	sns = PdSense & PD_SNMSK;
	acc = (PdSense & PD_ACMSK) >> 9;

        /* process acceleration */
	if (acc != 0) {
		if ((ax = x) < 0) ax = - ax;
		if ((ay = y) < 0) ay = - ay;
		if ((max_xy = ax) < ay) max_xy = ay;
		if (max_xy > (bias = threshold[acc])) {
			bias *= (ACC_RATE - 1);
			ax = ax * ACC_RATE - (bias * ax / max_xy);
			ay = ay * ACC_RATE - (bias * ay / max_xy);
			x = (x < 0) ? (- ax) : (ax);
			y = (y < 0) ? (- ay) : (ay);
		}
                /* adjust magnification based on sensitivity */
		base += sns;
	}

        /* magnification based on sensitivity : ratio / base */
	ratio = sensetbl[sns];

	if (x_max > 0) {
                /* for tablet, the maximum movement along X-axis is PDMOV_XMAX
                   and the sensitivity is adjusted to keep the maximum value to the above value. */
		ratio *= PDMOV_XMAX / base;
		base = x_max;
	}
        /* instead of correcting the residue accurately, if the residue is below 1,
                        we accumulate the residue, then we achieve better behavior */
	x = x * ratio + fract->x;
	evt->p.xpos = x / base;
	fract->x = (evt->p.xpos == 0) ? x : 0;

	y = y * ratio + fract->y;
	evt->p.ypos = y / base;
	fract->y = (evt->p.ypos == 0) ? y : 0;
}
/*
        send PD event
*/
EXPORT	void	kpSendPdEvt(RawEvt *evt, UW *lsts, RawEvt *last)
{
	union {
		PdInStat	stat;
		UW		uw;
	} u;

        /* if there is a difference in events, send KB/PD event */
	u.stat = evt->p.stat;
	if (lsts[0] == u.uw &&
		evt->p.stat.inv == 0 && evt->p.stat.vst == 0) {

		if (evt->p.stat.abs == 0) {	/* relative movement */
			if (evt->p.xpos == 0 && evt->p.ypos == 0) return;
		} else {			/* absolute movement */
			if (evt->p.xpos == last->p.xpos &&
			    evt->p.ypos == last->p.ypos) return;
		}
	}

        /* save the event to send */
	lsts[0] = u.uw;
	*last = *evt;

        /* send event */
	kpSendMsg(evt);
}
/*
        send wheel event
*/
EXPORT	void	kpSendWheelEvt(W z)
{
	RawEvt	evt;
	union {
		PdIn2Stat	stat;
		UW		uw;
	} u;

	if (z != 0) {
		u.uw = 0;
		evt.p2.stat = u.stat;
		evt.p2.stat.cmd = INP_PD2;
		evt.p2.wheel = z;
		evt.p2.rsv = 0;
		kpSendMsg(&evt);
	}
}
/*
        send mouse event
*/
EXPORT	void	kpSendMouseEvt(W but, W x, W y, W z, UW *lsts)
{
	RawEvt	evt, dummy;
	union {
		PdInStat	stat;
		UW		uw;
	} u;

        /* send mouse wheel event */
	kpSendWheelEvt(z);

        /* scaling processing based on sensitivity */
	kpScalingPos(&evt, x, y, (PNT*)&lsts[1], 0);

        /* set up PD event information */
	u.uw = 0;
	evt.p.stat = u.stat;
	evt.p.stat.cmd = INP_PD;
	evt.p.stat.butrev = 1;
	evt.p.stat.onebut = 1;
	evt.p.stat.main = (but & 0x01) ? 1 : 0;
	evt.p.stat.sub = (but & 0x02) ? 1 : 0;
	evt.p.stat.qpress = (but & 0x04) ? 1 : 0;

        /* if there is a difference in events, send KB/PD event */
	kpSendPdEvt(&evt, lsts, &dummy);
}
/*
        send key event
*/
EXPORT	void	kpSendKeyEvt(W sts, W keycode)
{
	RawEvt	evt;
	union {
		KeyInStat	stat;
		UW		uw;
	} u;

	if (keycode <= 0) return;

        /* process NumLock key */
	if (keycode == NumLock) {
		if (sts & 1) {	/* send command to reverse NumLock status */
			kpSendDrvCmd(InputModeCmd(
				((InpMode ^ NumLockON) & NumLockON) | 0x100));
		}
		return;
	}

	u.uw = 0;
	evt.k.stat = u.stat;
	evt.k.stat.cmd = INP_KEY;
	evt.k.stat.kbid = KbdId;
	evt.k.stat.press = sts & 1;

        /* process ten key pad */
	if (keycode >= NumLockLo && keycode <= NumLockHi) {
		if ((InpMode & NumLockON) == 0)
			keycode = numtbl[keycode - NumLockLo];
		evt.k.stat.tenkey = 1;
	}
	evt.k.keytop = keycode;

        /* send KB/PD event */
	kpSendMsg(&evt);
}
/*
        send command to command processing task (within this driver)
*/
EXPORT	ER	kpSendDrvCmd(UW cmd)
{
	ER	er;
	UINT	dmy;

        /* wait for ready to send command
                        this is aborted by timeout in order to avoid deadlock */
	er = tk_wai_flg(CmdFlg, DeviceCommandReady, TWF_ORW | TWF_CLR,
			&dmy, 200);
	if (er >= E_OK) {	/* send command */
		er = tk_set_flg(CmdFlg, cmd);
	}
	if (er < E_OK) {
		DP(("kpSendDrvCmd: err=%#x\n", er));
	}
	return er;
}
/*
        data receiving task
*/
EXPORT	void	kpDataTask(void)
{
	InMsg	msg;
	W	sz;
	
	for (;;) {	/* it never quits itself */
                /* data receive: ignored if error or suspended state is in effect. */
		if ((sz = tk_rcv_mbf(InpMbf, &msg, TMO_FEVR)) < E_OK) {
			continue;
		}
		if (sz != sizeof(msg)) continue;
		if (Suspended == TRUE) continue;

                /* process received data */
		hwProc(&msg);
	}
}
/*
        command processing task
*/
EXPORT	void	kpCmdTask(void)
{
	UW	dt;
	UINT	cmd;
	
	for (;;) {	/* it never quits itself */

                /* command receive permitted */
		if (tk_set_flg(CmdFlg, DeviceCommandReady) < E_OK) continue;

                /* wait for receiving a command */
		if (tk_wai_flg(CmdFlg, ~DeviceCommandReady, TWF_ORW | TWF_CLR,
			       &cmd, TMO_FEVR) < E_OK) continue;

                /* do processing based on command */
		dt = cmd & 0x00ffffff;
		switch (cmd & 0xff000000) {
		case ScanRateCmd(0):	/* scan speed - unsupported */
			break;

		case SenseCmd(0):	/* scan speed - unsupported */
			PdSense = dt;
			break;

		case InputModeCmd(0):	/* input mode */
                        /* lower two bits control normal input mode, and the upper bits
                           are for extended input mode (machine dependent), and these are set independently. */
			dt |= InpMode & ((dt & ~0x3) ? 0x3 : ~0x3);
			if ((dt &= 0xff) != InpMode) {
				InpMode = dt;
				if (Suspended == FALSE) {
					hwImode(dt);
				}
			}
			break;

		case SuspendKBPD:	/* suspend / resume */
			if (cmd == SuspendKBPD) {	/* suspend */
				if (Suspended == FALSE) {
					Suspended = TRUE;
					hwInit(DC_SUSPEND);
				}
			} else {			/* resume */
				if (Suspended == TRUE) {
					hwInit(DC_RESUME);
					Suspended = FALSE;
				}
			}
			break;

		default:
			DP(("kpCmdTask: cmd=%#08x\n", cmd));
		}
	}
}
