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

#include <tk/typedef.h>

#include <tstdlib/round.h>

#include <bk/kernel.h>
#include <bk/elf.h>
#include <bk/fs/vdso.h>
#include <bk/fs/vfs.h>
#include <bk/fs/load_elf.h>
#include <bk/fs/ramfs/ramfs.h>
#include <bk/memory/page.h>
#include <bk/uapi/sys/stat.h>
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
LOCAL struct vnode *vdso_vnode;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:make_vdso_file
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:make a vdso file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int make_vdso_file(void)
{
	size_t size;
	
	vdso_vnode = vfs_alloc_vnode(get_rootfs_sb());
	
	if (UNLIKELY(!vdso_vnode)) {
		return(-ENOMEM);
	}
	
	vdso_vnode->v_fops = &ramfs_file_ope;
	vdso_vnode->v_mode = S_IFREG |
				(S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	vdso_vnode->v_size = getVdsoSize();
	vdso_vnode->v_blocks = ROUNDUP(vdso_vnode->v_size, S_BLKSIZE);
	vdso_vnode->v_uid = 0;
	vdso_vnode->v_gid = 0;
	vdso_vnode->v_nlink = 1;
	vdso_vnode->v_mtime.tv_sec = 0;
	vdso_vnode->v_ctime.tv_sec = vdso_vnode->v_mtime.tv_sec;
	
	size = mem_page_cache(vdso_vnode, (void*)getVdsoAddress(),
				0, 0, vdso_vnode->v_size);
	
	if (size != vdso_vnode->v_size) {
		return(-EFAULT);
	}
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_vdso_vnode
 Input		:void
 Output		:void
 Return		:struct vnode*
 		 < vnode of vdso file >
 Description	:get vnode of vdso file >
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct vnode* get_vdso_vnode(void)
{
	return(vdso_vnode);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:load_vdso
 Input		:struct vdso_load_info *vdso_load_info
 		 < vdso load information >
 Output		:struct vdso_load_info *vdso_load_info
 		 < vdso load information >
 Return		:int
 		 < result >
 Description	:load vdso into user memory space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int load_vdso(struct vdso_load_info *vdso_load_info)
{
	struct elf_info elf_info;
	int interp_exist;
	int err;
	int fd;
	
	fd = open_vdso_file();
	
	if (UNLIKELY(fd < 0)) {
		return(fd);
	}
	
	interp_exist = read_elf(fd, &elf_info);
	
	if (UNLIKELY(interp_exist < 0)) {
		err = interp_exist;
		goto failed_read_elf;
	}
	
	err = load_vdso_elf(fd, &elf_info, vdso_load_info);
	
	if (UNLIKELY(err)) {
		goto failed_load_elf;
	}
	
	/* go through								*/
failed_load_elf:
failed_read_elf:
	free_elf_info(&elf_info);
	close(fd);
	return(err);
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
