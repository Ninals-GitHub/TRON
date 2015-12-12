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

#ifndef	__BK_SIGNAL_H__
#define	__BK_SIGNAL_H__

#include <bk/typedef.h>

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
	siginfo
----------------------------------------------------------------------------------
*/
typedef int	sigval_t;

typedef struct {
	int		si_signo;	/* signal number			*/
	int		si_errno;	/* an errno value			*/
	int		si_code;	/* signal code				*/
	int		si_trapno;	/* trap number that caused hardware-	*/
					/* generated signal			*/
					/* (unused on most architectures)	*/
	pid_t		si_pid;		/* sending process id			*/
	uid_t		si_uid;		/* real user id of sending process	*/
	int		si_status;	/* exit value or signal			*/
	clock_t		si_utime;	/* user time consumed			*/
	clock_t		si_stime;	/* system time consumed			*/
	sigval_t	si_value;	/* signal value				*/
	int		si_int;		/* POSIX.1b signal			*/
	void		*si_ptr;	/* POSIX.1b signal			*/
	int		si_overrun;	/* timer overrun count; POSIX.1b timers	*/
	int		si_timerid;	/* timer id; POSIX.1b timers		*/
	void		*si_addr;	/* memory location which caused fault	*/
	long		si_band;	/* band event				*/
	int		si_fd;		/* file descriptor			*/
	short		si_addr_lsb;	/* leaset significat bit of address	*/
	void		*si_call_addr;	/* address of system call instruction	*/
	int		si_syscall;	/* number of attempted system call	*/
	unsigned int	si_arch;	/* architecture of attempted system call*/
} siginfo_t;

/*
----------------------------------------------------------------------------------
	sigset
----------------------------------------------------------------------------------
*/
typedef unsigned long	sigset_t;

/*
----------------------------------------------------------------------------------
	sigaction
----------------------------------------------------------------------------------
*/
typedef void (*_sighandler_t)(int);
typedef void (*_sigaction_t)(int, siginfo_t*, void*);
typedef void (*sigrestore_t)(void);

struct sigaction {
	union {
		_sighandler_t	_sa_handler;
		_sigaction_t	_sa_sigaction;
	} _u;
	sigset_t	sa_mask;
	int		sa_flags;
	sigrestore_t	sa_restore;
};

#define	sa_handler	_u._sa_handler
#define	sa_sigaction	_u._sa_sigaction

/*
----------------------------------------------------------------------------------
	signal
----------------------------------------------------------------------------------
*/
#define	SIGHUP		1
#define	SIGINT		2
#define	SIGQUIT		3
#define	SIGILL		4
#define	SIGTRAP		5
#define	SIGABRT		6
#define	SIGIOT		6
#define	SIGSTKFLT	7
#define	SIGFPE		8
#define	SIGKILL		9
#define	SIGBUS		10
#define	SIGSEGV		11
#define	SIGXCPU		12
#define	SIGPIPE		13
#define	SIGALRM		14
#define	SIGTERM		15
#define	SIGUSR1		16
#define	SIGUSR2		17
#define	SIGCHLD		18
#define	SIGPWR		19
#define	SIGVTALRM	20
#define	SIGPROF		21
#define	SIGIO		22
#define	SIGPOLL		SIGIO
#define	SIGWINCH	23
#define	SIGSTOP		24
#define	SIGTSTP		25
#define	SIGCONT		26
#define	SIGTTIN		27
#define	SIGTTOU		28
#define	SIGURG		29
#define	SIGXFSZ		30
#define	SIGUNUSED	31
#define	SIGSYS		31

#define	NR_SIG		32

#define	SIGRTMIN	32
#define	SIGRTMAX	NR_SIG

/*
----------------------------------------------------------------------------------
	sa_flags
----------------------------------------------------------------------------------
*/
#define	SA_ONSTACK	0x00000001	/* a registered stack_t will be used	*/
#define	SA_RESETHAND	0x00000004
#define	SA_NOCLDSTOP	0x00000008	/* turn off SIGCHLD when children stop	*/
#define	SA_SIGINFO	0x00000010
#define	SA_NODEFER	0x00000020	/* prevent the current signal from being*/
					/* masked in the handerl		*/
#define	SA_RESTART	0x00000040	/* restart singals (which were the	*/
					/* default long ago)			*/
#define	SA_NOCLDWAIT	0x00000080	/* on SIGCHLD to infibit zombies	*/
#define	_SA_SIGGFAULT	0x00000100	/* HPUX					*/

#define	SA_NOMASK	SA_NODEFER
#define	SA_ONESHOT	SA_RESETHAND

#define	MINSIGSTKSZ	2048
#define	SIGSTKSZ	8192

/*
----------------------------------------------------------------------------------
	how to examine and change blocked signals
----------------------------------------------------------------------------------
*/
#define	SIG_BLOCK	1		/* for blocking signals			*/
#define	SIG_UNBLOCK	2		/* for unblocking signals		*/
#define	SIG_SETMASK	3		/* for setting the signal mask		*/

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

#endif	// __BK_SIGNAL_H__
