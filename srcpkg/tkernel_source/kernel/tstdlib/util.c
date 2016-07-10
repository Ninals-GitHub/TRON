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

#include <compiler.h>
#include <t2ex/string.h>
#include <bk/kernel.h>
#include <bk/memory/slab.h>

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
 Funtion	:kstrndup
 Input		:const char *s
 		 < a string to dubplicate >
 		 size_t n
 		 < max size of a string length >
 		 int gfp
 		 < gfp flags >
 Output		:void
 Return		:char*
 		 < duplicated string >
 Description	:duplicate a string
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT char* kstrndup(const char *s, size_t n, int gfp)
{
	size_t len;
	char *dup;
	
	if (UNLIKELY(!s)) {
		return(NULL);
	}
	
	len = strnlen(s, n);
	
	dup = (char*)kmalloc(len + 1, gfp);
	
	if (UNLIKELY(!dup)) {
		return(dup);
	}
	
	memcpy((void*)dup, (void*)s, len);
	dup[len] = '\0';
	
	return(dup);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kmemdup
 Input		:const void *src
 		 < memory region to duplicate >
 		 size_t len
 		 < length of memory region >
 		 int gfp
 		 < gfp flags >
 Output		:void
 Return		:void*
 		 < duplicated memory >
 Description	:duplicate a memory region
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* kmemdup(const void *src, size_t len, int gfp)
{
	void *dup;
	
	if (UNLIKELY(!src)) {
		return(NULL);
	}
	
	dup = (void*)kmalloc(len, gfp);
	
	if (UNLIKELY(!dup)) {
		return(dup);
	}
	
	memcpy(dup, src, len);
	
	return(dup);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kstrncpy_len
 Input		:char *dest
 		 < a copy destination string >
 		 char *source
 		 < a string to copy >
 		 size_t n
 		 < max size of a string length >
 		 int gfp
 		 < gfp flags >
 Output		:void
 Return		:ssize_t
 		 < copied string length >
 Description	:copy a string and return its length
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ssize_t kstrncpy_len(char *dest, const char *source, size_t n, int gfp)
{
	size_t len;
	
	if (UNLIKELY(!source)) {
		return(-EFAULT);
	}
	
	len = strnlen(source, n);
	
	memcpy((void*)dest, (void*)source, len);
	dest[len] = '\0';
	
	return(len);
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
