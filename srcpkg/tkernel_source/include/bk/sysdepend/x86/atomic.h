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

#ifndef	__BK_SYSDEP_X86_ATOMIC_H__
#define	__BK_SYSDEP_X86_ATOMIC_H__

#include <stdint.h>
#include <bk/typedef.h>

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
#define	ACCESS_ONCE(x)	(*(volatile typeof(x) *)&(x))
//#define	LOCK_PREFIX	"lock"
#define	LOCK_PREFIX



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
 Funtion	:atomic_init
 Input		:value
 Output		:void
 Return		:void
 Description	:initialize atomic value
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	atomic_init(value)	{ (value) }

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:atomic_read
 Input		:const atomic_t *v
 		 < pointer to counter >
 Output		:void
 Return		:int
 		 < read value >
 Description	:atomically read a value
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
static ALWAYS_INLINE int atomic_read(const atomic_t *v)
{
	return(ACCESS_ONCE((v)->counter));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:atomic_write
 Input		:atomic_t *v
 		 < pointer to counter >
 		 int i
 		 < value to write >
 Output		:atomic_t *v
 		 < pointer to counter >
 Return		:void
 Description	:atomically write a value
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
static ALWAYS_INLINE void atomic_write(atomic_t *v, int i)
{
	v->counter = i;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:atomic_add
 Input		:int i
 		 < value to add >
 		 atomic_t *v
 		 < pointer to counter >
 Output		:atomic_t *v
 		 < pointer to counter >
 Return		:void
 Description	:atomically add a value
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
static ALWAYS_INLINE void atomic_add(int i, atomic_t *v)
{
	ASM (
		LOCK_PREFIX ";"
		"addl	%[value], %[counter]		\n\t"
		: [counter]"+m"(v->counter)
		: [value]"ir"(i)
		:
	);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:atomic_sub
 Input		:int i
 		 < value to subtract >
 		 atomic_t *v
 		 < pointer to counter >
 Output		:atomic_t *v
 		 < pointer to counter >
 Return		:void
 Description	:atomically subtract a value
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
static ALWAYS_INLINE void atomic_sub(int i, atomic_t *v)
{
	ASM(
		LOCK_PREFIX ";"
		"subl	%[value], %[counter]		\n\t"
		: [counter]"+m"(v->counter)
		: [value]"ir"(i)
		:
	);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:atomic_inc
 Input		:atomic_t *v
 		 < pointer to counter >
 Output		:atomic_t *v
 		 < pointer to counter >
 Return		:void
 Description	:atomically increment a value
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
static ALWAYS_INLINE void atomic_inc(atomic_t *v)
{
	ASM(
		LOCK_PREFIX ";"
		"incl	%[counter]			\n\t"
		: [counter]"+m"(v->counter)
		:
		:
	);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:atomic_inc_and_test
 Input		:atomic_t *v
 		 < pointer to counter >
 Output		:atomic_t *v
 		 < pointer to counter >
 Return		:int
 		 < boolean result >
 Description	:atomically increment a value and test it
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
static ALWAYS_INLINE int atomic_inc_and_test(atomic_t *v)
{
	asm goto(
		LOCK_PREFIX ";"
		"incl	%[counter]			\n\t"
		"je	%l[result]			\n\t"
		: 
		: [counter]"m"(v->counter)
		: "memory"
		: result
	);
	return 0;
result:
	return 1;
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:atomic_dec
 Input		:atomic_t *v
 		 < pointer to counter >
 Output		:atomic_t *v
 		 < pointer to counter >
 Return		:void
 Description	:atomically decrement a value
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
static ALWAYS_INLINE void atomic_dec(atomic_t *v)
{
	ASM(
		LOCK_PREFIX ";"
		"decl	%[counter]			\n\t"
		: [counter]"+m"(v->counter)
		:
		:
	);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:atomic_sub_and_test
 Input		:int i
 		 < value to subtract >
 		 atomic_t *v
 		 < pointer to counter >
 Output		:atomic_t *v
 		 < pointer to counter >
 Return		:int
 		 < boolean result >
 Description	:atomically subtract a value and test it
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
static ALWAYS_INLINE int atomic_sub_and_test(int i, atomic_t *v)
{
	asm goto(
		LOCK_PREFIX ";"
		"subl	%[value], %[counter]		\n\t"
		"je	%l[result]			\n\t"
		: 
		: [value]"ir"(i), [counter]"m"(v->counter)
		: "memory"
		: result
	);
	return 0;
result:
	return 1;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:atomic_dec_and_test
 Input		:atomic_t *v
 		 < pointer to counter >
 Output		:atomic_t *v
 		 < pointer to counter >
 Return		:int
 		 < boolean result >
 Description	:atomically decrement a value and test it
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
static ALWAYS_INLINE int atomic_dec_and_test(atomic_t *v)
{
	asm goto(
		LOCK_PREFIX ";"
		"decl	%[counter]			\n\t"
		"je	%l[result]			\n\t"
		: 
		: [counter]"m"(v->counter)
		: "memory"
		: result
	);
	return 0;
result:
	return 1;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:cmpxchg
 Input		:uint32 *ptr
 		 < pointer to excange >
 		 uint32 old
 		 < old value >
 		 uint32 new
 		 < new value >
 Output		:void
 Return		:void
 Description	:atomically compare and exchange for 32 bit value
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	cmpxchg(ptr, old, new)							\
({										\
	volatile uint32_t	*__ptr = (volatile uint32_t *)(ptr);		\
	uint32_t		__ret;						\
	asm volatile (								\
		LOCK_PREFIX ";"							\
		"cmpxchgl	%2, %1			\n\t"			\
		: "=a"(__ret), "+m"(*__ptr)					\
		: "r"(new), "0"(old)						\
		: "memory"							\
	);									\
	__ret;									\
})

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	// __BK_SYSDEP_X86_ATOMIC_H__
