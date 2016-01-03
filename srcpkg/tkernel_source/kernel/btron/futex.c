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
#include <bk/memory/mmap.h>
#include <bk/uapi/futex.h>
#include <bk/uapi/sys/time.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct futex_hlist;
struct futex_queue;

LOCAL ALWAYS_INLINE int check_uaddr(int *uaddr);
LOCAL ALWAYS_INLINE int check_uaddr2(int *uaddr2, int op);
LOCAL ALWAYS_INLINE int
check_copy_timeout(const struct timespec *timeout,
			struct timespec *ktimeout, int op);
LOCAL void init_futex_queue(struct futex_queue *fqueue, int *uaddr, int val);
LOCAL ALWAYS_INLINE void
insert_fhlist(struct futex_hlist *hlist, struct futex_queue *fqueue);
LOCAL int futex_wait(int *uaddr, int val, const struct timespec *timeout);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	futex hash list
----------------------------------------------------------------------------------
*/
#define	FUTEX_HLIST_SIZE	64

struct futex_hlist {
	struct list	list_futex;
};

struct futex_queue {
	struct list	node_futex;
	struct task	*wait_task;
	int		flags;
	int		val;
	int		*addr;			// also kernel space object
};

#define	FUTEX_FLAGS_KVALUE	1
#define	FUTEX_FLAGS_UVALUE	2

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct futex_hlist *fhlist;


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_futex
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize futex hash list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_futex(void)
{
	int i;
	
	/* -------------------------------------------------------------------- */
	/* allocate memory for futex hash list					*/
	/* -------------------------------------------------------------------- */
	fhlist = (struct futex_hlist*)
		allocLowMemory(sizeof(struct futex_hlist) * FUTEX_HLIST_SIZE);
	
	if (!fhlist) {
		vd_printf("cannot alloc futex hlist\n");
		return(-ENOMEM);
	}
	
	for (i = 0;i < FUTEX_HLIST_SIZE;i++) {
		init_list(&fhlist[i].list_futex);
	}
	
	return(0);
}



/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:futex
 Input		:int *uaddr
 		 < addess of user space to store the counter >
 		 int op
 		 < lock operation >
 		 int val
 		 < lock operation value depends on the operation >
 		 const struct timespec *timeout
 		 < timemout value depends on the operation >
 		 int *uaddr2
 		 < lock operation address2 or value2 depends on the operation >
 		 int val3
 		 < operation value3 >
 Output		:void
 Return		:int
 		 < result depends on the operation >
 Description	:fast user-space locking
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int futex(int *uaddr, int op, int val, const struct timespec *timeout,
			int *uaddr2, int val3)
{
	struct process *current;
	struct timespec _ktimeout;
	struct timespec *ktimeout;
	int val2;
	
	current = get_current();
	
	printf("futex[current pid=%d]!!!!!!\n", current->pid);
#if 1
	printf("*uaddr=0x%08X, ", uaddr);
	printf("op=%d, ", op);
	printf("val=%d, ", val);
	if (timeout) {
		printf("timeout->tv_sec=%ld, ", timeout->tv_sec);
		printf("timeout->tv_nsec=%ld, ", timeout->tv_nsec);
	} else {
		printf("*timeout=NULL, ");
	}
	if (uaddr2) {
		printf("*uaddr2=0x%08X, ", uaddr2);
	} else {
		printf("*uaddr2=NULL, ");
	}
	printf("val3=%d]\n", val3);
#endif
	for (;;); // futex is currently not supported
	/* -------------------------------------------------------------------- */
	/* as for now, only wait and wake are supported				*/
	/* -------------------------------------------------------------------- */
	switch (op) {
	case	FUTEX_WAIT:
		break;
	case	FUTEX_WAKE:
		break;
	case	FUTEX_FD:
	case	FUTEX_REQUEUE:
	case	FUTEX_CMP_REQUEUE:
	case	FUTEX_WAKE_OP:
	case	FUTEX_LOCK_PI:
	case	FUTEX_UNLOCK_PI:
	case	FUTEX_TRYLOCK_PI:
	case	FUTEX_WAIT_BITSET:
	case	FUTEX_WAKE_BITSET:
	case	FUTEX_WAIT_REQUEUE_PI:
	case	FUTEX_CMP_REQUEUE_PI:
		goto unsupported_op;
	default:
		return(-ENOSYS);
	}
	
	/* -------------------------------------------------------------------- */
	/* user space parameter check and timeout copied if needed		*/
	/* -------------------------------------------------------------------- */
	if (check_uaddr(uaddr)) {
		return(-EACCES);
	}
	
	if (check_copy_timeout(timeout, &_ktimeout, op)) {
		return(-EFAULT);
	}
	
	if (!timeout) {
		ktimeout = NULL;
	}
	
	if (check_uaddr2(uaddr2, op)) {
		return(-EACCES);
	}
	
	/* -------------------------------------------------------------------- */
	/* specific commands need timeout value as val2				*/
	/* -------------------------------------------------------------------- */
	switch (op) {
	case	FUTEX_REQUEUE:
	case	FUTEX_CMP_REQUEUE:
	case	FUTEX_WAKE_OP:
	case	FUTEX_CMP_REQUEUE_PI:
		val2 = (int)timeout;
	default:
		break;
	}
	
	/* -------------------------------------------------------------------- */
	/* execute futex command						*/
	/* -------------------------------------------------------------------- */
	switch (op) {
	case	FUTEX_WAIT:
		return(futex_wait(uaddr, val, (const struct timespec*)ktimeout));
	case	FUTEX_WAKE:
		break;
	case	FUTEX_FD:
	case	FUTEX_REQUEUE:
	case	FUTEX_CMP_REQUEUE:
	case	FUTEX_WAKE_OP:
	case	FUTEX_LOCK_PI:
	case	FUTEX_UNLOCK_PI:
	case	FUTEX_TRYLOCK_PI:
	case	FUTEX_WAIT_BITSET:
	case	FUTEX_WAKE_BITSET:
	case	FUTEX_WAIT_REQUEUE_PI:
	case	FUTEX_CMP_REQUEUE_PI:
		goto unsupported_op;
	default:
		return(-ENOSYS);
	}
	
	for(;;);
	return(0);

unsupported_op:
	printf("unsupported operation!\n");
	for(;;);
	return(-ENOSYS);
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
 Funtion	:check_uaddr
 Input		:int *uaddr
 		 < user specified address >
 Output		:void
 Return		:int
 		 < result >
 Description	:check user specified address
==================================================================================
*/
LOCAL ALWAYS_INLINE int check_uaddr(int *uaddr)
{
	unsigned long align = (unsigned long)uaddr;
	
	if (align & (sizeof(int) - 1)) {
		return(-EACCES);
	}

	return(ChkUsrSpaceR(uaddr, sizeof(int)));
}

/*
==================================================================================
 Funtion	:check_uaddr2
 Input		:int *uaddr2
 		 < user specified address >
 		 int op
 		 < futex command >
 Output		:void
 Return		:int
 		 < result >
 Description	:check user specified address
==================================================================================
*/
LOCAL ALWAYS_INLINE int check_uaddr2(int *uaddr2, int op)
{
	switch (op) {
	case	FUTEX_WAIT:		// go through
	case	FUTEX_WAKE:		// go through
	case	FUTEX_FD:
		return(0);
	case	FUTEX_REQUEUE:		// go through
	case	FUTEX_CMP_REQUEUE:	// go through
	case	FUTEX_WAKE_OP:
		break;
	case	FUTEX_LOCK_PI:		// go through
	case	FUTEX_UNLOCK_PI:	// go through
	case	FUTEX_TRYLOCK_PI:	// go through
	case	FUTEX_WAIT_BITSET:	// go through
	case	FUTEX_WAKE_BITSET:
		return(0);
	case	FUTEX_WAIT_REQUEUE_PI:	// go through
	case	FUTEX_CMP_REQUEUE_PI:
		break;
	default:
		return(-ENOSYS);
	}
	
	return(ChkUsrSpaceR(uaddr2, sizeof(int)));
}

/*
==================================================================================
 Funtion	:check_copy_timeout
 Input		:const struct timespec *timeout
 		 < user specified address >
 		 struct timespec *ktimeout
 		 < kernel buffer >
 		 int op
 		 < futex command >
 Output		:void
 Return		:int
 		 < result >
 Description	:check user specified address
==================================================================================
*/
LOCAL ALWAYS_INLINE int
check_copy_timeout(const struct timespec *timeout,
			struct timespec *ktimeout, int op)
{
	int err;
	
	switch (op) {
	case	FUTEX_WAIT:
		if (!timeout) {
			return(0);
		}
		break;
	case	FUTEX_WAKE:		// go through
	case	FUTEX_FD:		// go through
	case	FUTEX_REQUEUE:		// go through. timeout is used as val2
	case	FUTEX_CMP_REQUEUE:	// go through. timeout is used as val2
	case	FUTEX_WAKE_OP:		// timeout is used as val2
		return(0);
	case	FUTEX_LOCK_PI:
		if (!timeout) {
			return(0);
		}
		break;
	case	FUTEX_UNLOCK_PI:	// go through
	case	FUTEX_TRYLOCK_PI:	// go through
	case	FUTEX_WAIT_BITSET:
		return(0);
	case	FUTEX_WAKE_BITSET:	// go through
	case	FUTEX_WAIT_REQUEUE_PI:	// go through
		if (!timeout) {
			return(0);
		}
		break;
	case	FUTEX_CMP_REQUEUE_PI:	// timeout is used as val2
		return(0);
	default:
		return(-ENOSYS);
	}
	
	err = ChkUsrSpaceR(timeout, sizeof(struct timespec));
	
	if (err) {
		return(err);
	}
	
	memcpy((void*)ktimeout, (void*)timeout, sizeof(struct timespec));
	
	return(0);
}

/*
==================================================================================
 Funtion	:init_futex_queue
 Input		:struct futex_queu *fqueue
 		 < initializee >
 		 int *uaddr
 		 < address of user space to store the counter >
 		 int val
 		 < lock operation value >
 Output		:void
 Return		:void
 Description	:initialize futex queue
==================================================================================
*/
LOCAL void init_futex_queue(struct futex_queue *fqueue, int *uaddr, int val)
{
	struct vm *vm;
	
	init_list(&fqueue->node_futex);
	
	fqueue->wait_task = get_current_task();
	
	vm =get_address_vm(get_current(),
			(unsigned long)uaddr, (unsigned long)uaddr + sizeof(int));
	
	if (UNLIKELY(!vm)) {
		printf("init_futex_queue:unexpected behavior. stopped\n");
		for(;;);
	}
	
	fqueue->addr = uaddr;
	
	if (vm->mmap_flags & MAP_ANONYMOUS) {
		fqueue->flags = FUTEX_FLAGS_KVALUE;
	} else {
		fqueue->flags = FUTEX_FLAGS_UVALUE;
	}
	
	fqueue->val = val;
}

/*
==================================================================================
 Funtion	:insert_fhlist
 Input		:struct futex_hlist *hlist
 		 < futext hash list >
 		 struct futex_queue *fqueue
 		 < insertee >
 Output		:void
 Return		:void
 Description	:insert a futex queue to hash list chaing
==================================================================================
*/
LOCAL ALWAYS_INLINE void
insert_fhlist(struct futex_hlist *hlist, struct futex_queue *fqueue)
{
	if (UNLIKELY(is_empty_list(&hlist->list_futex))) {
		add_list(&fqueue->node_futex, &hlist->list_futex);
	}
	
	add_list_tail(&fqueue->node_futex, &hlist->list_futex);
}

/*
==================================================================================
 Funtion	:futext_wait
 Input		:int *uaddr
 		 < address of user space to store the counter >
 		 int val
 		 < lock operation value >
 		 const struct timespec *timeout
 		 < timeout value on the operation >
 Output		:void
 Return		:int
 		 < reuslt >
 Description	:futex wait
==================================================================================
*/
LOCAL int futex_wait(int *uaddr, int val, const struct timespec *timeout)
{
	struct futex_queue fqueue;
	struct futex_hlist *hlist;
	int hash_index;
	int err;
	TMO_U tmou;
	
	init_futex_queue(&fqueue, uaddr, val);
	
	hash_index = ((unsigned long)uaddr >> 2) % FUTEX_HLIST_SIZE;
	
	hlist = &fhlist[hash_index];
	
	insert_fhlist(hlist, &fqueue);
	
	printf("[futex]sleep zzz...\n");
	
	if (!timeout) {
		tmou = TMO_FEVR;
	} else {
		tmou = TIME_SEC_TO_US(timeout->tv_sec);
		tmou += timeout->tv_nsec;
	}
	
	err = _tk_slp_tsk_u(tmou);
	
	printf("[futex]i'm waken [tmou=%lu]!!!\n", tmou);
	printf("pid = %d\n", get_current()->pid);
	for(;;);
	
	if (UNLIKELY(err)) {
		printf("_tk_slp_tsk:unexpected error[%d]\n", err);
	}
	
	return(err);
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
