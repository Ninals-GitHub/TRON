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
#include <bk/wait.h>
#include <bk/resource.h>
#include <bk/uapi/wait.h>

#include <tstdlib/list.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL pid_t xwait(struct wait4_args *wargs);
LOCAL INLINE pid_t xwait_children(struct wait4_args *wargs);
LOCAL INLINE pid_t xwait_a_child(struct wait4_args *wargs);
LOCAL int xwait_for(void);
LOCAL void exit_process(struct process *proc);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	SUPP_WAIT4_OPTIONS							\
		(WNOHANG | WUNTRACED | WCONTINUED | __WALL | __WCLONE )


#define	WAIT_FOR_GROUP_ID	0
#define	WAIT_FOR_CHILDREN	1
#define	WAIT_FOR_A_CHILD	2

/*
----------------------------------------------------------------------------------
	for wait4
----------------------------------------------------------------------------------
*/
struct wait4_args {
	int			wait_for;
	pid_t			wait_pid;
	pid_t			wait_tid;
	int			options;
	pid_t			wake_upper_pid;
	struct rusage		*rusage;
	struct list		wait4_node;
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
 Funtion	:wait4
 Input		:pid_t pid
 		 < pid of a process to wait for changing its status >
 		 int *status
 		 < wait status >
 		 int options
 		 < wait options >
 		 struct rusage *rusage
 		 < resource usage information >
 Output		:void
 Return		:pid_t
 		 < pid of a process of which state is changed >
 Description	:wait for process to change state
 		 this system call issues tk_slp_tsk
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL pid_t wait4(pid_t pid, int *status, int options, struct rusage *rusage)
{
	struct wait4_args	w4a;
	struct process		*current;
	
	if (options & ~SUPP_WAIT4_OPTIONS) {
		return(-EINVAL);
	}
	
	options |= WEXITED;
	
#if 0
	printf("wait4[pid=%d, ", pid);
	printf("*status=0x%08X, ", status);
	printf("options=0x%08X, ", options);
	printf("*rusage=0x%08X]\n", rusage);
#endif
	if (pid < -1) {
		w4a.wait_for = WAIT_FOR_GROUP_ID;
		w4a.wait_pid = -pid;
	} else if (pid == -1) {
		w4a.wait_for = WAIT_FOR_CHILDREN;
		w4a.wait_pid = pid;
	} else if (pid == 0) {
		current = get_current();
		
		w4a.wait_for = WAIT_FOR_GROUP_ID;
		w4a.wait_pid = current->pid;
	} else {
		w4a.wait_for = WAIT_FOR_A_CHILD;
		w4a.wait_pid = pid;
	}
	
	w4a.wait_tid = get_current_task()->tskid;
	w4a.options = options;
	w4a.rusage = rusage;
	w4a.wake_upper_pid = 0;
	
	return(xwait(&w4a));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:wakeup4
 Input		:struct process *child
 		 < child process about to exit >
 Output		:void
 Return		:void
 Description	:wake up slept parent by wait4
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void wakeup4(struct process *child)
{
	struct process *parent = child->parent;
	struct process *current;
	struct process *children;
	struct wait4_args *w4a = child->w4a;
	struct task *task;
	int err;
	
	/* -------------------------------------------------------------------- */
	/* no one waits								*/
	/* -------------------------------------------------------------------- */
	if (!w4a) {
		return;
	}
	
	current = get_current();
	
	/* -------------------------------------------------------------------- */
	/* delete w4a reference							*/
	/* -------------------------------------------------------------------- */
	if (w4a->wait_for == WAIT_FOR_A_CHILD) {
		child->w4a = NULL;
	} else {
		list_for_each_entry (children, &current->list_children, sibling) {
			children->w4a = NULL;
		}
	}
	
	/* -------------------------------------------------------------------- */
	/* set up w4a								*/
	/* -------------------------------------------------------------------- */
	w4a->wake_upper_pid = child->pid;
	
	/* -------------------------------------------------------------------- */
	/* wake up parent tasks							*/
	/* -------------------------------------------------------------------- */
#if 0
	list_for_each_entry (task, &parent->list_tasks, task_node) {
		err = _bk_wup_tsk(task);
		
		if (UNLIKELY(err)) {
			printf("_bk_slp_tsk:unexpected error[%d]\n", err);
		}
	}
#endif
	_bk_wup_tsk(get_tcb(w4a->wait_tid));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:exit_group
 Input		:int status
 		 < status >
 Output		:void
 Return		:void
 Description	:exit all threads in a process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL void exit_group(int status)
{
	struct process *current = get_current();
	
#if 0
	printf("exit_group[status=%d]\n", status);
	printf("process %d is exitting\n", current->pid);
#endif
	/* -------------------------------------------------------------------- */
	/* next task is dispatched in free_process function			*/
	/* -------------------------------------------------------------------- */
	wakeup4(current);
	free_process(current->pid);
	
	printf("exit_group:unexpected error\n");
	for(;;);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:_exit
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL void exit(int status)
{
	struct process *current = get_current();
	
#if 0
	printf("exit[status=%d]\n", status);
	printf("process %d is exitting\n", current->pid);
#endif
	/* -------------------------------------------------------------------- */
	/* next task is dispatched in free_process function			*/
	/* -------------------------------------------------------------------- */
	wakeup4(current);
	free_process(current->pid);
	
	printf("exit:unexpected error\n");
	
	for(;;);
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
 Funtion	:xwait
 Input		:struct wait4_args *w4a
 		 < wait arguments >
 Output		:void
 Return		:pid_t
 		 < pid of a process of which state is changed >
 Description	:wait for process to change state
==================================================================================
*/
LOCAL INLINE pid_t xwait(struct wait4_args *w4a)
{
	switch (w4a->wait_for) {
	case	WAIT_FOR_GROUP_ID:
		//return(xwait_group_id(w4a));
	case	WAIT_FOR_CHILDREN:
		return(xwait_children(w4a));
	case	WAIT_FOR_A_CHILD:
		return(xwait_a_child(w4a));
		break;
	default:
		printf("xwait:unexpected waiting for type[%d]\n", w4a->wait_for);
	}
	
	return(-ECHILD);
}

/*
==================================================================================
 Funtion	:xwait_children
 Input		:struct wait4_args *w4a
 		 < wait arguments >
 Output		:void
 Return		:pid_t
 		 < pid of a process of which state is changed >
 Description	:wait for child processes to change state
==================================================================================
*/
LOCAL INLINE pid_t xwait_children(struct wait4_args *w4a)
{
	struct process *child;
	struct process *current;
	int err;
	
	current = get_current();
	
	list_for_each_entry (child, &current->list_children, sibling) {
		child->w4a = w4a;
	}
	
	err = xwait_for();
	
	if (UNLIKELY(err)) {
		child->w4a = NULL;
		goto error_out;
	}
	
	return(w4a->wake_upper_pid);

error_out:
	list_for_each_entry (child, &current->list_children, sibling) {
		child->w4a = NULL;
	}
	
	return(-ECHILD);
}

/*
==================================================================================
 Funtion	:xwait_a_child
 Input		:struct wait4_args *w4a
 		 < wait arguments >
 Output		:void
 Return		:pid_t
 		 < pid of a process of which state is changed >
 Description	:wait for a child process to change state
==================================================================================
*/
LOCAL INLINE pid_t xwait_a_child(struct wait4_args *w4a)
{
	struct process *child;
	struct process *current;
	int err;
	
	child = get_pid_process(w4a->wait_pid);
	
	if (!child) {
		return(-ECHILD);
	}
	
	current = get_current();
	
	if (child->parent != current) {
		return(-ECHILD);
	}
	
	child->w4a = w4a;
	
	err = xwait_for();

	
	if (UNLIKELY(err)) {
		child->w4a = NULL;
		return(-ECHILD);
	}
	
	return(w4a->wake_upper_pid);
}

/*
==================================================================================
 Funtion	:xwait_for
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:tasks of current process are sleep in. zzz...
==================================================================================
*/
LOCAL int xwait_for(void)
{
	struct task *task;
	struct task *current_task;
	struct process *current;
	int err;
	
	current = get_current();
	current_task = get_current_task();
	
	list_for_each_entry (task, &current->list_tasks, task_node) {
		if (UNLIKELY(current_task == task)) {
			continue;
		}
		
		err = _bk_slp_tsk(task);
		
		if (UNLIKELY(err)) {
			//printf("_bk_slp_tsk:unexpected error[%d]\n", err);
		}
	}
	
	printf("sleep zzz...\n");
	
	err = _tk_slp_tsk(TMO_FEVR);
	
	printf("i'm waken!!!\n");
	
	if (UNLIKELY(err)) {
		printf("_tk_slp_tsk:unexpected error[%d]\n", err);
	}
	
	return(err);
}

/*
==================================================================================
 Funtion	:exit_process
 Input		:void
 Output		:void
 Return		:void
 Description	:exit process
==================================================================================
*/
LOCAL void exit_process(struct process *proc)
{
	wakeup4(proc);
	free_process(proc->pid);
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