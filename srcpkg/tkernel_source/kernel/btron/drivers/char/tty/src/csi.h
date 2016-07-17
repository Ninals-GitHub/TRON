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

#ifndef	__TTY_CSI_H__
#define	__TTY_CSI_H__

#include <typedef.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct tty;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	CSI code
	
	CSI n A		CUU - Cursor Up
	CSI n B		CUD - Cursor Down
	CSI n C		CUF - Cursor Forward
	CSI n D		CUB - Cursor back
			"->moves the cursor n [default 1] cells in the given
			 direction. if the cursor is already at the edge of
			 the screen, this has no effect"
	CSI n E		CNL - Cursor Next Line
			"->moves cursor to begining of the line n [default 1]
			lines down"
	CSI n F		CPL - Cursor Previous Line
			"->moves cursor to begining of the line n [default 1]
			lines up"
	CSI n G		CHA - Cursor Horizontal Absolute
			"->moves the cursor to column n [default 1]"
	CSI n ; m H	CUP - Cursor Position
			"->moves the cursor to row n, column m. the values are
			1-based, and default to 1 [1 is indicated to top lef
			corner] if omitted. a sequence such a CSI ;5H is a
			synonym for CSI 1;5H as well as CSI 17;H is the save
			as CSI 17H and CSI 17;1H"
	CSI n J		ED  - Erase Display
			"->clears part of the screen. if n is 0 or missing,
			 clear from cursor to end of screen. if n is 1, clear
			 from cursor to begining of the screen. if n is 2,
			 clear entire screen"
	CSI n K		EL  - Erase in Line
			"->eraces part of the line. if n is 0 or missing, clear
			from cursor to the end of the line. if n is 1, clear
			from cursor to begining of the line. if n is 2, clear
			entire line. cursor position does not change."
	CSI n S		SU  - Scroll Up
			"->scroll whole page up by n [default 1] lines. new
			lines are added at the bottom"
	CSI n T		SD  - Scroll Down
			"->scroll whole page down by n [default 1] lines. new
			lines added at the top"
	CSI n ; m f	HVP - Horizontal and Vertical Position
			"->moves the cursor to row n, colomn m. both default to
			 1 if omitted. save as CUP."
	CSI n m		SGR - Select Graphic Rendition
			"->sets SGR parameters, including text color. after CSI
			can be zero or more parametes separated with ;. with no
			parameters, CSI m is treated as CSI 0 m [reset/normal],
			which is typical of most of the ANSI escape sequences.
	CSI 5i		5i  - AUX Port On
			"->enables aux serial port usually for local serial
			printer"
	CSI 4i		4i  - AUX Port Off
			"->disables aux serial port usually for local serial
			printer"
	CSI 6n		DSR - Device Status Report
			"->reports the cursor position CPR to the application as
			[as though typed at the keyboard] ESC [ n ; m R, where
			n is the row and m is the column."
	CSI s		SCP - Save Cursor Position
			"->saves the cursor position"
	CSI u		RCP - Restore Cursor Position
			"->restore the cursor position"
	CSI ?25l	DECTCEM - Hides the cursor
			"->hides the cursor"
	CSI ?25h	DECTCEM - Shows the cursor
			"->shows the cursor"
----------------------------------------------------------------------------------
*/
#define	CSI_CUU		'A'
#define	CSI_CUD		'B'
#define	CSI_CUF		'C'
#define	CSI_CUB		'D'
#define	CSI_CNL		'E'
#define	CSI_CPL		'F'
#define	CSI_CHA		'G'
#define	CSI_CUP		'H'
#define	CSI_ED		'J'
#define	CSI_EL		'K'
#define	CSI_SU		'S'
#define	CSI_SD		'T'
#define	CSI_HVP		'f'
#define	CSI_SGR		'm'
#define	CSI_5I		'5'
#define	CSI_4I		'4'
#define	CSI_6N		'6'
#define	CSI_SCP		's'
#define	CSI_RCP		'u'
#define	CSI_HIDE_CUR	'l'
#define	CSI_SHOW_CUR	'h'

enum csi_analysis{
	CSI_NONE,
	MAY_CSI_ESC,		// escape is done
	CSI_ESC_BR,		// [ is done
	MAY_CSI_N,		// n is being analyzed
	CSI_N_4,		// n is 4
	CSI_N_5,		// n is 5
	CSI_N_6,		// n is 6
	CSI_SEMI,		// ; is done
	MAY_CSI_M,		// m is being analyzed
	CSI_Q,			// ? is done
	NR_CSI_STATE
};

struct csi_state {
	enum csi_analysis	state;
	int			n;
	int			m;
};

/*
----------------------------------------------------------------------------------
	SGR [select graphic rendition] parameters
	
	code	effect			note
	0	reset/normal		all attributes off
	1	bold or increased
		intensity
	2	faint[decreased		not widely supported
		intensity]
	3	itallic: on		not widely supported. sometimes treated
					as inverse
	4	underline: single
	5	blink: slow		less than 150 per minute
	6	blink: rapid		ms-dos ansi.sys; 150+ per minute; not
					widely supported
	7	image: negative		inverse or reverse; swap foreground and
					background
	8	conceal			not widely supported
	9	crossed-out		characters legible, but marked for
					deletion. not widely supported
	10	primary(default)
		font
	11-19	n-th alternate font	select the n-th alternate font(14 being
					the fourth alternate font, up to 19 being
					the 9th alternate font)
	20	fraktur			hardly ever supported
	21	bold: off or
		underline: double	bold off not widely suppported; double
					underline hardly ever supported
	22	normal color or		neither bold nor faint
		intensity
	23	not italic,
		not fraktur
	24	underline: none		not singly or doubly underlined
	25	blink: off
	26	reserved
	27	image: positive
	28	reveal			conceal off
	29	not crossed out
	30-37	set text color		30+n, where n is from the color table
		(foreground)
	38	reserved for		typical supported next arguments are 5;n
		extended set		where n is color index(0-255) or 2;r;g;b
		foreground color	where r,g,b are red, green and blue color
					channels(out of 255)
	39	default text color	implementation defined
	40-47	set background color	40+n, where n is from the color table
	48	reserved for extended	see the note of code 38
		set background color
	49	default backgournd
		color
	50	reserved
	51	framed
	52	encircled
	53	overlined
	54	not framed or encircled
	55	not overlined
	56-59	reserved
	60	ideogram underline or	hardly ever supported
		right side line
	61	ideogram double under-	hardly ever supported
		line or double on the
		right side
	62	ideogram overline or	hardly ever supported
		left side line
	63	ideogram double over-	hardly ever supported
		line or double line
		on the left side
	64	ideogram stress marking	hardly ever supported
	65	ideogram attributes off	hardly ever supported, reset the effects
					of all of 60-64
	90-97	set foreground text	aixterm [not in standard]
		color, high intensity
	100-107	set backgound color,	aixterm [not in standard]
		high intensity
----------------------------------------------------------------------------------
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
 Funtion	:analyze_csi
 Input		:const char *buf
 		 < user buffer to write >
 		 struct csi_state *state
 		 < csi analysis state >
 		 struct tty *tty
 		 < tty management >
 Output		:void
 Return		:int
 		 < result : 0 non csi ;1 cis, try next sequence >
 Description	:anayalyze csi
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int
analyze_csi(const char *buf, struct csi_state *state, struct tty *tty);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:read_csi_report
 Input		:char *buf
 		 < user buffer to output >
 		 size_t len
 		 < length of the buffer >
 		 struct tty *tty
 		 < tty management >
 Output		:char *buf
 		 < user buffer to output >
 Return		:size_t
 		 < atucal read length >
 Description	:read tty report
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT size_t read_csi_report(char *buf, size_t len, struct tty *tty);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:read_csi_cursor
 Input		:struct file *filp
 		 < open file >
 		 char *buf
 		 < user buffer to output >
 		 size_t len
 		 < length of the buffer >
 		 struct tty *tty
 		 < tty management >
 Output		:char *buf
 		 < user buffer to output >
 Return		:size_t
 		 < atucal read length >
 Description	:read tty report
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT size_t
read_csi_cursor(struct file *filp, char *buf, size_t len, struct tty *tty);

#endif	// __TTY_CSI_H__
