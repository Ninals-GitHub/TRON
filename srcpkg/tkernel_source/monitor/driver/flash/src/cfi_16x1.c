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
 *	cfi_16x1.c
 *
 *       Flash ROM write: CFI(Intel) specification 16bit x 1 configuration
 */

#include "flash.h"

#ifndef	SECSZ
#define	SECSZ		0x20000			// sector size 128kB x 1
#endif

#ifndef	BSECSZ
#define	BSECSZ		0x02000			// boot sector size 8KB x 1
#endif

#define	SECMSK		(SECSZ - 1)		// sector mask

#define	MAX_RETRY	(3)			// maximum number of retries
#define	WAIT_CNT	0x10000000		// wait count (enough time)

#define	WBSZ		(16)			// write buffer size (H unit)

EXPORT	const UW	FROM_SECSZ = SECSZ;	// sector size

/*
 * check Flash ROM
 */
LOCAL	const JEDEC_SPEC	*checkFlashROM(_UH *rom)
{
	const JEDEC_SPEC	*spec;
	UH	man, dev;
	W	i;

        /* read Signature */
	rom[0] = 0x0090;
	man = rom[0] & 0x00ff;	// ignore upper 8 bits
	dev = rom[1] & 0x00ff;	// ignore upper 8 bits

        /* obtain Flash ROM specification */
	for (i = 0; i < N_JedecSpec; ++i) {
		spec = &JedecSpec[i];
		if (spec->man != man || spec->dev != dev) continue;

		return spec;
	}

	return NULL;	// unsupported target
}

/*
 * write one sector
 *       rom should be the beginning of sector
 *       if data = NULL, only erasure is performed
 */
LOCAL	ER	writesec(_UH *rom, UH *data, const JEDEC_SPEC *spec)
{
	_UH	*rp, *xp;
	UH	*dp, *ep;
	UH	d;
	UW	n, mask, ptn;
	W	i;

	mask = (spec->size * (1024*1024) - 1) & ~SECMSK;
	n = (UW)rom & mask;

        /* sector configuration */
	ptn = ( n == 0    )? spec->bsec:	/* bottm sector */
	      ( n == mask )? spec->tsec:	/* top sector */
			     0x8000;		/* other */

        /* erase sector */
	mask = 0x10000;
	for ( rp = rom; (ptn & (mask - 1)) != 0; rp += BSECSZ/sizeof(UH) ) {
		if ( (ptn & (mask >>= 1)) == 0 ) continue;

                /* wait for Ready */
		*rp = 0x0070;
		for (i = WAIT_CNT; --i >= 0 && (*rp & 0x0080) == 0; );
		if (i < 0) return E_IO;

                /* release lock */
		*rp = 0x0060;
		*rp = 0x00D0;

                /* wait for Ready */
		*rp = 0x0070;
		for (i = WAIT_CNT; --i >= 0 && (*rp & 0x0080) == 0; );
		if (i < 0) return E_IO;

                /* erase sector */
		*rp = 0x0020;
		*rp = 0x00D0;

                /* wait for completion of erasure */
		for (i = WAIT_CNT; --i >= 0 && ((d = *rp) & 0x0080) == 0; );
		if (i < 0 || (d & 0x003A) != 0) {
			*rp = 0x0050;		// clear error
			return E_IO;
		}
	}

	if (data == NULL) return E_OK;	// erase only

        /* write (using a buffer) */
	rp = rom;
	ep = data + SECSZ / sizeof(UH);
	for (dp = data; dp < ep; ) {
		xp = rp;
		for (i = WAIT_CNT; --i >= 0; ) {
			*rp = 0x00E8;
			if (*xp & 0x0080) break;	// XSR check
		}
		if (i < 0) goto abort;

		*rp = WBSZ - 1;
		for (i = 0; i < WBSZ; i++) *rp++ = *dp++;
		*xp = 0x00D0;

                /* wait for completion of write */
		// *xp = 0x0070;
		for (i = WAIT_CNT; --i >= 0 && ((d = *xp) & 0x0080) == 0; );
		if (i < 0 || (d & 0x001A) != 0) {
			*xp = 0x0050;	// clear error
			goto abort;
		}
	}

 abort:
        /* write end */
	*rom = 0x00FF;
	if (dp < ep) return E_IO;

        /* Verify write */
	for (dp = data; dp < ep; ) {
		if (*rom ++ != *dp++) return E_IO;
	}

	return E_OK;
}

/*
 * FlashROM write
 */
EXPORT	ER	flashwr(UW addr, void *data, W nsec, BOOL reset)
{
	const JEDEC_SPEC	*spec;
	_UH	*rom;
	W	sec, retry;
	ER	err;

        /* FlashROM sector address */
	rom = (_UH *)NOCACHE_ADDR(addr & ~SECMSK);

        /* check FlashROM */
	spec = checkFlashROM(rom);

        /* reset FlashROM */
	*rom = 0x00FF;

        /* report error for unsupported FlashROM */
	if (spec == NULL) return E_IO;

        /* erase or write in sector unit */
	for (sec = 0; sec < nsec; sec++) {
		retry = MAX_RETRY;

		do {
			err = writesec(rom, data, spec);
			if (err >= E_OK) break;

                        /* firstly, reset */
			*rom = 0x00FF;
		} while (--retry > 0);

		if (err < E_OK) return err;

		rom = (_UH *)((B *)rom + SECSZ);
		data = (B *)data + SECSZ;
	}

	if (reset) flashwr_reset();	// do not return

	return E_OK;
}
