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
 *	load.c
 *
 *       load processing
 */

#include "cmdsvc.h"

LOCAL	UW	s_addr;			// start address
LOCAL	UW	e_addr;			// end address + 1
LOCAL	UW	offset;			// address offset
LOCAL	UW	loaddr;			// address lower limit
LOCAL	UW	hiaddr;			// address upper limit
LOCAL	W	blkno;			// block number
LOCAL	W	blkptr;			// read pointer
#define	XBLK_SZ	1024			// XMODEM block size (extended)
#define	BLK_SZ	128			// XMODEM block size
#define	blkbuf	wrkBuf			// block buffer
LOCAL	W	blksz;			// block size

LOCAL	FUNCP	readFn;			// read function

#define	inputByte(tmo)	getSIO(tmo)	// input one byte
#define	outputByte(c)	putSIO(c)	// output one byte

// XMODEM control codes
#define SOH		(0x01)
#define STX		(0x02)
#define EOT		(0x04)
#define ACK		(0x06)
#define NAK		(0x15)
#define CAN		(0x18)
#define	CTLC		('C' - '@')
#define	CTLZ		('Z' - '@')

/* XMODEM time out specification
 *       wait for SOH : 10 seconds / 10 retries
 *       timeout between two characters : 1 seconds
 *
 * how to cope with communication software (mainly for TeraTerm) that does not start automatic transfer
 *     (this falls outside XMODEM specification)
 *      Since the timeout value for SOH is 10 seconds, the initial transfer always seem to wait for 10 seconds.
 *       The initial timeout for SOH is set to 3 seconds (only for the very first transfer.)
 */
#define	IDLE_TMO	( 1 * 1000)	// idle wait(milliseconds)
#define	RECV_TMO	( 1 * 1000)	// timeout for data input (milliseconds)
#define	SOH_TMO		(10 * 1000)	// timeout for SOH input (milliseconds)
#define	SOH1_TMO	( 3 * 1000)	// timeout for the initial SOH input (milliseconds)
#define	MAX_RETRY	10		// maximum number of retries

/*
        skip until there is no more input
*/
LOCAL	void	purgeInput(void)
{
	while (inputByte(IDLE_TMO) >= 0);
}
/*
        asynchronous read processing (without any protocol)
*/
LOCAL	W	textRead(void)
{
	W	c;

	while ((c = inputByte(0)) < 0);
	return (c == CTLC || c == CAN) ? E_CANCEL : c;
}
/*
        XMODEM read processing
*/
LOCAL	W	xmodemRead(void)
{
	W	i, c, ctlch;
	UB	cksum;

	if (blkptr < blksz) return blkbuf[blkptr++];

	c = 0;
	ctlch = ACK;
	if (blkno++ == 0) {
                /* only for the initial packet transfer */
		outputByte(NAK);
		c = inputByte(SOH1_TMO);
		ctlch = -1;
	}

	for (;;) {
                // receiving block
		for (i = 0;;) {
			if (ctlch >= 0) {
                                // ack/beginning character is transmitted
				outputByte(ctlch);

                                // leading letter in the ack is extracted
				c = inputByte(SOH_TMO);
			}
			ctlch = NAK;
			if (c == SOH) {blksz = BLK_SZ;	break;}
			if (c == STX) {blksz = XBLK_SZ; break;}
			if (c == CAN || c == CTLC) {	// cancel transfer
                                // Is CAN followed by another CAN?
				c = inputByte(IDLE_TMO);
				if (c < 0 || c == CAN || c == CTLC)
							return E_CANCEL;
			} else if (c == EOT) {	// end of transmission
				outputByte(ACK);
				return E_END;
			}
			purgeInput();	// skip data
			if (++i >= MAX_RETRY) return E_XMODEM;
		}

                // read a block number & check
		if ((i = inputByte(RECV_TMO)) < 0) continue;
		if ((c = inputByte(RECV_TMO)) < 0) continue;
		if (i + c != 0xff) continue;

		if (i != (blkno & 0xff)) {
			if (i != ((blkno - 1) & 0xff)) return E_XMODEM;
                        // skip if the previous block is read
			ctlch = ACK;
		}

                // read the block itself
		for (cksum = 0, i = 0; i < blksz; i++) {
			if ((c = inputByte(RECV_TMO)) < 0) break;
			cksum += (blkbuf[i] = c);
		}
		if (c < 0) continue;

                // validate checksum
		if (inputByte(RECV_TMO) == cksum && ctlch != ACK) break;
	}
	blkptr = 0;
	return blkbuf[blkptr++];
}
/*
        XMODEM termination processing
*/
LOCAL	void	xmodemEnd(W er)
{
        // finish XMODEM protocol
	while (er >= 0) er = xmodemRead();

	if (er != E_END && er != E_CANCEL) {
		purgeInput();		// wait until there is no more data
		outputByte(CAN);	// transmite two (or more) consecutive CANs
		outputByte(CAN);
		outputByte(CAN);
	}
}
/*
        load memory image
*/
LOCAL	W	loadImage(void)
{
	W	i, c;
	UB	buf[512];

	for (i = 0;;) {
		if ((c = (*readFn)()) >= 0) buf[i++] = c;
		if (i < sizeof(buf) && c >= 0) continue;
		if (i > 0) {
			if (e_addr < loaddr ||
				e_addr - 1 > hiaddr - i) return E_RANGE;
			if (writeMem(e_addr, buf, i, 1) != i) return E_MACV;
			e_addr += i;
			i = 0;
		}
		if (c < 0) return c;
	}
}
/*
        read a hexadecimal character
*/
LOCAL	W	readHex(void)
{
	W	c;

	if ((c = (*readFn)()) < 0)	return c;
	if (c >= '0' && c <= '9')	return c - '0';
	if (c >= 'A' && c <= 'F')	return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')	return c - 'a' + 10;
	return E_LOAD;
}
/*
        load S format data
*/
LOCAL	W	loadSform(void)
{
	W	i, c, bcnt, v, v1, dcnt, rtype;
	UW	addr, a_addr;
	UB	cksum, buf[512];

	a_addr = s_addr;		// real address
	s_addr = 0xffffffff;		// highest load address
	e_addr = 0;			// lowest load address

	for (;;) {
		if ((c = (*readFn)()) < 0) return c;

		if (c != 'S') {
			if (c == CTLZ) break;	// end
			continue;
		}

		if ((c = (*readFn)()) < 0) return c;
		switch(c) {
		case '0':	// header
				rtype = 0;		break;
		case '1':	// 2 byte address data
		case '2':	// 3 byte address data
		case '3':	// 4 byte address data
				rtype = c - '0' + 2;	break;
		case '7':	// 4 byte address termination
		case '8':	// 3 byte address termination
		case '9':	// 2 byte address termination
				rtype = -1;		break;
		default:	return E_LOADFMT;
		}

		for (cksum = bcnt = addr = dcnt = i = 0;; i++) {
			if ((v1 = readHex()) < 0) return v1;
			if ((v = readHex()) < 0)  return v;
			cksum += (v += (v1 << 4));

			if (i == 0) {		// byte counts
				if ((bcnt = v - 1) < 0) return E_LOAD;
				addr = 0;
				continue;
			}
			if (i > bcnt) {		// checksum
				if (cksum != 0xff) return E_LOAD;
				break;
			}
			if (rtype <= 0) continue;

			if (i < rtype) {	// load address
				addr = (addr << 8) + v;
			} else {		// data
				buf[dcnt++] = (UB)v;
			}
		}
		if (dcnt > 0) {
                        // if we have address specification, then the first address
                        // to be used as the designated address after suitable adjustment.
			if (a_addr != 0) {
				offset = a_addr - addr;
				a_addr = 0;
			}
			addr += offset;
			if (addr < loaddr || addr - 1 > hiaddr - dcnt)
					return E_RANGE;
			if (writeMem(addr, buf, dcnt, 1) != dcnt)
					return E_MACV;
			if (addr < s_addr) s_addr = addr;
			if ((addr += dcnt) > e_addr) e_addr = addr;
		}
		if (rtype < 0) break;	// end
	}
	return E_OK;
}
/*
        loading processing
*/
EXPORT	ER	doLoading(W proto, UW addr, UW *range)
{
	ER	er;

	e_addr = s_addr = addr;		// load address

	if (range) {	// range specification
		loaddr = range[0];	// address lower limit
		hiaddr = range[1];	// address upper limit
		offset = range[2];	// address offset
	} else {
		loaddr = 0;		// address lower limit
		hiaddr = 0xFFFFFFFF;	// address upper limit
		offset = 0;		// address offset
	}

	if (proto & P_XMODEM) {	// XMODEM
		readFn = (FUNCP)xmodemRead;
		blkptr = blkno = blksz = 0;
	} else {		// no protocol
		readFn = (FUNCP)textRead;
	}

	if (proto & P_SFORM) {	// S format
		er = loadSform();
		if (er == E_END) er = E_LOAD;
	} else {		// memory image
		er = loadImage();
	}

        // read termination processing
	if (proto & P_XMODEM) xmodemEnd(er);

        // wait until there is no more data
	purgeInput();

	if (er == E_END) er = E_OK;
	DSP_LF;
	if (er == E_OK) {
		e_addr--;
		if (range) {
			range[0] = s_addr;	// load address
			range[1] = e_addr;
			s_addr -= offset;
			e_addr -= offset;
		}
		DSP_F5(S,"Loaded: ", 08X,s_addr, S," -> ", 08X,e_addr, CH,'\n');
	}
	return er;
}
