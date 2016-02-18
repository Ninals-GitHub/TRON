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
 Funtion	:alloc_fs_states
 Input		:struct process *proc
 		 < process to allocate fs states >
 Output		:void
 Return		:int
 		 < result >
 Description	:allocate fs states
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int alloc_fs_states(struct process *proc)
{
	if (UNLIKELY(has_fdtable(proc))) {
		return(0);
	}
	
	return(extend_fdtable(proc));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_fs_states
 Input		:struct process *proc
 		 < process to free its fs states >
 Output		:void
 Return		:void
 Description	:free fs states
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_fs_states(struct process *proc)
{
	if (UNLIKELY(!has_fdtable(proc))) {
		return;
	}
	
	free_fdtable(proc->fs.files);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_fs_states
 Input		:struct process *to
 		 < copy to >
 		 struct process *from
 		 < copy from >
 Output		:void
 Return		:void
 Description	:copy fs states
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void copy_fs_states(struct process *to, struct process *from)
{
	struct fs_states *to_fs = &to->fs;
	struct fs_states *from_fs = &from->fs;
	
	copy_path(&to_fs->root, &from_fs->root);
	copy_path(&to_fs->cwd, &from_fs->cwd);
	
	to_fs->users = from_fs->users;
	to_fs->umask = from_fs->umask;
	to_fs->in_exec = from_fs->in_exec;
	
	to_fs->fd_count = from_fs->fd_count;
	
	copy_fdtable(to_fs->files, from_fs->files);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_cwd
 Input		:struct proces *proc
 		 < process to get its cwd >
 Output		:void
 Return		:struct path*
 		 < path of cwd >
 Description	:get current working directory path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct path* get_cwd(struct process *proc)
{
	return(&proc->fs.cwd);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:set_cwd
 Input		:struct process *proc
 		 < process to set its cwd >
 		 struct vfsmount *mnt_cwd
 		 < vfs mount information to which cwd belongs >
 		 struct dentry *d_cwd
 		 < directory entry object of cwd >
 Output		:void
 Return		:void
 Description	:set current working directory path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void
set_cwd(struct process *proc, struct vfsmount *mnt_cwd, struct dentry *d_cwd)
{
	/* future work:implementing lock for fs_states				*/
	
	struct path *cwd = &proc->fs.cwd;
	
	cwd->mnt = mnt_cwd;
	cwd->dentry = d_cwd;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_root
 Input		:struct process *proc
 		 < process to get its root directory >
 Output		:void
 Return		:struct path*
 		 < path of root >
 Description	:get root directory path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct path* get_root(struct process *proc)
{
	return(&proc->fs.root);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:set_root
 Input		:struct process *proc
 		 < process to set its root path >
 		 struct vfsmount *mnt_root
 		 < vfs mount information to which root belongs >
 		 struct dentry *d_root
 		 < root director entry object >
 Output		:void
 Return		:void
 Description	:set root directory path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void
set_root(struct process *proc, struct vfsmount *mnt_root, struct dentry *d_root)
{
	/* future work:implementing lock for fs_states				*/
	
	struct path *root = &proc->fs.root;
	
	root->mnt = mnt_root;
	root->dentry = d_root;
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
