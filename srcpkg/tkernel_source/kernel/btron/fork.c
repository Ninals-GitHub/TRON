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
#include <tk/sysmgr.h>
#include <tstdlib/round.h>
#include <cpu.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL INLINE int copy_process(struct process *new);
LOCAL void copy_task(struct task *new);
LOCAL int copy_task_context(struct task *new);

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
 Input		:struct ctx_reg *reg
 		 < register context >
 Output		:void
 Return		:pid_t
		 < child process id
		   parent : return pid of a child process if sucessed
		   child  : return 0 if scucessed
		   error  : if failed >
 Description	:create a child process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL pid_t fork(struct ctx_reg *reg)
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
	copy_task(new_task);
	
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
		goto failed_copy_process;
	}
	
	/* -------------------------------------------------------------------- */
	/* set up new task context						*/
	/* -------------------------------------------------------------------- */
	copy_task_context(new_task);
	
	new_task->tskctxb.ip = (unsigned long)int_syscall_return;
		
	return(pid);

failed_copy_process:
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
	/* copy current task							*/
	/* -------------------------------------------------------------------- */
	current = get_current_task();
	new->exinf	= current->exinf;
	new->tskatr	= current->tskatr;
	new->task	= current->task;
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
 Funtion	:copy_task_context
 Input		:struct task *new
 		 < copy to >
 Output		:void
 Return		:int
 		 < resutl >
 Description	:copy current task context to the new one
==================================================================================
*/
LOCAL int copy_task_context(struct task *new)
{
	int rng = (new->tskatr & TA_RNG3) >> 8;
	struct task *current = get_current_task();
	struct task_context_block *new_ctx = &new->tskctxb;
	struct task_context_block *current_ctx = &current->tskctxb;
	unsigned long new_sstak_top;
	unsigned long org_sstak_top;
	
	if (!rng) {
		return(-ENOSYS);
	}
	
	new_ctx->sp0 = (unsigned long)new->isstack;
	new_ctx->ssp = (void*)new->isstack;
	
	new_ctx->ds = current_ctx->ds;
	new_ctx->es = current_ctx->es;
	new_ctx->fs = current_ctx->fs;
	new_ctx->sysenter_cs = current_ctx->sysenter_cs;

	new_ctx->sp = current_ctx->sp;
	
	new_ctx->cr2 = 0;
	new_ctx->trap_nr = 0;
	new_ctx->error_code = 0;
	new_ctx->io_bitmap = NULL;
	new_ctx->iopl = 0;
	new_ctx->io_bitmap_max = 0;
	
	new_ctx->need_iret = FALSE;
	
	/* -------------------------------------------------------------------- */
	/* copy kernel stack							*/
	/* -------------------------------------------------------------------- */
	org_sstak_top = (unsigned long)current->isstack;
	org_sstak_top += -current->sstksz + RESERVE_SSTACK(current->tskatr);
	
	new_sstak_top = (unsigned long)new->isstack;
	new_sstak_top += -new->sstksz + RESERVE_SSTACK(new->tskatr);
	memcpy((void*)org_sstak_top, (void*)new_sstak_top, current->sstksz);
	
	return(0);
}
