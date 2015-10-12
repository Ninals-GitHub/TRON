/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/09/22
 *
 *----------------------------------------------------------------------
 */

/*
 *	memory.c
 *
 *       memory & I/O access processing
 */

#include "cmdsvc.h"

/* multi-type memory pointer */
typedef union {
	UW	a;
	UW	*w;
	UH	*h;
	UB	*b;
} MP;

/*
        read memory
*/
EXPORT	W	readMem(UW addr, void *buf, W len, W unit)
{
	W	i, n, alen;
	MP	pa, bp;

        // address misalignment is reported as error
	if (addr & (unit - 1)) return 0;

	bp.w = buf;
	for (alen = 0; alen < len; alen += i) {

                // memory address check & conversion to physical address
		n = chkMemAddr(addr + alen, &pa.a, len - alen, 0);
		if (n < unit) break;	// illegal address

		i = 0;
		switch(unit) {
		case 4:
			for (; i < n; i += 4) *bp.w++ = rd_w(pa.w++);
			break;
		case 2:
			for (; i < n; i += 2) *bp.h++ = rd_h(pa.h++);
			break;
		default:
			for (; i < n; i++) *bp.b++ = rd_b(pa.b++);
		}
	}
	return alen;
}
/*
        write memory
                unit & 0x10 : fill
*/
EXPORT	W	writeMem(UW addr, void *buf, W len, W unit)
{
	W	i, n, sz, alen;
	MP	pa, bp;

        // address misalignment is reported as error
	sz = unit & 0x0f;
	if (addr & (sz - 1)) return 0;

	bp.w = buf;
	for (alen = 0; alen < len; alen += i) {

                // memory address check & conversion to physical address
		n = chkMemAddr(addr + alen, &pa.a, len - alen, 1);
		if (n < sz) break;	// illegal address

		i = 0;
		switch(sz) {
		case 4:
			if (unit & 0x10) {
#if TEF_EM1D
				for (; i < n; i += 4) wr_w(pa.w++, *bp.w);
#endif
			} else {
#if TEF_EM1D
				for (; i < n; i += 4) wr_w(pa.w++, *bp.w++);
#endif
			}
			break;
		case 2:
			if (unit & 0x10) {
#if TEF_EM1D
				for (; i < n; i += 2) wr_h(pa.h++, *bp.h);
#endif
			 } else {
#if TEF_EM1D
				for (; i < n; i += 2) wr_h(pa.h++, *bp.h++);
#endif
			}
			break;
		default:
			if (unit & 0x10) {
#if TEF_EM1D
				for (; i < n; i++) wr_b(pa.b++, *bp.b);
#endif
			} else {
#if TEF_EM1D
				for (; i < n; i++) wr_b(pa.b++, *bp.b++);
#endif
			}
		}
	}
	return alen;
}
/*
        read character string
*/
EXPORT	W	readMemStr(UW addr, void *buf, W len)
{
	W	i, n, alen;
	UW	pa;

	for (alen = 0; alen < len; alen += i) {
                // memory address check & conversion to physical address
		n = chkMemAddr(addr + alen, &pa, len - alen, 0);
		if (n == 0) break;	// illegal address
		for (i = 0; i < n; i++, buf++, pa++) {
			if ((*(UB*)buf = rd_b((UB*)pa)) == 0) return alen + i;
		}
	}
	return -1;
}
/*
        I/O read
*/
EXPORT	W	readIO(UW addr, UW *data, W unit)
{
	W	n;
	UW	pa;

        // address misalignment is reported as error
	if (addr & (unit - 1)) return 0;

        // I/O address check & conversion to physical address
	n = chkIOAddr(addr, &pa, unit);
	if (n < unit) return 0;

#if TEF_EM1D
	switch(unit) {
	case 4:		*data = in_w(pa);	break;
	case 2:		*data = in_h(pa);	break;
	default:	*data = in_b(pa);
	}
#endif
	return unit;
}
/*
        I/O write
*/
EXPORT	W	writeIO(UW addr, UW data, W unit)
{
	W	n;
	UW	pa;

        // address misalignment is reported as error
	if (addr & (unit - 1)) return 0;

        // I/O address check & conversion to physical address
	n = chkIOAddr(addr, &pa, unit);
	if (n < unit) return 0;

#if TEF_EM1D
	switch(unit) {
	case 4:		out_w(pa, data);	break;
	case 2:		out_h(pa, data);	break;
	default:	out_b(pa, data);
	}
#endif
	return unit;
}
