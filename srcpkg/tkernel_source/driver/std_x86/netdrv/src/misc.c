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
 *	@(#)misc.c
 *
 */

#include	"netdrv.h"

/*----------------------------------------------------------------------
	Functions related to memory
		(do not use malloc in order to reduce code size.)
----------------------------------------------------------------------*/
/*
 *	allocate memory area
 */
EXPORT	ER	AllocMem(W sz, VP *p)
{
	T_RSMB	msts;

	if (tk_ref_smb(&msts) >= E_OK) {
		sz = (sz + msts.blksz - 1) / msts.blksz;
		return tk_get_smb(p, sz, TA_RNG0);
	}
	return E_NOMEM;
}

/*
 *	free memory area
 */
EXPORT	void	FreeMem(VP p)
{
	if (p != NULL) tk_rel_smb(p);
}
/*----------------------------------------------------------------------
	Functions related to receive buffer
----------------------------------------------------------------------*/
/*
 *	Setting of receive buffer  (bp = NULL : Initialize)
 */
EXPORT	ER	SetRxBuf(NetInf *inf, VP bp)
{
	UW	imsk;

	if (bp == NULL) {	/* Initialize */
		DI(imsk);
		inf->ip_rxbuf = inf->op_rxbuf = 0;
		EI(imsk);
	} else {		/* Setting */
		if (inf->ip_rxbuf - inf->op_rxbuf >= N_RXBUF) return E_BUSY;
		inf->rxbuf[inf->ip_rxbuf % N_RXBUF] = bp;
		inf->ip_rxbuf++;
	}
	return E_OK;
}

/*
 *	get a receive buffer (called in interrupt handler)
 */
EXPORT	VP	GetRxBuf(NetInf *inf)
{
	if (inf->ip_rxbuf == inf->op_rxbuf) return NULL;
	return inf->rxbuf[(inf->op_rxbuf++) % N_RXBUF];
}
/*----------------------------------------------------------------------
	Functions related to message
----------------------------------------------------------------------*/
/*
 *	Send a message
 */
EXPORT	ER	SendMsg(NetInf *inf, VP bp, W len)
{
	ER		er;
	NetEvent	evt;

	evt.buf = bp;		/* Receive buffer	*/
	evt.len = len;		/* Receive Data length	*/

	/* Send message */
	er = tk_snd_mbf(inf->mbfid, &evt, sizeof(evt), TMO_POL);
	if (bp != NULL) {
		if (er >= E_OK) inf->stinf.rxpkt++;	/* Packets received. */
		else {
			inf->stinf.misspkt++;	/* Packets abandoned */
			SetRxBuf(inf, bp);	/* Set receive buffer	*/
		}
	}
	return er;
}
/*----------------------------------------------------------------------
	Functions related to MII
----------------------------------------------------------------------*/
/*
 *	MII related registers
 */
#define	BMCR_RESET	(1 << 15)	/* 0x8000 */
#define	BMCR_SPEED0	(1 << 13)	/* 0x2000 */
#define	BMCR_ANEG	(1 << 12)	/* 0x1000 */
#define	BMCR_PDN	(1 << 11)	/* 0x0800 */
#define	BMCR_ISOLATE	(1 << 10)	/* 0x0400 */
#define	BMCR_RANEG	(1 << 9)	/* 0x0200 */
#define	BMCR_FDX	(1 << 8)	/* 0x0100 */
#define	BMCR_SPEED1	(1 << 6)	/* 0x0040 */

#define	BMSR_GB		(1 << 8)	/* 0x0100 */

#define	ANAR_TX_FD	(1 << 8)	/* 0x0100 */
#define	ANAR_TX_HD	(1 << 7)	/* 0x0080 */
#define	ANAR_10_FD	(1 << 6)	/* 0x0040 */
#define	ANAR_10_HD	(1 << 5)	/* 0x0020 */

#define	GBCR_1000T_FD	(1 << 9)	/* 0x0200 */
#define	GBCR_1000T_HD	(1 << 8)	/* 0x0100 */

#define	peek_mii(inf, adr, reg)	(*inf->mii_read) ((inf), (adr), (reg))
#define	poke_mii(inf, adr, reg, dat)	\
				(*inf->mii_write)((inf), (adr), (reg), (dat))
/*
 *	Search MII address
 */
EXPORT	INT	MIIfind(NetInf *inf, W target)
{
	W	i, adr, reg, val, prev;

	if (inf->mii_read == NULL || inf->mii_write == NULL) return E_NOMDA;

	for (i = 0; i < 32; i++) {
		/* Criteria: when value of all registers are the same,
			assume not exist */
		adr = (i + target) & 31;
		prev = peek_mii(inf, adr, 0);
		for (reg = 1; reg < 32; reg++) {
			val = peek_mii(inf, adr, reg);
			if (val != prev) goto fin;
			prev = val;
		}
	}
	adr = E_NOMDA;
 fin:
    return adr;
}

/*
 *	Initialize MII
 */
EXPORT	INT	MIIinit(NetInf *inf, W target)
{
	W	adr, mii_reg0, mii_reg4, mii_reg9;
	W	i, bmcr, gbe;

	if (inf->mii_read == NULL || inf->mii_write == NULL) return E_NOMDA;

	/* Search MII address */
	adr = MIIfind(inf, target);
	if (adr < 0) {
		DP(("MIIinit(): MII not found\n"));
		goto fin;
	}

	DP(("MIIinit(): MII found, address #%d\n", adr));

	/* Reset MII, then sets depending on the mode */
	poke_mii(inf, adr, 0, BMCR_RESET);
	for (i = 0; i < 2000; i++) {	/* max 1sec */
		WaitUsec(500);
		bmcr = peek_mii(inf, adr, 0);
		if (!(bmcr & BMCR_RESET)) break;
	}
	DP(("MIIinit(): MII reset: wait: %d count(s)\n", i + 1));

	/* Confirm support of Giga-bit mode */
	gbe = peek_mii(inf, adr, 1) & BMSR_GB;
	DP(("MIIinit(): GbE %supported\n", gbe ? "" : "not "));

	/* Setting of auto-negotiation advertisement.
	   Copy capability bits regardless to the initial value of
	   advertisement register. does not support 1000BASE-T(Half-Duplex) */
	mii_reg0  = 0;
	mii_reg4  = peek_mii(inf, adr, 4) &
		~(ANAR_TX_FD | ANAR_TX_HD | ANAR_10_FD | ANAR_10_HD);
	mii_reg4 |= (peek_mii(inf, adr, 1) & 0x7800) >> 6;
	if (gbe) {
		mii_reg9  = peek_mii(inf, adr, 9) &
			~(GBCR_1000T_FD | GBCR_1000T_HD);
		mii_reg9 |= (peek_mii(inf, adr, 15) & 0x2000) >> 4;
	} else {
		mii_reg9  = 0;
	}

	switch (inf->di.ifconn = IFC_TYPE(inf)) {
	case	IFC_AUI:
	case	IFC_TPE:
	case	IFC_BNC:
		if (isFDX(inf)) mii_reg0 |= BMCR_FDX;
		break;
	case	IFC_100TX:
	case	IFC_100FX:
		mii_reg0 |= BMCR_SPEED0;
		if (isFDX(inf)) mii_reg0 |= BMCR_FDX;
		break;
	case	IFC_1000T:
		// Auto negotiation is mandatory for 1000BASE-T,
		// regard as same to IFC_AUTO.
		/* fall-through */
	default:
		inf->di.ifconn = IFC_AUTO;
		mii_reg0 |= BMCR_ANEG | BMCR_RANEG;
		if (!isFDX(inf)) {
			mii_reg4 &= ~(ANAR_TX_FD | ANAR_10_FD);
			mii_reg9 &= ~(GBCR_1000T_FD);
		}
		break;
	}

	if (gbe) poke_mii(inf, adr, 9, mii_reg9);
	poke_mii(inf, adr, 4, mii_reg4);
	poke_mii(inf, adr, 0, mii_reg0);

#ifdef	DEBUG
	for (i = 0; i < 32; i++) printf("%04x%c", peek_mii(inf, adr, i),
					((i + 1) % 16) ? ' ' : '\n');
#endif

	/* Clear PHY status register */
	peek_mii(inf, adr, 1);

 fin:
	return adr;
}
/*----------------------------------------------------------------------
	Other common functions
----------------------------------------------------------------------*/
/*
 *	Setting of product name
 */
EXPORT	void	SetProdName(NetInf *inf, UB **nm1, UB **nm2)
{
	W	n;

	if (nm1 == NULL) n = 0;
	else {
		nm1 += (inf->devix >> 8) & 0x0f;	/* Series name*/
		strncpy(inf->di.name, *nm1, L_NETPNAME);
		n = strlen(*nm1);
	}
	if (n < L_NETPNAME) {
		nm2 += inf->devix & 0xff;		/* individual name */
		strncpy(&inf->di.name[n], *nm2, L_NETPNAME - n);
	}

DP(("Net%c: '%.40s'\n", NetUnit(inf), inf->di.name));

}

/*
 *	Calculation of CRC
 *	* when the MAC address is 00:11:22:33:44:55, the order in memory is
 *	  00, 11, 22, 33, 44, 55. (i.e. bigendian)
 *	* Bit inversion may be required, in order to make the calculated
 *	  results to bigendian.
 */
EXPORT	UW	CalcCRC32(UB *data, W len)
{
#define	CRC_POLY	0x04c11db6

	UW	crc, byteindex, addrbyte, bitindex, carry;

	crc = 0xffffffff;

	for (byteindex = 0; byteindex < len; byteindex++) {
		addrbyte = data[byteindex];

		for (bitindex = 0; bitindex < 8; bitindex++) {
			carry = (crc >> 31) ^ (addrbyte & 1);
			crc <<= 1;

			if (carry) crc = (crc ^ CRC_POLY) | carry;

			addrbyte >>= 1;
		}
	}

	return crc;
}

