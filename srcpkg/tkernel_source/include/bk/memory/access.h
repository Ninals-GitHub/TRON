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

#ifndef	__BK_ACCESS_H__
#define	__BK_ACCESS_H__

#include <sys/segment.h>
#include <libstr.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
IMPORT ER _ChkSpace( CONST void *laddr, INT len, UINT mode, UINT env );

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
 Funtion	:copy_to_user
 Input		:void *to_user
 		 < user address to which copy from kernel memory >
 		 const void *from_kernel
 		 < kernel address from which copy to user memory >
 		 size_t size
 		 < size of copy >
 Output		:void
 Return		:size_t
 		 < copied length >
 Description	:copy kernel memory to user memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT size_t
copy_to_user(void *to_user, const void *from_kernel, size_t size);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_form_user
 Input		:void *to_kernel
 		 < kernel address to which copy from user memory >
 		 const void *from_user
 		 < user address from which copy to kernel memory >
 		 size_t size
 		 < size of copy >
 Output		:void
 Return		:size_t
 		 < copied length >
 Description	:copy user memory to kernel memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT size_t
copy_form_user(void *to_kernel, const void *from_user, size_t size);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:ChkSpaceR
 Input		:CONST void *addr
 		 < address to check >
 		 INT len
 		 < check length >
 Output		:void
 Return		:ER
 		 < result >
 Description	:check read permission of address space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER ChkSpaceR( CONST void *addr, INT len );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:ChkUsrSpaceR
 Input		:const void *addr
 		 < address of user space to check >
 		 size_t len
 		 < check length >
 Output		:void
 Return		:ER
 		 < result >
 Description	:check read permission of user address space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
//LOCAL ER ChkUsrSpaceR( const void *addr, size_t len )
#define	ChkUsrSpaceR(addr, len)	_ChkSpace(addr, len, MA_READ, TMF_PPL(USER_RPL))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:ChkSpaceRW
 Input		:CONST void *addr
 		 < address to check >
 		 INT len
 		 < check length >
 Output		:void
 Return		:ER
 		 < result >
 Description	:check write permisstion of address space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER ChkSpaceRW( CONST void *addr, INT len );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:ChkUsrSpaceRW
 Input		:const void *addr
 		 < address to check >
 		 size_t len
 		 < check length >
 Output		:void
 Return		:ER
 		 < result >
 Description	:check write permisstion of user address space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
//LOCAL ER ChkUsrSpaceRW( const void *addr, size_t len )
#define	ChkUsrSpaceRW(addr, len)						\
		_ChkSpace(addr, len, MA_READ|MA_WRITE, TMF_PPL(USER_RPL))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:ChkSpaceRE
 Input		:CONST void *addr
 		 < address to check >
 		 INT len
 		 < check length >
 Output		:void
 Return		:ER
 		 < result >
 Description	:check execution permisstion of address space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER ChkSpaceRE( CONST void *addr, INT len );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:ChkSpaceRE
 Input		:const void *addr
 		 < user address to check >
 		 size_t len
 		 < check length >
 Output		:void
 Return		:ER
 		 < result >
 Description	:check execution permisstion of user address space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
//LOCAL ER ChkUsrSpaceRE( const void *addr, size_t len )
#define	ChkUsrSpaceRE(addr, len)						\
		_ChkSpace(addr, len, MA_READ|MA_EXECUTE, TMF_PPL(USER_RPL))

#endif	// __BK_ACCESS_H__
