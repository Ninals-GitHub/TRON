/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *
 *----------------------------------------------------------------------
 */

/*
 *	@(#)consio.h (sys)
 *
 *	Console I/O
 */

#ifndef	__SYS_CONSIO_H__
#define __SYS_CONSIO_H__

#include <basic.h>
#include <tk/typedef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/*
 *	Interface between application and buffer
 */

/* Default port definition */
#define CONSOLE_PORT	1	/* Debug console */
#define RS_PORT		2	/* Download serial port */

IMPORT int _GetChar( void );
IMPORT int _PutChar( int c );
IMPORT int _GetString( char *s );
IMPORT int _PutString( const char *s );
IMPORT int cons_ioctl( int req, int arg );

/* Serial port (RS_PORT) only */
IMPORT int RS_putchar( int c );
IMPORT int RS_getchar();
IMPORT int RS_ioctl( int req, int arg );

/*
 * cons_ioctl() request code
 */
#define GETCTL		0x100U	/* Get setting:
				   Get current settings (as function values)
				   via an OR with something other than INIT.
				   (GETCTL | ECHO), etc */

#define ECHO		1	/* Echo presence (1 = yes, 0 = no) */
#define INPUT		2	/* Input mode (RAW, etc.) */
#define NEWLINE 	3	/* When transmission, convert LF to CR, LF
				   (1 = convert, 0 = don't convert) */
#define FLOWC		4	/* Flow control (0 = no, IXON, etc.) */

#define SNDTMO		0x81U	/* Transmission timeout(milliseconds)
				   -1 = none */
#define RCVTMO		0x82U	/* Reception timeout (milliseconds)
				   -1 = none */

#define RCVBUFSZ	0x83U	/* Input buffer size: GET only */
#define SNDBUFSZ	0x84U	/* Output buffer size: GET only */

/* Input mode */
#define RAW		1	/* Raw input character by character */
#define CANONICAL	3	/* Single row input (CR converted to LF) */
#define EDIT		5	/* Single row editing input */

/* Flow control */
#define IXON		0x01U	/* XON/XOFF output flow control */
#define IXANY		0x02U	/* Output restarted if any character
				   received during IXON */
#define IXOFF		0x04U	/* XON/XOFF input flow control */

/* ------------------------------------------------------------------------ */
/*
 *	Interface between console and buffer
 */

IMPORT W cons_put(W port, B *buf, UW len, W tmout);
IMPORT W cons_get(W port, B *buf, UW len, W tmout);
IMPORT W cons_conf(W req, UW *arg);

/*
 * cons_conf() request code
 */
#define CS_CREATE	0x11U	/* Generate console port		*/
				/* arg[0] = port number		    OUT	*/
				/* arg[1] = configuration	    IN	*/
				/* arg[2] = input buffer size	    IN	*/
				/* arg[3] = output buffer size	    IN	*/

#define CS_DELETE	0x12U	/* Delete console port			*/
				/* arg[0] = port number		    IN	*/

#define CS_SETCONF	0x13U	/* Console structure setting		*/
				/* arg[0] = port number		    IN	*/
				/* arg[1] = configuration	    IN	*/
				/* arg[2] = input buffer size	    IN	*/
				/* arg[3] = output buffer size      IN	*/

#define CS_GETCONF	0x14U	/* Get console structure		*/
				/* arg[0] = port number		    IN	*/
				/* arg[1] = configuration	    OUT	*/
				/* arg[2] = input buffer size	    OUT	*/
				/* arg[3] = output buffer size	    OUT */

#define CS_GETPORT	0x21U	/* Get standard console port		*/
				/* arg[0] = port number		    OUT	*/

#define CS_SETPORT	0x22U	/* Standard console port setting	*/
				/* arg[0] = port number		    IN	*/

#define CS_SRCHPORT	0x23U	/* Console port search			*/
				/* arg[0] = port number		 IN/OUT	*/
				/* arg[1] = configuration	    IN	*/

/* Configuration */
#define CONF_SERIAL_0	(0)	/* Serial port #0			*/
#define CONF_SERIAL(n)	(n)	/* Serial port #N			*/
#define CONF_SELF	(-1)	/* Self console				*/
#define CONF_BUFIO	(-2)	/* Buffer IO				*/

/* ------------------------------------------------------------------------ */

/*
 * Definitions for interface library auto generation (mkiflib)
 */
/*** DEFINE_IFLIB
[INCLUDE FILE]
<sys/consio.h>

[PREFIX]
CONSIO
***/

/* [BEGIN SYSCALLS] */
/* Input/output between console and buffer */
/* ALIGN_NO 0x10 */
IMPORT ER console_get(W port, B *buf, UW len, W tmout);
IMPORT ER console_put(W port, B *buf, UW len, W tmout);
IMPORT ER console_conf(W req, UW *arg);

/* Input/output between application and buffer */
/* ALIGN_NO 0x10 */
IMPORT ER console_in(W port, B *buf, UW len);
IMPORT ER console_out(W port, B *buf, UW len);
IMPORT ER console_ctl(W port, W req, W arg);
/* [END SYSCALLS] */

#ifdef __cplusplus
}
#endif
#endif /* __SYS_CONSIO_H__ */
