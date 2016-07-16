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

#include <bk/uapi/ioctl.h>
#include <bk/uapi/ioctl_tty.h>
#include <bk/uapi/termios.h>

#include "csi.h"
#include "tty.h"

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int
analyze_csi_esc_br(const char *buf, struct csi_state *state, struct tty *tty);
LOCAL int
analyze_may_csi_n(const char *buf, struct csi_state *state, struct tty *tty);
LOCAL int
analyze_semi(const char *buf, struct csi_state *state, struct tty *tty);
LOCAL int
analyze_q(const char *buf, struct csi_state *state, struct tty *tty);
LOCAL int analyze_done(const char *buf, struct csi_state *state, struct tty *tty);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	CSI_REPORT_ROW	100
#define	CSI_REPORT_SEMI	200
#define	CSI_REPORT_COL	300

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL int (*csi_func[NR_CSI_STATE])(const char *buf,
					struct csi_state *state,
					struct tty *tty) = {
	[CSI_NONE]	= analyze_csi_esc_br,	// not used
	[MAY_CSI_ESC]	= analyze_csi_esc_br,	// not used
	[CSI_ESC_BR]	= analyze_csi_esc_br,
	[MAY_CSI_N]	= analyze_may_csi_n,
	[CSI_N_4]	= analyze_may_csi_n,
	[CSI_N_5]	= analyze_may_csi_n,
	[CSI_N_6]	= analyze_may_csi_n,
	[CSI_SEMI]	= analyze_semi,
	[MAY_CSI_M]	= analyze_semi,
	[CSI_Q]		= analyze_q,
};

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
EXPORT int
analyze_csi(const char *buf, struct csi_state *state, struct tty *tty)
{
	if ((*buf == ESCAPE) && (state->state == CSI_NONE)) {
		state->state = MAY_CSI_ESC;
		return(1);
	}
	
	if ((*buf == '[') && (state->state == MAY_CSI_ESC)) {
		state->state = CSI_ESC_BR;
		state->n = 0;
		state->m = 0;
		return(1);
	}
	
	if (state->state < CSI_ESC_BR) {
		return(0);
	} else if (UNLIKELY(NR_CSI_STATE <= state->state)) {
		state->state = CSI_NONE;
		return(0);
	}
	
	return(csi_func[state->state](buf, state, tty));
}

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
EXPORT size_t read_csi_report(char *buf, size_t len, struct tty *tty)
{
	int col;
	int row;
	
	switch (tty->report) {
	case	1:
		tty->report++;
		*buf = ESCAPE;
		return(1);
	case	2:
		tty->report++;
		*buf = '[';
		return(1);
	case	3:
		tty->report = CSI_REPORT_ROW;
		break;
	case	CSI_REPORT_SEMI:
		tty->report = CSI_REPORT_COL;
		break;
	default:
		break;
	}
	
	if (CSI_REPORT_ROW <= tty->report && tty->report < CSI_REPORT_SEMI) {
		int i = 0;
		char char_row[16];
		row  = tty->pos_cur /  tty->winsize->ws_col;
		row += 1;
		
		while (row) {
			int _row = row / 10;
			
			char_row[i] = row - (_row * 10) + '0';
			row = _row;
			
			i++;
		}
		
		if (UNLIKELY(i <= (tty->report - CSI_REPORT_ROW))) {
			tty->report = CSI_REPORT_SEMI;
			*buf = ';';
			//printf(";");
		} else {
			*buf = char_row[i - (tty->report - CSI_REPORT_ROW) - 1];
			tty->report++;
			//printf("%c", *buf);
		}
		
		return(1);
	}
	
	if (CSI_REPORT_COL <= tty->report) {
		int i = 0;
		char char_col[16];
		col = tty->pos_cur % tty->winsize->ws_col;
		col += 1;
		
		while (col) {
			int _col = col / 10;
			
			char_col[i] = col - (_col * 10) + '0';
			col = _col;
			
			i++;
		}
		
		if (UNLIKELY(i <= (tty->report - CSI_REPORT_COL))) {
			tty->report = 0;
			*buf = 'R';
			//printf("\n");
		} else {
			*buf = char_col[i - (tty->report - CSI_REPORT_COL) - 1];
			tty->report++;
			//printf("%c", *buf);
		}
		
		return(1);
	}
	
	return(0);
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
 Funtion	:analyze_csi_esc_br
 Input		:const char *buf
 		 < user buffer to write >
 		 struct csi_state *state
 		 < csi analysis state >
 		 struct tty *tty
 		 < tty management >
 Output		:void
 Return		:int
 		 < result >
 Description	:analyze csi after escape and bracket
==================================================================================
*/
LOCAL int
analyze_csi_esc_br(const char *buf, struct csi_state *state, struct tty *tty)
{
	if (UNLIKELY(*buf == 's')) {
		tty->saved_cur = tty->pos_cur;
		return(1);
	}
	
	if (UNLIKELY(*buf == 'u')) {
		tty->pos_cur = tty->saved_cur;
		return(1);
	}
	
	if (UNLIKELY(*buf == '?')) {
		state->state = CSI_Q;
		return(1);
	}
	
	if (UNLIKELY(*buf == ';')) {
		state->state = CSI_SEMI;
		state->n = 1;
		return(1);
	}
	
	if (('0' <= *buf) && (*buf <= '9')) {
		if (*buf == '4') {
			state->state = CSI_N_4;
		} else if (*buf == '5') {
			state->state = CSI_N_5;
		} else if (*buf == '6') {
			state->state = CSI_N_6;
		} else {
			state->state = MAY_CSI_N;
		}
		
		state->n = *buf - '0';
		
		return(1);
	}
	
	return(0);
}

/*
==================================================================================
 Funtion	:analyze_may_csi_n
 Input		:const char *buf
 		 < user buffer to write >
 		 struct csi_state *state
 		 < csi analysis state >
 		 struct tty *tty
 		 < tty management >
 Output		:void
 Return		:int
 		 < result >
 Description	:analyze csi after numeric ascii code
==================================================================================
*/
LOCAL int
analyze_may_csi_n(const char *buf, struct csi_state *state, struct tty *tty)
{
	if (state->state == CSI_N_4) {
		if (*buf == 'i') {
			state->state = CSI_NONE;
			tty->aux = 0;
			return(1);
		}
		
		state->state = MAY_CSI_N;
		/* through */
	} else if (state->state == CSI_N_5) {
		if (*buf == 'i') {
			state->state = CSI_NONE;
			tty->aux = 1;
			return(1);
		}
		
		state->state = MAY_CSI_N;
		/* through */
	} else if (state->state == CSI_N_6) {
		if (*buf == 'n') {
			state->state = CSI_NONE;
			tty->report = 1;
			return(1);
		}
		
		state->state = MAY_CSI_N;
		/* through */
	}
	
	if (UNLIKELY(*buf == ';')) {
		state->state = MAY_CSI_M;
		return(1);
	}
	
	if (UNLIKELY('A' <= *buf && *buf <= 'm')) {
		int err = analyze_done(buf, state, tty);
		
		return(err);
	}
	
	if ('0' <= *buf && *buf <= '9') {
		state->n = state->n * 10 + *buf - '0';
		
		return(1);
	}
	
	state->state = CSI_NONE;
	state->n = 0;
	state->m = 0;
	
	return(0);
}

/*
==================================================================================
 Funtion	:analyze_semi
 Input		:const char *buf
 		 < user buffer to write >
 		 struct csi_state *state
 		 < csi analysis state >
 		 struct tty *tty
 		 < tty management >
 Output		:void
 Return		:int
 		 < result >
 Description	:analyze csi after semicolon
==================================================================================
*/
LOCAL int
analyze_semi(const char *buf, struct csi_state *state, struct tty *tty)
{
	if (UNLIKELY('A' <= *buf && *buf <= 'm')) {
		int err = analyze_done(buf, state, tty);
		
		return(err);
	}
	
	if ('0' <= *buf && *buf <= '9') {
		state->m = state->m * 10 + *buf - '0';
		
		return(1);
	}
	
	state->state = CSI_NONE;
	state->n = 0;
	state->m = 0;
	
	return(0);
}

/*
==================================================================================
 Funtion	:analyze_q
 Input		:const char *buf
 		 < user buffer to write >
 		 struct csi_state *state
 		 < csi analysis state >
 		 struct tty *tty
 		 < tty management >
 Output		:void
 Return		:int
 		 < result >
 Description	:analyze csi after question mark
==================================================================================
*/
LOCAL int
analyze_q(const char *buf, struct csi_state *state, struct tty *tty)
{
	if (UNLIKELY(*buf =='l' || *buf == 'h')) {
		int err = analyze_done(buf, state, tty);
		
		return(err);
	}
	
	if ('2' == *buf && *buf == '5') {
		return(1);
	}
	
	state->state = CSI_NONE;
	state->n = 0;
	state->m = 0;
	
	return(0);
}

/*
==================================================================================
 Funtion	:analyze_done
 Input		:const char *buf
 		 < user buffer to write >
 		 struct csi_state *state
 		 < csi analysis state >
 		 struct tty *tty
 		 < tty management >
 Output		:void
 Return		:int
 		 < result : 0 is not done 1 is done >
 Description	:analysis of csi is done
==================================================================================
*/
LOCAL int analyze_done(const char *buf, struct csi_state *state, struct tty *tty)
{
	int n, m;
	switch (*buf) {
	case	'A':
		break;
	case	'B':
		break;
	case	'C':
		break;
	case	'D':
		break;
	case	'E':
		break;
	case	'F':
		break;
	case	'G':
		break;
	case	'H':
		if (state->n) {
			n = state->n - 1;
		} else {
			n = 0;
		}
		if (state->m) {
			m = state->m - 1;
		} else {
			m = 0;
		}
		tty->pos_cur = n * tty->winsize->ws_col + m;
		goto out;
	case	'J':
		break;
	case	'K':
		break;
	case	'S':
		break;
	case	'T':
		break;
	case	'f':
		break;
	case	'h':
		tty->hide_cur = 0;
		goto out;
	case	'm':
		break;
	case	'l':
		tty->hide_cur = 1;
		goto out;
	default:
		state->state = CSI_NONE;
		state->n = 0;
		state->m = 0;
	}
	
	return(0);

out:
	state->state = CSI_NONE;
	state->n = 0;
	state->m = 0;
	
	return(1);
}
