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
 *	@(#)fimp_fat_enc.h
 *
 */

#ifndef __FIMP_FAT_ENC_H__
#define __FIMP_FAT_ENC_H__

#include <t2ex/fs.h>
#include <tk/tkernel.h>
#include <string.h>
#include <ctype.h>

/*
 *  String operations
 */
#define fat_strncpy(dst,src,n)	(UB*)strncpy((B*)(dst),(const B*)(src),(size_t)(n))
IMPORT	W	fat_strncasecmp(const UB *str1, const UB *str2, UW n);
IMPORT	W	fat_strlen16(const UH *utf16str);

/*
 *  Endian conversion
 */
#if	BIGENDIAN
#define CEH(x)	swapH(x)
#define CEW(x)	swapW(x)
#else
#define CEH(x)	(x)
#define CEW(x)	(x)
#endif

Inline	UH	swapH(UH x)
{
	return (x << 8) | (x >> 8);
}
Inline	UW	swapW(UW x)
{
	return (UW)swapH((UH)x) << 16 | swapH((UH)(x >> 16));
}

/*
 *  Missalign operations
 */
Inline	UH	GetMisalignH(UB d[2])
{
#if ALLOW_MISALIGN
	return *(UH *)d;
#else
#if BIGENDIAN
	return ((UH)d[0] << 8) | (UH)d[1];
#else
	return ((UH)d[1] << 8) | (UH)d[0];
#endif
#endif
}

Inline	void	SetMisalignH(UB d[2], UH x)
{
#if ALLOW_MISALIGN
	*(UH *)d = x;
#else
#if BIGENDIAN
	d[0] = x >> 8;
	d[1] = x;
#else
	d[1] = x >> 8;
	d[0] = x;
#endif
#endif
}

Inline	UW	GetMisalignW(UB d[4])
{
#if ALLOW_MISALIGN
	return *(UW *)d;
#else
#if BIGENDIAN
	return ((UW)d[0] << 24) | ((UW)d[1] << 16) | \
				((UW)d[2] << 8) | (UW) d[3];
#else
	return ((UW)d[3] << 24) | ((UW)d[2] << 16) | \
				((UW)d[1] << 8) | (UW) d[0];
#endif
#endif
}

Inline	void	SetMisalignW(UB d[4], UW x)
{
#if ALLOW_MISALIGN
	*(UW *)d = x;
#else
#if BIGENDIAN
	d[0] = x >> 24;
	d[1] = x >> 16;
	d[2] = x >> 8;
	d[3] = x;
#else
	d[3] = x >> 24;
	d[2] = x >> 16;
	d[1] = x >> 8;
	d[0] = x;
#endif
#endif
}

/*
 *  Encode definitions
 */
#define UTF8_MAX		(6)
#define ENC_CHAR_MAX		(4)

#define ASCII_MAX		(0x7f)

/* ASCII tables for character check */
IMPORT const	UB	fatEncAsciiKindTable[ASCII_MAX + 1];

#define AsciiToUP(ascii)	((UB)toupper((ascii) & ASCII_MAX))
#define AsciiToLOW(ascii)	((UB)tolower((ascii) & ASCII_MAX))

#define getAsciiKind(ascii)	(fatEncAsciiKindTable[(ascii)] & 0xf)
#define ASCII_LOW		(0)	/* Lower	*/
#define ASCII_UP		(1)	/* Upper	*/
#define ASCII_NUM		(2)	/* Number	*/
#define ASCII_SYM		(3)	/* Symbol	*/
#define ASCII_CON		(4)	/* Control code */
#define ASCII_KIND_MAX		(5)

#define AsciiIsLOW(ascii)	(getAsciiKind(ascii) == ASCII_LOW)
#define AsciiIsUP(ascii)	(getAsciiKind(ascii) == ASCII_UP)
#define AsciiIsNUM(ascii)	(getAsciiKind(ascii) == ASCII_NUM)
#define AsciiIsSYM(ascii)	(getAsciiKind(ascii) == ASCII_SYM)

#define AsciiIsSFN(ascii)	((fatEncAsciiKindTable[(ascii)] & 0x10) == 0x10)
#define AsciiIsLFN(ascii)	((fatEncAsciiKindTable[(ascii)] & 0x20) == 0x20)


#define CharIsAscii(c)		((0 < (c)) && ((c) <= ASCII_MAX))

#define Utf8IsAscii(utf8)	((0 < (utf8)) && ((utf8) <= ASCII_MAX))

#define Utf16IsUpSurrogate(utf16)	\
			((0xD800 <= (utf16)) && ((utf16) <= 0xDBFF))
#define Utf16IsLowSurrogate(utf16)	\
			((0xDC00 <= (utf16)) && ((utf16) <= 0xDFFF))
#define Utf16ToUnicode(up, low) 		\
			(UW)((((up) - 0xd800) * 0x400) + ((low) - 0xdc00))

#define UnicodeIsAscii(unicode) 	\
			((0 < (unicode)) && ((unicode) <= ASCII_MAX))
#define UnicodeIsNormal(unicode)	\
			((0 < (unicode)) && ((unicode) < 0x10000))
#define UnicodeIsSurrogate(unicode)	\
			((0x10000 <= (unicode)) && ((unicode) < 0x20000))
#define UnicodeIsInvalid(unicode)	\
			(((unicode) <= 0) || (0x20000 <= (unicode)))

#define UnicodeToUtf16UpSurrogate(unicode)	\
			(UH)(((unicode - 0x10000) / 0x400) + 0xd800)
#define UnicodeToUtf16LowSurrogate(unicode)	\
			(UH)(((unicode - 0x10000) % 0x400) + 0xdc00)

/*
 *  Encode functions
 */
IMPORT	W	fatEncUtf8ToUnicode(const UB utf8str[], UW *_unicode);

IMPORT	W	fatEncUnicodeToUtf8(UW unicode, UB utf8a[UTF8_MAX]);

IMPORT	W	fatEncUtf8strToUtf16str(const UB *utf8str,
		W utf8len, UH *utf16str, W utf16buflen, BOOL toUpper);

IMPORT	W	fatEncUtf16strToUtf8str(const UH *utf16str,
		W utf16len, UB *utf8str, W utf8buflen);

#ifndef	FAT_ASCII_FN_ONLY

IMPORT	W	fatEncCP932ToUnicode(const UB cp932[], UW *unicode);
IMPORT	W	fatEncUnicodeToCP932(UW unicode, UB cp932[ENC_CHAR_MAX]);

Inline	W	fatEncUnicodeToLocal(UW unicode, UB local[ENC_CHAR_MAX])
{
	return fatEncUnicodeToCP932(unicode, local);
}

Inline	W	fatEncLocalToUnicode(const UB local[], UW *unicode)
{
	return fatEncCP932ToUnicode(local, unicode);
}
#endif	/* ~ FAT_ASCII_FN_ONLY */

#endif /* __FIMP_FAT_ENC_H__ */

