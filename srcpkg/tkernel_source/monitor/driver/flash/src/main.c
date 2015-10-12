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
 *	main.c
 *
 *       Flash ROM write processing
 */

#include "flash.h"

IMPORT B __flashwr_org[], __flashwr_start[], __flashwr_end[];

/*
 * Flash ROM sector erase / write
 *       addr    Flash ROM write start address (must be on a sector boundary)
 *       data    write data start address (RAM)
 *       nsec    number of sectors to write
 *       msg      0 : no message display  no verify write
 *                1 : message display    with verify write
 *               -1 : message display no verify write
 *       return value error code
 */
EXPORT ER writeFrom( UW addr, UW data, W nsec, W msg )
{
	ER (* volatile flashwr_p)( UW addr, void *data, W nsec, BOOL reset );
	BOOL	reset;
	W	sz;
	UB	c;
	ER	er;

        /* parameter check */
	if ( nsec <= 0 ) return E_PAR;
	if ( (addr & (FROM_SECSZ-1)) != 0 ) return E_PAR;
	sz = FROM_SECSZ * nsec;
	if ( !inMemArea(addr, addr + sz, MSA_FROM) ) return E_PAR;
	if ( !inMemArea(data, data + sz, MSA_OS|MSA_ERAM|MSA_WRK) )
							return E_PAR;

        /* check the write-protect status */
	er = flashwr_protect(addr, nsec);
	if ( er < E_OK ) return er;

        /* Confirm if monitor itself is to be written */
	reset = isOverlapMemArea(addr, addr + sz, MSA_MON);
	if ( reset && msg != 0 ) {
		DSP_S("Update Monitor Area, ");
		if (msg > 0) {
			DSP_S("OK (y/n)? ");
			c = (UB)getChar(1);
			putChar(c);
			putChar('\n');
			if (c != 'Y' && c != 'y') return E_OK;
		} else if (msg < 0) {
			DSP_S("Restart System after Writing.");
		}
	}

        /* initial set-up before Flash ROM write */
	flashwr_setup(reset);

	if ( __flashwr_start != __flashwr_org ) {
                /* transfer flashwr() to RAM area */
		memcpy(__flashwr_start, __flashwr_org,
					__flashwr_end - __flashwr_start);
	}

        /* Flash ROM sector erase / write
         *       flashwr() is executed in RAM area.
         *        if reset = TRUE, doesn't return.
         *       The offset is too large, and so we use indirect address.
	 */
	flashwr_p = &flashwr;
	er = (*flashwr_p)(addr, (UH*)data, nsec, reset);

        /* restore after Flash ROM write */
	flashwr_done();

	return er;
}

/*
 * set up Flash ROM loading processing
 *       mode     0 : set up for loading write data
 *               -1 : set up for writing already loaded data
 *
 *       in the case of setting up loading (mode= 0)
 *         addr returns the following value.
 *           addr[0]  the start address in RAM area for loading data
 *           addr[1]  the end address in RAM area for loading data
 *               addr[1] - addr[0] + 1 = load area size
 *               in principle, load area size matches the size of FLASH ROM.
 *               But if RAM is small, there may be cases
 *               in which load area size is smaller than that of Flash ROM size.
 *           addr[2]  the distance between the data load RAM area and Flash ROM area
 *               adjustment is made so that the addr[0] position is written to the beginning of Flash ROM.
 *               addr[2] = addr[0] - Flash ROM start address
 *
 *       in the case of setting up for writing (mode = -1),
 *         we set the writing area based on the addr value when we called this function using mode = 0.
 *           addr[0]  starting address of loaded data in RAM area (to be written)
 *            addr[1]  ending address of loaded data in RAM area (to be written)
 *               addr[1] - addr[0] + 1 = size of written data
 *           addr[2]  the value remains the same after it was set by mode = 0 (ignored)
 *         the modified values are returned in addr.
 *           addr[0]  Flash ROM write start address
 *           addr[1]  start address of write data in RAM
 *               address will be adjusted to the sector boundary of Flash ROM.
 *           addr[2]  number of sectors to write
 *               Since writing is done in the unit of sectors, the writing will be done from the sector boundary,
 *               areas immediately before and after the designated area may be part of the write operation.
 */
EXPORT void setupFlashLoad( W mode, UW addr[3] )
{
	UW	SECMSK = FROM_SECSZ - 1;
	UW	ofs, sa, romsize, ramtop, ramend;
	const MEMSEG *rom, *ram;

        /* Flash ROM capacity */
	rom = MemArea(MSA_FROM, 1);
	romsize = rom->end - rom->top;

        /* RAM area for writing */
	ram = MemArea(MSA_OS|MSA_WRK, 1);
	ramtop = (ram->top + SECMSK) & ~SECMSK;
	ramend = ram->end & ~SECMSK;

        /* Use the end of RAM area for working area
            if we have enough RAM, we set aside the area as large as the last sector  */
	sa = (ramend - FROM_SECSZ) - romsize;
	if ( sa < ramtop ) sa = ramtop;
	ofs = sa - (UW)rom->top;	/* the distance between the ROM area and RAM work area */

	if ( mode >= 0 ) {
                /* set up loading */
		addr[0] = rom->top + ofs;		/* RAM address lower limit */
		addr[1] = rom->end + ofs - 1;		/* RAM address upper limit */
		addr[2] = ofs;				/* offset */
		if ( addr[1] >= ramend ) addr[1] = ramend - 1;
	} else {
                /* set up writing */
		sa = addr[0] & ~SECMSK;			/* RAM start address */
		addr[2] = ((addr[1] & ~SECMSK) - sa) / FROM_SECSZ + 1;
							/* number of sectors */
		addr[1] = sa;				/* RAM start address */
		addr[0] = sa - ofs;			/* ROM start address */
	}
}
