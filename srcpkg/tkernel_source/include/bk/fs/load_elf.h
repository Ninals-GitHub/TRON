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

#ifndef	__BK_FS_LOAD_ELF_H__
#define	__BK_FS_LOAD_ELF_H__


#include <bk/fs/vfs.h>
#include <bk/elf.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct vdso_load_info;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	ELF_HAS_INTERP		(1)

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
 Funtion	:init_elf_info
 Input		:struct elf_info *elf_info
 		 < elf information to initialize >
 Output		:struct elf_info *elf_info
 		 < elf information to initialize >
 Return		:void
 Description	:initialize a elf information 
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void init_elf_info(struct elf_info *elf_info);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_elf_info
 Input		:struct elf_info *elf_info
 		 < elf information to free >
 Output		:void
 Return		:void
 Description	:free memories for elf inforamtion
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void free_elf_info(struct elf_info *elf_info);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:read_elf
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < read elf information >
 Output		:struct elf_info *elf_info
 		 < read elf information >
 Return		:int
 		 < result >
 Description	:read elf header
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int read_elf(int fd, struct elf_info *elf_info);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:read_interp_name
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < elf information >
 Output		:void
 Return		:char*
 		 < interp name >
 Description	:read intepreter name from a segment
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT char* read_interp_name(int fd, struct elf_info *elf_info);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:load_elf
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < elf information >
 Output		:void
 Return		:int
 		 < result >
 Description	:load executable file into user memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int load_elf(int fd, struct elf_info *elf_info);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:load_vdso_elf
 Input		:int fd
 		 < open executable file descriptor >
 		 struct elf_info *elf_info
 		 < elf information >
 		 struct vdso_load_info *vdso_info
 		 < vdso load information >
 Output		:struct vdso_load_info *vdso_info
 		 < vdso load information >
 Return		:int
 		 < result >
 Description	:load executable file into user memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int load_vdso_elf(int fd, struct elf_info *elf_info,
				struct vdso_load_info *vdso_info);

#endif	// __BK_FS_LOAD_ELF_H__
