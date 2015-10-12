/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *
 *----------------------------------------------------------------------
 */

/*
 *	quetemplate.c (common)
 */

#include <sys/queue.h>

#define	QUESEARCH(NAME, TYPE, COND)					\
QUEUE* QueSearch##NAME( QUEUE *start, QUEUE *end, TYPE val, W offset )	\
{									\
	QUEUE	*que;							\
	for ( que = start->next; que != end; que = que->next ) {	\
		if ( *(TYPE*)((VB*)que + offset) COND val ) {	\
			break;	\
		}	\
	}								\
	return que;							\
}

#define	QUESEARCHREV(NAME, TYPE, COND)					\
QUEUE* QueSearchRev##NAME( QUEUE *start, QUEUE *end, TYPE val, W offset )\
{									\
	QUEUE	*que;							\
	for ( que = start->prev; que != end; que = que->prev ) {	\
		if ( *(TYPE*)((VB*)que + offset) COND val ) {	\
			break;	\
		}	\
	}								\
	return que;							\
}
