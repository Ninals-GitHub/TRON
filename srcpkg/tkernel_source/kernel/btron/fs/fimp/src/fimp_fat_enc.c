/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2013/03/08.
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
 *	@(#)fimp_fat_enc.c
 *
 */

#include "fimp_fat_enc.h"

/*
 * ASCII character kind table
 */
#define LOW	ASCII_LOW
#define UP	ASCII_UP
#define NUM	ASCII_NUM
#define SYM	ASCII_SYM
#define CON	ASCII_CON

#define DENY	(0x00)
#define SHRT	(0x10)		/* Allowed by Short name */
#define LONG	(0x20)		/* Allowed by Long name  */
#define BOTH	(0x30)

EXPORT	const	UB	fatEncAsciiKindTable[ASCII_MAX + 1] = {
	/* 00-07: NUL SOH STX ETX EOT ENQ ACK BEL   */
	CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY,
	/* 08-0f: BS  HT  LF  VT  FF  CR  SO  SI    */
	CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY,
	/* 10-17: DLE DC1 DC2 DC3 DC4 NAK SYN ETB   */
	CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY,
	/* 18-1f: CAN EM  SUB ESC FS  GS  RS  US    */
	CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY, CON|DENY,
	/* 20-27: Spa !   "   #   $   %   &   '     */
	SYM|LONG, SYM|BOTH, SYM|DENY, SYM|BOTH, SYM|BOTH, SYM|BOTH, SYM|BOTH, SYM|BOTH,
	/* 28-2f: (   )   *   +   ,   -   .   /     */
	SYM|BOTH, SYM|BOTH, SYM|DENY, SYM|LONG, SYM|LONG, SYM|BOTH, SYM|LONG, SYM|DENY,
	/* 30-37: 0   1   2   3   4   5   6   7     */
	NUM|BOTH, NUM|BOTH, NUM|BOTH, NUM|BOTH, NUM|BOTH, NUM|BOTH, NUM|BOTH, NUM|BOTH,
	/* 38-3f: 8   9   :   ;   <   =   >   ?     */
	NUM|BOTH, NUM|BOTH, SYM|DENY, SYM|LONG, SYM|DENY, SYM|LONG, SYM|DENY, SYM|DENY,
	/* 40-47: @   A   B   C   D   E   F   G     */
	SYM|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,
	/* 48-4f: H   I   J   K   L   M   N   O     */
	 UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,
	/* 50-57: P   Q   R   S   T   U   V   W     */
	 UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,  UP|BOTH,
	/* 58-5f: X   Y   Z   [   \   ]   ^   _     */
	 UP|BOTH,  UP|BOTH,  UP|BOTH, SYM|LONG, SYM|DENY, SYM|LONG, SYM|BOTH, SYM|BOTH,
	/* 60-67: `   a   b   c   d   e   f   g     */
	SYM|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH,
	/* 68-6f: h   i   j   k   l   m   n   o     */
	LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH,
	/* 70-77: p   q   r   s   t   u   v   w     */
	LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH, LOW|BOTH,
	/* 78-7f: x   y   z   {   |   }   ~   DEL   */
	LOW|BOTH, LOW|BOTH, LOW|BOTH, SYM|BOTH, SYM|DENY, SYM|BOTH, SYM|BOTH, CON|DENY,
};

/*
 *  Compare two strings ignoring different of capital letter.
 */
EXPORT	W	fat_strncasecmp(const UB *str1, const UB *str2, UW n)
{
	UB	c1, c2;

	while (n-- != 0) {
		c1 = toupper(*str1++);
		c2 = toupper(*str2++);
		if (c1 != c2) return (W)c1 - (W)c2;
		if (c1 == '\0') break;
	}
	return 0;
}

/*
 *  UTF16 string length
 */
EXPORT	W	fat_strlen16(const UH *utf16str)
{
	W	len;

	for (len = 0; utf16str[len] != (UH)'\0'; len++);
	return len;
}

/*
 *  Convert Utf8 -> Unicode : returns the number of encoded bytes.
 */
EXPORT	W	fatEncUtf8ToUnicode(const UB utf8str[], UW *unicode)
{
#ifdef	FAT_ASCII_FN_ONLY

	if (utf8str[0] < 0x80) {
		*unicode = (UW)utf8str[0];
		return 1;
	}
	*unicode = 0;
	return 0;

#else	/* FAT_ASCII_FN_ONLY */

	UB	c0, c1, c2, c3, c4, c5;
	UW	uc;
	W	len;

	len = 0;
	uc = 0;

	/* 0xxx xxxx : 0 - 0x7f */
	c0 = utf8str[0];
	if ((c0 & 0x80) == 0x00) {
		uc = (UW)c0;
		len = 1;
		goto exit0;
	}

	/* 110yyy yx, 10xx xxxx : 0x80 - 0x7ff */
	if ((c1 = utf8str[1] ^ 0x80) > 0x3f) goto exit0;
	if ((c0 & 0xe0) == 0xc0) {
		uc = (((UW)c0 & 0x1f) << 6) | (UW)c1;
		if (uc > 0x7f) len = 2;
		goto exit0;
	}

	/* 1110yyyy, 10yxxx xx, 10xx xxxx : 0x800 - 0xffff */
	if ((c2 = utf8str[2] ^ 0x80) > 0x3f) goto exit0;
	if ((c0 & 0xf0) == 0xe0) {
		uc = (((UW)c0 & 0x0f) << 12) | ((UW)c1 << 6) | (UW)c2;
		if (uc > 0x7ff) len = 3;
		goto exit0;
	}
	/* 11110y yy, 10yy xxxx, 10xxxx xx, 10xx xxxx : 0x10000 - 0x1fffff */
	if ((c3 = utf8str[3] ^ 0x80) > 0x3f) goto exit0;
	if ((c0 & 0xf8) == 0xf0) {
		uc = (((UW)c0 & 0x07) << 18) | ((UW)c1 << 12) | ((UW)c2 << 6)
			| (UW)c3;
		if (uc > 0xffff) len = 4;
		goto exit0;
	}
	/* 111110yy, 10yyyx xx, 10xx xxxx, 10xxxx xx, 10xx xxxx :
		0x200000 - 0x3ffffff */
	if ((c4 = utf8str[4] ^ 0x80) > 0x3f) goto exit0;
	if ((c0 & 0xfc) == 0xf8) {
		uc = (((UW)c0 & 0x03) << 24) | ((UW)c1 << 18) | ((UW)c2 << 12)
			| ((UW)c3 << 6) | (UW)c4;
		if (uc > 0x1fffff) len = 5;
		goto exit0;
	}
	/* 1111110y 10yy yyxx 10xxxx xx 10xx xxxx 10xxxx xx 10xx xxxx :
		0x4000000 - 0x7fffffff */
	if ((c5 = utf8str[5] ^ 0x80) > 0x3f) goto exit0;
	if ((c0 & 0xfe) == 0xfc) {
		uc = (((UW)c0 & 0x01) << 30) | ((UW)c1 << 24) | ((UW)c2 << 18)
			| ((UW)c3 << 12) | ((UW)c4 << 6) | (UW)c5;
		if (uc > 0x3ffffff) len = 6;
		goto exit0;
	}
exit0:
	*unicode = (len == 0) ? 0 : uc;
	return len;
#endif	/* FAT_ASCII_FN_ONLY */
}

/*
 *  Convert Unicode -> Utf8
 */
EXPORT	W	fatEncUnicodeToUtf8(UW unicode, UB utf8a[UTF8_MAX])
{
#ifdef	FAT_ASCII_FN_ONLY

	utf8a[0] = (unicode <= 0x7f) ? (UB)unicode : '_';
	return 1;

#else	/* FAT_ASCII_FN_ONLY */

	W	len;

	/* 0xxx xxxx */
	if (unicode <= 0x7f) {
		utf8a[0] = (UB)(unicode & 0x7f);
		len = 1;
	}
	/* 110yyy yx, 10xx xxxx */
	else if (unicode <= 0x7ff) {
		utf8a[0] = ((unicode >> 6) & 0x1f) | 0xc0;
		utf8a[1] = (unicode & 0x3f) | 0x80;
		len = 2;
	}
	/* 1110yyyy, 10yxxx xx, 10xx xxxx */
	else if (unicode <= 0xffff) {
		utf8a[0] = ((unicode >> 12) & 0x1f) | 0xe0;
		utf8a[1] = ((unicode >> 6) & 0x3f) | 0x80;
		utf8a[2] = (unicode & 0x3f) | 0x80;
		len = 3;
	}
	/* 11110y yy, 10yy xxxx, 10xxxx xx, 10xx xxxx */
	else if (unicode <= 0x1fffff) {
		utf8a[0] = ((unicode >> 18) & 0x07) | 0xf0;
		utf8a[1] = ((unicode >> 12) & 0x3f) | 0x80;
		utf8a[2] = ((unicode >> 6) & 0x3f) | 0x80;
		utf8a[3] = (unicode & 0x3f) | 0x80;
		len = 4;
	}
	/* 111110yy, 10yyyx xx, 10xx xxxx, 10xxxx xx, 10xx xxxx */
	else if (unicode <= 0x3ffffff) {
		utf8a[0] = ((unicode >> 24) & 0x03) | 0xf8;
		utf8a[1] = ((unicode >> 18) & 0x3f) | 0x80;
		utf8a[2] = ((unicode >> 12) & 0x3f) | 0x80;
		utf8a[3] = ((unicode >> 6) & 0x3f) | 0x80;
		utf8a[4] = (unicode & 0x3f) | 0x80;
		len = 5;
	}
	/* 1111110y 10yy yyxx 10xxxx xx 10xx xxxx 10xxxx xx 10xx xxxx */
	else if (unicode <= 0x7fffffff) {
		utf8a[0] = ((unicode >> 30) & 0x01) | 0xfc;
		utf8a[1] = ((unicode >> 24) & 0x3f) | 0x80;
		utf8a[2] = ((unicode >> 18) & 0x3f) | 0x80;
		utf8a[3] = ((unicode >> 12) & 0x3f) | 0x80;
		utf8a[4] = ((unicode >> 6) & 0x3f) | 0x80;
		utf8a[5] = (unicode & 0x3f) | 0x80;
		len = 6;
	} else {
		len = 0;
	}
	return len;

#endif	/* FAT_ASCII_FN_ONLY */
}

/*
 *  Convert Utf8 string to UTF16 string.
 *	If success, return length of the utf16 string (not byte).
 *	Else, return -1.
 *	utf8len is the length of utf8str, not include '\0'.
 *	utf16buflen is the byte number of the utf16str, include '\0'.
 */
EXPORT	W	fatEncUtf8strToUtf16str(const UB *utf8str,
			W utf8len, UH *utf16str, W utf16buflen, BOOL toUpper)
{
	W	i8, i16;
	UW	unicode;

	for (utf16buflen--, i8 = i16 = 0;
		i8 < utf8len && i16 < utf16buflen && utf8str[i8] != '\0'; ) {

		i8 += fatEncUtf8ToUnicode(&utf8str[i8], &unicode);
		if (UnicodeIsAscii(unicode)) {
			if (AsciiIsLFN(unicode)) {
				utf16str[i16++] = (toUpper == FALSE) ?
					unicode : AsciiToUP(unicode);
			} else {
				i16 = -1;
				break;
			}
#ifndef	FAT_ASCII_FN_ONLY
		} else if (UnicodeIsNormal(unicode)) {
			utf16str[i16++] = (UH)unicode;
		} else if (UnicodeIsSurrogate(unicode)) {
			utf16str[i16++] = UnicodeToUtf16UpSurrogate(unicode);
			utf16str[i16++] = UnicodeToUtf16LowSurrogate(unicode);
#endif	/* ~ FAT_ASCII_FN_ONLY */
		} else {
			i16 = -1;
			break;
		}
	}
	if (i16 >= 0) utf16str[i16] = (UH)'\0';
	return i16;
}

/*
 *  Convert UTF16 string to Utf8 string.
 */
EXPORT	W	fatEncUtf16strToUtf8str(const UH *utf16str,
				W utf16len, UB *utf8str, W utf8buflen)
{
	W	i8, i16;
	UB	utf8a[UTF8_MAX];

	for (i8 = i16 = 0; i16 < utf16len && i8 < utf8buflen
					&& utf16str[i16] != (UH)'\0'; ) {
#ifdef	FAT_ASCII_FN_ONLY
		if (fatEncUnicodeToUtf8(utf16str[i16++], utf8a) > 0 &&
				i8 < utf8buflen) {
			utf8str[i8++] = utf8a[0];
		}
#else	/* FAT_ASCII_FN_ONLY */
		W	i, len;
		UW	unicode;
		
		if (Utf16IsUpSurrogate(utf16str[i16])
			&& Utf16IsLowSurrogate(utf16str[i16 + 1])) {
			unicode = Utf16ToUnicode(
					utf16str[i16], utf16str[i16 + 1]);
			i16 += 2;
		} else {
			unicode = utf16str[i16++];
		}
		len = fatEncUnicodeToUtf8(unicode, utf8a);
		for (i = 0; i < len && i8 < utf8buflen; i++, i8++) {
			utf8str[i8] = utf8a[i];
		}
#endif	/* FAT_ASCII_FN_ONLY */
	}
	if (i8 < utf8buflen) {
		utf8str[i8] = '\0';
	} else {
		utf8str[i8 - 1] = '\0';
	}
	return i8;
}

