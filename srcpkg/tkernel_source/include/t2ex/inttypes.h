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
 *	@(#)inttypes.h
 *
 */

#ifndef _INTTYPES_H_
#define	_INTTYPES_H_

#include <basic.h>
#include <stdint.h>

typedef struct {
	intmax_t	quot;
	intmax_t	rem;
} imaxdiv_t;

#define PRId8	"d"
#define PRId16	"d"
#define PRId32	"d"
#define PRId64	"lld"

#define PRIdLEAST8	"d"
#define PRIdLEAST16	"d"
#define PRIdLEAST32	"d"
#define PRIdLEAST64	"lld"

#define PRIdFAST8	"d"
#define PRIdFAST16	"d"
#define PRIdFAST32	"d"
#define PRIdFAST64	"lld"

#define PRIdMAX	"jd"
#define PRIdPTR	"ld"

#define PRIi8	"i"
#define PRIi16	"i"
#define PRIi32	"i"
#define PRIi64	"lli"

#define PRIiLEAST8	"i"
#define PRIiLEAST16	"i"
#define PRIiLEAST32	"i"
#define PRIiLEAST64	"lli"

#define PRIiFAST8	"i"
#define PRIiFAST16	"i"
#define PRIiFAST32	"i"
#define PRIiFAST64	"lli"

#define PRIiMAX	"ji"
#define PRIiPTR	"li"

#define PRIo8	"o"
#define PRIo16	"o"
#define PRIo32	"o"
#define PRIo64	"llo"

#define PRIoLEAST8	"o"
#define PRIoLEAST16	"o"
#define PRIoLEAST32	"o"
#define PRIoLEAST64	"llo"

#define PRIoFAST8	"o"
#define PRIoFAST16	"o"
#define PRIoFAST32	"o"
#define PRIoFAST64	"llo"

#define PRIoMAX	"jo"
#define PRIoPTR	"lo"

#define PRIu8	"u"
#define PRIu16	"u"
#define PRIu32	"u"
#define PRIu64	"llu"

#define PRIuLEAST8	"u"
#define PRIuLEAST16	"u"
#define PRIuLEAST32	"u"
#define PRIuLEAST64	"llu"

#define PRIuFAST8	"u"
#define PRIuFAST16	"u"
#define PRIuFAST32	"u"
#define PRIuFAST64	"llu"

#define PRIuMAX	"ju"
#define PRIuPTR	"lu"

#define PRIx8	"x"
#define PRIx16	"x"
#define PRIx32	"x"
#define PRIx64	"llx"

#define PRIxLEAST8	"x"
#define PRIxLEAST16	"x"
#define PRIxLEAST32	"x"
#define PRIxLEAST64	"llx"

#define PRIxFAST8	"x"
#define PRIxFAST16	"x"
#define PRIxFAST32	"x"
#define PRIxFAST64	"llx"

#define PRIxMAX	"jx"
#define PRIxPTR	"lx"

#define PRIX8	"X"
#define PRIX16	"X"
#define PRIX32	"X"
#define PRIX64	"llX"

#define PRIXLEAST8	"X"
#define PRIXLEAST16	"X"
#define PRIXLEAST32	"X"
#define PRIXLEAST64	"llX"

#define PRIXFAST8	"X"
#define PRIXFAST16	"X"
#define PRIXFAST32	"X"
#define PRIXFAST64	"llX"

#define PRIXMAX	"jX"
#define PRIXPTR	"lX"

#define SCNd8	"hhd"
#define SCNd16	"hd"
#define SCNd32	"d"
#define SCNd64	"lld"

#define SCNdLEAST8	"hhd"
#define SCNdLEAST16	"hd"
#define SCNdLEAST32	"d"
#define SCNdLEAST64	"lld"

#define SCNdFAST8	"hhd"
#define SCNdFAST16	"hd"
#define SCNdFAST32	"d"
#define SCNdFAST64	"lld"

#define SCNdMAX	"jd"
#define SCNdPTR	"ld"

#define SCNi8	"hhi"
#define SCNi16	"hi"
#define SCNi32	"i"
#define SCNi64	"lli"

#define SCNiLEAST8	"hhi"
#define SCNiLEAST16	"hi"
#define SCNiLEAST32	"i"
#define SCNiLEAST64	"lli"

#define SCNiFAST8	"hhi"
#define SCNiFAST16	"hi"
#define SCNiFAST32	"i"
#define SCNiFAST64	"lli"

#define SCNiMAX	"ji"
#define SCNiPTR	"li"

#define SCNo8	"hho"
#define SCNo16	"ho"
#define SCNo32	"o"
#define SCNo64	"llo"

#define SCNoLEAST8	"hho"
#define SCNoLEAST16	"ho"
#define SCNoLEAST32	"o"
#define SCNoLEAST64	"llo"

#define SCNoFAST8	"hho"
#define SCNoFAST16	"ho"
#define SCNoFAST32	"o"
#define SCNoFAST64	"llo"

#define SCNoMAX	"jo"
#define SCNoPTR	"lo"

#define SCNu8	"hhu"
#define SCNu16	"hu"
#define SCNu32	"u"
#define SCNu64	"llu"

#define SCNuLEAST8	"hhu"
#define SCNuLEAST16	"hu"
#define SCNuLEAST32	"u"
#define SCNuLEAST64	"llu"

#define SCNuFAST8	"hhu"
#define SCNuFAST16	"hu"
#define SCNuFAST32	"u"
#define SCNuFAST64	"llu"

#define SCNuMAX	"ju"
#define SCNuPTR	"lu"

#define SCNx8	"hhx"
#define SCNx16	"hx"
#define SCNx32	"x"
#define SCNx64	"llx"

#define SCNxLEAST8	"hhx"
#define SCNxLEAST16	"hx"
#define SCNxLEAST32	"x"
#define SCNxLEAST64	"llx"

#define SCNxFAST8	"hhx"
#define SCNxFAST16	"hx"
#define SCNxFAST32	"x"
#define SCNxFAST64	"llx"

#define SCNxMAX	"jx"
#define SCNxPTR	"lx"

#ifdef __cplusplus
extern "C" {
#endif

IMPORT	intmax_t	imaxabs(intmax_t j);
IMPORT	imaxdiv_t	imaxdiv(intmax_t numer, intmax_t denom);
IMPORT	intmax_t	strtoimax(const char *nptr, char **endptr, int base);
IMPORT	uintmax_t	strtoumax(const char *nptr, char **endptr, int base);

#ifdef __cplusplus
}
#endif
#endif /* _INTTYPES_H_ */

