/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
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

#include <stdarg.h>
#include <libstr.h>
#include <tk/typedef.h>

#include <debug/vdebug.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/

/*
==================================================================================

	Management 

==================================================================================
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion		:vd_vsnprintf
	Input		:char *buf
			 < output buffer >
			 size_t len
			 < maximum size of the buffer >
			 const char *format
			 < printf like format >
			 va_list ap
			 < arguments >
	Output		:char *buf
			 < output buffer >
	Return		:int
	Description	:simple vsnprintf operation
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int vd_vsnprintf(char *buf, size_t len, const char *format, va_list ap )
{
	int	ret = 0;
	int	zeroflag = 0;
	int	sign = 0;
	
	enum LEN_FIELD {
		UN_LEN,		// unspecified
		L_LEN,		// 'l' long
		LL_LEN,		// 'll' long long
		Z_LEN,		// 'z' sized integer
	};

	int		width;
	enum LEN_FIELD	lenf;
	
	if (!len) {
		return(ret);
	}
	
	while (*format) {
		/* ------------------------------------------------------------ */
		/* normal string						*/
		/* ------------------------------------------------------------ */
		if (*format != '%') {
			*(buf++) = *(format++);
			ret++;
			if ((len - 1) < ret) {
				break;
			}
			continue;
		}
		/* ------------------------------------------------------------ */
		/* % character							*/
		/* ------------------------------------------------------------ */
		if (!(*(++format))) {
			/* null termination					*/
			break;
		}
		/* ------------------------------------------------------------ */
		/* zero pad flag						*/
		/* ------------------------------------------------------------ */
		zeroflag = 0;
		if (*format == '0' ) {
			zeroflag = 1;
			format++;
			if (!*format) {
				/* null termination				*/
				break;
			}
		}
		/* ------------------------------------------------------------ */
		/* width							*/
		/* ------------------------------------------------------------ */
		width = 0;
		
		while (('0' <= *format) && (*format <= '9')) {
			width = width * 10 + ( *(format++) - '0' );
			if (!*format) {
				/* null termination				*/
				break;
			}
		}
		
		if (!*format) {
			/* null termination					*/
			break;
		}
		
		/* ------------------------------------------------------------ */
		/* length							*/
		/* ------------------------------------------------------------ */
		switch (*format) {
		case	'l':
			lenf = L_LEN;
			break;
		case	'z':
			lenf = Z_LEN;
			break;
		default:
			lenf = UN_LEN;
			break;
		}
		
		if (*format == 'l') {
			if (!*(++format)) {
				/* null termination				*/
				break;
			}
			if (*format == 'l') {
				lenf = LL_LEN;
			}
		}
		
		/* ------------------------------------------------------------ */
		/* type:%s							*/
		/* ------------------------------------------------------------ */
		if (*format == 's') {
			char *cp;
			size_t slen;
			int overrun = 0;
			
			cp = va_arg(ap, char* );
			slen = strlen(cp);
			
			if (slen < width) {
				int spacepad = width - slen;
				while (spacepad--) {
					*(buf++) = ' ';
					ret++;
					if (len - 1 < ret) {
						overrun = 1;
						break;
					}
				}
			}
			
			if(overrun) {
				break;
			}
			
			while (slen--) {
				*(buf++) = *(cp++);
				ret++;
				if (len - 1 < ret) {
					overrun = 1;
					break;
				}
			}
			
			/* buffer overrun					*/
			if (overrun) {
				break;
			}
			
			format++;
			continue;
		}
		
		/* ------------------------------------------------------------ */
		/* type:%c							*/
		/* ------------------------------------------------------------ */
		if (*format == 'c') {
			int code;
			
			code = va_arg(ap, int);
			
			*(buf++) = (char)code;
			ret++;
			format++;
			continue;
		}
		
		
		/* ------------------------------------------------------------ */
		/* type:%d, %i							*/
		/* ------------------------------------------------------------ */
		if (*format == 'd' || *format == 'i' ||
			*format == 'u' || *format == 'z' ||
			*format == 'x' || *format == 'X' )
		{
			uint64_t ull_num;
			uint32_t ul_num = 0;
			char minus = '\0';
			int rs_len;
			int pad_len;
			int i;
			int copy_len;
			int overrun = 0;
			char cnv[24];
			char pad;
			char *cp;
			
			char xdigs[] = {'0', '1', '2', '3', '4', '5', '6', '7',
					'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
			char sxdigs[] = {'0', '1', '2', '3', '4', '5', '6', '7',
					'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
			
			/* ---------------------------------------------------- */
			/* unsigned						*/
			/* ---------------------------------------------------- */
			sign = 1;
			if (*format == 'u' || *format == 'x' || *format == 'X') {
				sign = 0;
			}
			
			if (*format == 'z') {
				if (*(format + 1) && *(format + 1) == 'u') {
					format++;
				}
			}
			
			if (zeroflag) {
				pad = '0';
			} else {
				pad = ' ';
			}
			
			for (i = 0 ; i < sizeof(cnv) ; i++) {
				cnv[i] = pad;
			}

			cnv[sizeof(cnv) - 1] = '\0';
			cp = cnv + sizeof(cnv) - 1;
			
			switch (lenf) {
			case	LL_LEN:
				ull_num = va_arg(ap, uint64_t);
				if (sign) {
					if (ull_num & (1ULL << (64 - 1))) {
						minus = '-';
						ull_num = ~ull_num + 1;
					}
				}
				break;
			case	UN_LEN:
			case	L_LEN:
			case	Z_LEN:
			default:
				ul_num = va_arg(ap, uint32_t);
				if (sign) {
					if( ul_num & (1UL << (32 - 1))) {
						minus = '-';
						ul_num = ~ul_num + 1;
					}
				}
				break;
			}
			
			
			switch (*format) {
			case	'd':
			case	'i':
			case	'u':
			case	'z':
				while (ul_num >= 10) {
					*(--cp) = ( ul_num % 10 ) + 0x30;
					ul_num /= 10;
				}
				*(--cp) = ul_num + 0x30;
				if (minus == '-') {
					*(--cp) = minus;
				}
				break;
			case	'x':
			case	'X':
				do {
					if (*format == 'x') {
						*(--cp) = sxdigs[ul_num & 0xF];
					} else {
						*(--cp) = xdigs[ul_num & 0xF];
					}
					ul_num = ul_num >> 4;
				} while (ul_num);
				break;
			default:
				break;
			}
			
			rs_len = (cnv + sizeof(cnv) -1) - cp;
			
			if (rs_len < width) {
				pad_len = width - rs_len;
				cp -= pad_len;
				
				copy_len = width;
				
			} else {
				copy_len = rs_len;
			}
			
			while (copy_len--) {
				*(buf++) = *(cp++);
				ret++;
				if(len - 1 < ret) {
					overrun = 1;
					break;
				}
			}
			
			if (overrun) {
				break;
			}
			
			format++;
			continue;
		}
		
		/* % escape							*/
		*(buf++) = *(format++);
		ret++;
		if (len - 1 < ret) {
			break;
		}
	}
	
	*buf = '\0';
	
	return(ret);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
