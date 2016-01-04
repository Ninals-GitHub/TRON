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

#ifndef	__BK_BPROCESS_H__
#define	__BK_BPROCESS_H__

#include <typedef.h>

#include <tk/typedef.h>
#include <tk/syscall.h>
#include <tk/kernel.h>
#include <tk/task.h>
#include <tk/timer.h>
#include <tk/winfo.h>

#include <bk/typedef.h>
#include <bk/atomic.h>
#include <bk/memory/vm.h>
#include <bk/uapi/signal.h>
#include <bk/uapi/sys/resource.h>

#include <sys/queue.h>
#include <sys/str_align.h>
/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct process;
struct task;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	process information
----------------------------------------------------------------------------------
*/
struct p_info {
	UW	etime;		/* accumulated elapsed time (s)			*/
	UW	utime;		/* user cpu time	 			*/
	UW	stime;		/* system cpu time				*/
	W	tmem;		/* memory size for a process to execute		*/
	W	wmem;		/* size of allocated memor for a process	*/
	W	resv[11];	/* reserved					*/
};

/*
----------------------------------------------------------------------------------
	process state
----------------------------------------------------------------------------------
*/
struct p_state {
	UW	state;		/* state of a process				*/
	W	priority;
	W	parpid;		/* parent's process id				*/
};

/*
----------------------------------------------------------------------------------
	user infromation of a process
----------------------------------------------------------------------------------
*/
struct p_user {
	TC	usr_name[14];	/* user name (includes secretive 2 characters)	*/
	TC	grp_name1[14];	/* group name1 (includes secretive 2 characters)*/
	TC	grp_name2[14];
	TC	grp_name3[14];
	TC	grp_name4[14];
	W	level;		/* user level (0 - 15)				*/
	W	net_level;	/* user level of network (1 -15)		*/
};

/*
----------------------------------------------------------------------------------
	signal management
----------------------------------------------------------------------------------
*/
struct signals {
	int			count;
	struct sigaction	action[NR_SIG];
	sigset_t		blocked;
	sigset_t		real_blocked;
	sigset_t		saved_sigmask;
};

/*
----------------------------------------------------------------------------------
	for wait4
----------------------------------------------------------------------------------
*/
struct wait4_args;

/*
----------------------------------------------------------------------------------
	process
----------------------------------------------------------------------------------
*/
#define	P_NONEXIST	0x0000		/* unregistered				*/
#define	P_DORMANT	0x1000		/* dormant				*/
#define P_WAIT		0x2000		/* wait					*/
#define P_READY 	0x4000		/* executable		 		*/
#define P_RUN		0x8000		/* running				*/

struct process {
	/* -------------------------------------------------------------------- */
	/* process states			 				*/
	/* -------------------------------------------------------------------- */
	long			state;
	unsigned int		flags;
	int			exit_state;
	int			exit_code;
	long			priority;
	
	atomic_t		usage;		/* usage counter		*/
	/* -------------------------------------------------------------------- */
	/* process relationship			 				*/
	/* -------------------------------------------------------------------- */
	struct list		list_tasks;
	pid_t			pid;
	struct process		*parent;
	struct list		list_children;	/* list of the children		*/
	struct list		sibling;	/* list entry of list_children	*/
	struct task		*group_leader;	/* task group leader		*/
	/* -------------------------------------------------------------------- */
	/* cpu time of which process consumed					*/
	/* -------------------------------------------------------------------- */
	cputime_t		utime;
	cputime_t		stime;
	/* -------------------------------------------------------------------- */
	/* file system				 				*/
	/* -------------------------------------------------------------------- */
	uid_t			uid;
	uid_t			euid;
	gid_t			gid;
	gid_t			egid;
	/* -------------------------------------------------------------------- */
	/* rlimit				 				*/
	/* -------------------------------------------------------------------- */
	struct rlimit		rlimits[RLIMIT_NLIMITS];
	/* -------------------------------------------------------------------- */
	/* signal				 				*/
	/* -------------------------------------------------------------------- */
	struct signals		signals;
	//struct wait4_args	*w4a;
	struct list		wait4_list;
	struct list		wait4_node;
	/* -------------------------------------------------------------------- */
	/* process's memory space						*/
	/* -------------------------------------------------------------------- */
	struct memory_space	*mspace;
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
 Funtion	:get_current
 Input		:void
 Output		:void
 Return		:struct process*
		 < current process >
 Description	:get current process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE struct process* get_current(void)
{
	return(get_current_task()->proc);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_current_mspace
 Input		:void
 Output		:void
 Return		:struct memory_space*
		 < memory_space of current process >
 Description	:get memory space of current process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE struct memory_space* get_current_mspace(void)
{
	return(get_current()->mspace);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_scheduled
 Input		:void
 Output		:void
 Return		:struct process*
		 < next scheduled process >
 Description	:get next scheduled process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE struct process* get_scheduled(void)
{
	return(get_scheduled_task()->proc);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_scheduled_mspace
 Input		:void
 Output		:void
 Return		:struct memory_space*
		 < memory_space of next scheduled process >
 Description	:get memory space of next scheduled process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE struct memory_space* get_scheduled_mspace(void)
{
	return(get_scheduled()->mspace);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_proc_management
 Input		:void
 Output		:void
 Return		:ER
		 < error code >
 Description	:initialize process management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER init_proc_management(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_pid
 Input		:void
 Output		:void
 Return		:pid_t
		 < allocated process id >
 Description	:allocate a pid
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT pid_t alloc_pid(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_pid
 Input		:pid_t pid
		 < to be freed pid >
 Output		:void
 Return		:void
 Description	:free pid
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void free_pid(pid_t pid);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_process
 Input		:pid_t pid
 		 < process id >
 Output		:void
 Return		:struct process*
		 < allocated process object >
 Description	:allocate a process object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct process* alloc_process(pid_t pid);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_process
 Input		:pid_t pid
		 < pid of a process to be freed >
 Output		:void
 Return		:void
 Description	:free process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void free_process(pid_t pid);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:startup_init_proc
 Input		:CONST T_CTSK *pk_ctsk
		 < tron create task packet >
 Output		:void
 Return		:void
 Description	:startup a initial process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER init_proc( CONST T_CTSK *pk_ctsk );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_pid_process
 Input		:pid_t pid
 		 < pid to get a process >
 Output		:void
 Return		:struct process*
 		 < process corresponds to the specified pid >
 Description	:get a process information from pid
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct process* get_pid_process(pid_t pid);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:is_child_process
 Input		:struct process *child
 		 < child process to examine >
 Output		:void
 Return		:BOOL
 		 < result >
 Description	:test whether the *child is current's child or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT BOOL is_child_prcess(struct process *child);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_init_proc
 Input		:void
 Output		:void
 Return		:struct process *
 		 < init process >
 Description	:get init process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct process* get_init_proc(void);

#endif	// __BK_BPROCESS_H__
