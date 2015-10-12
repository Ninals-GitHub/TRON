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
 *	command.c
 *
 *       command processing
 */

#include "cmdsvc.h"
#include "help.h"
#include <tk/dbgspt.h>

#define	DEF_MEM_SIZE	64		// default memory dump size
#define	DEF_DA_STEP	16		// default disassbmle size
#define	MAX_DSP_CNT	64		// maximum cut off count for display
#define	MAX_RANGE	0x1000000	// maximum range (16 MB)
#define	IMPLICIT_SIZE	0x1000		// implicit size specification

EXPORT	UB	lineBuf[L_LINE];	// line buffer
EXPORT	W	killProcReq;		// request to forcibly kill a process

#define	L_SYMBOL	23		// effective symbol length
#define	SETDT_SZ	128		// data size

#define	CMD_FINISH	(9999)		// command end specification

EXPORT	W	errinfo;		// error information
LOCAL	W	errcode;		// error code

LOCAL	UW	dAddr;			// D address command
LOCAL	UW	mAddr;			// M command address
LOCAL	UW	daAddr;			// DA command address
LOCAL	UW	cAddr;			// the current start address
LOCAL	W	cLen;			// the current memory byte length

LOCAL	W	token;			// token type
LOCAL	UW	tokenVal;		// numeric token / register number
LOCAL	UB	*tokenStr;		// character string / symbol item pointer
LOCAL	W	tokenLen;		// character string / symbol item length
LOCAL	UB	tokenSym[L_SYMBOL + 1];	// symbol item string(capital letters)
LOCAL	UB	symExt[2];		// extended symbol letters
LOCAL	UB	*lptr;			// line pointer

#define	PROMPT	"TM> "			// prompt

// item type
#define	tEOL		0x00		// line end
#define	tEOC		0x01		// end of command
#define	tDLM		0x02		// delimiter
#define	tSIZ		0x11		// size specification
#define	tOPADD		0x12		// + operator
#define	tOPSUB		0x13		// - operator
#define	tOPMUL		0x14		// * operator
#define	tOPDIV		0x15		// / operator
#define	tOPIND		0x16		// & operator
#define	tEOD		0x17		// end of data
#define	tUP		0x18		// previous data
#define	tSYM		0x20		// symbol
#define	tNUM		0x21		// numeric value
#define	tSTR		0x22		// character string
#define	tERRR		0x100		// error
#define	tERCH		0x100		// error: illegal character
#define	tERNUM		0x101		// error: illegal numeric form

// character classficiation
#define	isSpace(c)		((c) && (c) <= ' ')
#define	isNum(c)		((c) >= '0' && (c) <= '9')
#define	isAlpha(c)		( ((c) >= 'A' && (c) <= 'Z') ||\
					((c) >= 'a' && (c) <= 'z') )
#define	isAlNum(c)		(isNum(c) || isAlpha(c))
#define	isSym(c)		(isAlpha(c) || c == '$' || c == '_' ||\
					c == '?' || c == '@')
#define	isExtSym(c)		((c) && ((c) == symExt[0] || (c) == symExt[1]))

// alignment adjustment
#define	ALIGN_L(v, unit)	((v) & ~((unit) - 1))
#define	ALIGN_U(v, unit)	(((v) + (unit) - 1) & ~((unit) - 1))

// error return
#define	return_er(er)		return (errcode = er)
#define	er_return(er)		{errcode = er; return;}
#define	oer_return(er)		{if ((er) == E_NOEXS)\
					errcode = E_ONOEXS;\
				 else   errcode = er;\
				 return;}

#define	DB16		0x00000		// default base number
#define	DB10		0x10000

/*
        display error message
*/
LOCAL	void	dspError(void)
{
	UB	*mp = NULL;

	if (token >= tERRR) {	// priortize the item error
		switch(token) {
		case tERCH:	mp = "Illegal Character";		break;
		case tERNUM:	mp = "Illegal Number Format";		break;
		}
	} else {
		if (errinfo < 0) errcode = errinfo;
		switch(errcode) {
		case E_MACV:	mp = "Illegal Address";			break;
		case E_ROM:	mp = "ROM Address";			break;
		case E_LESS:	if (token <= tEOC)
					{mp = "Less Parameter";	break;}
		case E_PAR:	mp = "Illegal Parameter";		break;
		case E_ID:	mp = "Illegal ID Number";		break;
		case E_CTX:	mp = "Context Error";			break;
		case E_LIMIT:	mp = "Too Many Parameters";		break;
		case E_OBJ:	mp = "Abnormal Object Status";		break;
		case E_NOSPT:	mp = "Not Supported";			break;
		case E_NOEXS:	mp = "Unknown Device";			break;
		case E_IO:	mp = "I/O Error";			break;
		case E_RONLY:	mp = "Read Only";			break;
		case E_NOMDA:	mp = "No Media";			break;
		case E_PROTECT:	mp = "Write Protected";			break;

		case E_CMD:	mp = "Unknown Command";			break;
		case E_RANGE:	mp = "Illegal Address Range";		break;
		case E_EMPTY:	mp = "Empty String";			break;
		case E_ILREG:	mp = "Unknown Register Name";		break;
		case E_PC:	mp = "Illegal PC Value";		break;
		case E_BOOT:	mp = "No Bootable Disk";		break;

		case E_PROTO:	mp = "Unknown Load Protocol";		break;
		case E_NOADDR:	mp = "No Load Address";			break;
		case E_LOADFMT:	mp = "Illegal S-Format Record";		break;
		case E_LOAD:	mp = "Loading Error";			break;
		case E_CANCEL:	mp = "Loading Cancelled";		break;
		case E_XMODEM:	mp = "XMODEM Protocol Error";		break;

		case E_BPATR:	mp = "Unknown Break Point Attribute";	break;
		case E_BPBAD:	mp = "Illegal Break Point";		break;
		case E_BPDSLT:	mp = "Break Point at Delayed Slot";	break;
		case E_BPROM:	mp = "Break Point in ROM";		break;
		case E_BPCMD:	mp = "Too Long Break Point Command";	break;
		case E_BPUDF:	mp = "Undefined Break Point";		break;
		case E_SBPOVR:	mp = "Too Many Software Break Points";	break;
		case E_HBPOVR:	mp = "Too Many Hardware Break Points";	break;

		case E_ONOEXS:  mp = "Noexistent Object";		break;
		}
	}
	if (mp) {
		DSP_F3(S,"ERR: ", S,mp, CH,'\n');
	} else {
		DSP_F3(S,"ERR: [", 08X,errcode, S,"]\n");
	}
}
/*
        input of a line
*/
LOCAL	W	getLine(UB *msg)
{
	if (msg) DSP_S(msg);			// display prompt
	memset(lineBuf, 0, sizeof(lineBuf));	// clear buffer
	return getString(lptr = lineBuf);	// input a line and initialize the line pointer
}
/*
        skip spaces
*/
LOCAL	void	skipSpace(void)
{
	while (isSpace(*lptr)) lptr++;
}
/*
        extract hexadecimal value
*/
LOCAL	W	getHexVal(UB **ptr)
{
	W	c;
	UW	v;
	UB	*p;

	p = *ptr;
	for (v = 0; ((c = *p) >= '0' && c <= '9') ||
		(c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); p++) {
		if (v >= 0x10000000) break;	// overflow
		v <<= 4;
		v += (c >= 'a' ? (c - 'a' + 10) :
			     (c >= 'A' ? (c - 'A' + 10) : ( c - '0')));
	}
	*ptr = p;
	return v;
}
/*
        extract item

        numerical constant:   H'<digit>  0x<digit>  hexadecimal
                        D'<digital>                  decimal (base 10)
                        Q'<digit>                  octal (base 8)
                        B'<digit>                  binary(base 2)
                        <digit>            '<digit>           hexadecimal(base 16)
        character constant:  "<character>.."        only ASCII characters are allowed
        syjmbol\:   <letter$_><letter$_digit>..
        operator:       +       addition
                        -       subtraction
                        *        multiplication
                        /       division
                        &       indirect reference
        separator:      ,
        special symbol: ;       EndOfCommand(same as EndOfLine)
                        #       size specificication
                        .       end of data
                        ^       UP (return to previous line)
*/
LOCAL	W	getToken(W defbase)
{
	W	c, i, base;

	tokenVal = 0;
	skipSpace();
	if ((c = *lptr) == 0)	{i = tEOL;	goto EXIT;}
	lptr++;

	if (c == ';')		{i = tEOC;	goto EXIT;}
	if (c == ',')		{i = tDLM;	goto EXIT;}
	if (c == '#')		{i = tSIZ;	goto EXIT;}
	if (c == '.')		{i = tEOD;	goto EXIT;}
	if (c == '^')		{i = tUP;	goto EXIT;}
	if (c == '+')		{i = tOPADD;	goto EXIT;}
	if (c == '-')		{i = tOPSUB;	goto EXIT;}
	if (c == '*')		{i = tOPMUL;	goto EXIT;}
	if (c == '/')		{i = tOPDIV;	goto EXIT;}
	if (c == '&')		{i = tOPIND;	goto EXIT;}

	if (c == '"') {		// character string
		for (tokenStr = lptr; (c = *lptr) && c != '"'; lptr++);
		tokenLen = lptr - tokenStr;
		if (c) lptr++;
		i = tSTR;
		goto EXIT;
	}

	if (*lptr == '\'') {	// number with prefix
		if (c == 'Q' || c == 'q') {base = 8;	goto NUMVAL;}
		if (c == 'B' || c == 'b') {base = 2;	goto NUMVAL;}
		if (c == 'D' || c == 'd') {base = 10;
NUMVAL:
			while ((c = *++lptr) >= '0' && c < base + '0')
				tokenVal = tokenVal * base + c - '0';
			goto NUMEXIT;
		}
		if (c == 'H' || c == 'h') goto HEXVAL;
	}

	if (isNum(c)) {		// simple number
		if (c != '0' || (*lptr != 'x' && *lptr != 'X')) {
			lptr -= 2;
			if (defbase == DB10) {base = 10; goto NUMVAL;}
		}
		goto HEXVAL;
	}

	if (c == '\'') {	// hexadecimal number
		lptr--;
HEXVAL:
		lptr++;
		tokenVal = getHexVal(&lptr);
		c = *lptr;
NUMEXIT:
                // if the end of the numeric value is alphanumeric letter, then it is regarded as illegal numeric format.
		i = (isSym(c) || isNum(c)) ? tERNUM : tNUM;
		goto EXIT;
	}

	if (isSym(c)) {		// symbol
		tokenStr = --lptr;
		for (i = 0; isSym(c) || isNum(c) || isExtSym(c); c = *++lptr) {
                        // set to tokenSym[] in capital letters
			if (i < L_SYMBOL) {
				if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
				tokenSym[i++] = c;
			}
		}
                // Fill the rest of tokenSym[] with space
		while (i < L_SYMBOL) tokenSym[i++] = ' ';

		tokenLen = lptr - tokenStr;
		i = tSYM;
		goto EXIT;
	}
        // other: illegal character error
	i = tERCH;
EXIT:
	return token = i;
}
/*
        check for end of command
*/
LOCAL	W	isnotEOC(void)
{
	if (token <= tEOC) return 0;
	return_er(E_PAR);
}
/*
        check for separator(1)
*/
LOCAL	W	isDLM(void)
{
	if (token == tDLM) {getToken(0); return 1;}
	return 0;
}
/*
        check for separtor (2)
*/
LOCAL	W	isnotDLM(void)
{
	if (isDLM()) return 0;
	return_er(E_LESS);
}
/*
        obtain numeric parametr (with performing + - * / operations)

        [+|-] {symbol|numeric value} [{+|-|* |/} {symbol|numeric value}].. {,|EOL}
*/
LOCAL	W	getNumber(W defbase, W *val)
{
	W	op, v;
	UB	*p;

        // process leading + and -
	if ((op = token) == tOPADD || op == tOPSUB) getToken(defbase);

	for (*val = 0; ;) {
		if (token == tSYM) {	// register name
			if ((v = searchRegister(tokenSym, 0)) >= 0) {
				tokenVal = getRegister(v);
			} else {	// hexadecimal value
				if (tokenSym[L_SYMBOL - 1] != ' ') break;
				p = tokenSym;
				tokenVal = getHexVal(&p);
				if (*p != ' ') break;
			}
		} else if (token != tNUM) {
			return_er(E_LESS);	// non-numeric value
		}

		// Performing + - * / operations
		if (op == tOPADD)	*val += tokenVal;
		else if (op == tOPSUB)	*val -= tokenVal;
		else if (op == tOPMUL)	*val *= tokenVal;
		else if (op == tOPDIV)	*val /= tokenVal;
		else			*val = tokenVal;

                // & operation
		while (getToken(defbase) == tOPIND) {
			if (readMem(*val, &v, 4, 4) != 4) return_er(E_MACV);
			*val = v;
		}

                // extract the next itme: if the next item is among "+ - * /" then continue processing
		if (token < tOPADD || token > tOPDIV) break;
		op = token;
		getToken(defbase);
	}
	if (token > tDLM) return_er(E_LESS);
	return 0;
}
/*
        obtain address range parameter

		[start_addr][,{end_addr|#count}]
		cAddr = start_addr
		cLen  = count

        flg     0x01    start_addr cannot be omitted
                0x02    end_addr|#count (cannot be omitted)
*/
LOCAL	W	getAddrRange(W unit, W flg, W defsz)
{
	W	sizeflg;

        // start address
	if (token > tDLM) {
		if (getNumber(0, &cAddr)) return E_LESS;
	} else {
		if (flg & 0x01) return_er(E_LESS);	// cannot be omitted
	}

        // align start address
	cAddr = ALIGN_L(cAddr, unit);

        // end address
	cLen = defsz;
	if (token == tDLM) {
		sizeflg = 0;
		if (getToken(0) == tSIZ) {
			getToken(0);
			sizeflg++;
		}
		if (getNumber(0, (UW*)&cLen)) return E_LESS;
		if (sizeflg == 0) {	// end address: up to "+ size"
			if ((UW)cLen >= cAddr || (UW)cLen >= IMPLICIT_SIZE)
                                // truncate (using the size as unit)
				cLen = ((W)((UW)cLen - cAddr) + unit) / unit;
		}
		cLen *= unit;
	} else {
		if (flg & 0x02) return_er(E_LESS);	// cannot be omitted
	}

        // validate address range
	if (cLen <= 0 || cLen > MAX_RANGE)		return_er(E_RANGE);
	if (((cLen + cAddr - 1) ^ cAddr) & 0x80000000) {
		cLen = (0x80000000 - (cAddr & 0x7fffffff)) / unit;
		if ((cLen *= unit) == 0) return_er(E_RANGE);
	}
	return 0;
}
/*
        extract set data address

        {character string | numeric parameter}[, {character string | numeric parameter}]...EOC
*/
LOCAL	W	getSetData(UB *buf, W unit)
{
	W	n, k;
	UW	num;

	for (n = 0; ;) {
		if (token == tSTR) {	// character string
			if (tokenLen == 0) return_er(E_EMPTY);

                        // Fill with 0 using 'unit' as data unit.
			k = ALIGN_U(tokenLen, unit);
			if (n + k > SETDT_SZ) return_er(E_LIMIT);
			memcpy(&buf[n], tokenStr, tokenLen);
			n += tokenLen;
			if ((k -= tokenLen) > 0) memset(&buf[n], 0, k);
			n += k;
			getToken(0);
		} else {		// numeric parameter
			if (n + unit > SETDT_SZ) return_er(E_LIMIT);
			if (getNumber(0, &num)) return E_LESS;
			switch (unit) {
			case 4:		*((UW*)&buf[n]) = (UW)num;	break;
			case 2: 	*((UH*)&buf[n]) = (UH)num;	break;
			default:	buf[n] = (UB)num;
			}
			n += unit;
		}
		if (token <= tEOC) break;
		if (isnotDLM()) return E_LESS;
	}
	if (n == 0) return_er(E_EMPTY);
	return n;	// data length
}
/*
        memory read (with error message)
*/
LOCAL	W	reaMemory(UW addr, void *dt, W len, W unit)
{
	W	n;

	if ((n = readMem(addr, dt, len, unit)) == len) return 0;
	DSP_F3(S,"ERR: Memory Read at H'", 08X,(addr+n), CH,'\n');
	return -1;
}
/*
        memory write (with error message)
*/
LOCAL	W	wriMemory(UW addr, void *dt, W len, W unit)
{
	W	n;

	if ((n = writeMem(addr, dt, len, unit)) == len) return 0;
	DSP_F3(S,"ERR: Memory Write at H'", 08X,(addr+n), CH,'\n');
	return -1;
}
/*
        display memory content
*/
LOCAL	void	dspMemory(void *p, W unit)
{
	switch (unit) {
	case 4:		DSP_F2(08X,*((UW*)p), CH,' ');	break;
	case 2:		DSP_F2(04X,*((UH*)p), CH,' ');	break;
	default:	DSP_F2(02X,*((UB*)p), CH,' ');
	}
}
/*
        memroy dump command processing

	D  [start_addr][,{end_addr|#data_cnt}]
	DB [start_addr][,{end_addr|#data_cnt}]
	DH [start_addr][,{end_addr|#data_cnt}]
	DW [start_addr][,{end_addr|#data_cnt}]
*/
LOCAL	void	cmdDump(W unit)
{
	W	i, n, k;
	UB	*cp, *ep;

        // extract address range
	cAddr = dAddr;
	if (getAddrRange(unit, 0x00, DEF_MEM_SIZE) || isnotEOC()) return;

        // dump memory content
	ep = cp = wrkBuf;
	for (dAddr = cAddr, i = 0; i < cLen;) {
                // display address
		if ((i % 16) == 0) DSP_F2(08X,dAddr, S,": ");

                // obtain memory content
		if (cp >= ep) {
			if ((n = cLen - i) > WRKBUF_SZ) n = WRKBUF_SZ;
			k = readMem(dAddr, cp = wrkBuf, n, unit);
			if (n != k) {
				errcode = E_MACV;
				cLen = i + k;
			}
			ep = cp + k;
		}
                // display memory content
		if (i < cLen) {
			dspMemory(cp, unit);
			cp += unit;
			dAddr += unit;
			i += unit;
		}
                // display character
		if ((n = i % 16) == 0 || i >= cLen) {
			k = 16 - n;
			if (n) {	// move forward to where we start character dump
				n = k / unit * (unit * 2 + 1);
				while (n-- > 0)	DSP_CH(' ');
			}
			k = (i % 16) ? (i % 16) : 16;
			for (cp -= k; cAddr < dAddr; cAddr++) {
				n = *cp++;
				DSP_CH((n >= ' ' && n < 0x7f) ? n : '.');
			}
			DSP_LF;
		}
		if (checkAbort()) {DSP_LF; break;}
	}
}
/*
        memory update command processing

	M  [start_addr][,data]..
	MB [start_addr][,data]..
	MH [start_addr][,data]..
	MW [start_addr][,data]..
*/
LOCAL	void	cmdModify(W unit)
{
	W	n;
	UB	buf[4];
	UB	svbuf[L_LINE];
	UB	*svlptr, svtoken;
	UB	dt[SETDT_SZ];

        // start address
	cAddr = mAddr;
	if (token > tDLM && getNumber(0, &cAddr)) return;

        // align address
	cAddr = ALIGN_L(cAddr, unit);

	if (token <= tEOC) {		// interactive processing
                // save command line
		memcpy(svbuf, lineBuf, L_LINE);
		svlptr = lptr;
		svtoken = token;

		for (;;) {
			DSP_F2(08X,cAddr, S,": ");	// display address
			if (reaMemory(cAddr, buf, unit, unit)) break;
			dspMemory(buf, unit);		// display data

			if (getLine("-> ") < 0) break;		// input set data
			if (getToken(0) == tEOD) break;		// end of data
			if (token <= tEOC) cAddr += unit;	// skip
			else if (token == tUP) cAddr -= unit;	// previous
			else if ((n = getSetData(dt, unit)) < 0) break;
			else {
				if (wriMemory(cAddr, dt, n, unit)) break;
				cAddr += n;
			}
		}
                // restore command line
		memcpy(lineBuf, svbuf, L_LINE);
		lptr = svlptr;
		token = svtoken;
		if (errcode == E_LESS) errcode = E_PAR;

	} else if (! isnotDLM()) {		// set data processing
		if ((n = getSetData(dt, unit)) > 0) {
			if (wriMemory(cAddr, dt, n, unit) == 0) cAddr += n;
		}
	}
	mAddr = cAddr;
}
/*
        memory embedding command processing

	F  start_addr,{end_addr|#data_cnt}[,data]..
	FB start_addr,{end_addr|#data_cnt}[,data]..
	FH start_addr,{end_addr|#data_cnt}[,data]..
	FW start_addr,{end_addr|#data_cnt}[,data]..
*/
LOCAL	void	cmdFill(W unit)
{
	W	n;
	UB	dt[SETDT_SZ];

        // extract address range
	if (getAddrRange(unit, 0x03, DEF_MEM_SIZE)) return;

        // extract set data
	if (token <= tEOC) {
		*((UW*)&dt[0]) = 0;	// 0 by default
		n = unit;
	} else {
		if (isnotDLM()) return;
		if ((n = getSetData(dt, unit)) < 0) return;
	}

        // embed set data into memory
	if (n == unit) {	// fast processing
		wriMemory(cAddr, dt, cLen, unit | 0x10);
	} else {		// ordinary mode
		for (; cLen > 0; cLen -= n, cAddr += n) {
			if (n > cLen) n = cLen;
			if (wriMemory(cAddr, dt, n, unit)) break;
		}
	}
}
/*
        memory search command processing

	SC  start_addr,{end_addr|#data_cnt},search_data..
	SCB start_addr,{end_addr|#data_cnt},search_data..
	SCH start_addr,{end_addr|#data_cnt},search_data..
	SCW start_addr,{end_addr|#data_cnt},search_data..
*/
LOCAL	void	cmdSearch(W unit)
{
	W	n, len, cnt, ofs;
	UB	*cp, *ep;
	UB	dt[SETDT_SZ];

        // extract address range
	if (getAddrRange(unit, 0x01, DEF_MEM_SIZE) || isnotDLM()) return;

        // extract search data
	if ((len = getSetData(dt, unit)) < 0) return;

	ep = cp = wrkBuf;
	for (ofs = cnt = 0; ; ) {
                // obtain memory content
		if (cp >= ep) {
			if ((n = WRKBUF_SZ - ofs) > cLen) n = cLen;
			if (ofs + n < len) break;	// end
			if (reaMemory(cAddr, &wrkBuf[ofs], n, unit)) break;
			cAddr += n;
			cLen -= n;
			ep = (cp = wrkBuf) + ofs + n;
		}
                // check if the leading byte matches
		for ( ; cp < ep && *cp != dt[0]; cp += unit);
		if ((ofs = ep - cp) < len) {
                        // if enough data is not there, move to the beginning of buffer.
			if (ofs > 0) memcpy(wrkBuf, ep = cp, ofs);
			continue;
		}
                // check for the matching of whole data
		if (memcmp(cp, dt, len) == 0) {
			if (++cnt > MAX_DSP_CNT) {
				DSP_S("..More..\n");
				break;
			}
			DSP_F2(08X,(cAddr - (ep - cp)), S,":\n");
		}
		// next
		cp += unit;
		ofs = 0;
		if (checkAbort()) break;
	}
}
/*
        memory comparison / move command processing

	CMP start_addr,{end_addr|#data_cnt},target_addr
	MOV start_addr,{end_addr|#data_cnt},target_addr
*/
LOCAL	void	cmdCmpMov(W mov)
{
	UW	dst;
	W	i, n, cnt;
#define	BFSZ	(WRKBUF_SZ / 2)

        // extract address range
	if (getAddrRange(1, 0x01, DEF_MEM_SIZE) || isnotDLM()) return;

        // transfer / compare target
	if (getNumber(0, &dst) || isnotEOC()) return;

	if (mov) {	// memory transfer
		for (; (n = cLen) > 0 && checkAbort() == 0;
					cAddr += n, dst += n, cLen -= n) {
			if (n > WRKBUF_SZ) n = WRKBUF_SZ;
			if (reaMemory(cAddr, wrkBuf, n, 1)) break;
			if (wriMemory(dst, wrkBuf, n, 1)) break;
		}

	} else {	// memory comparison
		for (cnt = 0; (n = cLen) > 0 && checkAbort() == 0;
					cAddr += n, dst += n, cLen -= n) {
			if (n > BFSZ) n = BFSZ;
			if (reaMemory(cAddr, wrkBuf, n, 1)) break;
			if (reaMemory(dst, &wrkBuf[BFSZ], n, 1)) break;
			if (memcmp(wrkBuf, &wrkBuf[BFSZ], n) == 0) continue;
			for (i = 0; i < n; i++) {
				if (wrkBuf[i] == wrkBuf[BFSZ + i]) continue;
				if (++cnt > MAX_DSP_CNT) {
					DSP_S("..More..\n");
					cLen = 0;	// terminate
					break;
				}
				DSP_F4(08X,(cAddr + i), S,": ",
				       02X,wrkBuf[i], S," -> ");
				DSP_F4(08X,(dst + i), S,": ",
				       02X,(wrkBuf[BFSZ + i]), CH,'\n');
			}
		}
	}
}
/*
        I/O port input and output command processing

	IB/IH/IW	port
	OB/OH/OW	port, data
*/
LOCAL	void	cmdIO(W unit)
{
	UW	port, data;
	UB	*dir;

        // extract port number
	if (getNumber(0, &port)) return;

	if (unit & 0x10) {	// output command
		if (!isDLM()) er_return(E_LESS);
		if (getNumber(0, &data) || isnotEOC()) return;
		if (writeIO(port, data, unit &= 0x0f) == 0) er_return(E_MACV);
		dir = "<--";
	} else {		// input command
		if (isnotEOC()) return;
		if (readIO(port, &data, unit) == 0) er_return(E_MACV);
		dir = "-->";
	}
        // display result
	DSP_F2(S,"Port ", 08X,port);
	switch (unit) {
	case 4:	DSP_F5(S,":W ", S,dir, CH,' ', 08X,(UW)data, CH,'\n');
		break;
	case 2:	DSP_F5(S,":H ", S,dir, CH,' ', 04X,(UH)data, CH,'\n');
		break;
	default:DSP_F5(S,":B ", S,dir, CH,' ', 02X,(UB)data, CH,'\n');
		break;
	}
}
/*
        disasseble command processing

	DA [start_addr][,steps]
*/
LOCAL	void	cmdDisasm(void)
{
	er_return(E_NOSPT);
}
/*
        display / set register command processing

	R [register_name[,data]]
*/
LOCAL	void	cmdRegister(void)
{
	W	rno;
	UW	num;

	if (token <= tEOC) {	// ordinary register dump
		dispRegister(-1);

	} else {		// extract register name
		if (token != tSYM || (rno = searchRegister(tokenSym, 1)) < 0)
			er_return(E_ILREG);

		if (getToken(0) <= tEOC) {	// display register
			dispRegister(rno);

		} else if (!isnotDLM() && !getNumber(0, &num)) {	// set register
			if (!isnotEOC())
				er_return(setRegister(rno, num));
		}


	}
}
/*
        execute / trace command processing

	G	[start_addr][,end_addr]
	S/N	[start_addr][,steps]
*/
LOCAL	void	cmdGoTrace(W trace)
{
	UW	pc, par;

        // extract execution address
	pc = getCurPC();
	if (token > tDLM && getNumber(0, &pc)) return;
	if (invalidPC(pc)) er_return(E_PC);

        // extract end address or number of steps
	par = 0;
	if (isDLM()) {
		if (getNumber(0, &par)) return;
		if (trace == 0 && invalidPC(par)) er_return(E_MACV);
	}
	if (isnotEOC()) return;

	if (trace && par <= 0) par = 1;		// number of steps

        //execute program
	errcode = goTrace(trace, pc, par);
	if (errcode >= E_OK) errcode = CMD_FINISH;	//command process termination
}
/*
        display / set breakpoint command processing

	B [break_addr[,break_attr][,commands]]
*/
LOCAL	void	cmdBreak(VOID)
{
	UW	addr;
	W	atr, cmdlen;
	UB	*cmd;

	if (token <= tEOC) {	 // display breakpoint
		dspBreak();
		return;
	}

        // extract breakpoint address
	if (getNumber(0, &addr)) return;

        // extract break attribute and command
	atr = cmdlen = 0;
	cmd = NULL;
	while (token == tDLM) {
                // "+:" are handled as symbols
		symExt[0] = '+'; symExt[1] = ':';
		getToken(0);
		symExt[0] = symExt[1] = '\0';
		if (token == tSYM) {
			if (atr) break;
			if ((atr = getBreakAtr(tokenSym)) < 0)
				er_return(E_BPATR);
		} else if (token == tSTR) {
			if (cmdlen) break;
			if ((cmdlen = tokenLen) > L_BPCMD) er_return(E_BPCMD);
			cmd = tokenStr;
		}
		getToken(0);
	}

        //set breakpoint
	if (! isnotEOC()) {
		if ((atr = setBreak(addr, atr, cmd, cmdlen))) er_return(atr);
	}
}
/*
        clear breakpoint command processing

	BC [break_addr][,break_addr]..
*/
LOCAL	void	cmdBrkClr(void)
{
	UW	addr;

	if (token <= tEOC) {
		clearBreak(0);	// clear all
	} else {
		do {	// clear individual breakpoint
			if (getNumber(0, &addr)) return;
			if (clearBreak(addr) < 0) er_return(E_BPUDF);
		} while (isDLM());
		isnotEOC();
	}
}
/*
        download command processing

	LO protocol[,loading_addr]
*/
LOCAL	void	cmdLoad(void)
{
	W	i, par;
	UW	addr;
LOCAL	const	struct {
	UB	nm[2];
	UH	par;
} proto[] = {
	{"S ", P_TEXT | P_SFORM},
	{"XS", P_XMODEM | P_SFORM},
	{"XM", P_XMODEM | P_MEMIMG},
	{"  ", 0x00}
};

        // extract protocol
	if (token != tSYM) er_return(E_LESS);

	par = 0;
	if (tokenSym[2] == ' ')  {
		for (i = 0; proto[i].par != 0; i++) {
			if (*((UH*)tokenSym) == *((UH*)proto[i].nm)) {
				par = proto[i].par;
				break;
			}
		}
	}
	if (par == 0) er_return(E_PROTO);

        // extract start address
	getToken(0);
	if (isDLM()) {
		if (getNumber(0, &addr)) return;
	} else {
		addr = 0;
		if (par & P_MEMIMG) er_return(E_NOADDR);
	}
	if (isnotEOC()) return;

        // execute loading
	errcode = doLoading(par, addr, NULL);
}
/*
        backtrace command processing

	BTR
*/
LOCAL	void	cmdBackTrace(void)
{
	er_return(E_NOSPT);
}
/*
        disk command processing

	RD device, sblk, nblk, addr
	WD device, sblk, nblk, addr
	ID device
	BD [device]
*/
LOCAL	void	cmdDisk(W kind)
{
	W	i;
	UW	par[3], blksz, nblks;
	UB	c, devnm[L_DEVNM + 1];

        // extract device name
	if (token <= tEOC) {
		if (kind != 3) er_return(E_LESS);
		devnm[0] = '\0';
	} else {
		if (token != tSYM || tokenLen > L_DEVNM) er_return(E_LESS);
                // device names are to be given in lower case letters
		for (i = 0; i < tokenLen; i++) {
			c = tokenSym[i];
			if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
			devnm[i] = c;
		}
		devnm[i] = '\0';
		getToken(0);
	}

        // extract parameters
	if (kind <= 1) {
		for (i = 0; i < 3; i++) {
			if (isnotDLM()) return;
			if (getNumber(0, &par[i])) return;
		}
	}
	if (isnotEOC()) return;

	switch(kind) {
	case 0:		// ReadDisk
	case 1:		// WriteDisk
		errcode = rwDisk(devnm, par[0], par[1], (void*)par[2], kind);
		break;
	case 2:		// InfoDisk
		errcode = infoDisk(devnm, &blksz, &nblks);
		if (errcode >= E_OK) {
			DSP_S(devnm);
			DSP_F5(S,": Bytes/block: ", D,blksz,
			       S," Total blocks: ", D,nblks, CH,'\n');
		}
		break;
	case 3:		// BootDisk
		errcode = bootDisk(( devnm[0] == '\0' )? NULL: devnm);
		if (errcode >= E_OK) errcode = CMD_FINISH;	// Fin
		break;
	}
}
/*
        exit command processing

	EX	[par]
*/
LOCAL	void	cmdExit(void)
{
	W	par;

        // extract parameters
	if (token <= tDLM) par = 0;
	else if (getNumber(0, &par)) return;

	DSP_S((par < 0) ? "** System Reset\n" : "** System Power Off\n");
	waitMsec(100);	/* give extra time for draining the remaining output */

	sysExit(par);		// system reset or power off (never returnes)
}
/*
        forcible kill process command processing

	KILL
*/
LOCAL	void	cmdKill(void)
{
	if (isnotEOC()) return;
	if (isKillValid() == 0) {
		killProcReq = 1;
		errcode = CMD_FINISH;
	}
}
/*
        FROM write command processing

	WROM
*/
LOCAL	void	cmdWrom(void)
{
	UW	addr, data;
	W	nsec;

        // extract parameters
	if (getNumber(0, &addr)) return;
	if (isnotDLM() || getNumber(0, &data)) return;
	if (isnotDLM() || getNumber(0, &nsec)) return;
	if (isnotEOC()) return;
	errcode = writeFrom(addr, data, nsec, 1);
}
/*
        FLASH ROM load command processing

	FLLO [attr]
*/
LOCAL	void	cmdFlashLoad(void)
{
	W	i, proto, mode;
	UW	addr[3];

	proto = P_TEXT | P_SFORM;
	mode = 0;

        // extract attributes
	if (token > tEOC) {
		if (token != tSYM) er_return(E_PAR);
		for (i = 0; i < L_SYMBOL; i++) {
			switch(tokenSym[i]) {
			case 'X':	proto = P_XMODEM | P_SFORM;	break;
			case 'E':	mode = 1;			break;
			case ' ':	i = L_SYMBOL;			break;
			default:	er_return(E_PAR);
			}
		}
		getToken(0);
		if (isnotEOC()) return;
	}

        // execute loading
	setupFlashLoad(0, addr);
	i = addr[1] - addr[0] + 1;
	if (mode) {
		DSP_S("Fill Loading RAM Area with 0xFF\n");
		memset((void*)addr[0], 0xFF, i);
	} else {
		DSP_S("Copy Flash ROM Image to RAM Area\n");
		memcpy((void*)addr[0], (void*)(addr[0] - addr[2]), i);
	}
	DSP_S("> Load S-Format Data of Flash ROM\n");
	errcode = doLoading(proto, 0, addr);
	if (errcode < 0) return;

        // FLASH ROM write
	setupFlashLoad(-1, addr);
	DSP_F5(S,"Writing Flash ROM at ", 08X,addr[0],
	       S," [", D,addr[2], S," blks] ... wait\n");
	errcode = writeFrom(addr[0], addr[1], addr[2], -1);
}
/*
        command table
*/
typedef	struct {
	UB		fnm[12];	// full command name
	UB		snm[4];		// abbreviated command name
	FP		func;		// processing function
	W		para;		// parameter information and other
	const HELP	*help;		// help message
} CMDTAB;

#define	IGN_TRACE	0x1000

LOCAL	void	cmdHelp(void);

LOCAL	const	CMDTAB	cmdTab[] = {
	{"DUMP        ","D   ",	cmdDump,	1,		&helpD	  },
	{"DUMPBYTE    ","DB  ",	cmdDump,	1,		&helpDB	  },
	{"DUMPHALF    ","DH  ",	cmdDump,	2,		&helpDH	  },
	{"DUMPWORD    ","DW  ",	cmdDump,	4,		&helpDW	  },
	{"MODIFY      ","M   ",	cmdModify,	1,		&helpM	  },
	{"MODIFYBYTE  ","MB  ",	cmdModify,	1,		&helpMB	  },
	{"MODIFYHALF  ","MH  ",	cmdModify,	2,		&helpMH	  },
	{"MODIFYWORD  ","MW  ",	cmdModify,	4,		&helpMW	  },
	{"FILL        ","F   ",	cmdFill,	1,		&helpF	  },
	{"FILLBYTE    ","FB  ",	cmdFill,	1,		&helpFB	  },
	{"FILLHALF    ","FH  ",	cmdFill,	2,		&helpFH	  },
	{"FILLWORD    ","FW  ",	cmdFill,	4,		&helpFW	  },
	{"SEARCH      ","SC  ",	cmdSearch,	1,		&helpSC	  },
	{"SEARCHBYTE  ","SCB ",	cmdSearch,	1,		&helpSCB  },
	{"SEARCHHALF  ","SCH ",	cmdSearch,	2,		&helpSCH  },
	{"SEARCHWORD  ","SCW ",	cmdSearch,	4,		&helpSCW  },
	{"COMPARE     ","CMP ",	cmdCmpMov,	0,		&helpCMP  },
	{"MOVE        ","MOV ",	cmdCmpMov,	1,		&helpMOV  },
	{"INPUTBYTE   ","IB  ",	cmdIO,		1,		&helpIB	  },
	{"INPUTHALF   ","IH  ",	cmdIO,		2,		&helpIH	  },
	{"INPUTWORD   ","IW  ",	cmdIO,		4,		&helpIW	  },
	{"OUTPUTBYTE  ","OB  ",	cmdIO,		0x11,		&helpOB	  },
	{"OUTPUTHALF  ","OH  ",	cmdIO,		0x12,		&helpOH	  },
	{"OUTPUTWORD  ","OW  ",	cmdIO,		0x14,		&helpOW	  },
	{"DISASSEMBLE ","DA  ",	cmdDisasm,	0,		&helpDA	  },
	{"REGISTER    ","R   ",	cmdRegister,	0,		&helpR	  },
	{"BREAKPOINT  ","B   ",	cmdBreak,	0,		&helpB	  },
	{"BREAKCLEAR  ","BC  ",	cmdBrkClr,	0,		&helpBC	  },
	{"GO          ","G   ",	cmdGoTrace,	0 | IGN_TRACE,	&helpG	  },
	{"STEP        ","S   ",	cmdGoTrace,	1 | IGN_TRACE,	&helpS	  },
	{"NEXT        ","N   ",	cmdGoTrace,	2 | IGN_TRACE,	&helpN	  },
	{"BACKTRACE   ","BTR ",	cmdBackTrace,	0,		&helpBTR  },
	{"LOAD        ","LO  ",	cmdLoad,	0,		&helpLO	  },
	{"READDISK    ","RD  ",	cmdDisk,	0,		&helpRD	  },
	{"WRITEDISK   ","WD  ",	cmdDisk,	1,		&helpWD	  },
	{"INFODISK    ","ID  ",	cmdDisk,	2,		&helpID	  },
	{"BOOTDISK    ","BD  ",	cmdDisk,	3,		&helpBD	  },
	{"KILL        ","KILL",	cmdKill,	0,		&helpKILL },
	{"WRITEROM    ","WROM",	cmdWrom,	0,		&helpWROM },
	{"FLASHLOAD   ","FLLO",	cmdFlashLoad,	0,		&helpFLLO },
	{"HELP        ","H   ",	cmdHelp,	0,		&helpH	  },
	{"HELP        ","?   ",	cmdHelp,	0,		&helpH	  },
	{"EXIT        ","EX  ",	cmdExit,	0,		&helpEX	  },
	{ } /* end */
};
/*
        searching command
*/
LOCAL	W	searchCommand(void)
{
	W	i;

	if (token == tSYM && tokenSym[12] == ' ') {
		for (i = 0; cmdTab[i].func != NULL; i++) {
			if (memcmp(cmdTab[i].fnm, tokenSym, 12) == 0 ||
				(tokenSym[4] == ' ' &&
				*((UW*)cmdTab[i].snm) == *((UW*)tokenSym)) )
				return i;
		}
	}
	return	E_CMD;
}
/*
        help command pcrocessing

	H(?) [command]
*/
LOCAL	void	cmdHelp(void)
{
	W	i;

	i = searchCommand();
	printHelp(( i < 0 )? &helpALL: cmdTab[i].help);
}
/*
        command interpreter

        cmd : initial command line (if NULL : none)
        fin : 0 = continue, 1 = finish (command execution)
              < 0 : trace command execution
*/
EXPORT	void	procCommand(UB *cmd, W fin)
{
	W	i, par;

        // initialize command input
	if (cmd) {
		strcpy(lptr = lineBuf, cmd);
		token = tEOC;
	} else {
		token = tEOL;
		fin = 0;
	}

        // set DA address to PC
	daAddr = getCurPC();

	for (;;) {
                // skip the remainder of the previous command
		while (token > tEOC) getToken(0);

                // input a command line
		if (token == tEOL) {
			if (fin) break;		// end
			if (getLine(PROMPT) <= 0) continue;
		}

                // skip comment
		skipSpace();
		if (*lptr == '*') {
			getToken(0);
			continue;
		}
                // extract command
		if (getToken(0) <= tEOC) continue;	// skip empty line

                // searching command
		errcode = errinfo = 0;
		if ((i = searchCommand()) < 0) {
			errcode = E_CMD;
		} else {
			if (checkAbort()) continue;
			par = cmdTab[i].para;

                        // if there is an initial command, the execution command is ignored
			if (fin < 0 && (par & IGN_TRACE)) continue;

                        // read-ahead of parameters
			getToken(0);

                        // command execution
			(*(cmdTab[i].func))(par & 0xff);
		}
		if (errcode == CMD_FINISH) break;	// finish

                // display error
		if (errcode < 0) dspError();
	}
}
