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
#include <bk/uapi/berrno.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int test_readlink(const char *path, char *buf, size_t bufsiz);

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
 Funtion	:readlink
 Input		:const char *path
 		 < path name to read its link contents >
 		 char *buf
 		 < user buffer to store read link result >
 		 size_t bufsiz
 		 < size of a user buffer >
 Output		:char *buf
 		 < user buffer to store read link result >
 Return		:ssize_t
 		 < actual read size >
 Description	:read value of a symbolic link
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
	int err;
	ssize_t test_size;

	//printf("readlink[path=%s, ", path);

	if (UNLIKELY(!path)) {
		return(-ENOENT);
	}
	
	if (UNLIKELY(!buf)) {
		return(-EINVAL);
	}
	
	if (UNLIKELY(!bufsiz)) {
		return(0);
	}
	
	err = ChkUsrSpaceR(buf, bufsiz);
	
	if (UNLIKELY(err)) {
		return(-EINVAL);
	}
	
	test_size = test_readlink(path, buf, bufsiz);
	
	if (!test_size) {
		return(-EINVAL);
	}
	
	//printf("buf=%s, ", buf);
	//printf("bufsiz=%d]\n", bufsiz);
	
	return(test_size);
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:test_readlink
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
LOCAL int test_readlink(const char *path, char *buf, size_t bufsiz)
{
	#define	TEST_PROC_SELF_EXE "/proc/self/exe"
	#define TEST_EXE_DIR	"/ram/bin/"
	#define	TEST_EXE	"test"
	size_t size;
	struct memory_space *mspace = get_current()->mspace;
	size_t len = strnlen((const char*)mspace->start_arg, PAGESIZE);
	char *buf_user;
	
	if (!strncmp(path, TEST_PROC_SELF_EXE, sizeof(TEST_PROC_SELF_EXE))) {
		size = strncpy(buf, TEST_EXE_DIR TEST_EXE, 4096);
	}
	
	
	
	return(size);
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
