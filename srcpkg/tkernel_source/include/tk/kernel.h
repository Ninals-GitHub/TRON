/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/09/22
 *
 *----------------------------------------------------------------------
 */

/*
 *	kernel.h (T-Kernel/OS)
 *	T-Kernel Common Definition
 */

#ifndef _KERNEL_
#define _KERNEL_

#include <basic.h>
#include <tk/typedef.h>
#include <tk/errno.h>
#include <tk/syscall.h>
#include <tk/dbgspt.h>
#include <sys/imalloc.h>
#include <sys/queue.h>
#include <libstr.h>

/*
 * Kernel configuration file
 */
#include <tk/config.h>
#include <tk/cpu_conf.h>
#include <tk/tkdev_conf.h>
#include <tk/isysconf.h>

#include <tk/isyscall.h>

#define SYSCALL		EXPORT		/* Definition of system call */

#ifndef __tcb__
#define __tcb__
typedef struct task_control_block	TCB;
#endif

/*
 * Object lock
 *	Locked task is the highest run priority.
 *	Unable to nest lock.
 */
typedef struct objlock {
	QUEUE		wtskq;		/* Wait task queue */
} OBJLOCK;

Inline void InitOBJLOCK( OBJLOCK *loc )
{
	loc->wtskq.next = NULL;
}
IMPORT void LockOBJ( OBJLOCK* );
IMPORT void UnlockOBJ( OBJLOCK* );

Inline BOOL isLockedOBJ( OBJLOCK *loc )
{
	return ( loc->wtskq.next != NULL )? TRUE: FALSE;
}

/*
 * Extended SVC lock
 *	The priority does not change during lock.
 *	Able to nest lock.
 */
typedef struct svclock {
	QUEUE		wtskq;		/* Wait task queue */
	struct svclock	*locklist;	/* List during lock */
} SVCLOCK;

Inline void InitSVCLOCK( SVCLOCK *loc )
{
	loc->wtskq.next = NULL;
}
IMPORT void LockSVC( SVCLOCK* );
IMPORT void UnlockSVC( void );
IMPORT void AllUnlockSVC( TCB *tcb );

/*
 * Maximum ID number of each object
 *	1 - max_??? is valid ID.
 */
IMPORT ID	max_tskid;
IMPORT ID	max_semid;
IMPORT ID	max_flgid;
IMPORT ID	max_mbxid;
IMPORT ID	max_mbfid;
IMPORT ID	max_porid;
IMPORT ID	max_mtxid;
IMPORT ID	max_mplid;
IMPORT ID	max_mpfid;
IMPORT ID	max_cycid;
IMPORT ID	max_almid;
IMPORT ID	max_ssyid;
IMPORT ID	max_resid;

#define SYS_RESID	MIN_RESID	/* System resource ID */

/*
 * Other system information
 */
IMPORT INT	default_sstksz;		/* Default system stack size */
IMPORT PRI	max_ssypri;		/* Maximum subsystem priority */

/*
 * CPU-dependent include file
 */
#include <tk/cpu_status.h>

/*
 * System initialization (each module)
 */
IMPORT ER task_initialize( void );
IMPORT ER semaphore_initialize( void );
IMPORT ER eventflag_initialize( void );
IMPORT ER mailbox_initialize( void );
IMPORT ER messagebuffer_initialize( void );
IMPORT ER rendezvous_initialize( void );
IMPORT ER mutex_initialize( void );
IMPORT ER memorypool_initialize( void );
IMPORT ER fix_memorypool_initialize( void );
IMPORT ER cyclichandler_initialize( void );
IMPORT ER alarmhandler_initialize( void );
IMPORT ER subsystem_initialize( void );
IMPORT ER resource_group_initialize( void );

/*
 * Start/Exit system (tkstart.c)
 */
IMPORT void t_kernel_main( T_CTSK* );
IMPORT void t_kernel_exit( void );

/*
 * Target system-dependent routine (cpu_init.c tkdev_init.c)
 */
IMPORT ER   cpu_initialize( void );
IMPORT void cpu_shutdown( void );
IMPORT ER   tkdev_initialize( void );
IMPORT void tkdev_exit( void );

/*
 * Mutex
 */
IMPORT void signal_all_mutex( TCB *tcb );
IMPORT INT chg_pri_mutex( TCB *tcb, INT priority );

#endif /* _KERNEL_ */
