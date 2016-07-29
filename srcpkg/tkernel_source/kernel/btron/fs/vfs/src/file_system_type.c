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
LOCAL struct file_system_type*
find_get_last_fs_type(struct file_system_type *fs_type);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	MAX_FS_NAME_LEN		PAGESIZE

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct file_system_type *root_fs_type;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:register_filesystem
 Input		:struct file_system_type *fs_type
 		 < file system type to register to the kernel >
 Output		:void
 Return		:int
 		 < result >
 Description	:register a file system type to the system
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int register_filesystem(struct file_system_type *fs_type)
{
	struct file_system_type *last_type;
	
	if (UNLIKELY(!root_fs_type)) {
		root_fs_type = fs_type;
		root_fs_type->next = NULL;
	}
	
	last_type = find_get_last_fs_type(fs_type);
	
	if (UNLIKELY(!last_type)) {
		return(-EBUSY);
	}
	
	last_type->next = fs_type;
	fs_type->next = NULL;
	init_list(&fs_type->fs_supers);
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_filesystem_type
 Input		:const char *name
 		 < name of file system to search >
 Output		:void
 Return		:struct file_system_type*
 		 < found file system type. return null if not found >
 Description	:search a list of file system type
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct file_system_type* get_filesystem_type(const char *name)
{
	struct file_system_type *fs_type = root_fs_type;
	
	while (fs_type) {
		if (!strncmp(name, fs_type->name, MAX_FS_NAME_LEN)) {
			break;
		}
		fs_type = fs_type->next;
	}
	
	return(fs_type);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:unregister_filesystem
 Input		:struct file_sytem_type *fs_type
 		 < file system type to unregister from the system >
 Output		:void
 Return		:int
 		 < result >
 Description	:unregister a file system type from the system
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int unregister_filesystem(struct file_system_type *fs_type)
{
	struct file_system_type *fs_element = root_fs_type;
	
	while (fs_element) {
		if (UNLIKELY(fs_element == fs_type)) {
			fs_element->next = fs_type->next;
			return(0);
		}
	}
	
	return(-EINVAL);
}



/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:find_get_last_fs_type
 Input		:struct file_sytem_type *fs_type
 Output		:void
 Return		:struct file_system_type*
 		 < a last element >
 Description	:get a last element of a file system list
==================================================================================
*/
LOCAL struct file_system_type*
find_get_last_fs_type(struct file_system_type *fs_type)
{
	struct file_system_type *fs_element = root_fs_type;
	
	while (fs_element) {
		if (UNLIKELY(fs_type == fs_element)) {
			return(NULL);
		}
		
		if (UNLIKELY(!fs_element->next)) {
			return(fs_element);
		}
		
		fs_element = fs_element->next;
	}
	
	return(NULL);
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
