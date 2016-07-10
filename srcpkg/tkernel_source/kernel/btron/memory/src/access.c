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


#include <cpu.h>
#include <bk/bprocess.h>
#include <bk/memory/vm.h>
#include <bk/memory/page.h>
#include <bk/memory/access.h>
#include <tk/sysdef.h>

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
 Funtion	:copy_to_user
 Input		:void *to_user
 		 < user address to which copy from kernel memory >
 		 const void *from_kernel
 		 < kernel address from which copy to user memory >
 		 size_t size
 		 < size of copy >
 Output		:void
 Return		:ssize_t
 		 < copied length >
 Description	:copy kernel memory to user memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ssize_t
copy_to_user(void *to_user, const void *from_kernel, size_t size)
{
	int err;
	size_t n;
	char *to;
	char *from;
	
	err = vm_check_access(to_user, size, PROT_WRITE);
	
	if (UNLIKELY(err)) {
		return(err);
	}
	
	to = (char*)to_user;
	from = (char*)from_kernel;
	
	for (n = 0;n < size;n++) {
		*(to++) = *(from++);
	}
	
	return(size);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_from_user
 Input		:void *to_kernel
 		 < kernel address to which copy from user memory >
 		 const void *from_user
 		 < user address from which copy to kernel memory >
 		 size_t size
 		 < size of copy >
 Output		:void
 Return		:ssize_t
 		 < copied length >
 Description	:copy user memory to kernel memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ssize_t
copy_from_user(void *to_kernel, const void *from_user, size_t size)
{
	int err;
	size_t n;
	char *to;
	char *from;
	
	err = vm_check_access((void*)from_user, size, PROT_READ);
	
	if (UNLIKELY(err)) {
		return(err);
	}
	
	to = (char*)to_kernel;
	from = (char*)from_user;
	
	for (n = 0;n < size;n++) {
		*(to++) = *(from++);
	}
	
	return(size);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:strncpy_from_user
 Input		:char *to_kernel
 		 < kernel buffer to which copy from user memory >
 		 const char *from_user
 		 < user address from which copy to kernel memory >
 		 size_t max
 		 < max length to copy >
 Output		:void
 Return		:ssize_t
 		 < copied length or result >
 Description	:copy string resides on user memory to kernel buffer
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ssize_t
strncpy_from_user(char *to_kernel, const char *from_user, size_t max)
{
	unsigned long uaddr = (unsigned long)from_user;
	unsigned long kaddr = (unsigned long)to_kernel;
	size_t copied = 0;
	int err;
	int i;
	
	if (UNLIKELY(!max)) {
		vd_printf("strncpy_from_user:max is 0\n");
		return(0);
	}
	
	max--;
	
	while (0 < max) {
		char *to;
		char *from;
		size_t copy_len;
		
		if (PAGE_ALIGN(uaddr) == PAGE_ALIGN(uaddr + max)) {
			copy_len = max;
		} else {
			copy_len = RoundPage(uaddr) - uaddr;
			
			if (UNLIKELY(max <= copy_len)) {
				copy_len = max;
			}
		}
		
		err = vm_check_access((void*)uaddr, copy_len, PROT_READ);
		
		if (UNLIKELY(err)) {
			vd_printf("strncpy_from_user:check space is failed\n");
			return(-EFAULT);
		}
		
		to = (char*)kaddr;
		from = (char*)uaddr;
		
		for (i = 0;i < copy_len;i++) {
			if (*from == '\0') {
				*to = '\0';
				return(copied);
			}
			*(to++) = *(from++);
			copied++;
		}
		
		uaddr += copy_len;
		kaddr += copy_len;
		max -= copy_len;
	}
	
	to_kernel[copied] = '\0';
	
	return(copied);
}

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
 Description	:check read permission of real pte of user address space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER ChkUsrSpaceR( const void *addr, size_t len )
{
	unsigned int rng = get_ring();
	
	if (LIKELY(rng)) {
		return(_ChkSpace(addr, len, MA_READ, TMF_PPL(rng)));
	}
	
	return(_ChkSpace(addr, len, MA_READ, TMF_PPL(rng)));
}

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
 Description	:check write permisstion of read pte of user address space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER ChkUsrSpaceRW( const void *addr, size_t len )
{
	unsigned int rng = get_ring();
	
	if (LIKELY(rng)) {
		return(_ChkSpace(addr, len, MA_READ|MA_WRITE, TMF_PPL(rng)));
	}
	
	return(_ChkSpace(addr, len, MA_READ|MA_WRITE, TMF_PPL(rng)));
}

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
 Description	:check execution permisstion of real pte of user address space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER ChkUsrSpaceRE( const void *addr, size_t len )
{
	unsigned int rng = get_ring();
	
	if (LIKELY(rng)) {
		return(_ChkSpace(addr, len, MA_READ|MA_EXECUTE, TMF_PPL(rng)));
	}
	
	return(_ChkSpace(addr, len, MA_READ|MA_EXECUTE, TMF_PPL(rng)));
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
