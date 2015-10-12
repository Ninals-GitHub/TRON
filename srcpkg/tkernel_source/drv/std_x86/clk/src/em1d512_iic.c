/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/09/22
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
	W	i=1;
	return i ? E_OK : E_TMOUT;
}

/* interrupt handler */
LOCAL	void	iic_inthdr(INTVEC vec)
{
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
	return(E_OK);
}

/* stop */
LOCAL	ER	send_stop(W ch)
{
	return(E_OK);
}

/* sending data */
LOCAL	ER	send_data(W ch, UH data)
{
	return(E_OK);
}

/* receiving data */
LOCAL	ER	recv_data(W ch, UH *cmddata)
{
	return(E_OK);
}

/* IIC transmission processing */
EXPORT	ER	IICXfer(W ch, UH *cmddata, W words)
{
	return(E_OK);
}

/* IICdriver prolog ends */
EXPORT	ER	IICup(W ch, BOOL start)
{
	return(E_OK);
}
