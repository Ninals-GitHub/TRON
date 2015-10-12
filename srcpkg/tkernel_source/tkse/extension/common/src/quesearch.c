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
 *	quesearch.c (common)
 */

#include "quetemplate.h"

/*EXPORT QUESEARCH(, W, ==)*/
QUEUE* QueSearch( QUEUE *start, QUEUE *end, W val, W offset )
{
	QUEUE	*que;
	for ( que = start->next; que != end; que = que->next ) {
		if ( *(W*)((VB*)que + offset) == val ) {
			break;
		}
	}
	return que;
}
