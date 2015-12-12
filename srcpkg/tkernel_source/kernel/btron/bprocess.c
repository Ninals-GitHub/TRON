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
#include <bk/typedef.h>
#include <bk/bprocess.h>
#include <bk/uapi/berrno.h>
#include <bk/uapi/errcode.h>
#include <bk/memory/vm.h>
#include <bk/memory/page.h>
#include <tk/sysmgr.h>
#include <sys/rominfo.h>
#include <sys/sysinfo.h>
#include <tstdlib/round.h>
#include <tstdlib/list.h>

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
	default rlimits
----------------------------------------------------------------------------------
*/
#define	RLIM_DEF_SOFT_STACK	(8 * 1024 * 1024)
#define	RLIM_DEF_SOFT_NOFILE	1024
#define	RLIM_DEF_HARD_NOFILE	4096
#define	RLIM_DEF_SOFT_MLOCK	(64 * 1024)
#define	RLIM_DEF_MSGQUEUE	819200

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL ID max_pid;
LOCAL unsigned long *pid_bitmap;
LOCAL struct process *proc_table;
LOCAL struct process *init;

/*
----------------------------------------------------------------------------------
	default rlimits
----------------------------------------------------------------------------------
*/
LOCAL struct rlimit def_rlim[RLIMIT_NLIMITS] = {
	[RLIMIT_CPU]		= { RLIM_INFINITY,	  RLIM_INFINITY		},
	[RLIMIT_FSIZE]		= { RLIM_INFINITY,	  RLIM_INFINITY		},
	[RLIMIT_DATA]		= { RLIM_INFINITY,	  RLIM_INFINITY		},
	[RLIMIT_STACK]		= { RLIM_DEF_SOFT_STACK,  RLIM_INFINITY		},
	[RLIMIT_CORE]		= { 0,			  RLIM_INFINITY		},
	[RLIMIT_RSS]		= { RLIM_INFINITY,	  RLIM_INFINITY		},
	[RLIMIT_NPROC]		= { 0,			  0			},
	[RLIMIT_NOFILE]		= { RLIM_DEF_SOFT_NOFILE, RLIM_DEF_HARD_NOFILE	},
	[RLIMIT_MEMLOCK]	= { RLIM_DEF_SOFT_MLOCK,  RLIM_DEF_SOFT_MLOCK	},
	[RLIMIT_AS]		= { RLIM_INFINITY,	  RLIM_INFINITY		},
	[RLIMIT_LOCKS]		= { RLIM_INFINITY,	  RLIM_INFINITY		},
	[RLIMIT_SIGPENDING]	= { 0,			  0			},
	[RLIMIT_MSGQUEUE]	= { RLIM_DEF_MSGQUEUE,    RLIM_DEF_MSGQUEUE	},
	[RLIMIT_NICE]		= { 0,			  0			},
	[RLIMIT_RTPRIO]		= { 0,			  0			},
	[RLIMIT_RTTIME]		= { RLIM_INFINITY,	  RLIM_INFINITY		},
};

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
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
EXPORT ER init_proc_management(void)
{
	ER err;

	/* -------------------------------------------------------------------- */
	/* Get system information						*/
	/* -------------------------------------------------------------------- */
	err = _tk_get_cfn(SCTAG_BMAXPID, &max_pid, 1);
	if ( err < 1 || NUM_PID < 1 ) {
		vd_printf("init_proc_management:%d\n", err);
		return EC_SYS;
	}
	
	/* -------------------------------------------------------------------- */
	/* allocate memory for a bitmap of process ids				*/
	/* -------------------------------------------------------------------- */
	pid_bitmap = (unsigned long*)
		allocLowMemory(DIV_ROUNDUP(max_pid, sizeof(unsigned long) * 8));
	
	if (!pid_bitmap) {
		vd_printf("cannot alloc pid_bitmap\n");
		return EC_NOMEM;
	}
	
	/* -------------------------------------------------------------------- */
	/* allocate memory for process information				*/
	/* -------------------------------------------------------------------- */
	proc_table = (struct process*)
		allocLowMemory(sizeof(struct process) * max_pid);
	
	if (!proc_table) {
		vd_printf("cannot alloc proc_table\n");
		goto err_proc_table;
	}
	
	return(EC_OK);

err_proc_table:
	Kfree((void*)pid_bitmap);
	return(EC_NOMEM);
}

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
EXPORT pid_t alloc_pid(void)
{
	pid_t pid;
	BOOL  old;
	
retry_search:
	pid = tstdlib_bitsearch0((void *)pid_bitmap, 0,
					max_pid * sizeof(unsigned long) * 8);

	if (0 <= pid) {
		old = tstdlib_bitset((void*)pid_bitmap, pid);
		
		/* ------------------------------------------------------------ */
		/* if already allocated the pid, retry to search		*/
		/* ------------------------------------------------------------ */
		if (old) {
			goto retry_search;
		}
	}
	
	return(pid);
}

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
EXPORT void free_pid(pid_t pid)
{
	if (pid < max_pid) {
		tstdlib_bitclr((void *)pid_bitmap, pid);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_process
 Input		:pid_t pid
		 < pid of new process >
 Output		:void
 Return		:struct process*
		 < allocated process object >
 Description	:allocate a process object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct process* alloc_process(pid_t pid)
{
	struct process *new_proc = NULL;
	
	/* -------------------------------------------------------------------- */
	/* allocate a process object from process information table		*/
	/* and initialize the object						*/
	/* future work : allocate from slab allocator				*/
	/* -------------------------------------------------------------------- */
	if (LIKELY(pid < max_pid)) {
		struct process *current = get_current();
		
		new_proc = &proc_table[pid];
		
		memset((void*)new_proc, 0x00, sizeof(struct process));
		
		new_proc->state = P_NONEXIST;
		init_list(&new_proc->list_tasks);
		new_proc->pid = pid;
		new_proc->tgid = pid;
		
		new_proc->parent = current;
		init_list(&new_proc->list_children);
		init_list(&new_proc->sibling);
		
		new_proc->utime = 0;
		new_proc->stime = 0;
		
		/* ------------------------------------------------------------ */
		/* initialize memory space					*/
		/* ------------------------------------------------------------ */
		new_proc->mspace = alloc_memory_space();
		
		if (!new_proc) {
			return(NULL);
		}
		/* ------------------------------------------------------------ */
		/* initialize new procwss's signals				*/
		/* ------------------------------------------------------------ */
		init_signal(new_proc);
		
		/* ------------------------------------------------------------ */
		/* set up default rlimit					*/
		/* ------------------------------------------------------------ */
		memcpy((void*)&new_proc->rlimits, (void*)&def_rlim,
								sizeof(def_rlim));
		
		/* ------------------------------------------------------------ */
		/* link to parent process					*/
		/* ------------------------------------------------------------ */
		add_list_tail(&new_proc->sibling, &current->list_children);
	}
	
	return(new_proc);
}

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
EXPORT void free_process(pid_t pid)
{
	struct process *proc;
	
	if (UNLIKELY(max_pid <= pid)) {
		return;
	}
	
	proc = &proc_table[pid];
	/* -------------------------------------------------------------------- */
	/* give children up for adoption to init				*/
	/* -------------------------------------------------------------------- */
	if (!is_empty_list(&proc->list_children)) {
		struct process *child_proc;

		list_for_each_entry(child_proc, &proc->list_children, sibling) {
			del_list(&child_proc->sibling);
			add_list_tail(&child_proc->sibling, &init->list_children);
		}
	}
	/* -------------------------------------------------------------------- */
	/* free all tasks							*/
	/* -------------------------------------------------------------------- */
	if (!is_empty_list(&proc->list_tasks)) {
		struct task *task;
		struct task *temp;

		list_for_each_entry_safe(task, temp,
						&proc->list_tasks, task_node) {
			del_list(&task->task_node);
			free_task(task);
		}
	}
	/* -------------------------------------------------------------------- */
	/* free all vm and page tables						*/
	/* -------------------------------------------------------------------- */
	free_vm_all(proc);
	
	proc->state		= P_NONEXIST;
	proc->flags		= 0;
	proc->priority		= 0;
	proc->pid		= 0;
	proc->tgid		= 0;
	proc->group_leader	= NULL;
	proc->parent		= NULL;
	proc->utime		= 0;
	proc->stime		= 0;
}

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
EXPORT ER init_proc( CONST T_CTSK *pk_ctsk )
{
	struct task *init_task;
	pid_t init_pid;
	ER ercd;
	
	/* -------------------------------------------------------------------- */
	/* first of all, create init task					*/
	/* future work : execute init process on ram disk			*/
	/* -------------------------------------------------------------------- */
	ercd = _tk_cre_tsk(pk_ctsk);
	
	if (UNLIKELY(ercd < E_OK)) {
		return(EC_INNER);
	}

	ercd = _tk_sta_tsk((ID)ercd, 0);

	if (UNLIKELY(ercd < E_OK)) {
		return(EC_INNER);
	}
	
	init_task = get_scheduled_task();
	
	/* -------------------------------------------------------------------- */
	/* set current task as init						*/
	/* -------------------------------------------------------------------- */
	set_current_task(init_task);
	
	/* -------------------------------------------------------------------- */
	/* allocate pid for init						*/
	/* -------------------------------------------------------------------- */
	init_pid = alloc_pid();
	
	if (UNLIKELY(init_pid < -1)) {
		_tk_del_tsk(ercd);
		return(EC_NOEXS);
	}
	
	/* -------------------------------------------------------------------- */
	/* set init process as current process					*/
	/* -------------------------------------------------------------------- */
	init_task->proc = &proc_table[init_pid];
	
	/* -------------------------------------------------------------------- */
	/* allocate memory for init						*/
	/* -------------------------------------------------------------------- */
	init = alloc_process(init_pid);
	
	if (UNLIKELY(!init)) {
		_tk_del_tsk(ercd);
		return(EC_NOEXS);
	}
	
	init->tgid = init_pid;
	
	/* -------------------------------------------------------------------- */
	/* initialize signal management						*/
	/* -------------------------------------------------------------------- */
	init_signal(init);
	
	/* -------------------------------------------------------------------- */
	/* set up default rlimit						*/
	/* -------------------------------------------------------------------- */
	memcpy((void*)&init->rlimits, (void*)&def_rlim, sizeof(def_rlim));
	
	/* -------------------------------------------------------------------- */
	/* attribute init task to init process					*/
	/* -------------------------------------------------------------------- */
	init->priority = 80;
	init->parent = init;
	init->group_leader = init_task;
	
	add_list_tail(&init_task->task_node, &init->list_tasks);
	
	/* -------------------------------------------------------------------- */
	/* set initial pde							*/
	/* -------------------------------------------------------------------- */
	init->mspace->pde = (pde_t*)get_system_pde();
	
	return(EC_OK);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_signal
 Input		:struct process *proc
 		 < process to initialize its sinals >
 Output		:void
 Return		:void
 Description	:initialize signal management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void init_signal(struct process *proc)
{
	int i;
	struct signals *sig = &proc->signals;
	
	sig->count = 0;
	sig->blocked = 0;
	sig->real_blocked = 0;
	sig->saved_sigmask = 0;	
	
	for (i = 0;i < NR_SIG;i++ ) {
		sig->action[i].sa_handler = NULL;
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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
