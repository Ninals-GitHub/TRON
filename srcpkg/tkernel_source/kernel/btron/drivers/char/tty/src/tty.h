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

#ifndef	__TTY_TTY_H__
#define	__TTY_TTY_H__

#include <stdint.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct termios2;
struct winsize;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	VRAM		((uint16_t*)0xC00B8000)

#define	COLOR_SHIFT	8
#define	BACK_SHIFT	4

#define	DEFAULT_COLOR	( (VGA_BLACK << BACK_SHIFT) | (VGA_WHITE & 0xFF) )

#define	X_MAX		80
#define	Y_MAX		25

#define	BACKSPACE	0x08
#define	ESCAPE		0x1B

#define	TTY_ARROW_UP	0
#define	TTY_ARROW_DOWN	1
#define	TTY_ARROW_RIGHT	2
#define	TTY_ARROW_LEFT	3

struct tty {
	struct termios2	*termios;
	struct winsize	*winsize;
	uint32_t	pos;
	uint32_t	pos_cur;
	uint32_t	saved_cur;
	uint32_t	line_start;
	uint16_t	text_color;
	uint16_t	fb[X_MAX * Y_MAX];
	uint32_t	report;
	uint32_t	aux:1;
	uint32_t	hide_cur:1;
	uint32_t	arrow_up:1;
	uint32_t	arrow_down:1;
	uint32_t	arrow_right:1;
	uint32_t	arrow_left:1;
};

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
 Funtion	:tty_notify_arrow
 Input		:int arrow
 		 < the type of pressed arrow button >
 Output		:void
 Return		:void
 Description	:notify the event which arrow up is pressed
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void tty_notify_arrow(int arrow);

#endif	// __TTY_TTY_H__
