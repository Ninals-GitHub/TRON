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

#ifndef	__BK_UAPI_FUTEXT_H__
#define	__BK_UAPI_FUTEXT_H__

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
----------------------------------------------------------------------------------
	futex command
----------------------------------------------------------------------------------
*/
#define	FUTEX_WAIT			0
#define	FUTEX_WAKE			1
#define	FUTEX_FD			2	/* no used			*/
#define	FUTEX_REQUEUE			3
#define	FUTEX_CMP_REQUEUE		4
#define	FUTEX_WAKE_OP			5	/* no used			*/
#define	FUTEX_LOCK_PI			6
#define	FUTEX_UNLOCK_PI			7
#define	FUTEX_TRYLOCK_PI		8
#define	FUTEX_WAIT_BITSET		9
#define	FUTEX_WAKE_BITSET		10
#define	FUTEX_WAIT_REQUEUE_PI		11
#define	FUTEX_CMP_REQUEUE_PI		12

#define	FUTEX_PRIVATE_FLAG		128
#define	FUTEX_CLOCK_REALTIME		256
#define	FUTEX_CMD_MASK			~(FUTEX_PRIVATE_FLAG | FUTEX_CLOCK_REALTIME)


#define	FUTEX_WAIT_PRIVATE		(FUTEX_WAIT | FUTEX_PRIVATE_FLAG)
#define	FUTEX_WAKE_PRIVATE		(FUTEX_WAKE | FUTEX_PRIVATE_FLAG)
#define	FUTEX_REQUEUE_PRIVATE		(FUTEX_REQUEUE | FUTEX_PRIVATE_FLAG)
#define	FUTEX_CMP_REQUEUE_PRIVATE	(FUTEX_CMP_REQUEUE | FUTEX_PRIVATE_FLAG)
#define	FUTEX_WAKE_OP_PRIVATE		(FUTEX_WAKE_OP | FUTEX_PRIVATE_FLAG)
#define	FUTEX_LOCK_PI_PRIVATE		(FUTEX_LOCK_PI | FUTEX_PRIVATE_FLAG)
#define	FUTEX_UNLOCK_PI_PRIVATE		(FUTEX_UNLOCK_PI | FUTEX_PRIVATE_FLAG)
#define	FUTEX_TRYLOCK_PI_PRIVATE	(FUTEX_TRYLOCK_PI | FUTEX_PRIVATE_FLAG)
#define	FUTEX_WAIT_BITSET_PRIVATE	(FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG)
#define	FUTEX_WAIT_REQUEUE_PI_PRIVATE	(FUTEX_WAIT_REQUEUE_PI | FUTEX_PRIVATE_FLAG)
#define	FUTEX_CMP_REQUEUE_PI_PRIVATE	(FUTEX_CMP_REQUEUE_PI | FUTEX_PRIVATE_FLAG)

/*
----------------------------------------------------------------------------------
	futex bitset
----------------------------------------------------------------------------------
*/
#define	FUTEX_BITSET_MATCH_ANY		0xFFFFFFFF

#define	FUTEX_OP_SET			0	/* *(int *)UADDR2 = OPARG;	*/
#define	FUTEX_OP_ADD			1	/* *(int *)UADDR2+= OPARG;	*/
#define	FUTEX_OP_OR			2	/* *(int *)UADDR2|= OPARG;	*/
#define	FUTEX_OP_ANDN			3	/* *(int *)UADDR2&=~OPARG;	*/
#define	FUTEX_OP_XOR			4	/* *(int *)UADDR2^= OPARG;	*/

#define	FUTEX_OP_OPARG_SHIFT		8	/* use (1 << OPARG) instead of OPARG*/

#define	FUTEX_OP_CMP_EQ			0	/* if (oldval == CMPARG) wake	*/
#define	FUTEX_OP_CMP_NE			1	/* if (oldval != CMPARG) wake	*/
#define	FUTEX_OP_CMP_LT			2	/* if (oldval <  CMPARG) wake	*/
#define	FUTEX_OP_CMP_LE			3	/* if (oldval <= CMPARG) wake	*/
#define	FUTEX_OP_CMP_GT			4	/* if (oldval >  CMPARG) wake	*/
#define	FUTEX_OP_CMP_GE			5	/* if (oldval >= CMPARG) wake	*/

#define	FUTEX_OP(op, oparg, cmp, cmparg)					\
	(((op & 0xF) << 28) | ((cmp & 0xF) << 24)				\
	 | ((oparg & 0xFFFF) << 12 | (cmparg & 0xFFF))


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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	// __BK_UAPI_FUTEXT_H__
