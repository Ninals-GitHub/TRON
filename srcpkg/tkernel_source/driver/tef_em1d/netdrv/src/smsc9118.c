/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	@(#)smsc9118.c
 *
 */

#include "netdrv.h"

/* Definitions of I/O Address and size */
#define	SMSC9118_IOSZ		0x100

/* Definitions of registers */
#define	RX_DATA			0x00
#define	TX_DATA			0x20
#define	RX_STS			0x40
#define	TX_STS			0x48
#define	ID_REV			0x50
#define	IRQ_CFG			0x54
#define	INT_STS			0x58
#define	INT_EN			0x5c
#define	BYTE_TEST		0x64
#define	FIFO_INT		0x68
#define	RX_CFG			0x6c
#define	TX_CFG			0x70
#define	HW_CFG			0x74
#define	RX_DP_CTL		0x78
#define	RX_FIFO_INF		0x7c
#define	TX_FIFO_INF		0x80
#define	PMT_CTRL		0x84
#define	GPIO_CFG		0x88
#define	ENDIAN			0x98
#define	RX_DROP			0xa0
#define	MAC_CSR_CMD		0xa4
#define	MAC_CSR_DATA		0xa8
#define	AFC_CFG			0xac
#define	E2P_CMD			0xb0
#define	E2P_DATA		0xb4

/* IRQ_CFG */
#define	IRQ_CFG_INT_DEAS(x)	((x) << 24)
#define	IRQ_CFG_IRQ_EN		(1 << 8)
#define	IRQ_CFG_IRQ_POL		(1 << 4)
#define	IRQ_CFG_IRQ_TYPE	(1 << 0)

/* INT_STS, INT_EN */
#define	INT_TXSTOP		(1 << 25)
#define	INT_RXSTOP		(1 << 24)
#define	INT_TSFF		(1 << 8)
#define	INT_TSFL		(1 << 7)
#define	INT_RSFF		(1 << 4)
#define	INT_RSFL		(1 << 3)

/* BYTE_TEST */
#define	BYTE_TEST_MAGIC		0x87654321

/* FIFO_INT */
#define	FIFO_INT_TSFL(x)	((x) << 16)
#define	FIFO_INT_RSFL(x)	((x) << 0)

/* RX_CFG */
#define	RX_CFG_ALIGN_4		(0 << 30)
#define	RX_CFG_DUMP		(1 << 15)
#define	RX_CFG_DOFF(x)		((x) << 8)

/* TX_CFG */
#define	TX_CFG_SDUMP		(1 << 15)
#define	TX_CFG_DDUMP		(1 << 14)
#define	TX_CFG_ON		(1 << 1)
#define	TX_CFG_STOP		(1 << 0)

/* HW_CFG */
#define	HW_CFG_SF		(1 << 20)
#define	HW_CFG_TX_FIF_SZ(x)	((x) << 16)
#define	HW_CFG_PHY_CLK_INT	(0 << 5)
#define	HW_CFG_SRST_TO		(1 << 1)
#define	HW_CFG_SRST		(1 << 0)

/* RX_DP_CTL */
#define	RX_DP_CTL_FFWD		(1 << 31)

/* PMT_CTRL */
#define	PMT_CTRL_PM_MODE(x)	((x) << 12)
#define	PMT_CTRL_PHY_RST	(1 << 10)
#define	PMT_CTRL_READY		(1 << 0)

/* MAC_CSR_CMD */
#define	MAC_CSR_CMD_BUSY	(1 << 31)
#define	MAC_CSR_CMD_READ	(1 << 30)
#define	MAC_CSR_CMD_ADDR(x)	((x) << 0)

/* E2P_CMD */
#define	E2P_CMD_BUSY		(1 << 31)
#define	E2P_CMD_READ		(0 << 28)
#define	E2P_CMD_TMO		(1 << 9)
#define	E2P_CMD_ADDR(x)		((x) << 0)

/* MAC Control and Status Registers */
#define	MAC_CR			0x01
#define	ADDRH			0x02
#define	ADDRL			0x03
#define	HASHH			0x04
#define	HASHL			0x05
#define	MII_ACC			0x06
#define	MII_DATA		0x07
#define	FLOW			0x08
#define	VLAN1			0x09
#define	VLAN2			0x0a
#define	WUFF			0x0b
#define	WUCSR			0x0c

/* MAC_CR */
#define	MAC_CR_FDPX		(1 << 20)
#define	MAC_CR_TXEN		(1 << 3)
#define	MAC_CR_RXEN		(1 << 2)

/* MII_ACC */
#define	MII_ACC_PHYADR(x)	((x) << 11)
#define	MII_ACC_MIIRINDA(x)	((x) << 6)
#define	MII_ACC_MIIWnR		(1 << 1)
#define	MII_ACC_MIIBZY		(1 << 0)

/* Parameters in NetInf */
#define	BadState		wrk.uw[0]
#define	ChipID			wrk.uw[3]
#define	IntMask			wrk.uw[6]

/* RX Status */
#define	RX_STS_ES		(1 << 15)

/* TX Command 'A' */
#define	TX_CMDA_ALIGN_4		(0 << 24)
#define	TX_CMDA_OFFSET(x)	((x) << 16)
#define	TX_CMDA_FS		(1 << 13)
#define	TX_CMDA_LS		(1 << 12)
#define	TX_CMDA_BUFSZ(x)	((x) << 0)

/* TX Command 'B' */
#define	TX_CMDB_PADDIS		(1 << 12)
#define	TX_CMDB_PKTSZ(x)	((x) << 0)

/* TX Status */
#define	TX_STS_ES		(1 << 15)

/* I/O access offset */
#define	IOBase			di.iobase

#define	POLL_LIMIT		100000		/* x 45ns */
#define	DLY_LIMIT		100		/* x 10ms */

#define	TX_FIFO_SIZE		5		// Tx FIFO size, kByte

/* Vendor Name, Product Name */
LOCAL	const UB	*SMSC9118_PNAME1[] = {
	"SMSC ",
};

LOCAL	const UB	*SMSC9118_PNAME2[] = {
	"LAN9118 family",
};


/*
 *	Wait for not busy of register
 */
LOCAL	void	wait_reg(NetInf *inf, W reg, W mask, W val)
{
	UW	IOB = inf->IOBase;
	W	i;

	for (i = POLL_LIMIT; i > 0; i--) {
		InW(BYTE_TEST);		/* needs delay of 1 read */
		if ((InW(reg) & mask) == val) goto fin0;
	}
	for (i = DLY_LIMIT; i > 0; i--) {
		if (tk_dly_tsk(10) < E_OK) InW(BYTE_TEST);
		if ((InW(reg) & mask) == val) goto fin0;
	}

	inf->BadState = 1;
	DP(("wait_reg: reg %#x mask %#x timeout\n", reg, mask));
 fin0:
	return;
}

/*
 *	Wait for not busy of MAC register
 */
#define	wait_mac(inf)	wait_reg((inf), MAC_CSR_CMD, MAC_CSR_CMD_BUSY, 0)

/*
 *	Write to MAC register
 */
LOCAL	void	poke_mac(NetInf *inf, W reg, W dat)
{
	UW	IOB = inf->IOBase;

	wait_mac(inf);
	OutW(MAC_CSR_DATA, dat);
	OutW(MAC_CSR_CMD, reg | MAC_CSR_CMD_BUSY);
	wait_mac(inf);

	return;
}

/*
 *	Read from MAC register
 */
LOCAL	W	peek_mac(NetInf *inf, W reg)
{
	UW	IOB = inf->IOBase;

	wait_mac(inf);
	OutW(MAC_CSR_CMD, reg | MAC_CSR_CMD_BUSY | MAC_CSR_CMD_READ);
	wait_mac(inf);

	return InW(MAC_CSR_DATA);
}

/*
 *	Wait for not busy of MII register
 */
LOCAL	void	wait_mii(NetInf *inf)
{
	W	i;

	for (i = POLL_LIMIT; i > 0; i--) {
		if (!(peek_mac(inf, MII_ACC) & MII_ACC_MIIBZY)) break;
	}

	if (!i) {
		inf->BadState = 1;
		DP(("wait_mii: timeout\n"));
	}

	return;
}

/*
 *	Write to MII register
 */
LOCAL	void	poke_mii(NetInf *inf, W adr, W reg, W dat)
{
	wait_mii(inf);
	poke_mac(inf, MII_DATA, dat);
	poke_mac(inf, MII_ACC, (MII_ACC_PHYADR(adr) | MII_ACC_MIIRINDA(reg) |
				MII_ACC_MIIBZY | MII_ACC_MIIWnR));
	wait_mii(inf);

	return;
}

/*
 *	Read from MII register
 */
LOCAL	UH	peek_mii(NetInf *inf, W adr, W reg)
{
	wait_mii(inf);
	poke_mac(inf, MII_ACC, (MII_ACC_PHYADR(adr) | MII_ACC_MIIRINDA(reg) |
				MII_ACC_MIIBZY));
	wait_mii(inf);

	return peek_mac(inf, MII_DATA);
}

/*
 *	Wait for not busy of Serial EEPROM
 */
#define	wait_srom(inf)	wait_reg((inf), E2P_CMD, E2P_CMD_BUSY, 0)

/*
 *	Read from Serial EEPROM
 */
LOCAL	W	peek_srom(NetInf *inf, W adr)
{
	UW	IOB = inf->IOBase;

	wait_srom(inf);
	OutW(E2P_CMD, (E2P_CMD_BUSY | E2P_CMD_READ |
		       E2P_CMD_TMO | E2P_CMD_ADDR(adr)));
	wait_srom(inf);

	return (InW(E2P_CMD) & E2P_CMD_TMO) ? -1 : InW(E2P_DATA);
}

/*
 *	Wake-up from D1(WOL), D2(Energy detect)
 */
LOCAL	void	smsc9118_wakeup(NetInf *inf)
{
	UW	IOB = inf->IOBase;

	/* D1(WOL), D2(Energy detect) -> D0(Normal operation) */
	if (InW(PMT_CTRL) & PMT_CTRL_PM_MODE(3)) {
		OutW(BYTE_TEST, BYTE_TEST_MAGIC);	/* wake-up! */
		wait_reg(inf, PMT_CTRL, PMT_CTRL_READY, PMT_CTRL_READY);
	}

	/* Initialize PHY */
	OutW(PMT_CTRL, PMT_CTRL_PHY_RST);
	WaitUsec(100);
	wait_reg(inf, PMT_CTRL, PMT_CTRL_PHY_RST, 0);

	return;
}

/*
 *	Power control of PHY
 */
LOCAL	void	smsc9118_phyen(NetInf *inf, BOOL power_on)
{
#define	BMCR_PDN	(1 << 11)

	W	adr;

	adr = MIIfind(inf, 1);
	if (adr >= 0) {
		if (power_on) {
			poke_mii(inf, adr, 0,
				 peek_mii(inf, adr, 0) & ~BMCR_PDN);
		} else {
			poke_mii(inf, adr, 0,
				 peek_mii(inf, adr, 0) | BMCR_PDN);
		}
	}

	return;
}

/*
 *	Initialize controller
 */
LOCAL	void	smsc9118_reset(NetInf *inf, BOOL start)
{
	UW	IOB = inf->IOBase;
	UW	cr;

	/* wake-up from sleep state */
	smsc9118_wakeup(inf);

	/* Disable interrupt handler */
	OutW(INT_EN, 0);
	inf->IntMask = 0;
	inf->BadState = 0;
	inf->txbusy = FALSE;

	/* Power on of PHY */
	smsc9118_phyen(inf, TRUE);

	/* Initialize */
	OutW(HW_CFG, HW_CFG_SRST);
	wait_reg(inf, HW_CFG, HW_CFG_SRST, 0);
	wait_reg(inf, PMT_CTRL, PMT_CTRL_READY, PMT_CTRL_READY);
	if (InW(HW_CFG) & HW_CFG_SRST_TO) inf->BadState = 1;
	wait_srom(inf);			// wait for EEPROM
	if (start && inf->BadState) goto fin0;

	OutW(HW_CFG, HW_CFG_SF | HW_CFG_TX_FIF_SZ(TX_FIFO_SIZE) |
	     HW_CFG_PHY_CLK_INT);
	OutW(AFC_CFG, 0x006e3740);
	OutW(FIFO_INT, FIFO_INT_TSFL(0) | FIFO_INT_RSFL(0));
	OutW(RX_CFG, RX_CFG_ALIGN_4 | RX_CFG_DUMP | RX_CFG_DOFF(0));
	wait_reg(inf, RX_CFG, RX_CFG_DUMP, 0);
	OutW(TX_CFG, TX_CFG_SDUMP | TX_CFG_DDUMP);
	wait_reg(inf, TX_CFG, TX_CFG_SDUMP | TX_CFG_DDUMP, 0);
#ifdef	GPIO_CFG_VAL
	OutW(GPIO_CFG, GPIO_CFG_VAL);
#else
	OutW(GPIO_CFG, 0x70000000);	/* LED1-3 enable */
#endif
	InW(RX_DROP);	/* RX Dropped Frame Counter: clear */

	/* Setting of MAC address */
	poke_mac(inf, ADDRH, *((UH *)&inf->eaddr.c[4]));
	poke_mac(inf, ADDRL, *((UW *)&inf->eaddr.c[0]));

	/* Disable multicast filter */
	poke_mac(inf, HASHH, 0);
	poke_mac(inf, HASHL, 0);

	/* Initialize MII */
	MIIinit(inf, 1);

	/* that's all, in case of operation stopped */
	if (!start) {
		smsc9118_phyen(inf, FALSE);
		goto fin0;
	}

	/* Wait for MII startup */
	tk_dly_tsk(1500);

	/* Clear PHY status register */
	peek_mii(inf, 1, 1);

	/* Enable both transmitter and receiver */
	OutW(TX_CFG, TX_CFG_ON);
	cr = MAC_CR_TXEN | MAC_CR_RXEN;
	if (isFDX(inf)) cr |= MAC_CR_FDPX;
	poke_mac(inf, MAC_CR, cr);

	/* Enable interrupt */
#ifdef	IRQ_CFG_VAL
	OutW(IRQ_CFG, IRQ_CFG_IRQ_EN | IRQ_CFG_VAL);
#else
	OutW(IRQ_CFG, (IRQ_CFG_INT_DEAS(22) |
		       IRQ_CFG_IRQ_EN | IRQ_CFG_IRQ_TYPE)); /* Low/Push-Pull */
#endif
	inf->IntMask = (INT_TXSTOP | INT_RXSTOP |
			INT_TSFF | INT_TSFL | INT_RSFF | INT_RSFL);
	OutW(INT_EN, inf->IntMask);
 fin0:
	return;
}

/*
 *	Read a packet from FIFO
 */
Inline	void	smsc9118_poprx(NetInf *inf, UB *bp, W len)
{
	UW	IOB = inf->IOBase;
	UW	d;

	/* Write into memory according to byte align */
	switch ((W)bp & 3) {
	case	0:	/* W units */
		for (; len > 7; len -= 4) {
			*(UW *)bp = InW(RX_DATA);
			bp += 4;
		}
		break;
	case	2:	/* H units */
		for (; len > 7; len -= 4) {
			d = InW(RX_DATA);
			*(UH *)bp = d;
			bp += 2;
			*(UH *)bp = d >> 16;
			bp += 2;
		}
		break;
	default:	/* B units */
		for (; len > 7; len -= 4) {
			d = InW(RX_DATA);
			*bp++ = d;
			*bp++ = d >> 8;
			*bp++ = d >> 16;
			*bp++ = d >> 24;
		}
		break;
	}

	/* Final effective data (before CRC) */
	if (len >= 4) {
		d = InW(RX_DATA);
		switch (len) {
		case	7:	bp[2] = d >> 16;	/* fall-through */
		case	6:	bp[1] = d >> 8;		/* fall-through */
		case	5:	bp[0] = d;		/* fall-through */
		default:	break;
		}
		len -= 4;
	}

	/* read out CRC */
	if (len) InW(RX_DATA);

	return;
}

/*
 *	Completion of reception
 */
Inline	void	smsc9118_receive(NetInf *inf)
{
	UW	IOB = inf->IOBase;
	UW	i, rxsused, sts, len, pktlen;
	UB	*bp;

	/* Get number of waiting status */
	rxsused = (InW(RX_FIFO_INF) >> 16) & 0xff;

	/* Process each waiting status */
	for (i = 0; i < rxsused; i++) {
		sts = InW(RX_STS);
		len = (sts >> 16) & 0x3fff;	/* size including CRC */
		pktlen = len - 4;		/* size excluding CRC */

		if (sts & RX_STS_ES) {
			inf->stinf.rxerr++;
			inf->stinf.other[2] |= sts;

		} else if ((pktlen < inf->bufsz.minsz) ||
			   (pktlen > inf->bufsz.maxsz)) {
			inf->stinf.invpkt++;

		} else if (inf->mbfid > 0) {
			bp = GetRxBuf(inf);

			if (bp == NULL) {
				inf->stinf.misspkt++;
			} else {
				/* Store received data into buffer */
				smsc9118_poprx(inf, bp, len);
				SendMsg(inf, bp, pktlen);
				continue;
			}
		}

		/* Abandon received data - FastForward can be used
				when the length is greater equal to 4 words */
		len = (len + 3) / 4;
		if (len >= 4) {
			OutW(RX_DP_CTL, RX_DP_CTL_FFWD);
			wait_reg(inf, RX_DP_CTL, RX_DP_CTL_FFWD, 0);
		} else {
			for (; len > 0; len--) InW(RX_DATA);
		}
	}

	return;
}

/*
 *	Check FIFO empty space - Max packet size and TxCommand A/B
 */
#define	TXFIFOisReady	((InW(TX_FIFO_INF) & 0xffff) >= MAXPKTLEN + 8)

/*
 *	Completion of transmission
 */
Inline	void	smsc9118_sendcomplete(NetInf *inf)
{
	UW	IOB = inf->IOBase;
	UW	i, txsused, sts;

	/* Get number of waiting status */
	txsused = (InW(TX_FIFO_INF) >> 16) & 0xff;

	/* Process each waiting status */
	for (i = 0; i < txsused; i++) {
		sts = InW(TX_STS);
		if (sts & TX_STS_ES) {
			inf->stinf.txerr++;
			inf->stinf.other[1] |= sts;
		}
		inf->stinf.collision += (sts >> 3) & 0x0f;
	}

	/* When the empty space in the FIFO becomes sufficient,
				notify the transmission permission. */
	if (inf->txbusy && TXFIFOisReady) {
		inf->txbusy = FALSE;
		SendMsg(inf, NULL, 0);
	}

	return;
}

/*
 *	Interrupt handling
 */
LOCAL	void	smsc9118_inthdr(NetInf *inf)
{
	UW	IOB = inf->IOBase;
	UW	sts, limit;

	inf->stinf.nint++;
	limit = 1000000;	/* Limit of interrupt counts */

	while (1) {
		/* Get interrupt status */
		sts = InW(INT_STS);
		OutW(INT_STS, sts & ~inf->IntMask);

		/* Finish when no effective interrupt */
		if (!(sts & inf->IntMask)) break;

		/* When too many interrupt occurs, finish as abnormal. */
		if (--limit < 0) {
			inf->stinf.hwerr++;
			smsc9118_reset(inf, FALSE);
			break;
		}

		/* Receive interrupt*/
		if (sts & (INT_RSFF | INT_RSFL)) {
			inf->stinf.rxint++;
			smsc9118_receive(inf);
			OutW(INT_STS, sts & (INT_RSFF | INT_RSFL));
		}

		/* Transmit interrupt */
		if (sts & (INT_TSFF | INT_TSFL)) {
			inf->stinf.txint++;
			smsc9118_sendcomplete(inf);
			OutW(INT_STS, sts & (INT_TSFF | INT_TSFL));
		}

		/* Other interrupt */
		if (sts & (INT_TXSTOP | INT_RXSTOP)) {
			inf->stinf.hwerr++;
			smsc9118_reset(inf, TRUE);
			break;
		}

		/* Wait for updating of interrupt status */
		InW(BYTE_TEST);
	}

	return;
}

/*
 *	Transmit interrupt processing
 */
LOCAL	INT	smsc9118_send(NetInf *inf, UB *buf, W len)
{
	UW	IOB = inf->IOBase;
	UW	i, d, ofs;

	/* In case of interrupt handler is inactive
					or no space in transmit buffer. */
	if (!inf->IntMask || inf->txbusy) {
		inf->stinf.txbusy++;
		return E_BUSY;
	}

	/* Avoid conflict with the interrupt handler */
	OutW(INT_EN, 0);

	/* Limit max length */
	len = (len < MAXPKTLEN) ? len : MAXPKTLEN;

	/* Get offset in order to read by W units */
	ofs = (W)buf & 3;

	/* Write TX Command A/B */
	OutW(TX_DATA, (TX_CMDA_ALIGN_4 | TX_CMDA_OFFSET(ofs) |
		       TX_CMDA_FS | TX_CMDA_LS | TX_CMDA_BUFSZ(len)));
	OutW(TX_DATA, TX_CMDB_PADDIS | TX_CMDB_PKTSZ(len));

	/* Top of data (only when not aligned by 4-byte) */
	if (ofs) {
		for (d = i = 0; i < 4 - ofs && len > 0; i++, len--, buf++) {
			d |= *buf << (((W)buf & 3) * 8);
		}
		OutW(TX_DATA, d);
	}

	/* Middle of data (transfer by 4-byte units) */
	for (; len > 3; len -= 4, buf += 4) OutW(TX_DATA, *(UW *)buf);

	/* Bottom of data (only when not aligned by 4-byte)  */
	if (len) {
		d = 0;
		switch (len) {
		case	3:	d |= (buf[2] << 16);	/* fall-through */
		case	2:	d |= (buf[1] << 8);	/* fall-through */
		case	1:	d |=  buf[0];		/* fall-through */
		default:	break;
		}
		OutW(TX_DATA, d);
	}

	/* When the empty space in the FIFO becomes sufficient,
				notify the transmission permission. */
	if (TXFIFOisReady) {
		SendMsg(inf, NULL, 0);
	} else {
		inf->txbusy = TRUE;
	}

	/* Enable interrupt */
	OutW(INT_EN, inf->IntMask);
	inf->stinf.txpkt++;

	return len;
}

/*
 *	Get controller status
 */
LOCAL	void	smsc9118_getstatus(NetInf *inf)
{
	UW	IOB = inf->IOBase;

	/* do nothing in close. */
	if (!inf->opencnt) goto fin0;

	inf->stinf.overrun += InW(RX_DROP);

	/* Regards as Invalid, when the following cases:
	 * - Value of PMT_CTRL register is not READY.
	 * - Value of ENDIAN register is invalid (malfunction of controller).
	 * - Transmitter busy, or any status/data remains in transmit FIFO.
	 *   (at this time, transmission have been completed)
	 * - Can't read PHY Identifier (malfunction of PHY).
	 */
	if (!(InW(PMT_CTRL) & PMT_CTRL_READY) ||
	    InW(BYTE_TEST) != BYTE_TEST_MAGIC ||
	    inf->txbusy || ((InW(TX_FIFO_INF) >> 16) & 0xff) ||
	    (InW(TX_FIFO_INF) & 0xffff) < (TX_FIFO_SIZE * 1024 - 512) ||
	    (peek_mii(inf, 1, 2) != 0x0007)) {
		inf->BadState = 1;
	}

	/* Recover from the invalid state */
	if (inf->BadState) {
		inf->stinf.other[0]++;
		DP(("smsc9118_getstatus: Bad state, try restart\n"));

		smsc9118_reset(inf, TRUE);
	}

 fin0:
	return;
}

/*
 *	Probe the controller
 */
LOCAL	W	smsc9118_probe(NetInf *inf)
{
	UW	IOB = inf->IOBase;
	W	i, err;

	/* Disable interrupt handler */
	inf->IntMask = 0;

	/* Wake-up from sleep state */
	smsc9118_wakeup(inf);

	/* Read EndianCheck register */
	if (InW(BYTE_TEST) != BYTE_TEST_MAGIC) {
		DP(("smsc9118_probe: LAN911x not found\n"));
		err = E_NOMDA;
		goto fin0;
	}

	/* Invalid configuration, finish and return as OK */
	if (inf->di.stat < E_OK) {
		err = E_OK;
		goto fin0;
	}

	/* Disable interrupt, stop both transmitter and receiver */
	OutW(INT_EN, 0);				/* disable intr */
	OutW(TX_CFG, TX_CFG_STOP);			/* stop xmit */
	wait_reg(inf, TX_CFG, TX_CFG_STOP, 0);
	poke_mac(inf, MAC_CR,
		 peek_mac(inf, MAC_CR) & ~(MAC_CR_TXEN | MAC_CR_RXEN));

	/* Power off PHY */
	smsc9118_phyen(inf, FALSE);

	/* Set controller chip name */
	inf->ChipID = InW(ID_REV);
	DP(("smsc9118_probe: ID_REV %#x\n", inf->ChipID));

	/* Get MAC address */
	for (i = 0; i < L_EADDR; i++) {
		inf->eaddr.c[i] = peek_srom(inf, i + 1);
		DP(("%02x ", inf->eaddr.c[i]));
	}
	DP(("\n"));

	/* Stay MAC on ( do not sleep) */
	err = E_OK;
 fin0:
	return err;
}

/*
 *	Initialize SMSC LAN9118 driver
 *
 *	At return, the operation should not yet started, and the interrupt
 *	should not occurred.
 *
 *	When the target device was detected, even if the normal operation
 *	will not be expected, it should returns OK with error code in di.stat,
 *	because when it returns error, the driver will be deleted.
 *
 *	PcCard
 *		Performs only internal initialization. and set iosize.
 *		Detection of the controller and hardware initialization will be
 *		done in (*inf->cardchk)().
 *		Always returns OK.
 *
 *	ISA
 *		Performs detection of the controller, and hardware
 *		initialization, then internal initialization.
 *		The values of iob and irq are set by configuration, but when
 *		the value is 0, it should detect automatically as much as
 *		possible.
 *		Returns error, only if detection of the controller was failed.
 *
 *	PCI
 *		Performs hardware initialization and internal initialization.
 *		the values of iob, iosize, irq, pciadr, and devix are set,
 *		Always return OK, because the controller was detected.
 *
 *  <Processing of functions >
 *
 *	void inthdr(NetInf *inf) - Interrupt handler.
 *		EndOfInt() does not needed.
 *
 *	INT sendfn(NetInf *inf, UB *buf, W len) - Transmit processing.
 *		Transmits len bytes of data in the buf.
 *		Returns len when the transmission is successful,
 *		otherwise, returns E_BUSY.
 *
 *	void reset(NetInf *inf, BOOL start) - Reset processing.
 *		start == TRUE : Start
 *			 FALSE: Stop (No interrupt occurs)
 *
 *	ER cardchk(NetInf *inf) - Detection of PC card & initalization.
 *		Returns OK when the detection of PC card is successful,
 *		otherwise, returns error.
 *		The values of iob, irq, devix, and name are set.
 *		When returns OK, the controller should be in stop state 
 *		(no interrupt occurs).
 *		This function pointer may be NULL if no PC card supported.
 */
EXPORT	ER	InitSMSC9118(NetInf *inf, W init)
{
	ER	err;

	/* Finish processing */
	if (init < 0) goto fin1;

	inf->di.iosize = SMSC9118_IOSZ;

	DP(("Net%c: InitSMSC9118 kind:%#x\n", NetUnit(inf), inf->di.kind));

	/* Set MII functions (should be set before probe) */
	inf->mii_read  = peek_mii;
	inf->mii_write = poke_mii;

	/* Probe controller */
	err = smsc9118_probe(inf);
	if (err < E_OK) goto fin0;

	/* Set product name */
	SetProdName(inf, (UB **)SMSC9118_PNAME1, (UB **)SMSC9118_PNAME2);

	/* Set processing functions */
	inf->inthdr  = smsc9118_inthdr;
	inf->sendfn  = smsc9118_send;
	inf->reset   = smsc9118_reset;
	inf->cardchk = NULL;
	inf->misc    = NULL;
	inf->mcast   = NULL;
	inf->tmout   = 10000;
	inf->tmofn   = smsc9118_getstatus;

 fin1:
	err = E_OK;
 fin0:
	return err;
}

