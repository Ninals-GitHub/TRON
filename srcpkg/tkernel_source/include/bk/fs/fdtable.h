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

#ifndef	__BK_FS_FDTABLLE_H__
#define	__BK_FS_FDTABLLE_H__

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct proc;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	open file descriptor table
----------------------------------------------------------------------------------
*/
#define	DEFAULT_NR_FDS			32
#define	INC_NR_FDS			32

struct fdtable {
	int				next_fd;
	unsigned int			max_fds;
	struct file			**fd;
	unsigned long			*close_on_exec;
	unsigned long			*open_fds;
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
 Funtion	:init_fdtable
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize fdtable management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int init_fdtable(void);

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
IMPORT int extend_fdtable(struct process *proc);

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
IMPORT void free_fdtable(struct fdtable *fdtable);

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
IMPORT int copy_proc_fdtable(struct process *to, struct process *from);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:has_fdtable
 Input		:struct process *proc
 		 < process to test whether it has already a fd table >
 Output		:void
 Return		:int
 		 < boolean >
 Description	:test whether the process has already a fd table or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL ALWAYS_INLINE int has_fdtable(struct process *proc)
{
	struct fs_states *fs = &proc->fs;
	return((int)fs->files);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:install_fdtable
 Input		:struct process *proc
 		 < process to be installed >
 		 struct fdtable *fdtable
 		 < a fd table to install >
 Output		:void
 Return		:void
 Description	:install a fd table to a process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL ALWAYS_INLINE
void install_fdtable(struct process *proc, struct fdtable *fdtable)
{
	if (UNLIKELY(!atomic_read(&proc->fs.fd_count))) {
		atomic_inc(&proc->fs.fd_count);
	}
	
	proc->fs.files = fdtable;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_fdtable
 Input		:struct process *proc
 		 < process to get its fd table >
 Output		:void
 Return		:struct fdtable*
 		 < fd table >
 Description	:get fd table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL ALWAYS_INLINE struct fdtable* get_fdtable(struct process *proc)
{
	if (UNLIKELY(!proc->fs.files)) {
		return(NULL);
	}
	
	atomic_inc(&proc->fs.fd_count);
	
	return(proc->fs.files);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:put_fdtable
 Input		:struct process *proc
 		 < process to put its fd table >
 Output		:void
 Return		:void
 Description	:put fd table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL ALWAYS_INLINE void put_fdtable(struct process *proc)
{
	if (atomic_read(&proc->fs.fd_count)) {
		atomic_dec(&proc->fs.fd_count);
	}
}

#endif	// __FORMAT_H__
