/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
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

/*
 *	@(#)fcntl.h
 *
 */
#ifndef __SYS_FCNTL_H__
#define __SYS_FCNTL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
   open() access mode
 */
/* open-only flags */
#define O_RDONLY    0x00000000U	/* Read only */
#define O_WRONLY    0x00000001U	/* Write only */
#define O_RDWR	    0x00000002U	/* Read and write */
#define O_ACCMODE   0x00000003U	/* mask for above modes */

/*
 * Kernel encoding of open mode; separate read and write bits that are
 * independently testable: 1 greater than the above.
 *
 * FREAD and FWRITE are excluded from the #ifdef _KERNEL so that TIOCFLUSH,
 * which was documented to use FREAD/FWRITE, continues to work.
 */
#define O_NONBLOCK  0x00000004U	/* no delay */
#define O_SYNC	    0x00000080U	/* synchronous writes */
#define O_DSYNC     0x00001000U	/* write: I/O data completion */
#define O_RSYNC     0x00002000U	/* read: I/O completion as for write */

/* defined by POSIX 1003.1; BSD default, but required to be bitwise distinct */
#define O_NOCTTY    0x00008000U	/* don't assign controlling terminal */

/* all bits settable during open(2) */
#define O_MASK	    (O_ACCMODE | O_NONBLOCK | O_APPEND \
		     | O_SYNC | O_CREAT | O_TRUNC | O_EXCL \
		     | O_DSYNC | O_RSYNC | O_NOCTTY | O_DIRECTORY)

/* open() option */
#define O_CREAT     0x00000200U	/* Generate file if there is none */
#define O_TRUNC     0x00000400U	/* Delete file content */
#define O_EXCL	    0x00000800U	/* If there is file, return error. */
#define O_APPEND    0x00000008U	/* Always add to the end */
#define O_DIRECTORY 0x00200000U	/* If the file is not directory, return error */

/*
 * Constants used for fcntl()
 */
/* fcntl command structure: (same as ioctl)
    bits 29-31	direction
    bits 16-28	parameter length
    bits  8-15	cmd group
    bits  0-7	command
 */
#define	FCTLPARM_MASK	0x1fff		/* parameter length */
#define	FCTLPARM_SHIFT	16
#define	FCTLGROUP_MASK	0xff
#define	FCTLGROUP_SHIFT	8
#define	FCTLCMD_MASK	0xff
#define	FCTLCMD_SHIFT	0

#define	FCTL_VOID	0x20000000UL		/* no params */
#define	FCTL_OUT	0x40000000UL		/* out params (read) */
#define	FCTL_IN		0x80000000UL		/* in params (write) */
#define	FCTL_INOUT	(FCTL_IN|FCTL_OUT)	/* in/out params */
#define	FCTL_DIRMASK	0xe0000000UL

/* Generic macro to form fcntl command */
#define _FCTL_(dir, grp, cmd, len)	( (dir) | \
			(((len) & FCTLPARM_MASK) << FCTLPARM_SHIFT) | \
			(((grp) & FCTLGROUP_MASK) << FCTLGROUP_SHIFT) | \
			(((cmd) & FCTLCMD_MASK) << FCTLCMD_SHIFT) )

/* Useful macros to form directed commands */
#define _FCTL(g, c)		_FCTL_(FCTL_VOID,  (g), (c), 0)
#define _FCTLR(g, c)		_FCTL_(FCTL_OUT,   (g), (c), 0)
#define _FCTLW(g, c)		_FCTL_(FCTL_IN,    (g), (c), 0)
#define _FCTLRW(g, c)		_FCTL_(FCTL_INOUT, (g), (c), 0)
#define _FCTLRp(g, c, t)	_FCTL_(FCTL_OUT,   (g), (c), sizeof(t))
#define _FCTLWp(g, c, t)	_FCTL_(FCTL_IN,    (g), (c), sizeof(t))
#define _FCTLRWp(g, c, t)	_FCTL_(FCTL_INOUT, (g), (c), sizeof(t))

/* command values */
#define F_DUPFD		_FCTLW('f',0)		/* duplicate file desc. */
#define F_GETFD		_FCTL('f', 1)		/* get file desc. flags */
#define F_SETFD		_FCTLW('f', 2)		/* set file desc. flags */
#define F_GETFL		_FCTL('f', 3)		/* get file status flags */
#define F_SETFL		_FCTLW('f', 4)		/* set file status flags */

/* file descriptor flags (F_GETFD, F_SETFD) */
#define FD_CLOEXEC	1			/* close-on-exec flag */

#ifdef __cplusplus
}
#endif
#endif	/* __SYS_FCNTL_H__ */

