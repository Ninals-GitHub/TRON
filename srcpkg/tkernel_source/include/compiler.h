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
#ifndef	__COMPILER_H__
#define	__COMPILER_H__

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
#define	__user
#define	__kernel
#define	__safe
#define	__force
#define	__nocast
#define	__iomem
#define	__chk_user_ptr(x)		(void)0
#define	__chk_io_ptr(x)			(void)0
#define	__builtin_warning(x, y...)	(1)
#define	__must_hold(x)
#define	__acquires(x)
#define	__releases(x)
#define	__acquire(x)			(void)0
#define	__release(x)			(void)0
#define	__cond_lock(x,c)		(c)
#define	__percpu
#define	__rcu
#define	__pmem
#define	__private
#define	ACCESS_PRIVATE(p, member)	((p)->member)

/*
==================================================================================

	description : Inline Assembler

==================================================================================
*/
#define	ASM			__asm__ __volatile__

/*
==================================================================================

	description : Inline

==================================================================================
*/
#define	INLINE			__inline__
#define	ALWAYS_INLINE inline	__attribute__((always_inline))

/*
==================================================================================

	description : Static Branch Prediction

==================================================================================
*/
#define	LIKELY( x )		__builtin_expect(!!( x ),1)
#define	UNLIKELY( x )		__builtin_expect(!!( x ),0)

/*
==================================================================================

	description : Restrain Out of Order Execution

==================================================================================
*/
#define	BARRIER( )		ASM("":::"memory")

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
 Funtion	:container_of
 Input		:pointer
		 < pointer to the member of the struct >
		 type
		 < type of the struct >
		 member
		 < member of the struct >
 Output		:void
 Return		:pointer to the struct entry which contains a specified member
 Description	:get the address of struct entry from its member
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	container_of(pointer, type, member)					\
(										\
{										\
	const typeof(((type*)0)->member) *_m_pointer = (pointer);		\
	(type*)((char*)_m_pointer - offsetof(type, member));			\
}										\
)

#endif	// __COMPILER_H__
