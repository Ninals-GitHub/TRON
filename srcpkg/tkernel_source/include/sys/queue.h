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
 *	@(#)queue.h (sys)
 *
 *	Queuing operation
 */

#ifndef	__SYS_QUEUE_H__
#define __SYS_QUEUE_H__

#include <basic.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Double-link queue (ring)
 */
typedef struct queue {
	struct queue	*next;
	struct queue	*prev;
} QUEUE;

/*
 * Queue initialization
 */
Inline void QueInit( QUEUE *que )
{
	que->next = (struct queue *)que;
	que->prev = (struct queue *)que;
}

/*
 * TRUE if the queue is empty
 */
Inline BOOL isQueEmpty( QUEUE *que )
{
	return ( que->next == que )? TRUE: FALSE;
}

/*
 * Insert in queue
 *	Inserts entry directly prior to que
 */
Inline void QueInsert( QUEUE *entry, QUEUE *que )
{
	entry->prev = (struct queue*) que->prev;
	entry->next = que;
	que->prev->next = entry;
	que->prev = entry;
}

/*
 * Delete from queue
 *	Deletes entry from queue
 *	No action is performed if entry is empty.
 */
Inline void QueRemove( QUEUE *entry )
{
	if ( entry->next != entry ) {
		entry->prev->next = (struct queue*) entry->next;
		entry->next->prev = (struct queue*) entry->prev;
	}
}

/*
 * Remove top entry
 *	Deletes the entry directly after que from the queue,
 *	and returns the deleted entry.
 *	Returns NULL if que is empty.
 */
Inline QUEUE* QueRemoveNext( QUEUE *que )
{
	QUEUE	*entry;

	if ( que->next == que ) {
		return NULL;
	}

	entry = que->next;
	que->next = (struct queue*)entry->next;
	entry->next->prev = que;

	return entry;
}

/*
 * Queue search
 *	Searches entries in the forward direction from start->next
 *	to end->prev.
 *	Compares the value at the offset position with val, and returns
 *	matching entries.
 *	Returns end if no suitable entries are found.
 *	start and end are not included in the search.
 *
 *	QueSearch [conditions][data type](start, end, val, offset)
 *
 *	Conditions
 *	None	offset position value = val
 *	NE	offset position value != val
 *	GT	offset position value > val
 *	GE	offset position value >= val
 *	LT	offset position value < val
 *
 *	Data type
 *	None	W
 *	U	UW
 *	H	H
 *	UB	UB
 *
 *	for ( que = start->next; que != end; que = que->next ) {
 *		if ( *(data type*)((VB*)que + offset) conditions val ) {
 *			break;
 *		}
 *	}
 *	return que;
 */
IMPORT QUEUE* QueSearch    ( QUEUE *start, QUEUE *end, W  val, W offset );
IMPORT QUEUE* QueSearchH   ( QUEUE *start, QUEUE *end, H  val, W offset );
IMPORT QUEUE* QueSearchNE  ( QUEUE *start, QUEUE *end, W  val, W offset );
IMPORT QUEUE* QueSearchNEH ( QUEUE *start, QUEUE *end, H  val, W offset );
IMPORT QUEUE* QueSearchGT  ( QUEUE *start, QUEUE *end, W  val, W offset );
IMPORT QUEUE* QueSearchGTUB( QUEUE *start, QUEUE *end, UB val, W offset );
IMPORT QUEUE* QueSearchGE  ( QUEUE *start, QUEUE *end, W  val, W offset );
IMPORT QUEUE* QueSearchGEU ( QUEUE *start, QUEUE *end, UW val, W offset );

/*
 * Queue search (reverse order)
 *	Searches entries in the reverse direction from start->prev to
 *	end->next.
 *	Otherwise identical to QueSearch.
 *	QueSearchRev[conditions][data type](start, end, val, offset)
 *
 *	for ( que = start->prev; que != end; que = que->prev ) {
 *		if ( *(data type*)((VB*)que + offset) conditions val ) {
 *			break;
 *		}
 *	}
 *	return que;
 */
IMPORT QUEUE* QueSearchRevLTU( QUEUE *start, QUEUE *end, UW val, W offset );

#ifdef __cplusplus
}
#endif
#endif /* __SYS_QUEUE_H__ */
