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
#include <vd_stdarg.h>
#include <libstr.h>
#include <sys/types.h>
#include <tk/typedef.h>

#include <debug/vdebug.h>

/*
================================================================================

	PROTOTYPE

================================================================================
*/
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

