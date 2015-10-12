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

#ifndef	__VGA_H__
#define	__VGA_H__

#include <tk/typedef.h>
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
 Funtion	:initVga
 Input		:void
 Output		:void
 Return		:void
 Description	:initialize vga text mode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void initVga(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:startup_vga
 Input		:void
 Output		:void
 Return		:ER
		 < error code >
 Description	:create vga task and start up him
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER startup_vga(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vga_out
 Input		:B *buf
		 < buffer to output >
		 UW len
		 < output length >
 Output		:void
 Return		:ER
		 < actual output length or error code >
 Description	:output to vga fb
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER vga_out(B *buf, UW len);

#endif	// __VGA_H__
