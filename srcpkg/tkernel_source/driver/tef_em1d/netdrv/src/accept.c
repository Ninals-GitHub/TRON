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
 *	@(#)accept.c
 *
 */

#include	"netdrv.h"

/*
 *	Read / Write request
 */
LOCAL	ER	ReadWriteReq(NetInf *inf, T_DEVREQ *req)
{
	ER	er;
	W	i, cmd, len, dsz;
	VP	mp;

#define	cmR	0x01			/* Read	command 	*/
#define	cmW	0x02			/* Write command	*/
#define	ckST	0x10			/* Needs stat check	*/

static	const	struct {
	W	dno;
	UH	dsz;
	UH	flg;
} DaTab[] = {
	{0,			0,			cmW | ckST	},
	{DN_NETEVENT,		sizeof(ID),		cmR | cmW	},
	{DN_NETRESET,		sizeof(W),		cmR | cmW | ckST},
	{DN_NETADDR,		sizeof(NetAddr),	cmR | ckST	},
	{DN_NETDEVINFO,		sizeof(NetDevInfo),	cmR		},
	{DN_NETSTINFO,		sizeof(NetStInfo),	cmR		},
	{DN_NETCSTINFO,		sizeof(NetStInfo),	cmR		},
	{DN_NETRXBUF,		sizeof(VP),		cmW		},
	{DN_NETRXBUFSZ,		sizeof(NetRxBufSz),	cmR | cmW	},
	{DN_NETMCASTLIST,	0,			cmR | cmW | ckST},
	{DN_NETALLMCAST,	0,			cmW | ckST	},
};

#define	N_DaTab		(sizeof(DaTab) / (sizeof(W) + sizeof(UH) * 2))

	/* Check parameter */
	if ((len = req->size) < 0) {er = E_PAR; goto EEXIT;}

	/* Set task space */
	er = tk_set_tsp(TSK_SELF, &req->tskspc);
	if (er < E_OK) goto EEXIT;

	cmd = (req->cmd == TDC_READ) ? cmR : cmW;
	mp = req->buf;

	/* Check data number */
	for (i = 0; i < N_DaTab && req->start != DaTab[i].dno; i++);
	if (i >= N_DaTab) goto PAR_ERR;		/* Invalid */

	/* Check R/W support */
	if (DaTab[i].flg == 0) {er = E_NOSPT; goto EEXIT;}  /* Unsupported */
	if ((DaTab[i].flg & cmd) == 0) goto PAR_ERR;

	/* Check device status */
	if ((DaTab[i].flg & ckST) != 0 && inf->di.stat < E_OK)
				{er = E_OBJ; goto EEXIT;}

	/* Check data size */
	if ((dsz = DaTab[i].dsz) != 0 && len < dsz) {
		er = (len == 0) ? dsz : E_PAR;
		goto NEXIT;
	}

	/* Processing depends on data number */
	er = E_OK;
	switch (req->start) {
	case 0:				/* Specific data (W)	*/
		if (len <= 0 || len > MAXPKTLEN) er = E_PAR;
		else {
			/* When the packet length is less than MINPKTLEN,
			   do padding here, because auto padding may not
			   done in some hardware. */
			if ((dsz = len) < MINPKTLEN) {
				UB	buf[MINPKTLEN];
				memcpy(buf, mp, len);
				memset(&buf[len], 0, MINPKTLEN - len);
				mp = (VP)buf;
				len = MINPKTLEN;
			}
			er = (*(inf->sendfn))(inf, (UB*)mp, len);
		}
		break;

	case DN_NETEVENT:	/* Event notification MBF ID (RW)	*/
		if (cmd == cmR)	*((W*)mp) = inf->mbfid;
		else		inf->mbfid = *((W*)mp);
		break;

	case DN_NETRESET:	/* Reset (RW)				*/
		(*(inf->reset))(inf, TRUE);
		break;

	case DN_NETADDR:	/* Physical address (R)			*/
		*((NetAddr*)mp) = inf->eaddr;
		break;

	case DN_NETDEVINFO:	/* Device information (R)		*/
		*((NetDevInfo*)mp) = inf->di;
		break;

	case DN_NETSTINFO:	/* Statistical information (R)		*/
	case DN_NETCSTINFO:	/* Get & Clear Statistical information (R) */
		if (inf->misc != NULL && inf->di.stat >= E_OK)
			(*(inf->misc))(inf, 0, DN_NETSTINFO, NULL, 0);
		*((NetStInfo*)mp) = inf->stinf;
		if (req->start == DN_NETCSTINFO)
			memset(&inf->stinf, 0, sizeof(NetStInfo));
		break;

	case DN_NETRXBUF:	/* Receive buffer setting	*/
		/* !! Omit address space check !! */
		er = SetRxBuf(inf, *((VP*)mp));
		break;

	case DN_NETRXBUFSZ:	/* Receive buffer size	*/
		if (cmd == cmR)	{
			*((NetRxBufSz*)mp) = inf->bufsz;
		} else {
			NetRxBufSz	bsz = *((NetRxBufSz*)mp);

			bsz.maxsz &= (~3);		/* 4-byte units */
			if (bsz.maxsz > MAXPKTLEN) bsz.maxsz = MAXPKTLEN;
			if (bsz.minsz < 0 || bsz.minsz >= MINRXBUFSZ ||
			    bsz.maxsz <= bsz.minsz || bsz.maxsz < MINRXBUFSZ)
				goto PAR_ERR;
			inf->bufsz = bsz;
		}
		break;
	case	DN_NETMCASTLIST:
	case	DN_NETALLMCAST:
		dsz = er = (inf->mcast == NULL) ? E_NOSPT :
			(*(inf->mcast))(inf, cmd & cmW,
					(req->start == DN_NETALLMCAST),
					mp, len);
		break;
	}
NEXIT:
	if (er < E_OK) goto EEXIT;

	req->asize = dsz;
	return E_OK;

PAR_ERR:
	er = E_PAR;
EEXIT:

DBG( if (er != E_BUSY)
	DP(("Net%c: ReadWrite cmd:%d st:%d sz:%d [%#x]\n", NetUnit(inf),
		req->cmd, req->start, req->size, er));	);

	return er;
}

/*
 *	Open processing
 */
EXPORT	ER	OpenProc(ID devid, UINT omode, GDI gdi)
{
	NetInf	*inf = GDI_exinf(gdi);
	ER	er = E_OK;

	if (! inf->exist) {		/* Not exist	*/
		er = E_NOMDA;
	} else if (inf->opencnt == 0) {	/* First open */
		SetRxBuf(inf, NULL);	/* Initialize receive buffer */
		if ((er = CardPowerOn(inf, TRUE)) >= E_OK) {
			if (inf->di.stat >= E_OK)
				(*(inf->reset))(inf, TRUE);
		}
	}
	if (er >= E_OK) inf->opencnt++;
	return er;
}

/*
 *	Close processing
 */
EXPORT	ER	CloseProc(ID devid, UINT option, GDI gdi)
{
	NetInf	*inf = GDI_exinf(gdi);

	if (--inf->opencnt <= 0) {	/* Last close */
		inf->opencnt = 0;
		inf->mbfid = INVALID_ID;
		if (inf->exist) {
			if (inf->di.stat >= E_OK && ! inf->poweroff)
				(*(inf->reset))(inf, FALSE);
			CardPowerOff(inf, TRUE);
		}
	}

	return E_OK;
}

/*
 *	Event processing
 */
EXPORT	INT	EventProc(INT evttyp, VP evtinf, GDI gdi)
{
	NetInf	*inf = GDI_exinf(gdi);

	switch (evttyp) {
	case TDV_SUSPEND:	/* Suspend request */
		if (inf->exist) {
			if (inf->di.stat >= E_OK && ! inf->poweroff)
				(*(inf->reset))(inf, FALSE);
			CardPowerOff(inf, FALSE);
		}
		inf->suspended = TRUE;
		return E_OK;

	case TDV_RESUME:	/* Resume request */
		if (inf->suspended) {
			if (inf->exist) {
				CardPowerOn(inf, FALSE);
				if (inf->di.stat >= E_OK && inf->opencnt > 0)
					(*(inf->reset))(inf, TRUE);
			}
			inf->suspended = FALSE;
		}
		return E_OK;

	case TDV_CARDEVT:	/* Card Event  */
		return CardEvent(inf, evtinf);

	default:		/* Unknown event */
DP(("Net%c: EventProc evttyp:%d\n", evttyp));
		return E_PAR;
	}
}

/*
 *	Device request acceptance entry
 */
EXPORT	void	AcceptRequest(NetInf *inf)
{
	T_DEVREQ	*devReq;
	ER	er;

	/* Infinite loop of request acceptance */
	for (;;) {
		/* Accept request */
		er = GDI_Accept(&devReq, DRP_NORMREQ,
				(inf->suspended || inf->tmofn == NULL ||
				 inf->tmout <= 0) ? TMO_FEVR : inf->tmout,
				inf->Gdi);

		if (er < E_OK) {
			if (er == E_TMOUT && inf->tmofn != NULL) {
				/* Do timeout processing */
				(*(inf->tmofn))(inf);
			} else {

DP(("Net%c: GDI_accept [%#x]\n", NetUnit(inf), er));

			}
			continue;
		}

		/* Do normal processing */
		er = (inf->exist) ? ReadWriteReq(inf, devReq) : E_NOMDA;

DBG( if (er < E_OK && er != E_BUSY)
	DP(("Net%c: cmd:%d [%#x]\n", NetUnit(inf), devReq->cmd,er));	);

		/* reply to the request */
		devReq->error = er;
		GDI_Reply(devReq, inf->Gdi);
	}
}

