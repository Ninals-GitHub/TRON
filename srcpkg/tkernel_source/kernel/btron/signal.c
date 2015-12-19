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

#include <bk/kernel.h>
#include <bk/uapi/signal.h>


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
 Funtion	:rt_sigprocmask
 Input		:int how
 		 < how to change >
 		 const sigset_t *set
 		 < signal set to examine and change blocked signals >
 		 sigset_t *oldset
 		 < old signal set before changed >
 Output		:void
 Return		:void
 Description	:examine and change blocked signals
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int rt_sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	struct signals *sig;
	int err;
	
	sig = &(get_current())->signals;
	
	if (UNLIKELY(!set)) {
		if (!oldset) {
			printf("rt_sigprocmask:error:oldset is NULL\n");
			return(-EFAULT);
		}
		
		err = ChkUsrSpaceRW((const void*)oldset, sizeof(sigset_t));
		
		if (err) {
			printf("rt_sigprocmask:error:invalid access to oldset\n");
			return(-EFAULT);
		}
		
		*oldset = sig->blocked;
		
		return(0);
	}
	
	err = ChkUsrSpaceR((const void*)set, sizeof(sigset_t));

	if (UNLIKELY(err)) {
		printf("rt_sigprocmask:error:invalid access to set\n");
		return(-EFAULT);
	}
	
	printf("rt_sigprocmask[how=%d", how);
	printf(", *set=0x%08X\n", *set);
	if (oldset) {
		err = ChkUsrSpaceRW((const void*)oldset, sizeof(sigset_t));
	
		if (UNLIKELY(err)) {
			printf("rt_sigprocmask:error:invalid access to oldset\n");
			return(-EFAULT);
		}
		
		*oldset = sig->blocked;
		printf(", *oldset=0x%08X]\n", *oldset);
	}
	
	switch (how) {
	case	SIG_BLOCK:
		sig->blocked |= *set;
		sig->real_blocked |= *set;
		break;
	case	SIG_UNBLOCK:
		sig->blocked &= ~*set;
		sig->real_blocked &= ~*set;
		break;
	case	SIG_SETMASK:
		sig->blocked = *set;
		sig->real_blocked = *set;
		break;
	default:
		return(-EINVAL);
	}
	
	return(E_OK);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:rt_sigaction
 Input		:int signum
 		 < signal number >
 		 const struct sigaction *act
 		 < new signal action >
 		 struct sigaction *oldact
 		 < old signal action >
 Output		:void
 Return		:int
 		 < result >
 Description	:examine and change a signal action
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int rt_sigaction(int signum,
		const struct sigaction *act, struct sigaction *oldact)
{
	
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:tgkill
 Input		:int tgid
 		 < thread group id >
 		 int tid
 		 < thread id >
 		 int sig
 		 < signal >
 Output		:void
 Return		:int
 		 < result >
 Description	:send a signal to threads
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int tgkill(int tgid, int tid, int sig)
{
	printf("tgkill[tgid=%d", tgid);
	printf(", tid=%d", tid);
	printf(", sig=%d]\n", sig);
	
	//for(;;);
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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
