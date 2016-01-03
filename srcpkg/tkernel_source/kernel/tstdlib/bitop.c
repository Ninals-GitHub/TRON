/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */

/*
 *	bitop.c (tstdlib)
 *	T-Kernel common standard library
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <tstdlib/bitop.h>

/*** macros ***/
/* bit operation macro */
#if BIGENDIAN
#define _BIT_SET_N(n) ( (UB)0x80 >> ((n) & 7) )
#define _BIT_SHIFT(n) ( (UB)n >> 1 )
#else
#define _BIT_SET_N(n) ( (UB)0x01 << ((n) & 7) )
#define _BIT_SHIFT(n) ( (UB)n << 1 )
#define _BIT_SET_LONG(n) ( 1UL << ((n) & (32 - 1)))
#define _BIT_SHIFT_LONG(n) ( (unsigned long)n << 1 )
#endif

/*** bit operation ***/
/* tstdlib_bitclr : clear specified bit */
BOOL
tstdlib_bitclr( void *base, W offset )
{
	register UB *cp, mask;
	register UB old;
	
	if (offset < 0) {
		return(FALSE);
	}

	cp = (UB*)base;
	cp += offset / 8;

	mask = _BIT_SET_N(offset);

	old = *cp & mask;
	*cp &= ~mask;
	
	return((!old)? FALSE:TRUE);
}

/* tstdlib_bitset : set specified bit */
BOOL
tstdlib_bitset( void *base, W offset )
{
	register UB *cp, mask;
	register UB old;
	
	if (offset < 0) {
		return(FALSE);
	}

	cp = (UB*)base;
	cp += offset / 8;

	mask = _BIT_SET_N(offset);
	
	old = *cp & mask;
	*cp |= mask;
	
	return((!old)? FALSE:TRUE);
}

/* tstdlib_bitset : set specified bit */
void
tstdlib_bitset_window( void *base, W offset, int window_len )
{
	register UB *cp, mask;
	
	if ((offset < 0) || (window_len < 0 )) {
		return;
	}

	cp = (UB*)base;
	cp += offset / 8;

	mask = _BIT_SET_N(offset);
	
	while(window_len--) {
		*cp |= mask;
		if (mask == _BIT_SET_N(7)) {
			mask = _BIT_SET_N(0);
			cp++;
		} else {
			mask = _BIT_SHIFT(mask);
		}
	}
}

/* tstdlib_bittest : check specified bit */
BOOL
tstdlib_bittest( void *base, W offset )
{
	register UB *cp, mask;
	
	if (offset < 0) {
		return FALSE;
	}

	cp = (UB*)base;
	cp += offset / 8;

	mask = _BIT_SET_N(offset);

	return (BOOL)(*cp & mask);
}

/* tstdlib_bitsearch0 : perform 0 search on bit string */
W
tstdlib_bitsearch0( void *base, W offset, W width )
{
	register UB *cp, mask;
	register W position;

	if ((offset < 0) || (width < 0)) {
		return -1;
	}

	cp = (UB*)base;
	cp += offset / 8;

	position = 0;
	mask = _BIT_SET_N(offset);

	while (position < width) {
		if (*cp != 0xFF) {	/* includes 0 --> search bit of 0 */
			while (1) {
				if (!(*cp & mask)) {
					if (position < width) {
						return position;
					} else {
						return -1;
					}
				}
				mask = _BIT_SHIFT(mask);
				++position;
			}
		} else {		/* all bits are 1 --> 1 Byte skip */
			if (position) {
				position += 8;
			} else {
				position = 8 - (offset & 7);
				mask = _BIT_SET_N(0);
			}
			cp++;
		}
	}

	return -1;
}

/* tstdlib_bitsearch0 : perform 0 search on bit string and set*/
BOOL
tstdlib_bitsearch0_set( void *base, W offset, W width )
{
	register UB *cp, mask;
	register W position;

	if ((offset < 0) || (width < 0)) {
		return -1;
	}

	cp = (UB*)base;
	cp += offset / 8;

	position = 0;
	mask = _BIT_SET_N(offset);

	while (position < width) {
		if (*cp != 0xFF) {	/* includes 0 --> search bit of 0 */
			while (1) {
				if (!(*cp & mask)) {
					if (position < width) {
						int old;
						old = (*cp & mask);
						*cp |= mask;
						return((BOOL)old);
					} else {
						return(FALSE);
					}
				}
				mask = _BIT_SHIFT(mask);
				++position;
			}
		} else {		/* all bits are 1 --> 1 Byte skip */
			if (position) {
				position += 8;
			} else {
				position = 8 - (offset & 7);
				mask = _BIT_SET_N(0);
			}
			cp++;
		}
	}

	return(FALSE);
}


/* tstdlib_bitsearch0 : perform 0 search on bit string and set based on window*/
long
tstdlib_bitsearch0_window( void *base, W width, int window_len )
{
	register unsigned long *cp, mask;
	register long position;
	long long first = 0;
	long len = 0;

	if ((width < 0) || (window_len < 0)) {
		return -1;
	}

	cp = (unsigned long*)base;

	position = 0;
	mask = _BIT_SET_LONG(0);

	while (position < width) {
		if (*cp != ~0UL) {	/* includes 0 --> search bit of 0 */
			while (1) {
				if (!(*cp & mask)) {
					if (position < width) {
						if (!len) {
							first = position;
						}
						if (len < window_len) {
							len++;
						} else {
							return(first);
						}
					} else {
						return -1;
					}
				} else {
					len = 0;
				}
				if (mask == (_BIT_SHIFT_LONG(sizeof(unsigned long) * 8 - 1))) {
					mask = _BIT_SET_LONG(0);
					++position;
					break;
				}
				mask = _BIT_SHIFT_LONG(mask);
				++position;
			}
		} else {
			mask = _BIT_SET_LONG(0);
			position += sizeof(unsigned long) * 8;
			cp++;
		}
	}

	return(-1);
}

/* tstdlib_bitsearch1 : perform 1 search on bit string */
W
tstdlib_bitsearch1( void *base, W offset, W width )
{
	register UB *cp, mask;
	register W position;

	if ((offset < 0) || (width < 0)) {
		return -1;
	}

	cp = (UB*)base;
	cp += offset / 8;

	position = 0;
	mask = _BIT_SET_N(offset);

	while (position < width) {
		if (*cp) {		/* includes 1 --> search bit of 1 */
			while (1) {
				if (*cp & mask) {
					if (position < width) {
						return position;
					} else {
						return -1;
					}
				}
				mask = _BIT_SHIFT(mask);
				++position;
			}
		} else {		/* all bits are 0 --> 1 Byte skip */
			if (position) {
				position += 8;
			} else {
				position = 8 - (offset & 7);
				mask = _BIT_SET_N(0);
			}
			cp++;
		}
	}

	return -1;
}

/*
 * Faster version using binary search within a word.
 * (it works only with correct alignment of `base', and so it is
 *  converted to (UW*).)
 */
W tstdlib_bitsearch1_binsearch(UW* base, W offset, W width)
{
    register UW *cp, v;
    register W position;

    if ((offset < 0) || (width < 0)) {
        return -1;
    }

    cp = base;
    cp += offset / 32;
    if (offset & 31) {
#if BIGENDIAN
        v = *cp & (((UW)1 << (32 - (offset & 31))) - 1);
#else
        v = *cp & ~(((UW)1 << (offset & 31)) - 1);
#endif
    } else {
        v = *cp;
    }

    position = 0;
    while (position < width) {
        if (v) {            /* includes 1 --> search bit of 1 */
            if (!position) position -= (offset & 31);
#if BIGENDIAN
            if (!(v & 0xffff0000)) {
                v <<= 16;
                position += 16;
            }
            if (!(v & 0xff000000)) {
                v <<= 8;
                position += 8;
            }
            if (!(v & 0xf0000000)) {
                v <<= 4;
                position += 4;
            }
            if (!(v & 0xc0000000)) {
                v <<= 2;
                position += 2;
            }
            if (!(v & 0x80000000)) {
                ++position;
            }
#else
            if (!(v & 0xffff)) {
                v >>= 16;
                position += 16;
            }
            if (!(v & 0xff)) {
                v >>= 8;
                position += 8;
            }
            if (!(v & 0xf)) {
                v >>= 4;
                position += 4;
            }
            if (!(v & 0x3)) {
                v >>= 2;
                position += 2;
            }
            if (!(v & 0x1)) {
                ++position;
            }
#endif
            if (position < width) {
                return position;
            } else {
                return -1;
            }
        } else {              /* all bits are 0 --> 1 Word skip */
            if (position) {
                position += 32;
            } else {
                position = 32 - (offset & 31);
            }
            v = *++cp;
        }
    }

    return -1;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:bit_value
 Input		:uint32_t value
 		 < bit value >
 		 uint32_t start
 		 < start bit to extract >
 		 uint32_t end
 		 < end bit to extract >
 Output		:void
 Return		:uint32_t
 		 < extracted value from bit value >
 Description	:extract specified bit value
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT
uint32_t bit_value(uint32_t value, uint32_t start, uint32_t end)
{
	if (UNLIKELY(end <= start)) {
		if ( end == start ) {
			return(value & MAKE_BIT32(start));
		}
		return(0);
	} else {
		return((value >> start ) & MAKE_MASK_SHIFT32(start, end));
	}
}
