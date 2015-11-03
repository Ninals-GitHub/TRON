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

#include <typedef.h>
#include <t2ex/string.h>
#include <bk/bprocess.h>
#include <bk/uapi/berrno.h>
#include <tk/sysmgr.h>
#include <tstdlib/round.h>

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
 Funtion	:fork
 Input		:void
 Output		:void
 Return		:pid_t
		 < child process id
		   parent : return pid of a child process if sucessed
		   child  : return 0 if scucessed
		   error  : if failed >
 Description	:create a child process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL pid_t fork(void)
{
	struct process *new_proc;
	struct task *new_task;
	pid_t pid;
	
	/* -------------------------------------------------------------------- */
	/* get new pid for new process						*/
	/* -------------------------------------------------------------------- */
	pid = alloc_pid();
	
	if (pid < 0) {
		return(-EAGAIN);
	}
	
	/* -------------------------------------------------------------------- */
	/* make an empty process						*/
	/* -------------------------------------------------------------------- */
	new_proc = alloc_process(pid);
	
	if (!new_proc) {
		goto failed_alloc_process;
	}
	
	/* -------------------------------------------------------------------- */
	/* make an empty task							*/
	/* -------------------------------------------------------------------- */
	new_task = alloc_task();
	
	if (!new_task) {
		goto failed_alloc_task;
	}
	
	/* -------------------------------------------------------------------- */
	/* link to current process						*/
	/* -------------------------------------------------------------------- */
	add_list_tail(&new_task->task_node, &new_proc->list_tasks);
	new_proc->group_leader = new_task;
	new_task->proc = new_proc;
	
	/* -------------------------------------------------------------------- */
	/* set up task context block						*/
	/* -------------------------------------------------------------------- */
	
	return(pid);
	
failed_alloc_task:
	free_process(pid);
failed_alloc_process:
	free_pid(pid);
	pid = -ENOMEM;
	
	return(pid);
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
 Funtion	:copy_task
 Input		:struct task *new
		 < copy to >
 Output		:void
 Return		:void
 Description	:copy current task to new one
==================================================================================
*/
LOCAL void copy_task(struct task *new)
{
	struct task *current;
	int name_len;
	/* -------------------------------------------------------------------- */
	/* clone current task							*/
	/* -------------------------------------------------------------------- */
	current = get_current_task();
	new->exinf	= current->exinf;
	new->tskatr	= current->tskatr;
	new->task	= current->task;
	new->ipriority	= current->ipriority;
	new->resid	= current->resid;
	new->stksz	= current->stksz;
	
	new->istack	= current->istack;
	
#if USE_OBJECT_NAME
	name_len = strnlen(current->name, OBJECT_NAME_LENGTH);
	strncpy(new->name, (const char*)current->name, name_len);
#endif
	new->isysmode	= current->isysmode;
	new->sysmode	= current->sysmode;
}

/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
