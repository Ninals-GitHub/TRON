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
#include <bk/memory/page.h>
#include <bk/memory/vm.h>
#include <bk/uapi/berrno.h>
#include <bk/uapi/sched.h>
#include <tk/sysmgr.h>
#include <tstdlib/round.h>
#include <cpu.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL INLINE int copy_process(struct process *new);
LOCAL int
copy_task(struct task *from, struct task *new, struct pt_regs *child_regs);
LOCAL long xfork(unsigned long flags, void *child_stack,
			void *parent_tidptr, void *child_tidptr,
			struct pt_regs *child_regs);

IMPORT unsigned long int_syscall_return;


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
 Input		:void *child_regs
 		 < child regs >
 Output		:void
 Return		:pid_t
		 < child process id
		   parent : return pid of a child process if sucessed
		   child  : return 0 if scucessed
		   error  : if failed >
 Description	:create a child process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL pid_t fork(void *syscall_args)
{
	struct pt_regs *child_regs = (struct pt_regs*)&syscall_args;
	pid_t child_pid;
	
	child_pid = xfork((unsigned long)child_regs->bx,// unsigned long flags
				(void*)child_regs->cx,	// void *child_stack
				(void*)child_regs->dx,	// void *parent_tidptr
				(void*)child_regs->si,	// void *child_tidptr
				child_regs);		// pt_regs *child_regs
	
	return(child_pid);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:clone
 Input		:void *syscall_args
 		 < for child regs information >
 Output		:void
 Return		:long
 		 < process id >
 Description	:create a child process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL long clone(void *syscall_args)
{
	struct pt_regs *child_regs;
	unsigned long clone_flags;
	pid_t child_pid;
	
	//memcpy((void*)&child_regs, (void*)&syscall_args, sizeof(struct pt_regs));
	
	child_regs = (struct pt_regs*)&syscall_args;
	
	clone_flags = (unsigned long)child_regs->bx;
	
#if 1
	printf("clone[flags=0x%08X, ", child_regs->bx);
	printf("child_stack=0x%08X, ", child_regs->cx);
	printf("parent_tidptr=0x%08X, ", child_regs->dx);
	printf("child_tidptr=0x%08X, ", child_regs->si);
	printf("child_regs=0x%08X\n", child_regs);
	printf("ip=0x%08X\n", child_regs->ip);
#endif
	
#if 1
	child_pid = xfork(clone_flags,
				(void*)child_regs->cx,	// void *child_stack
				(void*)child_regs->dx,	// void *parent_tidptr
				(void*)child_regs->si,	// void *child_tidptr
				child_regs);		// pt_regs *child_regs
#endif
	printf("child pid=%d\n", child_pid);
	
	return(child_pid);
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
 Funtion	:copy_process
 Input		:struct process *new
 Output		:void
 Return		:int
 		 < result >
 Description	:copy current process to new one
==================================================================================
*/
LOCAL INLINE int copy_process(struct process *new)
{
	struct process *current = get_current();
	
	return(copy_mm(current, new));
}

/*
==================================================================================
 Funtion	:copy_task
 Input		:struct task *from
 		 < copy from >
 		 struct task *new
		 < copy to >
		 struct pt_regs *child_regs
		 < child context >
 Output		:void
 Return		:int
 		 < result >
 Description	:copy current task to new one
==================================================================================
*/
LOCAL int
copy_task(struct task *from, struct task *new, struct pt_regs *child_regs)
{
	int name_len;

	/* -------------------------------------------------------------------- */
	/* copy current task							*/
	/* -------------------------------------------------------------------- */
	new->exinf	= from->exinf;
	new->tskatr	= from->tskatr;
	new->task	= from->task;
	new->resid	= from->resid;
	new->stksz	= from->stksz;
	
	new->istack	= from->istack;
	
#if USE_OBJECT_NAME
	name_len = strnlen(from->name, OBJECT_NAME_LENGTH);
	strncpy(new->name, (const char*)from->name, name_len);
#endif
	new->isysmode	= from->isysmode;
	new->sysmode	= from->sysmode;
	
	/* -------------------------------------------------------------------- */
	/* set up new task context						*/
	/* -------------------------------------------------------------------- */
	return(copy_task_context(from, new, child_regs));
}

/*
==================================================================================
 Funtion	:xfork
 Input		:unsigned long flags
 		 < flags of clone >
 		 void *child_stack
 		 < stack address for a new child process >
 		 void *parent_tidptr
 		 < a pointer to parent thread id >
 		 void *child_tidptr
 		 < a pointer to child thread id >
 		 struct pt_regs *child_regs
 		 < child regs >
 Output		:void
 Return		:long
 		 < process id >
 Description	:fork a process
==================================================================================
*/
LOCAL long xfork(unsigned long flags, void *child_stack,
			void *parent_tidptr, void *child_tidptr,
			struct pt_regs *child_regs)
{
	int rng = (get_current_task()->tskatr & TA_RNG3) >> 8;
	struct process *new_proc;
	struct task *new_task;
	pid_t pid;
	int err;
	
	/* -------------------------------------------------------------------- */
	/* cannot call from kernel task						*/
	/* -------------------------------------------------------------------- */
	if (!rng) {
		return(-ENOSYS);
	}
	
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
	/* copy current task to new one						*/
	/* -------------------------------------------------------------------- */
	err = copy_task(get_current_task(), new_task, child_regs);
	
	if (err) {
		goto failed_copy_task;
	}
	
	/* -------------------------------------------------------------------- */
	/* link new task to new process						*/
	/* -------------------------------------------------------------------- */
	add_list_tail(&new_task->task_node, &new_proc->list_tasks);
	new_proc->group_leader = new_task;
	new_task->proc = new_proc;
	
	/* -------------------------------------------------------------------- */
	/* copy current process to new one					*/
	/* -------------------------------------------------------------------- */
	err = copy_process(new_proc);
	
	if (err) {
		printf("fork:failed copy_process\n");
		goto failed_copy_process;
	}
	//new_proc->exit_signal = flags & CSIGNAL;
	
	/* -------------------------------------------------------------------- */
	/* start new process							*/
	/* -------------------------------------------------------------------- */
	//printf("fork:start _tk_sta_tsk\n");
	err = _tk_sta_tsk(new_task->tskid, 0);
	
	if (err) {
		goto failed_start_task;
	}
	
	//printf("fork:finished\n");
	
	return(pid);

failed_start_task:
failed_copy_process:
failed_copy_task:
failed_alloc_task:
	free_process(pid);
failed_alloc_process:
	free_pid(pid);
	pid = -ENOMEM;
	
	return(pid);
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

