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

#ifndef	__BK_UAPI_TERMIOS_H__
#define	__BK_UAPI_TERMIOS_H__


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
----------------------------------------------------------------------------------
	termios structures
----------------------------------------------------------------------------------
*/
typedef unsigned char	cc_t;
typedef unsigned int	speed_t;
typedef unsigned int	tcflag_t;

#define	NCCS		19

struct termios {
	tcflag_t	c_iflag;	/* input mode flags			*/
	tcflag_t	c_oflag;	/* output mode flags			*/
	tcflag_t	c_cflag;	/* control mode flags			*/
	tcflag_t	c_lflag;	/* local mode flags			*/
	cc_t		c_line;		/* line discipline			*/
	cc_t		c_cc[NCCS];	/* control characters			*/
};

struct termios2 {
	tcflag_t	c_iflag;	/* input mode flags			*/
	tcflag_t	c_oflag;	/* output mode flags			*/
	tcflag_t	c_cflag;	/* control mode flags			*/
	tcflag_t	c_lflag;	/* local mode flags			*/
	cc_t		c_line;		/* line discipline			*/
	cc_t		c_cc[NCCS];	/* control characters			*/
	speed_t		c_ispeed;	/* input speed				*/
	speed_t		c_ospeed;	/* output speed				*/
};

/*
----------------------------------------------------------------------------------
	c_iflag bits
----------------------------------------------------------------------------------
*/
#define	IGNBRK		000000001
#define	BRKINT		000000002
#define	IGNPAR		000000004
#define	PARMRK		000000010
#define	INPCK		000000020
#define	ISTRIP		000000040
#define	INLCR		000000100
#define	IGNCR		000000200
#define	ICRNL		000000400
#define	IUCLC		000001000
#define	IXON		000002000
#define	IXANY		000004000
#define	IXOFF		000010000
#define	IMAXBEL		000020000
#define	IUTF8		000040000

/*
----------------------------------------------------------------------------------
	c_oflag bits
----------------------------------------------------------------------------------
*/
#define	OPOST		000000001
#define	OLCUC		000000002
#define	ONLCR		000000004
#define	OCRNL		000000010
#define	ONOCR		000000020
#define	ONLRET		000000040
#define	OFILL		000000100
#define	OFDEL		000000200

#define	NLDLY		000000400
#define	NL0		000000000	// NLDLY bit
#define	NL1		000000400	// NLDLY bit

#define	CRDLY		000003000
#define	CR0		000000000	// CRDLY bit
#define	CR1		000001000	// CRDLY bit
#define	CR2		000002000	// CRDLY bit
#define	CR3		000003000	// CRDLY bit

#define	TABDLY		000014000
#define	TAB0		000000000	// TABDLY bit
#define	TAB1		000004000	// TABDLY bit
#define	TAB2		000010000	// TABDLY bit
#define	TAB3		000014000	// TABDLY bit
#define	XTABS		000014000	// TABDLY bit

#define	BSDLY		000020000
#define	BS0		000000000	// BSDLY bit
#define	BS1		000020000	// BSDLY bit

#define	VTDLY		000040000
#define	VL0		000000000	// VLDLY bit
#define	VL1		000040000	// VLDLY bit

#define	FFDLY		000100000
#define	FF0		000000000	// FFDLY bit
#define	FF1		000100000	// FFDLY bit

/*
----------------------------------------------------------------------------------
	c_cflag bits
----------------------------------------------------------------------------------
*/
#define	CBAUD		000010017
#define	B0		000000000	// CBAUD bit (hang up)
#define	B50		000000001	// CBAUD bit
#define	B75		000000002	// CBAUD bit
#define	B110		000000003	// CBAUD bit
#define	B134		000000004	// CBAUD bit
#define	B150		000000005	// CBAUD bit
#define	B200		000000006	// CBAUD bit
#define	B300		000000007	// CBAUD bit
#define	B600		000000010	// CBAUD bit
#define	B1200		000000011	// CBAUD bit
#define	B1800		000000012	// CBAUD bit
#define	B2400		000000013	// CBAUD bit
#define	B4800		000000014	// CBAUD bit
#define	B9600		000000015	// CBAUD bit
#define	B19200		000000016	// CBAUD bit
#define	B38400		000000017	// CBAUD bit
#define	EXTA		B19200
#define	EXTB		B38400

#define	CSIZE		000000060
#define	CS5		000000000	// CSIZE bit
#define	CS6		000000020	// CSIZE bit
#define	CS7		000000040	// CSIZE bit
#define	CS8		000000060	// CSIZE bit

#define	CSTOPB		000000100
#define	CREAD		000000200
#define	PARENB		000000400
#define	PARODD		000001000
#define	HUPCL		000002000
#define	CLOCAL		000004000

#define	CBAUDEX		000010000
#define	BOTHER		000010000	// CBAUDEX bit
#define	B57600		000010001	// CBAUDEX bit
#define	B115200		000010002	// CBAUDEX bit
#define	B230400		000010003	// CBAUDEX bit
#define	B460800		000010004	// CBAUDEX bit
#define	B500000		000010005	// CBAUDEX bit
#define	B576000		000010006	// CBAUDEX bit
#define	B921600		000010007	// CBAUDEX bit
#define	B1000000	000010010	// CBAUDEX bit
#define	B1152000	000010011	// CBAUDEX bit
#define	B1500000	000010012	// CBAUDEX bit
#define	B2000000	000010013	// CBAUDEX bit
#define	B2500000	000010014	// CBAUDEX bit
#define	B3000000	000010015	// CBAUDEX bit
#define	B3500000	000010016	// CBAUDEX bit
#define	B4000000	000010017	// CBAUDEX bit

#define	CIBAUD		002003600000	// input baud rate
#define	CMSPAR		010000000000	// mark or space (stick) parity
#define	CRTSCTS		020000000000	// flow control

#define	IBSHIFT		16		// shift from CBAUD to CIBAUD

/*
----------------------------------------------------------------------------------
	c_lflag bits
----------------------------------------------------------------------------------
*/
#define	ISIG		000000001
#define	ICANON		000000002
#define	ICASE		000000004
#define	ECHO		000000010
#define	ECHOE		000000020
#define	ECHOK		000000040
#define	ECHONL		000000100
#define	NOFLSH		000000200
#define	TOSTOP		000000400
#define	ECHOCTL		000001000
#define	ECHOPRT		000002000
#define	ECHOKE		000004000
#define	FLUSHO		000010000

#define	PENDIN		000040000
#define	IEXTEN		000100000
#define	EXTPROC		000200000


/*
----------------------------------------------------------------------------------
	c_cc characters
----------------------------------------------------------------------------------
*/
#define	VINTR		0
#define	VQUIT		1
#define	VERASE		2
#define	VKILL		3
#define	VEOF		4
#define	VTIME		5
#define	VMIN		6
#define	VSWTC		7
#define	VSTART		8
#define	VSTOP		9
#define	VSUSP		10
#define	VEOL		11
#define	VREPRINT	12
#define	VDISCARD	13
#define	VWERASE		14
#define	VLNEXT		15
#define	VEOL2		16

/*
----------------------------------------------------------------------------------
	for tcflow() and TCXONC
----------------------------------------------------------------------------------
*/
#define	TCOOFF		0
#define	TCOON		1
#define	TCIOFF		2
#define	TCION		3

/*
----------------------------------------------------------------------------------
	for tcflush() and TCFLSH
----------------------------------------------------------------------------------
*/
#define	TCIFLUSH	0
#define	TCOFLUSH	1
#define	TCIOFLUSH	2

/*
----------------------------------------------------------------------------------
	for tcsetattr() and TCFLSH
----------------------------------------------------------------------------------
*/
#define	TCSANOW		0
#define	TCSADRAIN	1
#define	TCSAFLUSH	2


/*
----------------------------------------------------------------------------------
	window size structures
----------------------------------------------------------------------------------
*/
struct winsize {
	unsigned short	ws_row;
	unsigned short	ws_col;
	unsigned short ws_xpixel;	// unused
	unsigned short ws_ypixel;	// unused
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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	// __BK_UAPI_TERMIOS_H__
