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
#include <bk/fs/vfs.h>
#include <bk/fs/fdtable.h>
#include <bk/memory/slab.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL struct fdtable* fdtable_cache_alloc(void);
LOCAL void fdtable_cache_free(struct fdtable *fdtable);

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
LOCAL struct kmem_cache *fdtable_cache;
LOCAL const char fdtable_cache_name[] = "fdtable_cache";


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_fdtable
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize fdtable management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_fdtable(void)
{
	fdtable_cache = kmem_cache_create(fdtable_cache_name,
					sizeof(struct fdtable), 0, 0, NULL);
	
	if (UNLIKELY(!fdtable_cache)) {
		vd_printf("error:fdtable_cache\n");
		return(-ENOMEM);
	}
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_fdtable
 Input		:size_t new_size
 		 < size of new fd table >
 Output		:void
 Return		:struct fdtable*
 		 < allocated fd table >
 Description	:allocate a fd table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct fdtable* alloc_fdtable(size_t new_size)
{
	struct fdtable *fdtable;
	
	fdtable = fdtable_cache_alloc();
	
	if (UNLIKELY(!fdtable)) {
		
		return(NULL);
	}
	
	fdtable->fd = (struct file**)kcalloc(sizeof(struct file*), new_size, 0);
	
	if (UNLIKELY(!fdtable->fd)) {
		goto failed_alloc_fd;
	}
	
	fdtable->open_fds = (unsigned long*)kcalloc(sizeof(unsigned long),
					new_size / (sizeof(unsigned long) * 8), 0);
	
	if (UNLIKELY(!fdtable->open_fds)) {
		goto failed_alloc_open_fds;
	}
	
	fdtable->close_on_exec = (unsigned long*)kcalloc(sizeof(unsigned long),
					new_size / (sizeof(unsigned long) * 8), 0);
	
	if (UNLIKELY(!fdtable->close_on_exec)) {
		goto failed_alloc_close_on_exec;
	}
	
	fdtable->max_fds = new_size;
	
	return(fdtable);
	
failed_alloc_close_on_exec:
	kfree(fdtable->open_fds);
failed_alloc_open_fds:
	kfree(fdtable->fd);
failed_alloc_fd:
	fdtable_cache_free(fdtable);
	
	return(NULL);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:extend_fdtable
 Input		:struct process *proc
 		 < process to extend its fd table >
 Output		:void
 Return		:int
 		 < result >
 Description	:extend a fd table or allocate a fd table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int extend_fdtable(struct process *proc)
{
	struct fdtable *new_table;
	struct fdtable *old_table;
	size_t new_size;
	unsigned long max_nofile;
	
	/* -------------------------------------------------------------------- */
	/* :begin critical section						*/
	/* -------------------------------------------------------------------- */
	BEGIN_CRITICAL_SECTION;
	if (UNLIKELY(!has_fdtable(proc))) {
		/* ------------------------------------------------------------ */
		/* allocate a new fd table					*/
		/* ------------------------------------------------------------ */
		new_table = alloc_fdtable(DEFAULT_NR_FDS);
		
		if (UNLIKELY(!new_table)) {
			goto error_out;
		}
		
		goto success_out;
	}
	
	max_nofile = get_soft_limit(RLIMIT_NOFILE);
	
	old_table = get_fdtable(proc);
	
	if (UNLIKELY(max_nofile < old_table->max_fds)) {
		return(-EMFILE);
	} else {
		new_size = old_table->max_fds + INC_NR_FDS;
	}
	
	if (UNLIKELY(max_nofile < new_size)) {
		new_size = max_nofile;
	}

	new_table = alloc_fdtable(new_size);

	if (!new_table) {
error_out:
		put_fdtable(proc);
		OUT_CRITICAL_SECTION;
		return(-ENOMEM);
	}
	
	copy_fdtable(new_table, old_table);
	free_fdtable(old_table);
success_out:
	install_fdtable(proc, new_table);
	put_fdtable(proc);
	/* -------------------------------------------------------------------- */
	/* :end critical section						*/
	/* -------------------------------------------------------------------- */
	END_CRITICAL_SECTION;
	
	return(0);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_fdtable
 Input		:struct fdtable *fdtable
 		 < a fd table to free >
 Output		:void
 Return		:void
 Description	:free a fd table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_fdtable(struct fdtable *fdtable)
{
	if (UNLIKELY(!fdtable)) {
		return;
	}
	
	kfree(fdtable->close_on_exec);
	kfree(fdtable->open_fds);
	kfree(fdtable->fd);
	fdtable_cache_free(fdtable);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_proc_fdtable
 Input		:struct process *to
 		 < copy fdtalbe to the process >
 		 struct process *from
 		 < copy fdtable from the process >
 Output		:void
 Return		:int
 		 < result >
 Description	:copy process's fdtable
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int copy_proc_fdtable(struct process *to, struct process *from)
{
	struct fdtable *from_fdtable = from->fs.files;
	struct fdtable *to_fdtable;
	int i;
	
	to->fs.files = alloc_fdtable(from_fdtable->max_fds);
	
	if (UNLIKELY(!to->fs.files)) {
		return(-ENOMEM);
	}
	
	to_fdtable = to->fs.files;
	
	copy_fdtable(to_fdtable, from_fdtable);
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_fdtable
 Input		:struct fdtable *to
 		 < copy to >
 		 struct fdtable *from
 		 < copy form >
 Output		:void
 Return		:void
 Description	:copy fd table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void copy_fdtable(struct fdtable *to, struct fdtable *from)
{
	memcpy(to->fd, from->fd, sizeof(struct file*) * from->max_fds);
	memcpy(to->close_on_exec, from->close_on_exec, from->max_fds / 8);
	memcpy(to->open_fds, from->open_fds, from->max_fds / 8);
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
 Funtion	:fdtable_cache_alloc
 Input		:void
 Output		:void
 Return		:struct fdtable*
 		 < fdtable object >
 Description	:allocate a fdtable object
==================================================================================
*/
LOCAL struct fdtable* fdtable_cache_alloc(void)
{
	struct fdtable *fdtable;
	
	fdtable = (struct fdtable*)kmem_cache_alloc(fdtable_cache, 0);
	
	memset((void*)fdtable, 0x00, sizeof(struct fdtable));
	
	return(fdtable);
}

/*
==================================================================================
 Funtion	:fdtable_cache_free
 Input		:struct fdtable *fdtable
 		 < fdtable object to free >
 Output		:void
 Return		:void
 Description	:free a fdtable object
==================================================================================
*/
LOCAL void fdtable_cache_free(struct fdtable *fdtable)
{
	kmem_cache_free(fdtable_cache, fdtable);
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
