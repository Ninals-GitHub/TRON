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
        em1d512_iic.c   RTC/Audio CODEC access (IIC) (EM1-D512)
 *
 */

#include "clkdrv.h"
#include <tk/util.h>
#include <device/em1d512_iic.h>

#ifdef	DEBUG
#define	DP(x)	printf x
#else
#define	DP(x)	/* do nothing */
#endif

#define	IICMAX		2
LOCAL	FastLock	IICLock[IICMAX];
LOCAL	ID		IICTskID[IICMAX];
LOCAL	const UW	IICBase[IICMAX] = {0x50040000, 0x50030000};
LOCAL	const UW	IICVec[IICMAX] = {IV_IRQ(33), IV_IRQ(39)};

#define	IIC_IIC(x)	(IICBase[x] + 0x0000)
#define	IIC_IICC(x)	(IICBase[x] + 0x0008)
#define	IIC_SVA(x)	(IICBase[x] + 0x000c)
#define	IIC_IICCL(x)	(IICBase[x] + 0x0010)
#define	IIC_IICSE(x)	(IICBase[x] + 0x001c)
#define	IIC_IICF(x)	(IICBase[x] + 0x0028)

#define	IICC_IICE	(1 << 7)
#define	IICC_LREL	(1 << 6)
#define	IICC_WREL	(1 << 5)
#define	IICC_SPIE	(1 << 4)
#define	IICC_WTIM	(1 << 3)
#define	IICC_ACKE	(1 << 2)
#define	IICC_STT	(1 << 1)
#define	IICC_SPT	(1 << 0)

#define	IICCL_CLD	(1 << 5)
#define	IICCL_DAD	(1 << 4)
#define	IICCL_SMC	(1 << 3)
#define	IICCL_DFC	(1 << 2)

#define	IICSE_MSTS	(1 << 15)
#define	IICSE_ALD	(1 << 14)
#define	IICSE_EXC	(1 << 13)
#define	IICSE_COI	(1 << 12)
#define	IICSE_TRC	(1 << 11)
#define	IICSE_ACKD	(1 << 10)
#define	IICSE_STD	(1 << 9)
#define	IICSE_SPD	(1 << 8)

#define	IICF_STCF	(1 << 7)
#define	IICF_IICBSY	(1 << 6)
#define	IICF_STCEN	(1 << 1)
#define	IICF_IICRSV	(1 << 0)

#define	TIMEOUT		1000000	// microsec

/* wait for register status */
LOCAL	ER	wait_state(UW addr, UW mask, UW value)
{
	W	i;

	for (i = TIMEOUT; i > 0; i--) {
		WaitUsec(1);
		if ((in_w(addr) & mask) == value) break;
	}

	return i ? E_OK : E_TMOUT;
}

/* interrupt handler */
LOCAL	void	iic_inthdr(INTVEC vec)
{
	W	i;

	for (i = 0; i < IICMAX; i++) {
		if (vec == IICVec[i]) {
			tk_wup_tsk(IICTskID[i]);
			break;
		}
	}

	ClearInt(vec);
	return;
}

/* wait for interrupt */
LOCAL	ER	wait_int(void)
{
	return tk_slp_tsk(TIMEOUT / 1000);
}

/* start/restart */
LOCAL	ER	send_start(W ch, UH addr)
{
	ER	er;
	UW	sts;

        /* generate start condition */
	out_w(IIC_IICC(ch), in_w(IIC_IICC(ch)) & ~IICC_ACKE);
	out_w(IIC_IICC(ch), in_w(IIC_IICC(ch)) |  IICC_STT);

        /* wait for acquiring master status */
	er = wait_state(IIC_IICSE(ch), IICSE_MSTS, IICSE_MSTS);
	if (er < E_OK) {
		DP(("send_start: wait_state %d\n", er));
		goto fin0;
	}

        /* sending slave address / transmission mode */
	out_w(IIC_IIC(ch), addr & 0xff);
	er = wait_int();
	if (er < E_OK) {
		DP(("send_start: wait_int %d\n", er));
		goto fin0;
	}

        /* error check */
	sts = in_w(IIC_IICSE(ch));
	if ((sts & IICSE_ALD) || !(sts & IICSE_ACKD)) {
		DP(("send_start: IICSE %#x\n", sts));
		er = E_IO;
		goto fin0;
	}

	er = E_OK;
fin0:
	return er;
}

/* stop */
LOCAL	ER	send_stop(W ch)
{
	ER	er;

        /* generate stop condition */
	out_w(IIC_IICC(ch), in_w(IIC_IICC(ch)) | IICC_SPT);

        /* wait for sending stop */
	er = wait_state(IIC_IICSE(ch), IICSE_SPD, IICSE_SPD);
	if (er < E_OK) {
		DP(("send_stop: wait_state %d\n", er));
	}

	return er;
}

/* sending data */
LOCAL	ER	send_data(W ch, UH data)
{
	ER	er;
	UW	sts;

        /* sending data */
	out_w(IIC_IIC(ch), data & 0xff);
	er = wait_int();
	if (er < E_OK) {
		DP(("send_data: wait_int %d\n", er));
		goto fin0;
	}

        /* checking NAK */
	sts = in_w(IIC_IICSE(ch));
	if (!(sts & IICSE_ACKD)) {
		DP(("send_data: IICSE %#x\n", sts));
		er = E_IO;
		goto fin0;
	}

	er = E_OK;
fin0:
	return er;
}

/* receiving data */
LOCAL	ER	recv_data(W ch, UH *cmddata)
{
	ER	er;
	W	cmd = *cmddata;

        /* On the initial data receive, switch to receive mode */
	if (cmd & IIC_TOPDATA) {
		out_w(IIC_IICC(ch), in_w(IIC_IICC(ch)) & ~IICC_WTIM);
		out_w(IIC_IICC(ch), in_w(IIC_IICC(ch)) |  IICC_ACKE);
	}

        /* initiate data receive */
	out_w(IIC_IICC(ch), in_w(IIC_IICC(ch)) | IICC_WREL);
	er = wait_int();
	if (er < E_OK) {
		DP(("recv_data: wait_int %d\n", er));
		goto fin0;
	}

        /* read data */
	*cmddata = (cmd & 0xff00) | (in_w(IIC_IIC(ch)) & 0xff);
	er = E_OK;
fin0:
        /* epilog processing invoked after an error or the last byte is read */
	if ((cmd & IIC_LASTDATA) || er < E_OK) {
		out_w(IIC_IICC(ch), in_w(IIC_IICC(ch)) |  IICC_WTIM);
		out_w(IIC_IICC(ch), in_w(IIC_IICC(ch)) & ~IICC_ACKE);
		out_w(IIC_IICC(ch), in_w(IIC_IICC(ch)) | IICC_WREL);
		wait_int();
	}

	return er;
}

/* IIC transmission processing */
EXPORT	ER	IICXfer(W ch, UH *cmddata, W words)
{
	ER	er;

        /* check channel number */
	if (ch < 0 || ch > 1) {
		er = E_PAR;
		goto fin0;
	}

	Lock(&IICLock[ch]);
	IICTskID[ch] = tk_get_tid();

        /* initialization */
	out_w(IIC_IICC(ch), 0);				// halt entire operation
	out_w(IIC_IICCL(ch), IICCL_SMC | IICCL_DFC);	// high-speed mode + filter
	out_w(IIC_IICF(ch), IICF_STCEN | IICF_IICRSV);	// force transmission
	out_w(IIC_IICC(ch), IICC_IICE | IICC_WTIM);	// IIC operation, 9 bits mode
	tk_can_wup(TSK_SELF);

        /* wait for bus free (since there is one master, the bus should be free, but just in case.) */
	wait_state(IIC_IICF(ch), IICF_IICBSY, 0);

        /* process according to the instruction */
	for (; words > 0; words--) {
		if (*cmddata & IIC_START) {
			er = send_start(ch, *cmddata);

		} else if (*cmddata & IIC_STOP) {
			er = send_stop(ch);

		} else if (*cmddata & IIC_SEND) {
			er = send_data(ch, *cmddata);

		} else if (*cmddata & IIC_RECV) {
			er = recv_data(ch, cmddata);

		} else {
			er = E_OK;	/* do nothing */

		}

                /* transmit stop condition upon encountering an error */
		if (er < E_OK) {
			send_stop(ch);
			goto fin1;
		}

		cmddata++;
	}

	er = E_OK;
fin1:
	out_w(IIC_IICC(ch), 0);	// halt entire operation
	Unlock(&IICLock[ch]);
fin0:
	return er;
}

/* IICdriver prolog ends */
EXPORT	ER	IICup(W ch, BOOL start)
{
#define	IICTag	"IIC_"

	ER	er;
	T_DINT	dint;

        /* check channel number */
	if (ch < 0 || ch > 1) {
		er = E_PAR;
		goto fin0;
	}

        /* epilog processing */
	if (!start) {
		er = E_OK;
		goto fin2;
	}

        /* creating a lock for exclusive control */
	er = CreateLock(&IICLock[ch], IICTag);
	if (er < E_OK) {
		DP(("IICup: CreateLock %d\n", er));
		goto fin0;
	}

        /* register interrupt handler */
	dint.intatr = TA_HLNG;
	dint.inthdr = iic_inthdr;
	er = tk_def_int(IICVec[ch], &dint);
	if (er < E_OK) {
		DP(("IICup: tk_def_int %d\n", er));
		goto fin1;
	}

        /* clear interrupt and permit it again */
	SetIntMode(IICVec[ch], IM_ENA);
	ClearInt(IICVec[ch]);
	EnableInt(IICVec[ch]);

	er = E_OK;
	goto fin0;

fin2:
	DisableInt(IICVec[ch]);
	ClearInt(IICVec[ch]);
	SetIntMode(IICVec[ch], IM_DIS);
	tk_def_int(IICVec[ch], NULL);
fin1:
	DeleteLock(&IICLock[ch]);
fin0:
	return er;
}
