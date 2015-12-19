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

#ifndef	__BK_SCHED_H__
#define	__BK_SCHED_H__


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
	cloning flags
----------------------------------------------------------------------------------
*/
#define	CSIGNAL		0x000000FF	/* signal mask to be sent at exit	*/
#define	CLONE_VM	0x00000100	/* vm shared between processes		*/
#define	CLONE_FS	0x00000200	/* fs info shared between processes	*/
#define	CLONE_FILES	0x00000400	/* open files shared between processes	*/
#define	CLONE_SIGHAND	0x00000800	/* signal handlers and blocked signals	*/
					/* are shared				*/
#define	CLONE_PTRACE	0x00002000	/* let tracing continue on the child	*/
#define	CLONE_VFORK	0x00004000	/* parent wants its child to wake it up	*/
					/* on mm_release			*/
#define	CLONE_PARENT	0x00008000	/* child has the same parent as that	*/
					/* of the calling process		*/
#define	CLONE_THREAD	0x00010000	/* same thread group?			*/
#define	CLONE_NEWNS	0x00020000	/* new mount namespace group		*/
#define	CLONE_SYSVSEM	0x00040000	/* share system V SEM_UNDO semantics	*/
#define	CLONE_SETTLS	0x00080000	/* create a new TLS for the child	*/
#define	CLONE_PARENT_SETTID	0x00100000	/* set the TID in the parent	*/
#define	CLONE_CHILD_CLEARTID	0x00200000	/* clear the TID in the child	*/
#define	CLONE_DETACHED	0x00400000	/* unused, ignored			*/
#define	CLONE_UNTRACED	0x00800000	/* the tracing process can't force	*/
					/* CLONE_PTRACE on this clone		*/
#define	CLONE_CHILD_SETTID	0x01000000	/* set the TID in the child	*/
#define	CLONE_RESERVED	0x02000000	/* previously the unused CLONE_STOPPED	*/
					/* (start in stopped state)		*/
#define	CLONE_NEWUTS	0x04000000	/* new utsname namespace		*/
#define	CLONE_NEWIPC	0x08000000	/* new ipc namespace			*/
#define	CLONE_NEWUSER	0x10000000	/* new user namespace			*/
#define	CLONE_NEWPID	0x20000000	/* new pid namespace			*/
#define	CLONE_NEWNET	0x40000000	/* new network namespace		*/
#define	CLONE_IO	0x80000000	/* clone io context			*/

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

#endif	// __BK_SCHED_H__
