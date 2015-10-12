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
 *	hwinfo.c
 *
 *       hardware configuration information
 */

#include "hwdepend.h"

/* ------------------------------------------------------------------------ */
/*
 *       memory region definition
 */

/*
 * obtaining memory region information
 *       no = 1 - (and up)
 *       'no'-th information in the region specified by the attr is returned.
 *       if attr = 0, no matter what the attribute is, 'no'-th information is returned unconditionally.
 *       If there was no such information, return NULL.
 */
EXPORT MEMSEG* MemArea( UW attr, W no )
{
	MEMSEG	*mp;
	W	i;

	if ( attr == 0 ) {
		i = no - 1;
		return ( i >= 0 && i < N_MemSeg )? &MemSeg[i]: NULL;
	}

	for ( i = 0; i < N_MemSeg; ++i ) {
		mp = &MemSeg[i];
		if ( (mp->attr & attr) != 0 ) {
			if ( --no <= 0 ) return mp;
		}
	}

	return NULL;
}

/*
 * obtaining memory region information (specify address)
 *       within the region specified by `attr', return the information that surrounds the position specified by `addr'.
 *
 *       if no such information is found, return NULL.
 */
EXPORT MEMSEG* AddrMatchMemArea( UW addr, UW attr )
{
	MEMSEG	*mp;
	W	i;

	for ( i = 0; i < N_MemSeg; ++i ) {
		mp = &MemSeg[i];
		if ( (mp->attr & attr) == 0 ) continue;

		if ( addr >= mp->top && addr <= mp->end-1 ) return mp;
	}

	return NULL;
}

/*
 * Decide whether two memory regions are included in another.
 *      if the region, from `top' to `end', is completely included in the region specified by `attr',
 *      TRUE
 *       the location of end is NOT included in the region (end - top) is the region size
 *       end = 0x00000000, by the way, means 0x100000000.
 */
EXPORT BOOL inMemArea( UW top, UW end, UW attr )
{
	const MEMSEG *mp;
	W	i;

	for ( i = 0; i < N_MemSeg; ++i ) {
		mp = &MemSeg[i];
		if ( (mp->attr & attr) == 0 ) continue;

		if ( top >= mp->top && end-1 <= mp->end-1 ) return TRUE;
	}
	return FALSE;
}

/*
 * Decide whether two memory regions overlap with each other
 *       if the area, from top to end, is included even partially in the region specified by `attr' - 'end',
 *       it is TRUE
 *       the location of end is NOT included in the region (end - top) is the region size
 *       end = 0x00000000, by the way, means 0x100000000.
 */
EXPORT BOOL isOverlapMemArea( UW top, UW end, UW attr )
{
	const MEMSEG *mp;
	W	i;

	for ( i = 0; i < N_MemSeg; ++i ) {
		mp = &MemSeg[i];
		if ( (mp->attr & attr) == 0 ) continue;

		if ( top <= mp->end-1 && end-1 >= mp->top ) return TRUE;
	}
	return FALSE;
}

/* ------------------------------------------------------------------------ */
/*
 * boot device following the standard boot order
 *       return the device name that is the 'no'-th device in the standard boot order.
 *
 *       if no such device name exists (when 'no' is given as a value larger or equal to the last number), it is NULL.
 */
EXPORT const UB* bootDevice( W no )
{
	if ( no < 0 || no >= N_ConfigDisk ) return NULL;

	return ConfigDisk[no].name;
}

/*
 * list of disk drives
 *       returns the disk drive device name, indicated by 'no' ( 0 - : a consecutive number )
 *       if no such device name exists (when 'no' is given as a value larger or equal to the last number), it is NULL.
 *       if attr is not NULL, disk driver attribute returns in `attr' )
 */
EXPORT const UB* diskList( W no, UW *attr )
{
	if ( no < 0 || no >= N_ConfigDisk ) return NULL;

	if ( attr != NULL ) *attr = ConfigDisk[no].attr;

	return ConfigDisk[no].name;
}

/* ------------------------------------------------------------------------ */
