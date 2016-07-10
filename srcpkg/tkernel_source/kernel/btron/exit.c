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
struct wait4_args;

LOCAL pid_t xwait(struct wait4_args *wargs);
LOCAL INLINE pid_t xwait_children(struct wait4_args *wargs);
LOCAL INLINE pid_t xwait_a_child(struct wait4_args *wargs);
LOCAL pid_t xwait_for(struct wait4_args *w4a);
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
	
	pid = xwait(&w4a);
	
	return(pid);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:waitpid
 Input		:pid_t pid
 		 < pid of a process to wait for changing its status >
 		 int *status
 		 < wait status >
 		 int options
 		 < wait options >
 Output		:void
 Return		:pid_t
 		 < pid of a process of which state is changed >
 Description	:wait for process to change state
 		 this system call issues tk_slp_tsk
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL pid_t waitpid(pid_t pid, int *status, int options)
{
	return(wait4(pid, status, options, NULL));
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
	
	return;
	
#if 1
	printf("exit_group[status=%d]\n", status);
	printf("process %d is exitting\n", current->pid);
#endif
	for(;;);
	/* -------------------------------------------------------------------- */
	/* next task is dispatched in free_process function			*/
	/* -------------------------------------------------------------------- */
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
	free_process(current->pid);
	
	printf("exit:unexpected error\n");
	
	for(;;);
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
	struct task *task;
	int err;
	
	add_list_tail(&child->wait4_node, &parent->wait4_list);
	
	/* -------------------------------------------------------------------- */
	/* wake up parent tasks							*/
	/* -------------------------------------------------------------------- */
	list_for_each_entry (task, &parent->list_tasks, task_node) {
		err = _bk_rsm_tsk(task);
		
		if (UNLIKELY(err)) {
			//printf("_bk_rsm_tsk(taskid=%d):unexpected error[%d]\n", task->tskid, err);
		}
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:exit_wait4_list
 Input		:struct process *proc
 		 < proc to free a wait4 list >
 Output		:void
 Return		:void
 Description	:free a wait4 list when a process itsel is killed
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void exit_wait4_list(struct process *proc)
{
	struct process *wait4_proc;
	struct process *temp;
	
	if (is_empty_list(&proc->wait4_list)) {
		return;
	}
	
	list_for_each_entry_safe(wait4_proc, temp,
					&proc->wait4_list, wait4_node) {
		exit_process(wait4_proc);
		del_list(&wait4_proc->wait4_node);
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
		for(;;);
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
	struct process *current;
	pid_t pid;
	
	current = get_current();
	
	if (!is_empty_list(&current->wait4_list)) {
		goto wait4;
	}
	
	if (is_empty_list(&current->list_children)) {
		//printf("no children1\n");
		return(-ECHILD);
	}
	
	if (UNLIKELY(current == get_init_proc()) &&
		is_singular_list(&current->list_children)) {
		//printf("no children2\n");
		return(-ECHILD);
	}
	
wait4:
	pid = xwait_for(w4a);
	
	return(pid);
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
	int pid;
	
	child = get_pid_process(w4a->wait_pid);
	
	if (!child) {
		return(-ECHILD);
	}
	
	current = get_current();
	
	if (child->parent != current) {
		return(-ECHILD);
	}
	
	if (!is_empty_list(&current->wait4_list)) {
		goto wait4;
	}
	
	if (is_empty_list(&current->list_children)) {
		//printf("no children1\n");
		return(-ECHILD);
	}
	
	if (UNLIKELY(current == get_init_proc()) &&
		is_singular_list(&current->list_children)) {
		//printf("no children2\n");
		return(-ECHILD);
	}
	
wait4:	
	pid = xwait_for(w4a);

	return(pid);
}

/*
==================================================================================
 Funtion	:xwait_for
 Input		:struct wait4_args *w4a
 		 < wait4 arguments >
 Output		:void
 Return		:pid_t
 		 < result >
 Description	:tasks of current process are sleep in. zzz...
==================================================================================
*/
LOCAL pid_t xwait_for(struct wait4_args *w4a)
{
	struct task *task;
	struct task *current_task;
	struct process *current;
	struct process *wait4_proc;
	int err;
	pid_t pid;
	
	current = get_current();
	current_task = get_current_task();
	
	if (!is_empty_list(&current->wait4_list)) {
		struct process *temp;
		
		if (w4a->wait_pid == -1) {
			goto wakenup;
		}
		
		list_for_each_entry_safe(wait4_proc, temp,
					&current->wait4_list, wait4_node) {
			if (UNLIKELY(wait4_proc->pid == w4a->wait_pid)) {
				del_list(&wait4_proc->wait4_node);
				goto wakenup_from_a_child;
			}
		}
	}
	
	list_for_each_entry (task, &current->list_tasks, task_node) {
		if (UNLIKELY(current_task == task)) {
			continue;
		}
		
		err = _bk_sus_tsk(task);
		
		if (UNLIKELY(err)) {
			//printf("_bk_slp_tsk:unexpected error[%d]\n", err);
		}
	}
	
	//printf("sleep zzz...\n");
	
	//err = _tk_slp_tsk(TMO_FEVR);
	err = _tk_sus_tsk(current_task->tskid);
		
	if (UNLIKELY(err)) {
		printf("_tk_slp_tsk:unexpected error[%d]\n", err);
		return(err);
	}
	
	if (is_empty_list(&current->wait4_list)) {
		printf("current->wait4_list is null\n");
		return(-ECHILD);
	}

wakenup:
	if (w4a->wait_pid == -1) {
		wait4_proc = get_first_entry(&current->wait4_list,
						struct process, wait4_node);
		//printf("wait4_proc:pid:%d\n", wait4_proc->pid);
		del_list(&wait4_proc->wait4_node);
	} else {
		struct process *temp;
		
		list_for_each_entry_safe(wait4_proc, temp,
					&current->wait4_list, wait4_node) {
			if (UNLIKELY(wait4_proc->pid == w4a->wait_pid)) {
				del_list(&wait4_proc->wait4_node);
				break;
			}
		}
	}
	
	if (!wait4_proc) {
		printf("xwaken_up:unexpected error\n");
		return(-ECHILD);
	}

wakenup_from_a_child:
	pid = wait4_proc->pid;
	
	exit_process(wait4_proc);
	
	//printf("i'm waken from[%d]!!!\n", pid);
	
	return(pid);
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
	free_pid(proc->pid);
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
