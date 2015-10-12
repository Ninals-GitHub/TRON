/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2013/03/05.
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
 *	@(#)tkn_malloc.c
 *
 */

/* T-Kernel/Network memory allocation routines */

#include <tk/tkernel.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/syslog.h>

#include <netmain/tkn_malloc.h>


#ifndef TKN_MEMWAIT
#define TKN_MEMWAIT	10	// task wait time(ms) if cannot alloc memory
#endif
#ifndef TKN_MEMWAIT_LOG
#define TKN_MEMWAIT_LOG	1000
#endif

/*
 * protocol stack memory tables standard definision
 */
MALLOC_DEFINE(M_DEVBUF, "devbuf", "device driver memory");
MALLOC_DEFINE(M_FREE, "free", "should be on free list");
MALLOC_DEFINE(M_PCB, "pcb", "protocol control block");
MALLOC_DEFINE(M_SOFTINTR, "softintr", "Softinterrupt structures");
MALLOC_DEFINE(M_RTABLE, "routetbl", "routing tables");
MALLOC_DEFINE(M_FTABLE, "fragtbl", "fragment reassembly header");
MALLOC_DEFINE(M_IPMOPTS, "ip_moptions", "internet multicast options");
MALLOC_DEFINE(M_TEMP, "temp", "temporary memory");
MALLOC_DEFINE(M_IOV, "temp", "temporary memory");
MALLOC_DEFINE(M_KMEM, "kmem", "kernel memory");

/* BSD kernel style malloc() */
void *
tkn_malloc(u_long size, int flags)
{
	void *ptr;

	ptr = Kmalloc(size);
	if (ptr != NULL) {
		/* clean memory? */
		if (flags & M_ZERO)
			memset(ptr, 0, size);
		return ptr;
	}

	if ((flags & M_NOWAIT) == 0) {
		log(LOG_ERR,
			"[TKN %s]: tid %d malloc failed(waitok)\n",
			__func__, tk_get_tid());
	}

	return NULL;
}

/* BSD kernel style free() */
void
tkn_free(void *addr)
{
	Kfree(addr);
}

ER
tkn_kmem_init(void)
{
	return 0;
}

ER
tkn_kmem_finish(void)
{
	return E_OK;
}

/* allocate 1 page for pool memory (page aligned memory) */
vaddr_t
tkn_alloc_poolpage1(boolean_t waitok)
{
	ER er;
	void *kaddr;

	er = tk_get_smb(&kaddr, 1, TA_RNG0);
	if (er != E_OK) {
		if (waitok) {
			log(LOG_ERR,
			    "[TKN %s]: tid %d tk_get_smb failed(waitok)\n",
			    __func__, tk_get_tid());
		}
		return NULL;
	}
	if ((vaddr_t)kaddr & (NBPG - 1))
		panic("[TKN]%s: Memory Alignment Error", __func__);

	return (vaddr_t)kaddr;
}

/* free 1 page for pool memory */
void
tkn_free_poolpage1(vaddr_t addr)
{
	if (addr & (NBPG - 1))
		panic("[TKN]%s: Memory Alignment Error", __func__);

	tk_rel_smb((void*)addr);
}
