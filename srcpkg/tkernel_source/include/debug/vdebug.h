/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 */

/*
 *	vdebug.h (x86)
 *	print debug information
 */

#ifndef _VDEBUG_
#define _VDEBUG_

#include <cpu/x86/cpu_insn.h>

/*
================================================================================

	Prototype Statements

================================================================================
*/

/*
================================================================================

	DEFINES

================================================================================
*/
#define	VGA_BLACK		0x00
#define	VGA_BLUE		0x01
#define	VGA_GREEN		0x02
#define	VGA_CYAN		0x03
#define	VGA_RED			0x04
#define	VGA_MAGENTA		0x05
#define	VGA_BROWN		0x06
#define	VGA_LIGHTGRAY		0x07
#define	VGA_DARKGRAY		0x08
#define	VGA_LIGHTBLUE		0x09
#define	VGA_LIGHTGREEN		0x0A
#define	VGA_LIGHTCYAN		0x0B
#define	VGA_LIGHTRED		0x0C
#define	VGA_LIGHTMAGENTA	0x0D
#define	VGA_YELLO		0x0E
#define	VGA_WHITE		0x0F


/*
================================================================================

	Management 

================================================================================
*/

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
IMPORT void initVdebug( uint32_t set_xmax, uint32_t set_ymax );

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
IMPORT void SetColor( uint8_t fore_color, uint8_t back_color );

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
IMPORT void vd_putc( char c );

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
IMPORT void vd_puts( char *s );

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
IMPORT int vd_printf( const char *format, ... );

#endif /* _VDEBUG_ */
