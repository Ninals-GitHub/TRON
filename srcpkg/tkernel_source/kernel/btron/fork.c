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
#include <bk/kernel.h>
#include <bk/bprocess.h>
#include <bk/tls.h>
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
LOCAL void copy_task(struct task *from, struct task *new);
LOCAL long xfork(unsigned long flags, unsigned long child_newsp,
			int *parent_tidptr, int *child_tidptr,
			unsigned long child_stack_size,
			unsigned long tls,
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
	
	child_pid = xfork(SIGCHLD,		// flags
				0,		// newsp
				NULL,		// parent_tidptr
				NULL,		// child_tidptr
				0,		// stack_size
				0,		// tls
				child_regs);	// child_regs
	
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
	pid_t child_pid;
	unsigned long clone_flags;
	int stack_size;
	unsigned long newsp;
	int *parent_tidptr;
	int *child_tidptr;
	unsigned long tls;
	
	child_regs = (struct pt_regs*)&syscall_args;
	
#ifdef	CONFIG_CLONE_BACKWARDS
	clone_flags = (unsigned long)child_regs->bx;
	newsp = (unsigned long)child_regs->cx;
	parent_tidptr = (int*)child_regs->dx;
	tls = (unsigned long)child_regs->si;
	child_tidptr = (int*)child_regs->di;
	stack_size = 0;
#elif defined(CONFIG_CLONE_BACKWARDS2)
	newsp = (unsigned long)child_regs->bx;
	clone_flags = (unsigned long)child_regs->cx;
	parent_tidptr = (int*)child_regs->dx;
	child_tidptr = (int*)child_regs->si;
	tls = (unsigned long)child_regs->di;
	stack_size = 0;
#elif defined(CONFIG_CLONE_BACKWARDS3)
	clone_flags = (unsigned long)child_regs->bx;
	newsp = (unsigned long)child_regs->cx;
	stack_size = (int)child_regs->dx;
	parent_tidptr = (int*)child_regs->si;
	child_tidptr = (int*)child_regs->di;
	tls = (unsigned long)child_regs->bp;
#else
	clone_flags = (unsigned long)child_regs->bx;
	newsp = (unsigned long)child_regs->cx;
	parent_tidptr = (int*)child_regs->dx;
	child_tidptr = (int*)child_regs->si;
	tls = (unsigned long)child_regs->di;
	stack_size = 0;
#endif
	
#if 0
	printf("clone[flags=0x%08X, ", clone_flags);
	printf("newsp=0x%08X, ", newsp);
	printf("parent_tidptr=0x%08X, ", parent_tidptr);
	printf("child_tidptr=0x%08X, ", child_tidptr);
	printf("tls=0x%08X, ", tls);
	printf("stack_size=0x%08X\n", stack_size);
#endif
	child_pid = xfork(clone_flags, newsp, parent_tidptr, child_tidptr,
				stack_size, tls, child_regs);

	return(child_pid);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fork_set_clear_child_tid
 Input		:struct task *task
 		 < task to set or clear child tid >
 Output		:void
 Return		:void
 Description	:set or clear child tid on execute forked process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void fork_set_clear_child_tid(struct task *task)
{
	if (task->set_child_tid) {
		copy_to_user((void*)task->set_child_tid,
				(const void*)&task->tskid, sizeof(int));
	}
	
	if (task->clear_child_tid) {
		int clear = 0;
		
		copy_to_user((void*)task->clear_child_tid,
				(const void*)&clear, sizeof(int));
	}
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
 Output		:void
 Return		:int
 		 < result >
 Description	:copy current task to new one
==================================================================================
*/
LOCAL void copy_task(struct task *from, struct task *new)
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
	
	copy_task_context(from, new);
}

/*
==================================================================================
 Funtion	:xfork
 Input		:unsigned long flags
 		 < flags of clone >
 		 unsigned long child_newsp
 		 < stack address for a new child process >
 		 int *parent_tidptr
 		 < a pointer to parent thread id >
 		 int *child_tidptr
 		 < a pointer to child thread id >
 		 unsigned long child_stack_size
 		 < new child stack size >
 		 unsigned long tls
 		 < new tls >
 		 struct pt_regs *child_regs
 		 < child regs >
 Output		:void
 Return		:long
 		 < process id >
 Description	:fork a process
==================================================================================
*/
LOCAL long xfork(unsigned long flags, unsigned long child_newsp,
			int *parent_tidptr, int *child_tidptr,
			unsigned long child_stack_size,
			unsigned long tls,
			struct pt_regs *child_regs)
{
	struct process *new_proc;
	struct task *new_task;
	struct task *current_task = get_current_task();
	int rng = (current_task->tskatr & TA_RNG3) >> 8;
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
	copy_task(get_current_task(), new_task);
	
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
	
	/* -------------------------------------------------------------------- */
	/* set up new task context and override pt_regs				*/
	/* -------------------------------------------------------------------- */
	setup_fork_user_context(new_task, child_regs);
	
	/* -------------------------------------------------------------------- */
	/* set tid								*/
	/* -------------------------------------------------------------------- */
	if (flags & CLONE_CHILD_SETTID) {
		new_task->set_child_tid = child_tidptr;
	} else {
		new_task->set_child_tid = NULL;
	}
	
	if (flags & CLONE_CHILD_CLEARTID) {
		new_task->clear_child_tid = child_tidptr;
	} else {
		new_task->clear_child_tid = NULL;
	}
	
	fork_set_clear_child_tid(new_task);
	
	if (flags & CLONE_PARENT_SETTID) {
		copy_to_user((void*)parent_tidptr,
				(const void*)&current_task->tskid,
				sizeof(int));
	}
	
	if (flags & CLONE_SETTLS) {
		set_thread_area((struct user_desc *)tls);
	}
	
	return(pid);
	
failed_start_task:
failed_copy_process:
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

