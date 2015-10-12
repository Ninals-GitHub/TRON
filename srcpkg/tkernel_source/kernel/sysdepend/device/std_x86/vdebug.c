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
 *	vdebug.c
 *	display debug information on x86 vga text mode
 */

#include <stdlib.h>

#include <typedef.h>
#include <stdarg.h>
#include <libstr.h>
#include <sys/types.h>
#include <tk/typedef.h>

#include <debug/vdebug.h>

/*
================================================================================

	PROTOTYPE

================================================================================
*/
LOCAL int vd_vsnprintf(char *buf, size_t len, const char *format, va_list ap );

LOCAL size_t NextPosition( uint32_t len );

/*
================================================================================

	DEFINE

================================================================================
*/
#define	VRAM		((uint16_t*)0xC00B8000)

#define	COLOR_SHIFT	8
#define	BACK_SHIFT	4

#define	DEFAULT_COLOR	( (VGA_BLACK << BACK_SHIFT) | (VGA_WHITE & 0xFF) )

/*
================================================================================

	Management 

================================================================================
*/
LOCAL uint32_t x_max;
LOCAL uint32_t y_max;

LOCAL uint32_t pos;

LOCAL uint16_t text_color = DEFAULT_COLOR << COLOR_SHIFT;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion		:initVdebug
	Input		:uint32_t set_xmax
			 < maximum of x axis >
			 uint32_t set_ymax
			 < maximum of y axis >
	Output		:void
	Return		:void
	Description	:initialize vdebug
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void initVdebug( uint32_t set_xmax, uint32_t set_ymax )
{
	x_max	= set_xmax;
	y_max	= set_ymax;
	
	pos	= 0;
	
	text_color = DEFAULT_COLOR << COLOR_SHIFT;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion		:SetColor
	Input		:uint8_t fore_color
			 < foreground color >
			 uint8_t back_color
			 < background color >
	Output		:void
	Return		:void
	Description	:set vga text mode color
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void SetColor( uint8_t fore_color, uint8_t back_color )
{
	text_color = back_color << BACK_SHIFT | ( fore_color & 0xFF );
	text_color <<= COLOR_SHIFT;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion		:vd_putc
	Input		:char c
			 < a character to put >
	Output		:void
	Return		:void
	Description	:put a character on screen
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void vd_putc( char c )
{
	uint16_t *vram = VRAM;
	
	if (c != '\n') {
		vram += pos;
	
		*vram = text_color | c;
		pos++;
		NextPosition( 1 );
	} else {
		int len;
		int new_line;
		
		new_line = x_max - pos % x_max;
		
		len = NextPosition(new_line);
		if ( !len ) {
			pos += new_line;
		}
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion		:vd_puts
	Input		:char *s
			 < character string to put >
	Output		:void
	Return		:void
	Description	:put character string on screen
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void vd_puts( char *s )
{
	size_t len;
	size_t cur	= strlen( s );
	uint16_t *vram	= VRAM;
	int new_line;
	
	do {
		len = NextPosition(cur);
		cur -= len;
		
		while (cur--) {
			while (*s == '\n') {
				int res_len;
				
				new_line = x_max - pos % x_max;
				res_len = NextPosition(new_line);
				if (!res_len) {
					pos += new_line;
				}
				s++;
				
				if (!cur) {
					goto next;
				}
			}
			if (*s) {
				*(vram + pos++) = text_color | *(s++);
			}
		}
next:
		cur = len;
	} while(len);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion		:vd_printf
	Input		:const char *format, ...
			 < printf like format >
	Output		:void
	Return		:void
	Description	:printf like debug information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int vd_printf( const char *format, ... )
{
	va_list	ap;
	size_t	len;
	uint8_t	buf[1024];
	
	va_start(ap, format);
	
	len = vd_vsnprintf(buf, sizeof(buf), format, ap );
	
	va_end(ap);
	
	vd_puts((char*)buf);
	
	return(len);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Funtion		:
	Input		:void
	Output		:void
	Return		:void
	Description	:
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
================================================================================
	Funtion		:NextPosition
	Input		:uint32_t len
			 < length of output >
	Output		:void
	Return		:void
	Description	:forward position
================================================================================
*/
LOCAL size_t NextPosition( uint32_t len )
{
	size_t res_len;
	size_t pos_line;
	size_t new_line;

	int i;
	uint16_t *vram;
	
	if ((pos + len) < (x_max * y_max)) {
		pos_line = pos / x_max;
		new_line = (pos + len) / x_max;
		if (pos_line < new_line) {
			/* clear the next line					*/
			vram = VRAM;
			for (i = 0;i < x_max;i++) {
				*(vram++ + new_line * x_max) = text_color | ' ';
			}
		}
		return 0;
	}
	
	res_len = pos - (x_max * y_max);
	
	pos = 0;

	/* clear the first line							*/
	vram = VRAM;
	for (i = 0;i < x_max;i++) {
		*(vram++) = text_color | ' ';
	}
	
	return(res_len);
}

/*
================================================================================
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
================================================================================
*/
LOCAL int vd_vsnprintf(char *buf, size_t len, const char *format, va_list ap )
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
			uint32_t ul_num;
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
