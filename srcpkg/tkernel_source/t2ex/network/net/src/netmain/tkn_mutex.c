/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2013/03/08.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
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

/*
 *	@(#)tkn_mutex.c
 *
 */

#include <tk/tkernel.h>
#include "rominfo.h"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/mutex.h>
#include <sys/_queue.h>
#include <sys/kmem.h>
#ifndef T2EX
#include <sys/atomic.h>
#else
#include <sys/_atomic.h>
#endif
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/malloc.h>

#include <sys/tkn_intr.h>

#include <tk/util.h>
#include <tk/dbgspt.h>

#include "tkn.h"


#define DEFAULT_MAX_COUNT ((UINT)128)
#define INDEX_OFFSET 5

#define INDEX(x)	(((x) >> INDEX_OFFSET) - 1)
#define NO(x)		((x) & 0x1fU)
#define ID(x, y)	((((x) + 1) << INDEX_OFFSET) | (y))

typedef struct {
	FastMLock lock;
	UW used_bitmap;
	INT owner_tid[32];
} MutexMLock;


LOCAL UINT max_mutex_count;
LOCAL UINT lock_count;
LOCAL MutexMLock* mutex_mlock;

INT mtx_oldspl = 0;
INT mtx_count = 0;


EXPORT ER tkn_mutex_initialize(void)
{
	LockTKN();

	/*
	 * Calculation of the number of mutexes.
	 *  - tkn_initialize() needs 57 mutexes.
	 *  - a socket needs 2 mutexes and the maximum number of sockets
	 *    is (maxfiles - NDFDFILE).
	 */
	max_mutex_count = 57 + (maxfiles - NDFDFILE) * 2;
	lock_count = (max_mutex_count + 31) / 32;

	mutex_mlock = malloc(sizeof(MutexMLock) * lock_count, M_KMEM, M_NOWAIT | M_ZERO);

	mtx_oldspl = 0;
	mtx_count = 0;

	UnlockTKN();

	return (mutex_mlock == NULL) ? E_NOMEM : E_OK;
}

EXPORT ER tkn_mutex_finish(void)
{
	int index;
	ER ercd = E_OK;

	LockTKN();

	for(index = 0; index < lock_count; index++) {
		if ( mutex_mlock[index].used_bitmap != 0 ) {
			ercd = DeleteMLock(&mutex_mlock[index].lock);
			if ( ercd < E_OK ) {
				break;
			}
		}
	}

	if ( ercd == E_OK ) {
		free(mutex_mlock, M_KMEM);
	}

	UnlockTKN();

	return ercd;
}

LOCAL int init_mutex(__volatile kmutex_t* mtx, kmutex_type_t type, int ipl)
{
	int index;
	int no;

	switch (type) {
	case MUTEX_ADAPTIVE:
		KASSERT(ipl == IPL_NONE);
		break;
	case MUTEX_DEFAULT:
	case MUTEX_DRIVER:
		if (ipl == IPL_NONE || ipl == IPL_SOFTCLOCK ||
		    ipl == IPL_SOFTBIO || ipl == IPL_SOFTNET ||
		    ipl == IPL_SOFTSERIAL) {
			type = MUTEX_ADAPTIVE;
		} else {
			type = MUTEX_SPIN;
		}
		break;
	default:
		break;
	}
	mtx->type = type;

	for(index = 0; index < lock_count; index++) {
		if ( mutex_mlock[index].used_bitmap != 0xffffffffU ) {
			break;
		}
	}

	if ( index >= lock_count ) {
		return ENOMEM;
	}

	if ( mutex_mlock[index].used_bitmap == 0 ) {
		ER ercd = CreateMLock(&mutex_mlock[index].lock, (CONST UB*)"Nmtx");
		if ( ercd < E_OK ) {
			return ENOMEM;
		}
	}

	for(no = 0; no < sizeof(UW)*8; no++) {
		if ( (mutex_mlock[index].used_bitmap & (1U << no)) == 0 ) {
			break;
		}
	}

	if ( no == sizeof(UW)*8 ) {
		return ENOENT;
	}

	mutex_mlock[index].used_bitmap |= 1U << no;

	mtx->mtxid = ID(index, no);
	mtx->ipl = ipl;

	return 0;
}

ER lock_mutex(__volatile kmutex_t* mtx, TMO tmo)
{
	int index = INDEX(mtx->mtxid);
	int no = NO(mtx->mtxid);
	ER ercd;
	int s;

	UnlockTKN();

	if ( tmo == TMO_FEVR ) {
		ercd = MLockTmo(&mutex_mlock[index].lock, no, TMO_POL);
		if ( ercd == E_TMOUT ) {
			s = tkn_spl_unlock(IPL_NONE);
			ercd = MLockTmo(&mutex_mlock[index].lock, no, tmo);
			tkn_spl_lock(s);
		}
	} else {
		ercd = MLockTmo(&mutex_mlock[index].lock, no, tmo);
	}

	LockTKN();
	if ( ercd >= E_OK ) {
		mutex_mlock[index].owner_tid[no] = tk_get_tid();
	}

	return ercd;
}

ER unlock_mutex(__volatile kmutex_t* mtx)
{
	int index = INDEX(mtx->mtxid);
	int no = NO(mtx->mtxid);

	if ( mutex_mlock[index].owner_tid[no] != tk_get_tid() ) {
		return E_ILUSE;
	}

	mutex_mlock[index].owner_tid[no] = 0;
	ER ercd = MUnlock(&mutex_mlock[index].lock, no);

	return ercd;
}

LOCAL void release_mutex(__volatile kmutex_t* mtx)
{
	int index = INDEX(mtx->mtxid);
	int no = NO(mtx->mtxid);
	int tid = mutex_mlock[index].owner_tid[no];

	if ( tid != 0 ) {
		ER ercd = unlock_mutex(mtx);
		if ( ercd < E_OK ) {
			panic("release_mutex: %d\n", MERCD(ercd));
		}
		if ( mtx->type == MUTEX_SPIN ) {
			if ( --mtx_count == 0 ) {
				int s = mtx_oldspl;
				mtx_oldspl = 0;
				tkn_spl_unlock(s);
			}
		}
	}

	mutex_mlock[index].used_bitmap &= ~(1U << no);

	if ( mutex_mlock[index].used_bitmap == 0 ) {
		ER ercd = DeleteMLock(&mutex_mlock[index].lock);
		if ( ercd < E_OK ) {
			panic("release_mutex: %d\n", MERCD(ercd));
		}
	}

	mtx->mtxid = 0;
}

/* ------------------------------------------------------------------------ */

int tkn_mutex_init(__volatile kmutex_t *mtx, kmutex_type_t type, int ipl)
{
	int error;
	(void)type;

	LockTKN();

#ifdef DEBUG
	unsigned long old_id = mtx->mtxid;
#endif
	error = init_mutex(mtx, type, ipl);
#ifdef DEBUG
	printf("mutex init %lu(at %p) by %d, old id = %lu\n", mtx->mtxid, mtx, tk_get_tid(), old_id);
#endif

	UnlockTKN();

	return error;
}

void tkn_mutex_destroy(__volatile kmutex_t *mtx)
{
	LockTKN();

	if ( mtx->mtxid != 0 ) {
#ifdef DEBUG
		printf("mutex destroy %lu(at %p) by %d\n", mtx->mtxid, mtx, tk_get_tid());
#endif
		release_mutex(mtx);
	} else {
#ifdef DEBUG
		printf("mutex destroy uninitialized %lu by %d\n", mtx->mtxid, tk_get_tid());
#endif
	}

	UnlockTKN();
}

void tkn_mutex_spin_enter( __volatile kmutex_t *mtx )
{
	LockTKN();
	if ( mtx->mtxid == 0 ) {
		panic("tkn_mutex_enter: not initialized.\n");
	}
	UnlockTKN();

	int s = tkn_spl_lock(mtx->ipl);

	LockTKN();
	if ( mtx_count++ == 0 ) {
		mtx_oldspl = s;
	}

	ER ercd = lock_mutex(mtx, TMO_FEVR);
	if ( ercd < E_OK ) {
		ID id = tk_get_tid();
		if ( ercd == E_ID ) {
			panic("tkn_mutex_enter: task#%d, error=%d, ID = %lu\n", id, MERCD(ercd), mtx->mtxid);
		} else {
			panic("tkn_mutex_enter: task#%d, error=%d\n", id, MERCD(ercd));
		}
	}
	UnlockTKN();
}

void tkn_mutex_adaptive_enter( __volatile kmutex_t *mtx )
{
	LockTKN();
	if ( mtx->mtxid == 0 ) {
		panic("tkn_mutex_enter: not initialized.\n");
	}

	ER ercd = lock_mutex(mtx, TMO_FEVR);
	if ( ercd < E_OK ) {
		ID id = tk_get_tid();
		if ( ercd == E_ID ) {
			panic("tkn_mutex_enter: task#%d, error=%d, ID = %lu\n", id, MERCD(ercd), mtx->mtxid);
		} else {
			panic("tkn_mutex_enter: task#%d, error=%d\n", id, MERCD(ercd));
		}
	}
	UnlockTKN();
}

void tkn_mutex_enter( __volatile kmutex_t *mtx )
{
	if ( mtx->type == MUTEX_SPIN ) {
		tkn_mutex_spin_enter(mtx);
	} else {
		tkn_mutex_adaptive_enter(mtx);
	}
}

void tkn_mutex_spin_exit( __volatile kmutex_t *mtx )
{
	LockTKN();
	if ( mtx->mtxid == 0 ) {
		panic("tkn_mutex_exit: not initialized.\n");
	}

	ER ercd = unlock_mutex(mtx);
	if ( ercd < E_OK ) {
		panic("tkn_mutex_exit: %d\n", MERCD(ercd));
	}

	if ( --mtx_count == 0 ) {
		int s = mtx_oldspl;
		mtx_oldspl = 0;
		tkn_spl_unlock(s);
	}
	UnlockTKN();
}

void tkn_mutex_adaptive_exit( __volatile kmutex_t *mtx )
{
	LockTKN();
	if ( mtx->mtxid == 0 ) {
		panic("tkn_mutex_exit: not initialized.\n");
	}

	ER ercd = unlock_mutex(mtx);
	if ( ercd < E_OK ) {
		panic("tkn_mutex_exit: %d\n", MERCD(ercd));
	}

	UnlockTKN();
}

void tkn_mutex_exit( __volatile kmutex_t *mtx )
{
	if ( mtx->type == MUTEX_SPIN ) {
		tkn_mutex_spin_exit(mtx);
	} else {
		tkn_mutex_adaptive_exit(mtx);
	}
}

int tkn_mutex_tryenter( __volatile kmutex_t *mtx )
{
	int	lock = 0;

	LockTKN();
	if ( mtx->mtxid == 0 ) {
		panic("tkn_mutex_tryenter: not initialized.\n");
	}
	UnlockTKN();

	if ( mtx->type == MUTEX_SPIN ) {
		int s = tkn_spl_lock(mtx->ipl);
		LockTKN();
		if ( mtx_count++ == 0 ) {
			mtx_oldspl = s;
		}
	} else {
		LockTKN();
	}

	ER ercd = lock_mutex(mtx, TMO_POL);

	lock = (ercd == E_TMOUT) ? 0 : 1;

	UnlockTKN();

	return lock;
}

int tkn_mutex_owned(__volatile kmutex_t *mtx)
{
	int	owned = 0;
	int index;
	int no;

	LockTKN();

	if ( mtx->mtxid == 0 ) {
		panic("tkn_mutex_owned: not initialized.\n");
	}

	index = INDEX(mtx->mtxid);
	no = NO(mtx->mtxid);

	owned = (mutex_mlock[index].owner_tid[no] == tk_get_tid()) ? 1 : 0;

	UnlockTKN();

	return owned;
}

struct kmutexobj {
	kmutex_t		mo_lock;
	unsigned int	mo_refcnt;
};

kmutex_t* tkn_mutex_obj_alloc(kmutex_type_t type, int ipl)
{
	struct kmutexobj *mo;
	int error;

	mo = kmem_alloc(sizeof *mo, KM_SLEEP);
	if ( mo == NULL ) {
		return NULL;
	}
	bzero(mo, sizeof *mo);
	error = tkn_mutex_init(&mo->mo_lock, type, ipl);
	if ( error != 0 ) {
		kmem_free(mo, sizeof(*mo));
		return NULL;
	}
	mo->mo_refcnt = 1;

	return (kmutex_t *)mo;
}

void tkn_mutex_obj_hold(__volatile kmutex_t *lock)
{
	struct kmutexobj *mo = (struct kmutexobj *)lock;

	atomic_inc_uint(&mo->mo_refcnt);
}

bool tkn_mutex_obj_free(__volatile kmutex_t *lock)
{
	struct kmutexobj *mo = (struct kmutexobj *)lock;

	if (atomic_dec_uint_nv(&mo->mo_refcnt) > 0) {
		return false;
	}
	tkn_mutex_destroy(&mo->mo_lock);
	kmem_free(mo, sizeof(*mo));
	return true;
}
