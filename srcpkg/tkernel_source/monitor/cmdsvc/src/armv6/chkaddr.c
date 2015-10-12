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
 *	chkaddr.c
 *
 *       Check address
 */

#include "../cmdsvc.h"

LOCAL	UW	validLA;		// valid logical start address
LOCAL	UW	validSz;		// valid logical address size
LOCAL	UW	mmuStat;		// MMU state

/*
        Initialize address check data (executed upon monitor entry)
*/
EXPORT	void	initChkAddr(void)
{
	validLA = validSz = 0;		// clear the previous effective addresses
	mmuStat = getCP15(1, 0);	// MMU state
}
/*
        Check memory address
                return  contiguous range <= len  (0 means illegal value)
                * pa     physical address to access
*/
EXPORT	W	chkMemAddr(UW addr, UW *pa, W len, W rw)
{
	const MEMSEG *mp;
	UW	n;

	if (mmuStat & 0x1) {	// MMU is enabled
                // if the prevous check range doesn't include the address,
                // if the address is a valid existing address is checked by looking at page table.
		if (addr < validLA || addr >= validLA + validSz) {
			UW pte, *ppte;

                        // Depending on the valid rage of TTBR0 described in TTBCR
                        // TTBR0/TTBR1(=TopPageTable) is switched
			pte  = 0xfe000000 << (7 - (getCP15(2, 2) & 0x07));
								// TTBCR
			ppte = (addr & pte) ? TopPageTable :
				(UW *)(getCP15(2, 0) & ~0x7f);	// TTBR0
			pte = ppte[addr >> 20];

			validSz = 0;
			switch(pte & 0x3) {
			case 0x2:	// Section Entry
				pte &= 0xFFF00000;	// Section Address
				if (rw && AddrMatchMemArea(pte,
						MSA_ROM|MSA_FROM) != NULL)
					errinfo = E_ROM;
				else	validSz = 0x100000;	// 1 MB
				break;
			case 0x1:	// Page Table Entry
				pte &= 0xFFFFFC00;	// Page Table Address
				pte = *((UW*)(pte + ((addr >>(12-2))& 0x3FC)));
				switch(pte & 0x3) {
				case 0x1:	// Large Page : 16 KB x 4
					validSz = 0x10000;	// 64 KB
					break;
				case 0x2:	// Small Page : 1 KB x 4
				case 0x3:	// Small Page with XN
					validSz = 0x1000;	// 4 KB
					break;
				}
				break;
			case 0x3:	// Fine Page Table Entry
				break;	// unsupported
			}
			validLA = (validSz) ? (addr & ~(validSz - 1)) : 0;
		}

		n = (validSz) ? (validLA + validSz - addr) : 0;

	} else {	// MMU is disabled, the unmodified address is used
		mp = AddrMatchMemArea(addr, MSA_HW);
		if ( mp != NULL ) {
			if ( rw && (mp->attr & (MSA_ROM|MSA_FROM)) != 0 ) {
				n = 0;
				errinfo = E_ROM;
			} else {
				n = mp->end - addr;
			}
		} else {
			n = 0;
		}
	}
	*pa = addr;	// access by logical address
	return (len > n) ? n : len;
}
/*
        I/O address check & conversion to physical address
                return  contiguous range <= len  (0 means illegal value)
                * pa     I/O address to access
*/
EXPORT	W	chkIOAddr(UW addr, UW *pa, W len)
{
	const MEMSEG *mp;
	UW	n;

	mp = AddrMatchMemArea(addr, MSA_IO);
	n = ( mp != NULL )? mp->end - addr: 0;

	*pa = addr;	// access by logical address
	return (len > n) ? n : len;
}
/*
        Validate PC
                return 0: OK, -1: illegal
*/
EXPORT	W	invalidPC(UW addr)
{
        // memory range check is not performed
        // an odd address needs to be regarded as THUMB, and so nothing is done here.
	return 0;
}
EXPORT	W	invalidPC2(UW addr)
{
        // memory range check is not performed
        // PC of an ARM instruction is always on WORD-boundary
	return (addr & 0x03) ? -1 : 0;
}
