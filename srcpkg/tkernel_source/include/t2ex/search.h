/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
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

/*
 *	@(#)search.h
 *
 */

#ifndef _SEARCH_H_
#define	_SEARCH_H_

#include <basic.h>

typedef struct {
	char	*key;
	void	*data;
} ENTRY;

typedef enum {
	FIND,
	ENTER
} ACTION;

typedef enum {
	preorder,
	postorder,
	endorder,
	leaf
} VISIT;

struct hsearch_data {
	void	*htable;
	int	htablesize;
};

#ifdef __cplusplus
extern "C" {
#endif

IMPORT	int	hcreate_r(size_t nel, struct hsearch_data *htab);
IMPORT	void	hdestroy_r(struct hsearch_data* htab);
IMPORT	int	hsearch_r(ENTRY item, ACTION action, ENTRY **result, struct hsearch_data *htab);
IMPORT	void	insque(void *element, void *pred);
IMPORT	void	remque(void *element);
IMPORT	void	*lfind(const void *key, const void *base, size_t *nelp, size_t width, int (*compar)(const void *, const void *));
IMPORT	void	*lsearch(const void *key, void *base, size_t *nelp, size_t width, int (*compar)(const void *, const void *));
IMPORT	void	*tdelete(const void *key, void **rootp, int(*compar)(const void *, const void *));
IMPORT	void	*tfind(const void *key, void *const *rootp, int(*compar)(const void *, const void *));
IMPORT	void	*tsearch(const void *key, void **rootp, int(*compar)(const void *, const void *));
IMPORT	void	twalk(const void *root, void (*action)(const void *, VISIT, int ));

#ifdef __cplusplus
}
#endif
#endif /* _SEARCH_H_ */

