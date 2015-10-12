/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2014/01/08.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
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

/*
 *  @(#)fimp_fat.c
 *
 */

#include "fimp_fat.h"
#include "fimp_fat_local.h"

/*
 *  Configuration information
 */
LOCAL	INT	MaxOpenF		= DEFAULT_MaxOpenF;
LOCAL	INT	TaskPriority		= DEFAULT_TaskPriority;
LOCAL	INT	TaskStackSize		= DEFAULT_TaskStackSize;
LOCAL	INT	CacheFATMemorySize	= DEFAULT_CacheFATMemorySize;
LOCAL	INT	CacheFATRatio		= DEFAULT_CacheFATRatio;
LOCAL	INT	CacheRootMemorySize	= DEFAULT_CacheRootMemorySize;
LOCAL	INT	CacheRootRatio		= DEFAULT_CacheRootRatio;
LOCAL	INT	CacheDataMemorySize	= DEFAULT_CacheDataMemorySize;
LOCAL	INT	CacheDataRatio		= DEFAULT_CacheDataRatio;
LOCAL	INT	LastAccess		= DEFAULT_LastAccess;

/*
 *  FAT FIMP entry
 */
LOCAL	ER	fatfs_service(fimp_t *req);
LOCAL	ER	fatfs_registfn(fimpinf_t *fimpinf, void *exinf);
LOCAL	ER	fatfs_unregistfn(fimpinf_t *fimpinf);
LOCAL	ER	fatfs_attachfn(coninf_t *coninf, void *exinf);
LOCAL	ER	fatfs_detachfn(coninf_t *coninf);
LOCAL	ER	fatfs_breakfn(coninf_t *coninf, ID tskid, BOOL set);

EXPORT	const	fs_fimp_t	fimp_fatfs_entry = {
	fatfs_service,				/* reqfn	*/
	fatfs_registfn,				/* registfn	*/
	fatfs_unregistfn,			/* unregistfn	*/
	fatfs_attachfn,				/* attachfn	*/
	fatfs_detachfn,				/* detachfn	*/
	NULL,					/* startupfn	*/
	NULL,					/* cleanupfn	*/
	fatfs_breakfn,				/* breakfn	*/
	FIMP_FLAG_64BIT | FIMP_FLAG_USEABORT,	/* flags	*/
	0,					/* priority	*/
};

/* Task status table [0 .. MaxTskId - 1] */
LOCAL	TaskSts	*tskStsTab;

/* Check flags or mode */
#define	isSET(flags, value)	(((UW)(flags) & (UW)(value)) != 0)
#define	isFAT_SUBDIR(typ)	isSET(typ, FAT_SUBDIR)
#define	isFAT_VOLUME(typ)	isSET(typ, FAT_VOLUME)
#define	isFAT_RDONLY(typ)	isSET(typ, FAT_RDONLY)
#define	isNOT_RDONLY(md)	(((UW)(md) & O_ACCMODE) != O_RDONLY)
#define	isDEV_RDONLY(flg)	isSET(flg, DEV_FLAG_READONLY)

/* Check dirent */
#define	isDE_TERM(de)		((de)->de_fname[0] == DE_B0_END)
#define	isDE_FREE(de)		((de)->de_fname[0] == DE_B0_FREE)
#define	isDE_LFN(de)		((de)->de_ftype == FAT_LONGNAME)
#define	isDE_VOL(de)		isFAT_VOLUME((de)->de_ftype)

/* Check break done */
#define	CHK_BREAK_AND_SET_ERR(fs, perr)		\
	{ if (((FS*)(fs))->fs_tsksts->c.break_done != 0) {*(perr) = EX_INTR;} }

/* Set second error */
#define	SetErr2(er,er2)		{if ((er) >= E_OK) (er) = (er2);}

/* Directory entry size */
#define DIRENTSZ	(sizeof(FAT_DIRENT))

/* FAT12, 16 Root directory */
#define ROOTENT		(fs->ff_rootent)
#define ROOTSZ		(ROOTENT * DIRENTSZ)

/* Cluster number <-> Block number */
#define CNtoBN(cn)	(fs->ff_clstart + (((cn) - 2) * fs->ff_ratio))
#define BNtoCN(bn)	((((bn) - fs->ff_clstart) / fs->ff_ratio) + 2)

/* Number of dirent per sector / cluster */
#define DEpSEC		((fs->ff_clsz / fs->ff_ratio) / DIRENTSZ)
#define	DEpCL		(fs->ff_clsz / DIRENTSZ)

/* Byte <-> Number of cluster */
#define BYTEtoCL(b)	(((b) + (fs->ff_clsz - 1)) / fs->ff_clsz)

/* Check rootdir inode */
#define IsRootINODE(fs, inode)	\
		(((INODE*)(inode))->ino_ino == ((FS*)(fs))->fs_rootino)

/* Disk sector(block) size */
#define	DSECSZ(fs)	((fs)->ff_c.fs_diskinfo.blocksize)

/* Byte offset corresponding to 'offset' byte of 'secno' in 'page' */
#define OFFSETinPAGE(fs, page, secno, offset)	\
	( ((secno) - (page)->dcp_secno) * DSECSZ(fs) + (offset) % DSECSZ(fs) )

/* is FAT32 ? */
#define	isFAT32(fstype)		((fstype) == FAT32)

/* Generic pointer */
typedef	union {
	UB	*b;
	UH	*h;
	UW	*w;
} GPTR;

/* FAT_DIRENT_LFN : fragmented file name offset & count */
LOCAL	const	struct {
	UB	ofs;
	UB	cnt;
} LDE_NMPOS[3] = {
	{offsetof(FAT_DIRENT_LFN, lde_fname1), 10},
	{offsetof(FAT_DIRENT_LFN, lde_fname2), 12},
	{offsetof(FAT_DIRENT_LFN, lde_fname3),	4}
};

/* Special directory entry name */
LOCAL	const	UB	nmDOT[BASE_EXTLEN + 1]	= ".          ";
LOCAL	const	UB	nmDOT2[BASE_EXTLEN + 1] = "..         ";

/* "root" path name */
LOCAL	const	UB	inode_root_path[] = "/";

/*----------------------------------------------------------------------------
	Misc functions
----------------------------------------------------------------------------*/

/*
 *  Get directory "ino"
 */
Inline	UD	dirINO(FATFS *fs, UW fclno)
{
	return (fclno == fs->ff_rootcl) ? CLSTART : CNtoBN(fclno) * DEpSEC;
}

/*
 *  Get file "ino"
 */
Inline	UD	fileINO(FATFS *fs, UW clno, UW offset)
{
	return (CNtoBN(clno) * DEpSEC) + offset;
}

/*
 *  Get  file "ino" of FAT12/16 root directory
 */
Inline	UD	rootINO(FATFS *fs, UW offset)
{
	return ((fs->ff_rootsc * DEpSEC) + offset);
}

/*
 *  Get directory start cluster number
 */
Inline	UW	dirCLNO(FATFS *fs, UD ino)
{
	return ((ino == CLSTART) ? fs->ff_rootcl : BNtoCN(ino / DEpSEC));
}

/*
 *  Get directory entry cluster number
 */
LOCAL	UW	dirent_to_fcl(FATFS *fs, FAT_DIRENT *dirent)
{
	UW	fcl;

	fcl = (UW)CEH(dirent->de_clno_lo);
	if (isFAT32(fs->ff_fstype)) {
		fcl |= (UW)CEH(dirent->de_clno_hi) << DE_SHIFT_CLNO_HI;
	}
	if (fcl == 0 && isFAT_SUBDIR(dirent->de_ftype)) {
		fcl = fs->ff_rootcl;
	}
	return fcl;
}

/*
 *  Set directory entry cluster number
 */
LOCAL	void	fcl_to_dirent(FATFS *fs, FAT_DIRENT *dirent, UW fcl)
{
	/* Modify start cluster = 0 */
	if (fcl == fs->ff_rootcl) fcl = 0;

	dirent->de_clno_lo = CEH((UH)fcl);
	dirent->de_clno_hi = isFAT32(fs->ff_fstype) ?
				(CEH((UH)(fcl >> DE_SHIFT_CLNO_HI))) : 0;
}

/*----------------------------------------------------------------------------
	Disk device operations
----------------------------------------------------------------------------*/

/*
 *  Disk read
 */
LOCAL	ER	fatDiskRead(FATFS *fs, D start, void *buf, W size)
{
	W	asize;
	ER	err;

	err = tk_srea_dev_d(fs->ff_c.fs_devid, start, buf, size, &asize);
	return (err >= E_OK && asize != size) ? EX_IO : err;
}

/*
 *  Disk write
 */
LOCAL	ER	fatDiskWrite(FATFS *fs, D start, void *buf, W size)
{
	W	asize;
	ER	err;

	err = tk_swri_dev_d(fs->ff_c.fs_devid, start, buf, size, &asize);
	return (err >= E_OK && asize != size) ? EX_IO : err;
}

/*
 *  Disk open
 */
LOCAL	ER	fatDiskOpen(const coninf_t *coninf, FATFS *fs)
{
	ER	err;
	UINT	omode;
	W	sz;
#define	OPEN_MODE(n)	((n) | TD_WEXCL | TD_NOLOCK)

	/* Check device name */
	if (coninf->devnm[0] == '\0') {
		err = EX_INVAL;
		goto exit0;
	}

	/* Open device */
	if (isDEV_RDONLY(coninf->dflags)) {
		omode = OPEN_MODE(TD_READ);
		fs->ff_c.fs_rdonly = TRUE;
	} else {
		omode = OPEN_MODE(TD_UPDATE);
		fs->ff_c.fs_rdonly = FALSE;
	}
	err = tk_opn_dev((UB*)coninf->devnm, omode);
	if (err == E_RONLY) {
		/* Try read-only open */
		fs->ff_c.fs_rdonly = TRUE;
		err = tk_opn_dev((UB *)coninf->devnm, OPEN_MODE(TD_READ));
	}
	//printf("tk_open_dev:%d\n", err);
	if (err < E_OK) goto exit0;
	fs->ff_c.fs_devid = err;

	/* Get disk info */
	err = tk_srea_dev(fs->ff_c.fs_devid, DN_DISKINFO,
				&fs->ff_c.fs_diskinfo, sizeof(DiskInfo), &sz);
	if (err < E_OK) goto exit0;

	if (DSECSZ(fs) == 0) {
		err = EX_NXIO;			/* Media does not exist */
	} else if (fs->ff_c.fs_diskinfo.protect != 0) {
		fs->ff_c.fs_rdonly = TRUE;	/* Media is protected */
	}
exit0:
	return err;
}

/*
 *  Disk close
 */
LOCAL	void	fatDiskClose(const coninf_t *coninf, FATFS *fs)
{
	(void)tk_cls_dev(fs->ff_c.fs_devid, 0);
}

/*
 *  Disk device number
 */
LOCAL	ER	fatGetDeviceNumber(UB *devnm, W *devnum)
{
	ID	devid;

	devid = tk_ref_dev(devnm, NULL);
	if (devid >= E_OK) {
		*devnum = (W)devid;
	}
	return devid;
}

/*----------------------------------------------------------------------------
	Disk cache operations
----------------------------------------------------------------------------*/

/*
 *  Sync a disk cache page
 */
LOCAL	ER	fatDCacheSyncPage(FATFS *fs, DCachePage *page, VW info)
{
	ER	err;

	err = E_OK;
	if (info == 0 || page->dcp_info == 0 || page->dcp_info == info) {
		if (page->dcp_update != FALSE) {
			err = fatDiskWrite(fs, page->dcp_secno,
					page->dcp_buf, page->dcp_seclen);
			page->dcp_update = FALSE;
			page->dcp_dcinf->dci_updatecnt--;
		}
		page->dcp_info = 0;
	}
	return err;
}

/*
 *  Initialize disk cache information
 */
LOCAL	ER	fatDCacheInfoInit(FATFS *fs, DCacheInfo *dcinf, D secno,
						D seclen, UW memsz, UW ratio)
{
	DCachePage	*page;
	UB		*buf;
	UW		pagememsz;
	W		i;
	ER		err;

	err = E_OK;

	/* Calcurate cache page size */
	if (ratio != 1 && (ratio % 2) != 0) {
		err = EX_INVAL;
		goto exit0;
	}
	pagememsz = ratio * DSECSZ(fs);

	dcinf->dci_secno = secno;
	dcinf->dci_endsec = secno + seclen;
	dcinf->dci_ratio = ratio;
	dcinf->dci_pagelen = memsz / pagememsz;
	if (dcinf->dci_pagelen == 0) {
		err = EX_INVAL;
		goto exit0;
	}

	/* Allocate CachePage structure */
	buf = (UB *)fimp_calloc(dcinf->dci_pagelen, sizeof(DCachePage));
	if (buf == NULL) {
		err = EX_NOMEM;
		goto exit0;
	}
	dcinf->dci_pages = (DCachePage *)buf;

	/* Allocate Cache buffer */
	buf = (UB *)fimp_malloc(dcinf->dci_pagelen * pagememsz);
	if (buf == NULL) {
		fimp_free(dcinf->dci_pages);
		dcinf->dci_pages = NULL;
		err = EX_NOMEM;
		goto exit0;
	}
	dcinf->dci_bufs = buf;

	/* Initialize queues */
	for (i = 0; i < DCI_NHSQUE; i++) {
		QueInit(&dcinf->dci_hsque[i]);
	}
	QueInit(&dcinf->dci_freeque);
	QueInit(&dcinf->dci_useque);
	QueInit(&dcinf->dci_actque);

	/* Initialize CachePages */
	for (i = 0; i < dcinf->dci_pagelen; i++) {
		page = &dcinf->dci_pages[i];
		page->dcp_dcinf = dcinf;
		page->dcp_secno = SECVOID;
		//page->dcp_seclen = 0;
		//page->dcp_accnt = 0;
		//page->dcp_update = FALSE;
		//page->dcp_info = 0;
		page->dcp_buf = &dcinf->dci_bufs[pagememsz * i];
		/* Insert to freeque */
		QueInsert(&page->dcp_q, &dcinf->dci_freeque);
	}
exit0:
	return err;
}

/*
 *  Cleanup disk cache information
 */
LOCAL	void	fatDCacheInfoCleanup(FATFS *fs, DCacheInfo *dcinf)
{
	QUEUE	*q;

	/* Sync active pages */
	for (q = dcinf->dci_actque.next;
				q != &dcinf->dci_actque; q = q->next) {
		(void)fatDCacheSyncPage(fs, PageFromActque(q), 0);
	}

	/* Sync use pages */
	for (q = dcinf->dci_useque.next;
				q != &dcinf->dci_useque; q = q->next) {
		(void)fatDCacheSyncPage(fs, PageFromUseque(q), 0);
	}

	/* Free cache */
	fimp_free(dcinf->dci_pages);
	fimp_free(dcinf->dci_bufs);
	memset(dcinf, 0, sizeof(DCacheInfo));
}

/*
 *  Read disk cache
 */
LOCAL	ER	fatDCacheRead(FATFS *fs, DCachePage ** pagep)
{
	ER		err;
	DCachePage	*page;

	page = *pagep;
	err = E_OK;

	if (page->dcp_read == FALSE) {
		err = fatDiskRead(fs, page->dcp_secno, page->dcp_buf,
							page->dcp_seclen);
		if (err < E_OK) {
			/* Remove from actque and hsque, insert to freeque */
			QueRemove(&page->dcp_uq);
			QueRemove(&page->dcp_q);
			QueInsert(&page->dcp_q, &page->dcp_dcinf->dci_freeque);
			*pagep = NULL;
		} else {
			page->dcp_read = TRUE;
		}
	}
	return err;
}

/*
 *  Search disk cache use page
 */
LOCAL	ER	fatDCacheSearchUsedPage(FATFS *fs, DCacheInfo *dcinf, D sec,
						DCachePage ** pagep)
{
	DCachePage	*page;
	QUEUE		*q, *hsq;
	D		secno;
	ER		err;

	page = NULL;

	/* Check sector number */
	if (sec < dcinf->dci_secno || sec >= dcinf->dci_endsec) {
		err = EX_INVAL;
		goto exit0;
	}

	/* Search used and active page using hsque (hash queue) */
	secno = sec - ((sec - dcinf->dci_secno) % dcinf->dci_ratio);
	hsq = &dcinf->dci_hsque[(secno / dcinf->dci_ratio) & DCI_HSQUEMSK];

	err = E_OK;
	if (isQueEmpty(hsq) == FALSE) {
		for (q = hsq->next; q != hsq; q = q->next) {
			if (PageFromHsque(q)->dcp_secno != secno) continue;
			/* Found */
			page = PageFromHsque(q);
			if (page->dcp_accnt++ == 0) {
				/* Move from useque to actque */
				QueRemove(&page->dcp_uq);
				QueInsert(&page->dcp_uq, &dcinf->dci_actque);
			}
			break;
		}
	}
exit0:
	*pagep = page;
	return err;
}

/*
 *  Search disk cache page with/ithout read
 */
LOCAL	ER	fatDCacheSearchPage(FATFS *fs, DCacheInfo *dcinf, D sec,
					DCachePage ** pagep, BOOL read)
{
	DCachePage	*page;
	D		secno;
	UW		seclen;
	ER		err;

	/* Search used / active page */
	err = fatDCacheSearchUsedPage(fs, dcinf, sec, &page);
	if (err < E_OK || page != NULL) goto exit0;	/* OK or error */

	/* Not found, get new page */
	if (isQueEmpty(&dcinf->dci_freeque) == FALSE) {
		/* Get a page from freeque */
		page = PageFromFreeque(dcinf->dci_freeque.next);
		QueRemove(&page->dcp_q);	/* Remove from freeque */

	} else if (isQueEmpty(&dcinf->dci_useque) == FALSE) {
		/* No free page, use oldest used page at the top of useque.
		   Do not use active page in actque. */
		page = PageFromUseque(dcinf->dci_useque.next);
		err = fatDCacheSyncPage(fs, page, 0);
		if (err >= E_OK) {
			/* Remove from both useque and hsque */
			QueRemove(&page->dcp_uq);
			QueRemove(&page->dcp_q);
		}
	} else {
		/* All pages are used */
		err = EX_NOMEM;
		goto exit0;
	}

	/* Initialize page info */
	page->dcp_accnt = 1;
	page->dcp_read = FALSE;
	page->dcp_update = FALSE;
	page->dcp_info = 0;
	secno = sec - ((sec - dcinf->dci_secno) % dcinf->dci_ratio);
	seclen = dcinf->dci_ratio;
	if (secno + seclen > dcinf->dci_endsec) {
		seclen = dcinf->dci_endsec - secno;
	}
	page->dcp_secno = secno;
	page->dcp_seclen = seclen;
	/* Insert page to both actque and hsque */
	QueInsert(&page->dcp_uq, &dcinf->dci_actque);
	QueInsert(&page->dcp_q,
		&dcinf->dci_hsque[(secno / dcinf->dci_ratio) & DCI_HSQUEMSK]);
exit0:
	*pagep = page;
	if (err >= E_OK && read != FALSE) {
		/* Read from disk */
		err = fatDCacheRead(fs, pagep);
	}
	return err;
}

/*
 *  Disk cache get info
 */
LOCAL	void	fatDCacheGetDcinf(FATFS *fs, D sec, DCacheInfo ** dcinf)
{
	if (	sec >= fs->ff_dc_fat.dci_secno &&
		sec <  fs->ff_dc_fat.dci_endsec) {
		/* For FAT */
		*dcinf = &fs->ff_dc_fat;

	} else if (! isFAT32(fs->ff_fstype) &&
		sec >= fs->ff_dc_rootdir.dci_secno &&
		sec <  fs->ff_dc_rootdir.dci_endsec) {
		/* For Root directory */
		*dcinf = &fs->ff_dc_rootdir;

	} else if (sec >= fs->ff_dc_data.dci_secno &&
		    sec <  fs->ff_dc_data.dci_endsec) {
		/* For Data */
		*dcinf = &fs->ff_dc_data;

	} else {
		/* For FSInfo, reserved serctors */
		*dcinf = &fs->ff_dc_other;
	}
}

/*
 *  Disk cache start
 */
LOCAL	ER	fatDCacheStart(FATFS *fs, D sec, DCachePage ** page)
{
	DCacheInfo	*dcinf;

	*page = NULL;
	fatDCacheGetDcinf(fs, sec, &dcinf);
	return fatDCacheSearchPage(fs, dcinf, sec, page, TRUE);
}

/*
 *  Disk cache end
 */
LOCAL	ER	fatDCacheEnd(FATFS *fs, DCachePage *page)
{
	ER	err;

	err = E_OK;

	if (--page->dcp_accnt <= 0) {
#if	FAT_SYNC_WITH_CACHEEND
		err = fatDCacheSyncPage(fs, page, 0);
#endif
		page->dcp_info = 0;
		page->dcp_accnt = 0;

		/* Move from actque to useque */
		QueRemove(&page->dcp_uq);
		QueInsert(&page->dcp_uq, &page->dcp_dcinf->dci_useque);
	}
	return err;
}

/*
 *  Disk cache update
 */
LOCAL	ER	fatDCacheUpdate(FATFS *fs, DCachePage *page, VW info)
{
	if (page->dcp_update == FALSE) {
		page->dcp_dcinf->dci_updatecnt++;
		page->dcp_update = TRUE;
	}
	page->dcp_read = TRUE;
	page->dcp_info = info;
	return E_OK;
}

/*
 *  Disk cache sync info
 */
LOCAL	ER	fatDCacheSyncInfo(FATFS *fs, DCacheInfo *dcinf, VW info)
{
	QUEUE	*q;
	ER	err, err2;

	err = E_OK;
	CHK_BREAK_AND_SET_ERR(fs, &err);
	if (err < E_OK) goto exit0;

	/* Sync pages in actque */
	for (q = dcinf->dci_actque.next; q != &dcinf->dci_actque &&
		dcinf->dci_updatecnt > 0 && err != EX_INTR; q = q->next) {
		err2 = fatDCacheSyncPage(fs, PageFromActque(q), info);
		SetErr2(err, err2);
		CHK_BREAK_AND_SET_ERR(fs, &err);
	}

	/* Sync pages in useque */
	for (q = dcinf->dci_useque.next; q != &dcinf->dci_useque &&
		dcinf->dci_updatecnt > 0 && err != EX_INTR; q = q->next) {
		err2 = fatDCacheSyncPage(fs, PageFromUseque(q), info);
		SetErr2(err, err2);
		CHK_BREAK_AND_SET_ERR(fs, &err);
	}
exit0:
	return err;
}

/*
 *  Disk cache sync FS
 */
LOCAL	ER	fatDCacheSyncFS(FATFS *fs, VW info)
{
	ER	err, err2;

	err = E_OK;
	if (fs->ff_c.fs_rdonly != FALSE) goto exit0;

	/* Cache for fsinfo, reserved sectors */
	err2 = fatDCacheSyncInfo(fs, &fs->ff_dc_other, info);
	SetErr2(err, err2);

	/* Cache for FAT */
	err2 = fatDCacheSyncInfo(fs, &fs->ff_dc_fat, info);
	SetErr2(err, err2);

	/* Cache for root directory */
	if (! isFAT32(fs->ff_fstype)) {
		err2 = fatDCacheSyncInfo(fs, &fs->ff_dc_rootdir, info);
		SetErr2(err, err2);
	}

	/* Cache for file data */
	err2 = fatDCacheSyncInfo(fs, &fs->ff_dc_data, info);
	SetErr2(err, err2);
exit0:
	return err;
}

/*
 *  Initialize disk caches
 */
LOCAL	ER	fatDCacheInit(FATFS *fs)
{
	ER	err;

	/* Cache for FAT */
	err = fatDCacheInfoInit(fs, &fs->ff_dc_fat, fs->ff_fat, fs->ff_fatsz,
			CacheFATMemorySize, CacheFATRatio);
	if (err < E_OK) goto exit0;

	/* Cache for root direcotry */
	if (! isFAT32(fs->ff_fstype)) {
		err = fatDCacheInfoInit(fs, &fs->ff_dc_rootdir, fs->ff_rootsc,
				ROOTSZ / DSECSZ(fs),
				CacheRootMemorySize, CacheRootRatio);
		if (err < E_OK) goto exit0;
	}

	/* Cache for file data */
	err = fatDCacheInfoInit(fs, &fs->ff_dc_data, fs->ff_clstart,
			CNtoBN(fs->ff_lastcl + 1) - fs->ff_clstart,
			CacheDataMemorySize,
			(CacheDataRatio == 0) ? fs->ff_ratio : CacheDataRatio);
	if (err < E_OK) goto exit0;

	/* Cache for fsinfo, reserved sectors */
	err = fatDCacheInfoInit(fs, &fs->ff_dc_other, 1, fs->ff_fat - 1,
			DSECSZ(fs) * 2, 1);
exit0:
	return err;
}

/*
 *  Cleanup disk caches
 */
LOCAL	void	fatDCacheCleanup(FATFS *fs)
{
	/* FSInfo, reserved sectors */
	fatDCacheInfoCleanup(fs, &fs->ff_dc_other);

	/* FAT */
	fatDCacheInfoCleanup(fs, &fs->ff_dc_fat);

	/* Root directory */
	if (! isFAT32(fs->ff_fstype)) {
		fatDCacheInfoCleanup(fs, &fs->ff_dc_rootdir);
	}

	/* Data */
	fatDCacheInfoCleanup(fs, &fs->ff_dc_data);
}

/*
 *  Fsync if the file have O_SYNC flag.
 */
LOCAL	ER	fatFDSync(FD *fd)
{
	return isSET(fd->fd_omode, O_SYNC) ?
		fatDCacheSyncFS((FATFS*)fd->fd_fs, (VW)fd->fd_inode) : E_OK;
}

/*----------------------------------------------------------------------------
	Disk map operations
----------------------------------------------------------------------------*/

/*
 *  End disk map
 */
LOCAL	ER	fatMapEnd(MapInfo *map)
{
	return (map->m_page == NULL) ? E_OK :
				fatDCacheEnd(map->m_fs, map->m_page);
}

/*
 *  Unmap disk map
 */
LOCAL	void	fatUnmapDisk(MapInfo *map)
{
	memset(map, 0, sizeof(MapInfo));
}

/*
 *  Get the sector and cache page of specified offset
 *	map->m_page : result cache page
 */
LOCAL	ER	fatMapGetCache(MapInfo *map, D offset, UW *bufofs, BOOL isread)
{
	FATFS	*fs;
	D	sec;
	UW	off, toff, secsz;
	CLAD	*clp, *cle;
	ER	err;

	fs = map->m_fs;
	secsz = DSECSZ(fs);

	/* Calcurate sector / cluster offset */ 
	off = offset / (isSET(map->m_flags, MAP_C) ? fs->ff_clsz : secsz);

	/* Search cluster / sector at offset byte */
	clp = map->m_clad;
	cle = clp + map->m_clen;

	/* Use last offset for fast search */
	if ((toff = map->m_lastoff) <= off) {
		clp += map->m_lastix;
	} else {
		toff = 0;
	}

	for ( ; clp < cle && (toff += clp->ca_len) <= off; clp++);
	if (clp >= cle) {
		err = EX_INVAL;
		goto exit0;
	}

	map->m_btop = CLVOID;				/* Invalidate data ix */
	map->m_bend = 0;
	map->m_lastix = clp - map->m_clad;		/* Save last info */
	map->m_lastoff = toff - clp->ca_len;
	sec = clp->ca_no + (off - map->m_lastoff);	/* Sector number */

	if (isSET(map->m_flags, MAP_C)) {
		/* Convert sector to cluster number */
		sec = CNtoBN(sec);
		/* Add sector offset within cluster */
		sec += offset / secsz - (off * fs->ff_ratio);
	}

	/* Get mapped page */
	if (map->m_page == NULL ||
		sec < map->m_page->dcp_secno ||
		sec >= map->m_page->dcp_secno + map->m_page->dcp_seclen) {
		if (map->m_page != NULL) {
			err = fatDCacheEnd(fs, map->m_page);
			if (err < E_OK) goto exit0;
		}
		/* Search mapped page of the sector */
		err = fatDCacheSearchPage(fs, map->m_dcinf, sec,
						&map->m_page, isread);
		if (err < E_OK) goto exit0;
	}

	/* Set cache page offset bytes */
	*bufofs = (sec - map->m_page->dcp_secno) * secsz + (offset % secsz);
	return E_OK;
exit0:
	return err;
}

/*
 *  Get disk map information for read
 */
LOCAL	ER	fatMapRead(MapInfo *map, D *offset, UB **sp, UW *len, UB **ep)
{
	FATFS	*fs;
	UW	bufofs, sz;
	ER	err;

	fs = map->m_fs;
	err = fatMapGetCache(map, *offset, &bufofs, TRUE);
	if (err < E_OK) goto exit0;

	sz = map->m_bpp - bufofs;		/* Remain size */
	*offset += sz;				/* Next offset */
	*sp = &map->m_page->dcp_buf[bufofs];	/* Map start pointer */
	if (sz > *len) sz = *len;		/* Map length */
	*len -= sz;				/* Remain length */
	*ep = *sp + sz;				/* Map end pointer */
exit0:
	return err;
}

/*
 *  Read N bytes from disk map
 */
LOCAL	ER	fatMapReadBytes(MapInfo *map, D offset, UB *buf, UW len)
{
	FATFS	*fs;
	UW	bufofs, dofs, sz;
	ER	err;

	err = E_OK;
	fs = map->m_fs;
	for (dofs = 0; len > 0; len -= sz, dofs += sz) {
		err = fatMapGetCache(map, offset, &bufofs, TRUE);
		if (err < E_OK) break;

		sz = map->m_bpp - bufofs;
		if (sz > len) sz = len;

		memcpy(&buf[dofs], &map->m_page->dcp_buf[bufofs], sz);
		offset += sz;
	}
	if (len > 0) {
		CHK_BREAK_AND_SET_ERR(fs, &err);
	}
	return err;
}

/*
 *  Write N bytes to disk map (write 0 when buf == NULL)
 */
LOCAL	ER	fatMapWriteBytes(MapInfo *map, D offset, const UB *buf, D len)
{
	FATFS		*fs;
	DCachePage	*page;
	UW		bufofs, dofs, sz;
	ER		err;

	err = E_OK;
	fs = map->m_fs;
	for (dofs = 0; len > 0; len -= sz, dofs += sz) {
		err = fatMapGetCache(map, offset, &bufofs, FALSE);
		if (err < E_OK) break;

		page = map->m_page;
		if (bufofs != 0 || len < map->m_bpp) {
			err = fatDCacheRead(fs, &page);
			if (err < E_OK) break;
		}
		sz = map->m_bpp - bufofs;
		if (sz > len) sz = len;

		if (buf == NULL) {
			memset(&page->dcp_buf[bufofs], 0, sz);
		} else {
			memcpy(&page->dcp_buf[bufofs], &buf[dofs], sz);
		}

		offset += sz;
		err = fatDCacheUpdate(fs, page, map->m_info);
		if (err < E_OK) break;
	}
	if (len > 0) {
		CHK_BREAK_AND_SET_ERR(fs, &err);
	}
	return err;
}

/*
 *  Map disk cluster or sector
 */
LOCAL	ER	fatMapDisk(FATFS *fs, CLAD *clad, W len, MapInfo *map,
							W flags, VW info)
{
	DCacheInfo	*dcinf;
	D		sec, sa, ea;
	W		i, secsz;
	UW		cnt;
	ER		err, err2;

	/* Clear map */
	memset(map, 0, sizeof(MapInfo));

	/* Initialize cache information */
	if (isSET(flags, MAP_C)) {
		sec = CNtoBN(clad[0].ca_no);
		fatDCacheGetDcinf(fs, sec, &dcinf);
		sa = BNtoCN(dcinf->dci_secno);
		ea = BNtoCN(dcinf->dci_endsec);
	} else {
		sec = clad[0].ca_no;
		fatDCacheGetDcinf(fs, sec, &dcinf);
		sa = dcinf->dci_secno;
		ea = dcinf->dci_endsec - 1;
	}

	/* Check clad[] */
	err = E_OK;
	for (i = 0; i < len; i++) {
		if (sa > clad[i].ca_no ||
			ea < (clad[i].ca_no + clad[i].ca_len - 1)) {
			err = EX_INVAL;
			break;
		}
	}
	if (err < E_OK) goto exit0;

	/* Initialize map inromation */
	map->m_fs = fs;
	map->m_dcinf = dcinf;
	map->m_flags = flags;
	map->m_info = info;
	map->m_clad = clad;
	map->m_clen = len;
	//map->m_lastoff = 0;
	//map->m_lastix = 0;
	//map->m_page = NULL;
	secsz = DSECSZ(fs);
	map->m_bpp = dcinf->dci_ratio * secsz;
	map->m_btop = CLVOID;

	/* Clear Map area */
	if (isSET(flags, MAP_CLR)) {
		for (cnt = 0, i = 0; i < len; i++) {
			cnt += clad[i].ca_len;
		}
		if (cnt > 0) {
			err = fatMapWriteBytes(map, 0, NULL, (D)cnt *
				(isSET(flags, MAP_C) ? fs->ff_clsz : secsz));
			err2 = fatMapEnd(map);
			SetErr2(err, err2);
		}
	} else if (isSET(flags, MAP_READ)) {
		/* Read first cache page */
		err = fatDCacheSearchPage(fs, dcinf, sec, &map->m_page, TRUE);
	}
exit0:
	return err;
}

/*
 *  Initialize FAT sector map
 */
LOCAL	ER	fatMapFATInit(FATFS *fs)
{
	fs->ff_fatmap_s[0].ca_no = fs->ff_fat;
	fs->ff_fatmap_s[0].ca_len = fs->ff_fatsz;

	return fatMapDisk(fs, fs->ff_fatmap_s, NUM_FATMAP_S,
					&fs->ff_fatmap, MAP_S, 0);
}

/*
 *  Cleanup FAT sector map
 */
LOCAL	void	fatMapFATCleanup(FATFS *fs)
{
	fatUnmapDisk(&fs->ff_fatmap);
}

/*----------------------------------------------------------------------------
	FAT inode queue operations
----------------------------------------------------------------------------*/

/*
 *  Initilization of file reference node connection queue
 */
LOCAL	void	fatInodeQueInit(FS *fs)
{
	W	i;

	for (i = 0; i < FS_NIQUE; i++) {
		QueInit(&fs->fs_ique[i]);
	}
}

/*
 *  Check file reference node in connection queue.
 */
LOCAL	ER	fatInodeCheckEmpty(FS *fs, INODE *ignorenode)
{
	QUEUE	*q;
	W	i;

	for (i = 0; i < FS_NIQUE; i++) {
		if (isQueEmpty(&fs->fs_ique[i]) != FALSE) continue;
		for (q = fs->fs_ique[i].next;
				q != &fs->fs_ique[i]; q = q->next) {
			if ((INODE *)q != ignorenode) {
				return EX_BUSY;
			}
		}
	}
	return E_OK;
}

/*
 *  Regist file reference node in connection queue.
 */
LOCAL	void	fatInodeRegister(FS *fs, INODE *inode, UB omode)
{
	/* Stop the root directory and standard input-output */
	if (inode != NULL) {
		/* Update of the number of references */
		if (isNOT_RDONLY(omode)) {
			inode->ino_wrefcnt++;
		}
		if (inode->ino_refcnt++ <= 0) {
			/* Insert into queue at 1st time */
			QueInsert(&inode->ino_q,
				&fs->fs_ique[(UW)inode->ino_ino & FS_IQUEMSK]);
		}
	}
}

/*
 *  Delete file reference node
 */
LOCAL	void	fatInodeFree(FS *fs, INODE *inode)
{
	/* Stop root directory and standard input-output */
	if (inode != NULL) {
		/* Delete if there is no reference */
		if (inode->ino_refcnt <= 0) {
			if (inode->ino_path != NULL &&
				inode->ino_path != inode_root_path) {
				fimp_free(inode->ino_path);
			}
			if (inode->ino_info != NULL) {
				fimp_free(inode->ino_info);
			}
			if (inode != (INODE*)&((FATFS*)fs)->ff_rootnode) {
				fimp_free(inode);
			} else {
				memset(inode, 0, sizeof(INODE));
			}
		}
	}
}

/*
 *  Release file reference node from connection queue
 */
LOCAL	void	fatInodeRelease(FS *fs, INODE *inode, UB omode)
{
	/* Stop root directory and standard input-output */
	if (inode != NULL) {
		/* Stop writing reference */
		if (isNOT_RDONLY(omode)) {
			if (--inode->ino_wrefcnt <= 0) {
				(void)fatDCacheSyncFS((FATFS *)fs, (VW)inode);
			}
		}
		/* Stop reference */
		if (--inode->ino_refcnt <= 0) {
			/* Delete from queue */
			QueRemove(&inode->ino_q);

			/* Release of file reference node */
			fatInodeFree(fs, inode);
		}
	}
}

/*
 *  Search of file reference node
 */
LOCAL	INODE	*fatInodeSearch(FS *fs, UD ino)
{
	QUEUE	*iq, *q;

	/* Search */
	iq = &fs->fs_ique[(UW)ino & FS_IQUEMSK];
	for (q = iq->next; q != iq; q = q->next) {
		if (((INODE *)q)->ino_ino == ino) return (INODE *)q;
	}
	return NULL;
}

/*
 *  Queue transfer of file reference node
 */
LOCAL	void	fatInodeChangeQue(FS *fs, INODE *inode)
{
	if (inode->ino_refcnt > 0) {
		/* Queue remove */
		QueRemove(&inode->ino_q);
		QueInsert(&inode->ino_q,
			&fs->fs_ique[(UW)inode->ino_ino & FS_IQUEMSK]);
	}
}

/*----------------------------------------------------------------------------
	File descriptor operations
----------------------------------------------------------------------------*/

/*
 *  Search file descriptor
 */
LOCAL	ER	fatSearchFileDesc(FS *fs, FD **fdp, W fildes)
{
	FD	*fd;

	if (fs->fs_uxinfo.uxi_fdtbl != NULL) {
		if (fildes >= 0 && fildes < MaxOpenF) {
			fd = fs->fs_uxinfo.uxi_fdtbl[fildes];
			if (fd != NULL && fd->fd_fs == fs) {
				*fdp = fd;
				return E_OK;
			}
		}
	}
	*fdp = NULL;
	return EX_BADF;
}

/*
 *  Create new file descriptor
 */
LOCAL	ER	fatNewFileDesc(FS *fs, FD **fdp, W *fdno)
{
	FD	*fd;
	W	i;
	ER	err;

	if (fs->fs_uxinfo.uxi_fdtbl == NULL) {
		err = EX_NOBUFS;
		goto exit0;
	}

	for (i = 0; i < MaxOpenF && fs->fs_uxinfo.uxi_fdtbl[i] != NULL; i++);

	if (i >= MaxOpenF) {
		err = EX_NFILE;
	} else {
		fd = (FD *)fimp_calloc(1, sizeof(FD));
		if (fd == NULL) {
			err = EX_NOMEM;
		} else {
			fd->fd_fs = fs;
			fd->fd_inode = NULL;
			fd->fd_fpos = 0;
			fd->fd_refcnt = 1;

			/* Registration of file descriptor */
			fs->fs_uxinfo.uxi_fdtbl[i] = fd;

			*fdp = fd;
			*fdno = i;
			err = E_OK;
		}
	}
exit0:
	return err;
}

/*
 *  Delete file descriptor
 */
LOCAL	void	fatDelFileDesc(FS *fs, FD *fd, W fdno)
{
	fs->fs_uxinfo.uxi_fdtbl[fdno] = NULL;

	if (--fd->fd_refcnt <= 0) {
		/* Release file reference node */
		fatInodeRelease(fd->fd_fs, fd->fd_inode, fd->fd_omode);
		fimp_free(fd);
	}
}

/*----------------------------------------------------------------------------
	Cluster operations
----------------------------------------------------------------------------*/

/*
 *  Allocate or expand cluster address buffer
 */
LOCAL	ER	alloc_clbuf(CLAD **clbufp, UH *maxcl)
{
	CLAD	*clbuf;
	UH	ncl;
	VP	tmp;

	if ((clbuf = *clbufp) == NULL) {
		/* Initial allocation */
		clbuf = (CLAD *)fimp_malloc(sizeof(CLAD) * MAX_CLEN);
		ncl = 1;
	} else {
		/* Expand buffer */
		ncl = *maxcl + 1;
		tmp = fimp_realloc(clbuf, sizeof(CLAD) * ncl * MAX_CLEN);
		if (tmp == NULL) {
			fimp_free(clbuf);
		}
		clbuf = tmp;
	}
	*clbufp = clbuf;
	if (clbuf == NULL) {
		*maxcl = 0;
		return EX_NOMEM;
	}
	*maxcl = ncl;
	return E_OK;
}

/*
 *  Get next cluster number
 */
LOCAL	ER	fatGetCluster(MapInfo *map, UW clno, UW *ncl)
{
	UW	cl, bufofs, n;
	ER	err;

	cl = CLEND;
	err = E_OK;

	switch (map->m_fs->ff_fstype) {
	case FAT12:
	{
		UB	buf[2];

		err = fatMapReadBytes(map, clno + clno / 2, buf, 2);
		if (err < E_OK) break;

		cl = CEH(GetMisalignH(buf));
		if ((clno & 1) != 0) cl >>= 4;
		cl &= FAT12_MASK_VALID_CLST;
		if (cl >= FAT12_EOC) cl = CLEND;
	}
		break;

	case FAT16:
		if (clno >= map->m_btop && clno < map->m_bend) {
			/* Use last page cache */
			n = clno - map->m_btop;
		} else {
			err = fatMapGetCache(map, clno * sizeof(UH),
							&bufofs, TRUE);
			if (err < E_OK) break;
			map->m_btop = clno - (n = bufofs / sizeof(UH));
			map->m_bend = map->m_btop + (map->m_bpp / sizeof(UH));
		}
		cl = CEH(((UH *)map->m_page->dcp_buf)[n]);
		if (cl >= FAT16_EOC) cl = CLEND;
		break;

	case FAT32:
	default:
		if (clno >= map->m_btop && clno < map->m_bend) {
			/* Use last page cache */
			n = clno - map->m_btop;
		} else {
			err = fatMapGetCache(map, clno * sizeof(UW),
							&bufofs, TRUE);
			if (err < E_OK) break;
			map->m_btop = clno - (n = bufofs / sizeof(UW));
			map->m_bend = map->m_btop + (map->m_bpp / sizeof(UW));
		}
		cl = CEW(((UW *)map->m_page->dcp_buf)[n]);
		cl &= FAT32_MASK_VALID_CLST;
		if (cl >= FAT32_EOC) cl = CLEND;
		break;
	}
	*ncl = cl;

	/* Check validity */
	if (err >= E_OK) {
		if (cl < CLSTART || cl > map->m_fs->ff_lastcl) {
			if (cl != CLEND) err = EX_IO;
			/* Note, return EX_IO when cl == 0 */
		}
	}
	return err;
}

/*
 *  Set cluster number
 */
LOCAL	ER	fatSetCluster(MapInfo *map, UW clno, UW new, UW *old)
{
	FATFS		*fs;
	UW		offset, cl, bufofs, n;
	ER		err;
	GPTR		dp;

	err = E_OK;
	fs = map->m_fs;
	switch (fs->ff_fstype) {
	case FAT12:
	{
		UB	buf[2];

		offset = clno + (clno / 2);
		err = fatMapReadBytes(map, offset, buf, 2);
		if (err < E_OK) break;

		cl = CEH(GetMisalignH(buf));
		new &= FAT12_MASK_VALID_CLST;
		if ((clno & 1) != 0) {
			new = (new << 4) | (cl & 0x0F);
			cl >>= 4;
		} else {
			new |= (cl & 0xF000);
		}
		if (old != NULL) {
			cl &= FAT12_MASK_VALID_CLST;
			*old = (cl >= FAT12_EOC) ? CLEND : cl;
		}
		SetMisalignH(buf, (UH)CEH(new));
		err = fatMapWriteBytes(map, offset, buf, 2);
		break;
	}

	case FAT16:
		if (clno >= map->m_btop && clno < map->m_bend) {
			/* Use last page cache */
			n = clno - map->m_btop;
		} else {
			err = fatMapGetCache(map, clno * sizeof(UH),
							&bufofs, TRUE);
			if (err < E_OK) break;
			map->m_btop = clno - (n = bufofs / sizeof(UH));
			map->m_bend = map->m_btop + (map->m_bpp / sizeof(UH));
		}
		dp.b = &map->m_page->dcp_buf[n * sizeof(UH)];
		if (old != NULL) {
			cl = CEH(*dp.h);
			*old = (cl >= FAT16_EOC) ? CLEND : cl;
		}
		*dp.h = CEH(new);
		err = fatDCacheUpdate(fs, map->m_page, map->m_info);
		break;

	case FAT32:
	default:
		if (clno >= map->m_btop && clno < map->m_bend) {
			/* Use last page cache */
			n = clno - map->m_btop;
		} else {
			err = fatMapGetCache(map, clno * sizeof(UW),
							&bufofs, TRUE);
			if (err < E_OK) break;
			map->m_btop = clno - (n = bufofs / sizeof(UW));
			map->m_bend = map->m_btop + (map->m_bpp / sizeof(UW));
		}
		dp.b = &map->m_page->dcp_buf[n * sizeof(UW)];
		if (old != NULL) {
			cl = CEW(*dp.w) & FAT32_MASK_VALID_CLST;
			*old = (cl >= FAT32_EOC) ? CLEND : cl;
		}
		new &= FAT32_MASK_VALID_CLST;
		*dp.w = CEW(new);
		err = fatDCacheUpdate(fs, map->m_page, map->m_info);
		break;
	}
	return err;
}

/*
 *  Get cluster list - from "clno" + "offset" bytes to the end.
 *	If "offset" is OFSNEXT, start from next of "clno".
 */
LOCAL	ER	fatGetClusterList(FATFS *fs, CLAD clad[], W cllen,
						UW clno, UW offset, W *len)
{
	MapInfo	*map;
	UW	ncl, pos;
	W	n;
	ER	err;

	if (clno < CLSTART || clno > fs->ff_lastcl) {
		err = EX_INVAL;
		goto exit0;
	}

	pos = fs->ff_clsz;
	if (offset == OFSNEXT) {
		offset = 0;
		n = -1;
	} else {
		n = (pos > offset) ? 0 : -1;
	}

	clad[0].ca_no = clno;
	clad[0].ca_len = 1;

	map = &fs->ff_fatmap;

	for (err = E_OK; err >= E_OK; clno = ncl) {
		err = fatGetCluster(map, clno, &ncl);
		if (err < E_OK || ncl == CLEND) break;

		if (offset == 0 || (pos += fs->ff_clsz) > offset) {
			if (ncl == clno + 1 && n >= 0) {
				clad[n].ca_len++;	/* continuous cluster */
			} else {
				if (n + 1 >= cllen) break;
				clad[++n].ca_no = ncl;
				clad[n].ca_len = 1;
			}
		}
		CHK_BREAK_AND_SET_ERR(fs, &err);
	}
	if (err >= E_OK) {
		*len = n + 1;
	}
	(void)fatMapEnd(map);
exit0:
	return err;
}

/*
 *  Get a cluster just before "clno" started from "stcl"
 */
LOCAL	ER	fatGetPrevCluster(FATFS *fs, CLAD *clad, UW stcl, UW clno)
{
	MapInfo	*map;
	UW	ncl, cl;
	W	len;
	ER	err;

	if (stcl < CLSTART || stcl > fs->ff_lastcl) {
		err = EX_INVAL;
		goto exit0;
	}
	err = E_OK;
	if (stcl == CLEND || stcl == clno) goto exit0;

	map = &fs->ff_fatmap;

	for (cl = stcl, len = 1; ; stcl = ncl) {
		err = fatGetCluster(map, stcl, &ncl);
		if (err < E_OK) break;
		if (ncl == CLEND) {
			len = 0;
			break;
		}
		if (ncl == clno) break;
		if (ncl == stcl + 1) {
			len++;			/* continuous cluster */
		} else {
			cl = ncl;
			len = 1;
		}
	}
	if (err >= E_OK) {
		clad->ca_no = cl;
		clad->ca_len = (UW)len;
	}
	(void)fatMapEnd(map);
exit0:
	return err;
}

/*
 *  Get the number of clusters
 */
LOCAL	ER	fatGetClusterCount(FATFS *fs, UW clno, FATNODE *inode, W *num)
{
	MapInfo	*map;
	W	n;
	UW	ncl;
	ER	err;

	if (clno < CLSTART || clno > fs->ff_lastcl) {
		err = EX_INVAL;
		goto exit0;
	}

	map = &fs->ff_fatmap;

	for (n = 1; ; n++, clno = ncl) {
		err = fatGetCluster(map, clno, &ncl);
		if (err < E_OK || ncl == CLEND) break;
	}
	if (err >= E_OK) {
		if (inode != NULL) {
			/* Set directory file size */
			inode->fino_filsz = n * fs->ff_clsz;
			if (inode->fino_filsz > FILE_SIZE_MAX) {
				inode->fino_filsz = FILE_SIZE_MAX;
			}
		}
		if (num != NULL) {
			*num = n;
		}
	}
	(void)fatMapEnd(map);
exit0:
	return err;
}

/*
 *  Search successive empty cluster of maximum "nbyte" follwing "clno".
 *	When not found, re-search from the top to "clno".
 */
LOCAL	ER	fatGetNewCluster(FATFS *fs, CLAD *clad, UW clno, D nbyte)
{
	MapInfo	*map;
	UW	reqlen, ccl, ncl, stcl, encl;
	W	n;
	ER	err;

	stcl = (clno < CLSTART || clno > fs->ff_lastcl) ? CLSTART : clno;
	encl = fs->ff_lastcl + 1;
	reqlen = (nbyte + fs->ff_clsz - 1) / fs->ff_clsz;

	clad->ca_no = CLVOID;
	clad->ca_len = 0;

	map = &fs->ff_fatmap;

	for (err = E_OK; ; ) {
		/* Search first free cluster */
		ncl = CLVOID;
		for (ccl = stcl; ccl < encl; ccl++) {
			err = fatGetCluster(map, ccl, &ncl);
			if (err < E_OK) {	/* if ncl == 0, EX_IO */
				if (ncl == 0) err = E_OK;
				break;
			}
			CHK_BREAK_AND_SET_ERR(fs, &err);
			if (err < E_OK) break;
		}
		if (err < E_OK) break;

		if (ncl == 0) {
			/* Count free clusters up to reqlen */
			clad->ca_no = ccl;
			n = 1;
			if (reqlen > 1) {
				while(++ccl < encl) {
					err = fatGetCluster(map, ccl, &ncl);
					if (ncl != 0) break;
					err = E_OK;	/* if ncl==0, EX_IO */
					if (++n >= reqlen) break;
					CHK_BREAK_AND_SET_ERR(fs, &err);
					if (err < E_OK) break;
				}
			}
			if (err >= E_OK) clad->ca_len = n;
			break;			/* Found */
		}
		if (stcl == CLSTART) break;	/* Re-search done */

		/* Re-search from top */
		stcl = CLSTART;
		encl = clno;
	}
	(void)fatMapEnd(map);
	return err;
}

/*
 *  Counts the total number of empty clusters
 */
LOCAL	ER	fatCountFreeCluster(FATFS *fs, UW *freecl)
{
	MapInfo	*map;
	D	offset;
	UW	free, totalcl, len, rem, dt;
	ER	err;
	UB	*ep;
	GPTR	dp;

	/* FAT map */
	map = &fs->ff_fatmap;

	/* Count free clusters */
	free = 0;
	offset = 0;
	totalcl = fs->ff_lastcl + 1;

	/* !! Do not use fatGetCluster() in order to count faster */
	switch (fs->ff_fstype) {
	case FAT12:
		for (len = (totalcl * 3 + 1) / 2, rem = 0; len > 0; ) {
			err = fatMapRead(map, &offset, &dp.b, &len, &ep);
			if (err < E_OK) break;
			while (dp.b + 3 <= ep) {
				switch(rem) {
				case 0:	dt = *dp.b++;
				case 1:	dt |= *dp.b++ << 8;
				case 2:	dt |= *dp.b++ << 16;
				}
				if (dt == 0) free += 2;
				else if ((dt & 0x000FFF) == 0) free++;
				else if ((dt & 0xFFF000) == 0) free++;
				rem = 0;
			}
			rem = ep - dp.b;
			if (rem == 0) continue;
			dt = *dp.b++;
			if (rem <= 1) continue;
			dt |= *dp.b++ << 8;
			if (len <= 0 && (dt & 0x000FFF) == 0) free++;
		}
		break;

	case FAT16:
		for (len = totalcl * sizeof(UH); len > 0; ) {
			err = fatMapRead(map, &offset, &dp.b, &len, &ep);
			if (err < E_OK) break;
			while (dp.b < ep) {
				if (*dp.h++ == 0) free++;
			}
		}
		break;

	case FAT32:
	default:
		for (len = totalcl * sizeof(UW); len > 0; ) {
			err = fatMapRead(map, &offset, &dp.b, &len, &ep);
			if (err < E_OK) break;
			while (dp.b < ep) {
				if (*dp.w++ == 0) free++;
			}
		}
		break;
	}
	if (err >= E_OK) {
		*freecl = free;
	}
	(void)fatMapEnd(map);
	return err;
}

/*
 *  Make cluster list specified by "clad" and bind at the end of "clno" list
 */
LOCAL	ER	fatChainCluster(FATFS *fs, UW clno, CLAD *clad, VW info)
{
	MapInfo	*map;
	UW	ccl, ncl, endcl;
	VW	oldinfo;
	ER	err;

	endcl = clad->ca_no + clad->ca_len - 1;
	if (clno < CLSTART || clno > fs->ff_lastcl ||
			clad->ca_no < CLSTART || endcl > fs->ff_lastcl) {
		err = EX_INVAL;
		goto exit0;
	}

	/* Save map info (used for update) */
	map = &fs->ff_fatmap;
	oldinfo = map->m_info;
	map->m_info = info;

	/* Create cluster list */
	err = E_OK;
	for (ccl = clad->ca_no; ccl < endcl && err >= E_OK; ccl++) {
		err = fatSetCluster(map, ccl, ccl + 1, NULL);
	}
	if (err < E_OK) goto exit1;

	/* Termination */
	err = fatSetCluster(map, ccl, CLEND, NULL);
	if (err < E_OK) goto exit1;

	/* Bind 'clad' list to the end of the 'clno' list */
	for (ccl = clno; ccl != clad->ca_no; ccl = ncl) {
		err = fatGetCluster(map, ccl, &ncl);
		if (err < E_OK) break;
		if (ncl == CLEND) {
			err = fatSetCluster(map, ccl, clad->ca_no, NULL);
			break;
		}
	}

	/* Update the number of empty clusters */
	if (err >= E_OK) {
		fs->ff_freecl -= clad->ca_len;
		fs->ff_nextfree = clad->ca_no + clad->ca_len;
	}
exit1:
	(void)fatMapEnd(map);

	/* Recover map info */
	map->m_info = oldinfo;
exit0:
	return err;
}

/*
 *  Delete cluster list from "clno" to the end.
 *	If "next" is TRUE, set CLEND instead of zero at "clno".
 */
LOCAL	ER	fatUnchainCluster(FATFS *fs, UW clno, BOOL next, VW info)
{
	MapInfo	*map;
	UW	clofs, ncl;
	W	cnt;
	VW	oldinfo;
	ER	err;

	/* Save map info (used for update) */
	map = &fs->ff_fatmap;
	oldinfo = map->m_info;
	map->m_info = info;

	for (cnt = 0, ncl = clno; ncl != CLEND;) {
		/* Check of cluster number */
		if (ncl < CLSTART || ncl > fs->ff_lastcl) {
			err = EX_IO;
			break;
		}
		/* Map of FAT area */
		clofs = ncl;
		if (ncl == clno && next != FALSE) {
			/* New termination */
			err = fatSetCluster(map, clofs, CLEND, &ncl);
		} else {
			/* Deletion of Cluster */
			err = fatSetCluster(map, clofs, 0, &ncl);
			cnt++;
		}
		if (err < E_OK) break;
	}
	if (err >= E_OK) {
		/* File system information update */
		fs->ff_freecl += cnt;
		/* Do not update next free cluster */
		//fs->ff_nextfree = CLSTART;
	}
	(void)fatMapEnd(map);

	/* Recover map info */
	map->m_info = oldinfo;
	return err;
}

/*
 *  Extend cluster list
 */
LOCAL	ER	fatExtendClusterList(FATFS *fs, CLAD *clad, FATNODE *inode)
{
	UW	clno;
	CLAD	*clbuf, *clp;
	ER	err;

	clbuf = inode->fino_c.ino_info;
	clp = NULL;

	/* Cluster's linkage location search */
	if (clbuf != NULL && inode->fino_cllen > 0) {
		/* Linkage location */
		clp = &clbuf[inode->fino_cllen - 1];
		clno = clp->ca_no + clp->ca_len - 1;
	} else if (inode->fino_fclno == 0) {
		/* File start cluster */
		inode->fino_fclno = clad->ca_no;
		clno = clad->ca_no;
	} else {
		/* Started search of linkage location  */
		clno = inode->fino_fclno;
	}

	/* Binding of cluster list  */
	err = fatChainCluster(fs, clno, clad, (VW)inode);
	if (err < E_OK) goto exit0;

	if (clbuf == NULL) goto exit0;

	/* Update cluster buffer */
	if (clp != NULL && clad->ca_no == clno + 1) {
		/* Update the number of successive clusters */
		clp->ca_len += clad->ca_len;
	} else {
		/* When cluster buffer is full, expand cluster buffer */
		if (inode->fino_maxcl <= inode->fino_cllen) {
			err = alloc_clbuf(&clbuf, &inode->fino_maxcl);
			inode->fino_c.ino_info = clbuf;
			if (err < E_OK) goto exit0;
		}
		/* Append new cluster */
		clbuf[inode->fino_cllen] = *clad;
		inode->fino_cllen++;
	}
exit0:
	return err;
}

/*
 *  Append a new cluster list
 */
LOCAL	ER	fatApdNewCluster(FATFS *fs, CLAD *clad, D size, FATNODE *inode)
{
	ER	err;

	/* Get empty cluster */
	err = fatGetNewCluster(fs, clad, fs->ff_nextfree, size);
	if (err >= E_OK) {
		/* Cluster expansion */
		err = fatExtendClusterList(fs, clad, inode);
	}
	return err;
}

/*
 *  Truncate cluster list
 */
LOCAL	ER	fatTruncClusterList(FATFS *fs, D newsz, FATNODE *inode)
{
	UW	clno, ofs;
	W	len;
	CLAD	*clbuf, clad;
	ER	err;

	err = E_OK;

	/* Location of detachment */
	ofs = (newsz > 0) ? newsz - 1 : 0;

	clbuf = (CLAD *)inode->fino_c.ino_info;
	if (clbuf != NULL) {
		/* Empty file - do nothing */
		if (inode->fino_cllen == 0) goto exit0;

		/* Offset cluster */
		ofs /= fs->ff_clsz;

		/* Location of detachment */
		for (len = 0; len < (W)inode->fino_cllen; len++) {
			if (ofs < clbuf[len].ca_len) break;
			ofs -= clbuf[len].ca_len;
		}
		if (len >= (W)inode->fino_cllen) len--;
		clno = clbuf[len].ca_no + ofs;
	} else {
		/* Empty file - do nothing */
		if (inode->fino_fclno == 0) goto exit0;

		/* Get cluster list */
		err = fatGetClusterList(fs, &clad, 1, inode->fino_fclno,
								ofs, &len);
		if (err < E_OK) goto exit0;

		/* Location of detachment */
		clno = clad.ca_no;
	}

	/* Delete cluster list */
	err = fatUnchainCluster(fs, clno, (newsz > 0)? TRUE : FALSE, (VW)inode);
	if (err < E_OK) goto exit0;

	/* File size update */
	if ((inode->fino_filsz = newsz) > 0) {
		if (clbuf != NULL) {	/* Cluster buffer update */
			clbuf[len].ca_len = ofs + 1;	/* len + CLEND */
			inode->fino_cllen = (UW)len + 1;
		}
	} else {
		inode->fino_fclno = 0;
		inode->fino_cllen = 0;
	}
exit0:
	return err;
}

/*
 *  Make cluster list of the inode
 */
LOCAL	ER	fatMakeClusterList(FATFS *fs, FATNODE *inode)
{
	UW	clno, cllen;
	UH	maxcl;
	W	len;
	CLAD	*clbuf,*clp;
	ER	err;

	err = E_OK;

	/* Confirmation of cluster list */
	if (inode->fino_c.ino_info != NULL ||
		(IsRootINODE(fs, inode) && ! isFAT32(fs->ff_fstype))) {
		goto exit0;
	}

	/* Allocate buffer */
	cllen = 0;
	clbuf = NULL;
	err = alloc_clbuf(&clbuf, &maxcl);
	if (err < E_OK) goto exit1;

	/* Empty file */
	if ((clno = inode->fino_fclno) == 0 &&
					inode->fino_filsz == 0) goto exit1;

	/* Started cluster */
	for ( ; ; ) {
		/* Location for storing cluster list */
		clp = &clbuf[cllen];

		/* Get cluster list */
		err = fatGetClusterList(fs, clp, MAX_CLEN, clno,
					(cllen == 0) ? 0 : OFSNEXT, &len);
		if (err < E_OK) break;

		cllen += (UW)len;

		/* Clsuter list termination */
		if (len < MAX_CLEN) break;

		/* Started cluster */
		clno = clbuf[cllen - 1].ca_no + clbuf[cllen - 1].ca_len - 1;

		/* Addition of buffer size */
		err = alloc_clbuf(&clbuf, &maxcl);
		if (err < E_OK) break;
	}
exit1:
	if (err < E_OK) {
		if (clbuf != NULL) {
			fimp_free(clbuf);
		}
		clbuf = NULL;
		maxcl = cllen = 0;
	}
	inode->fino_c.ino_info = clbuf;
	inode->fino_maxcl = maxcl;
	inode->fino_cllen = cllen;
exit0:
	return err;
}

/*
 *  Get & copy cluster list
 *	from the offset byte "offset" of file "inode" to "nbyte" byte.
 */
LOCAL	ER	fatCopyClusterList(FATFS *fs, CLAD clad[], D offset,
					D nbyte, FATNODE *inode, W *lenp)
{
	CLAD	*clp, *cle;
	UW	cllen, clofs, clend, len;
	W	n;
	ER	err;

	err = E_OK;

	/* Make cluster list of inode */
	if (inode->fino_c.ino_info == NULL) {
		err = fatMakeClusterList(fs, inode);
		if (err < E_OK) goto exit0;
	}

	/* Cluster offset */
	clofs = offset / fs->ff_clsz;
	clend = (nbyte <= 0) ? 0 :
			(offset + nbyte + (fs->ff_clsz - 1)) / fs->ff_clsz;

	/* Copy cluster list */
	clp = (CLAD *)inode->fino_c.ino_info;
	cle = clp + (W)inode->fino_cllen;

	for (cllen = 0, n = 0; n < MAX_CLEN && clp < cle; clp++) {
		cllen += (len = clp->ca_len);
		if (len <= clofs) {
			/* Skip to the offset location */
			clofs -= len;
		} else {
			/* Copy cluster list */
			clad[n].ca_no = clp->ca_no + clofs;
			clad[n++].ca_len = len - clofs;
			if (clend > 0 && cllen >= clend) {
				/* Adjust last cluster length */
				clad[n - 1].ca_len -= cllen - clend;
				break;
			}
			clofs = 0;
		}
	}
	*lenp = n;
exit0:
	return err;
}

/*----------------------------------------------------------------------------
	File name operations
----------------------------------------------------------------------------*/

/*
 *  Get UTF16 file name from short file name (SFN)
 */
LOCAL	W	fatGetSFN(UB sfname[BASE_EXTLEN], UB smallcaps,
						UH utf16nm[BASE_EXTLEN + 2])
{
	UB	c, nm[BASE_EXTLEN + 2];
	W	i, n, sflg;

	/* Normalize file name */
	sflg = SMALL_BASE;
	for (i = n = 0; n < BASE_EXTLEN; n++) {
		if ((c = sfname[n]) == ' ') {
			if (n == BASELEN) break;	/* No ext name */
		} else {
			if (n == BASELEN) {
				nm[i++] = '.';
				sflg = SMALL_EXT;
			}
			nm[i++] = isSET(smallcaps, sflg) ? AsciiToLOW(c) : c;
		}
	}
	nm[i] = '\0';

	if (nm[0] == DE_B0_KANJI_ESC) nm[0] = DE_B0_FREE;

	/* Convert to UTF16 */
	for (i = n = 0; nm[i] != '\0'; n++) {
#ifdef	FAT_ASCII_FN_ONLY
		utf16nm[n] = (nm[i] <= ASCII_MAX) ? (UH)nm[i] : (UH)'_';
		i++;
#else	/* FAT_ASCII_FN_ONLY */
		UW	unicode;
		i += fatEncLocalToUnicode(&nm[i], &unicode);
		if (UnicodeIsNormal(unicode)) {
			utf16nm[n] = (UH)unicode;
		} else if (UnicodeIsSurrogate(unicode)) {
			utf16nm[n++] = UnicodeToUtf16UpSurrogate(unicode);
			utf16nm[n] = UnicodeToUtf16LowSurrogate(unicode);
		} else {
			utf16nm[n] = (UH)'_';
		}
#endif	/* FAT_ASCII_FN_ONLY */
	}
	utf16nm[n] = (UH)'\0';
	return n;
}

/*
 *  Set short file name (SFN) from UTF8 long file name (LFN)
 */
LOCAL	W	fatSetSFN(const UB *utf8nm, W utf8len,
			UB sfname[BASE_EXTLEN], UB *smallcaps, W gen)
{
	W	n, i, kind, stat;
	UW	unicode, cbit;
	UB	*e_utf8nm, *cp;

	/* Initailize */
	memset(sfname, ' ', BASE_EXTLEN);
	*smallcaps = 0x0;
	e_utf8nm = (UB*)utf8nm + utf8len;

	cbit = 0;
	stat = FAT_SAME_NAME;

	/* Special for "." and ".." */
	if (utf8nm[0] == '.') {
		if (utf8len == 1) {
			sfname[0] = '.';
			goto exit0;
		}
		if (utf8nm[1] == '.' && utf8len == 2) {
			sfname[0] = sfname[1] = '.';
			goto exit0;
		}
		/* ".xxxxxxxx" */
		sfname[0] = '_';
		cbit |= 1 << 0;
		stat = FAT_CHG_WITH_N;
	}

	/* Set basename */
	for (kind = n = 0; n < BASELEN &&
				utf8nm < e_utf8nm && *utf8nm != '.'; ) {
		/* In case of multi-byte character, cbit is set first only */
		cbit |= 1 << n;
		utf8nm += fatEncUtf8ToUnicode(utf8nm, &unicode);
		if (UnicodeIsAscii(unicode)) {
			if (AsciiIsSFN(unicode)) {
				/* Valid ASCII */
				sfname[n++] = AsciiToUP(unicode);
				kind |= 1 << getAsciiKind(unicode);
				continue;
			}
#ifndef	FAT_ASCII_FN_ONLY
		} else if (! UnicodeIsInvalid(unicode)) {
			W	cnt;
			UB	buf[ENC_CHAR_MAX];

			cnt = fatEncUnicodeToLocal(unicode, buf);
			if (cnt > 0) {
				/* Non ASCII */
				if (n + cnt > BASELEN) {
					n = BASELEN;
					stat = FAT_CHG_WITH_N;
					break;
				}
				for (i = 0; i < cnt; i++) {
					sfname[n++] = buf[i];
				}
				kind |= 1 << ASCII_CON;	/* MultiByte */
				continue;
			}
#endif	/* ~ FAT_ASCII_FN_ONLY */
		}
		/* Invalid character */
		sfname[n++] = '_';
		stat = FAT_CHG_WITH_N;
	}

	if (stat != FAT_CHG_WITH_N) {
		/* Check characters */
		if ((kind & (1 << ASCII_LOW)) != 0) {
			if ((kind & (1 << ASCII_UP)) != 0) {
				/* Both lower and upper case */
				stat = FAT_CHG_WITHOUT_N;
			} else if ((kind & (1 << ASCII_CON)) == 0) {
				/* ASCII Lower case only */
				*smallcaps |= SMALL_BASE;
			}
		}
	}

	/* Process remain chacters */
	if (utf8nm < e_utf8nm) {
		if (*utf8nm != '.') {	/* Basename too long */
			stat = FAT_CHG_WITH_N;
		}
		/* Search last '.' position */
		for (cp = e_utf8nm; --cp >= utf8nm; ) {
			if (*cp == '.') break;
		}
		utf8nm = (cp < utf8nm) ? e_utf8nm : cp + 1;

		/* Set extension name */
		for (n = BASELEN, kind = 0;
				 n < BASE_EXTLEN && utf8nm < e_utf8nm; ) {
			utf8nm += fatEncUtf8ToUnicode(utf8nm, &unicode);
			if (UnicodeIsAscii(unicode)) {
				if (AsciiIsSFN(unicode)) {
					/* Valid ASCII */
					sfname[n++] = AsciiToUP(unicode);
					kind |= 1 << getAsciiKind(unicode);
					continue;
				}
#ifndef	FAT_ASCII_FN_ONLY
			} else if (! UnicodeIsInvalid(unicode)) {
				W	cnt;
				UB	buf[ENC_CHAR_MAX];

				cnt = fatEncUnicodeToLocal(unicode, buf);
				if (cnt > 0) {
					/* Non ASCII */
					if (n + cnt > BASE_EXTLEN) {
						n = BASE_EXTLEN;
						stat = FAT_CHG_WITH_N;
						break;
					}
					for (i = 0; i < cnt; i++) {
						sfname[n++] = buf[i];
					}
					kind |= 1 << ASCII_CON;	/* MultiByte */
					continue;
				}
#endif	/* ~ FAT_ASCII_FN_ONLY */
			}
			/* Invalid character */
			sfname[n++] = '_';
			stat = FAT_CHG_WITH_N;
		}
		if (utf8nm < e_utf8nm) {	/* Extension too long */
			stat = FAT_CHG_WITH_N;

		} else if (stat != FAT_CHG_WITH_N) {
			/* Check characters */
			if ((kind & (1 << ASCII_LOW)) != 0) {
				if ((kind & (1 << ASCII_UP)) != 0) {
					/* Both lower and upper case */
					stat = FAT_CHG_WITHOUT_N;
				} else if ((kind & (1 << ASCII_CON)) == 0) {
					/* ASCII Lower case only */
					*smallcaps |= SMALL_EXT;
				}
			}
		}
	}

	if (sfname[0] == DE_B0_FREE) sfname[0] = DE_B0_KANJI_ESC;

	if (stat != FAT_SAME_NAME) {
		*smallcaps = 0x0;

		/* Append generation number : ~n */
		if (stat == FAT_CHG_WITH_N && gen > 0) {
			UB	numstr[BASELEN];

			/* Create gen number string */
			for (i = BASELEN; --i > 0 && gen > 0; gen /= 10) {
				numstr[i] = '0' + (gen % 10);
			}

			/* Calcurate '~' position */
			while (i > 0 && ! isSET(cbit, 1 << i)) i--;

			/* Set generation number : ~n */
			sfname[i] = '~';
			while (++i < BASELEN) sfname[i] = numstr[i];
		}
	}
exit0:
	return stat;
}

/*
 *  Get long file name (UTF16) from the LFN directory entry
 */
LOCAL	W	fatGetLFN(FAT_DIRENT_LFN *lde, W *ldenum, W *chksum,
							UH *utf16nm)
{
	W	i, num;
	UB	*cp, *ep;

	if ((num = lde->lde_index & LFN_CNT) > LFN_MAXENT) goto exit0;

	if (*chksum == CKSVOID) {	/* 1st time, last LFN entry */
		if (! isSET(lde->lde_index, LFN_LAST)) goto exit0;
		*chksum = lde->lde_checksum;
		utf16nm[num * LFN_CHARS] = (UH)'\0';
	} else {
		if (num != *ldenum ||
			lde->lde_checksum != *chksum) goto exit0;
	}
	*ldenum = --num;	/* Next lde number */

	/* Copy LFN */
	for (utf16nm += num * LFN_CHARS, i = 0; i < 3; i++) {
		cp = (UB*)lde + LDE_NMPOS[i].ofs;
		ep = cp + LDE_NMPOS[i].cnt;
		for ( ; cp < ep; cp += 2) {
			*utf16nm++ = CEH(GetMisalignH(cp));
		}
	}
	return 0;
exit0:
	return -1;	/* Invalid entry */
}

/*
 *  Check long file name (LFN) with the directory entry.
 *	utf16nm is upper case character.
 */
LOCAL	W	fatChkLFN(UH *utf16nm, W utf16len, FAT_DIRENT_LFN *lde,
							W chksum, UH *realnm)
{
	UB	*cp, *ep;
	W	n, i, cks;
	UH	utf16;

	cks = CKSVOID;

	/* Get character position */
	n = (W)(((lde->lde_index & LFN_CNT) - 1) * LFN_CHARS);
	if ((utf16len -= n) <= 0) goto exit0;	/* Length differs */

	/* Check checksum */
	if (isSET(lde->lde_index, LFN_LAST)) {
		if (utf16len > (W)LFN_CHARS) goto exit0; /* Length differs */
		chksum = (W)lde->lde_checksum;
	} else {
		if (chksum != lde->lde_checksum) goto exit0;
	}

	/* Compare the fragment of file name */
	for (utf16nm += n, i = 0; i < 3; i++) {
		cp = (UB*)lde + LDE_NMPOS[i].ofs;
		ep = cp + LDE_NMPOS[i].cnt;
		for ( ; cp < ep; cp += 2) {
			utf16 = CEH(GetMisalignH(cp));
			if (--utf16len < 0) {
				if (utf16 != 0) goto exit0;
				if (realnm != NULL) realnm[n] = (UH)'\0';
				goto exit1;
			}
			if (*utf16nm++ != (CharIsAscii(utf16) ?
				AsciiToUP(utf16) : utf16)) goto exit0;
			if (realnm != NULL) realnm[n++] = utf16;
		}
	}
exit1:
	cks = chksum;
exit0:
	return cks;
}

/*
 *  Set long file name (LFN) : returns 0 if final entry
 */
LOCAL	void	fatSetLFN(UH *utf16nm, W utf16len, FAT_DIRENT_LFN *lde,
						W lfnidx, UB chksum)
{
	UB	*cp, *ep;
	UH	*utf16end;
	W	i;

	/* Initialize the entry */
	memset(lde, 0xFF, sizeof(FAT_DIRENT_LFN));
	lde->lde_index = (UB)lfnidx;
	lde->lde_ftype = FAT_LONGNAME;
	lde->lde_smallcaps = 0;
	lde->lde_checksum = chksum;
	lde->lde_clno = 0;

	/* Store the file name fragment */
	utf16end = utf16nm + utf16len;
	utf16nm += (lfnidx - 1) * LFN_CHARS;

	for (i = 0; i < 3; i++) {
		cp = (UB*)lde + LDE_NMPOS[i].ofs;
		ep = cp + LDE_NMPOS[i].cnt;
		for ( ; cp < ep && utf16nm < utf16end; cp += 2, utf16nm++) {
			SetMisalignH(cp, *utf16nm);
		}
		if (utf16nm < utf16end) continue;

		/* Last entry, set terminator(0x0000) */
		if (cp < ep) cp[0] = cp[1] = 0;
		else if (i < 2) continue;	/* for set terminator next */
		lde->lde_index |= LFN_LAST;
		break;
	}
}

/*----------------------------------------------------------------------------
	Directory operations
----------------------------------------------------------------------------*/

/*
 *  Check-sum calculation of file name
 */
LOCAL	UB	fatCalcChksum(UB *name)
{
	W	i;
	UB	s;

	for (s = 0, i = BASE_EXTLEN; --i >= 0; s += *name++) {
		s = (s << 7) | (s >> 1);	/* rotate right shift */
	}
	return s;
}

/*
 *  Read current dirent entry
 */
LOCAL	ER	fatReadCurDIR(FATFS *fs, DIRINF *dir)
{
	W	d_ofs;
	UW	ofs;
	ER	err;

	if ((d_ofs = dir->di_dirent_ofs) == OFSVOID) return EX_INVAL;

	if (d_ofs >= dir->di_map.m_btop && d_ofs < dir->di_map.m_bend) {
		/* The dirent exists in the last cache page, then use it */
		ofs = (d_ofs - dir->di_map.m_btop) * DIRENTSZ;
	} else {
		err = fatMapGetCache(&dir->di_map, d_ofs * DIRENTSZ, &ofs,TRUE);
		if (err < E_OK) return err;
		/* Save last dirent_ofs of the top of cache page */
		dir->di_map.m_btop = d_ofs - (ofs / DIRENTSZ);
		dir->di_map.m_bend = dir->di_map.m_btop +
			( ((fs->ff_clsz < dir->di_map.m_bpp) ?
				fs->ff_clsz : dir->di_map.m_bpp) / DIRENTSZ );
	}
	dir->di_dirent = (FAT_DIRENT *)&dir->di_map.m_page->dcp_buf[ofs];
	return E_OK;
}

/*
 *  Write current directory entry
 */
LOCAL	ER	fatWriteCurDIR(FATFS *fs, DIRINF *dir, VW info)
{
	if (dir->di_map.m_page == NULL || dir->di_access != AC_WRITE ||
			dir->di_dirent_ofs == OFSVOID) return EX_INVAL;

	return fatDCacheUpdate(fs, dir->di_map.m_page,
				(info == 0) ? dir->di_map.m_info : info);
}

/*
 *  Open the directory that specifies cluster "dircl" as  the top.
 */
LOCAL	ER	fatOpenDIR(FATFS *fs, DIRINF *dir, UW dircl, W offset,
				UW mode, FATNODE *inode)
{
	UW	dirofs;
	W	cllen;
	div_t	dv;
	ER	err;

	/* Initialization of directory */
	memset(dir, 0, sizeof(DIRINF));
	dir->di_dircl = dircl;		/* Directory top cluster */
	dir->di_access = mode;		/* Accesss mode */
	dir->di_dirent_ofs = OFSVOID;

	dirofs = 0;
	dv.quot = dv.rem = 0;
	err = E_OK;

	if (offset >= 0) {
		dv = div((dircl > 0)? (offset % fs->ff_clsz):offset, DIRENTSZ);
		if (dv.rem != 0) {
			err = EX_INVAL;
			goto exit0;
		}
		/* Directory present location */
		dir->di_offset = (UW)offset;
		dirofs = (UW)offset;
	}

	if (inode == NULL) {
		inode = (FATNODE *)fatInodeSearch((FS*)fs, dirINO(fs, dircl));
	}
	dir->di_inode = inode;

	if (dircl > 0) {	/* FAT32 or Subdirectory */
		/* Get next cluster list */
		if (inode != NULL && inode->fino_c.ino_info != NULL) {
			err = fatCopyClusterList(fs, dir->di_clno,
						dirofs, 0, inode, &cllen);
		} else {
			err = fatGetClusterList(fs, dir->di_clno, MAX_CLEN,
							dircl, dirofs, &cllen);
		}
		if (err < E_OK) goto exit0;

		/* Confirmation of offset */
		if (cllen == 0) {
			if (offset < 0) {
				err = EX_INVAL;
				goto exit0;
			}
			offset = OFSVOID;
			goto exit1;
		}

		/* Map clusters */
		dir->di_len = cllen;
		err = fatMapDisk(fs, dir->di_clno, cllen, &dir->di_map,
						MAP_C | MAP_READ, (VW)inode);
		if (err < E_OK) goto exit0;

		/* fatMapEnd() is called by fatCloseDIR() */
		while (--cllen >= 0) {
			dir->di_nent += dir->di_clno[cllen].ca_len;
		}
		dir->di_nent *= DEpCL;

	} else {	/* FAT12, FAT16 root directory */
		/* Confirmation of offset */
		if (dirofs >= ROOTSZ) {
			offset = OFSVOID;
			goto exit1;
		}

		/* FAT12, FAT16 Root map */
		dir->di_secno[0].ca_no = fs->ff_rootsc;
		dir->di_secno[0].ca_len = ROOTSZ / DSECSZ(fs);

		err = fatMapDisk(fs, dir->di_secno, ROOT_SLEN, &dir->di_map,
					MAP_S | MAP_READ, (VW)inode);
		if (err < E_OK) goto exit0;

		/* fatMapEnd() is called by fatCloseDIR() */
		dir->di_nent = ROOTENT;	/* The number of directory entries */
	}
exit1:
	/* Entry present location */
	if (offset >= 0) {
		if (dv.quot >= (W)dir->di_nent) {
			err = EX_INVAL;
		} else {
			dir->di_dirent_ofs = dv.quot;
			err = fatReadCurDIR(fs, dir);
		}
	}
exit0:
	return err;
}

/*
 *  Close opened directory
 */
LOCAL	ER	fatCloseDIR(FATFS *fs, DIRINF *dir, ER err)
{
	(void)fs;

	if (dir->di_dirent_ofs != OFSVOID) {
		if (err < E_OK) {	/* When any error, do not update */
			dir->di_access = AC_RDONLY;
		}
		(void)fatMapEnd(&dir->di_map);
		fatUnmapDisk(&dir->di_map);
		dir->di_dirent_ofs = OFSVOID;
		dir->di_dirent = NULL;
	}
	return err;
}

/*
 *  Directory map next directory entries
 */
LOCAL	ER	fatMapNextDIR(FATFS *fs, DIRINF *dir, UW mode)
{
	UW	clno;
	W	n;
	ER	err;

	/* Directory close */
	(void)fatCloseDIR(fs, dir, E_OK);

	err = E_OK;
	if (dir->di_dircl == 0) goto exit0;

	/* Get last cluster number */
	clno = dir->di_clno[dir->di_len - 1].ca_no +
				dir->di_clno[dir->di_len - 1].ca_len - 1;
	/* Get next cluster list */
	err = fatGetClusterList(fs, dir->di_clno, MAX_CLEN, clno, OFSNEXT,
								&dir->di_len);
	if (err < E_OK) goto exit0;

	if ((n = dir->di_len) > 0) {
		/* Map clusters */
		err = fatMapDisk(fs, dir->di_clno, n, &dir->di_map,
					MAP_C | MAP_READ, (VW)dir->di_inode);
		if (err < E_OK) goto exit0;

		/* Set number of directory entries */
		for (dir->di_nent = 0; --n >= 0; ) {
			dir->di_nent += dir->di_clno[n].ca_len;
		}
		dir->di_nent *= DEpCL;

		/* Read first directory entry */
		dir->di_access = mode;
		dir->di_dirent_ofs = 0;
		err = fatReadCurDIR(fs, dir);
	}
exit0:
	return err;
}

/*
 *  Directory map previous directory entry
 */
LOCAL	ER	fatMapPrevDIR(FATFS *fs, DIRINF *dir, UW mode)
{
	ER	err;

	/* Directory close */
	(void)fatCloseDIR(fs, dir, E_OK);

	/* Get a previous cluster */
	err = fatGetPrevCluster(fs, &dir->di_clno[0], dir->di_dircl,
						dir->di_clno[0].ca_no);
	if (err < E_OK) goto exit0;

	if (dir->di_clno[0].ca_len == 0) {
		err = EX_INVAL;
		goto exit0;
	}

	/* Map a cluster */
	dir->di_len = 1;
	err = fatMapDisk(fs, dir->di_clno, 1, &dir->di_map,
					MAP_C | MAP_READ, (VW)dir->di_inode);
	if (err < E_OK) goto exit0;

	/* Set number of directory entries */
	dir->di_nent = dir->di_clno[0].ca_len * DEpCL;

	/* Read last directory entry in the cluster */
	dir->di_access = mode;
	dir->di_dirent_ofs = dir->di_nent - 1;
	err = fatReadCurDIR(fs, dir);
exit0:
	return err;
}

/*
 *  Get the next directory entry
 */
LOCAL	ER	fatNextDIR(FATFS *fs, DIRINF *dir, UW mode)
{
	ER	err;

	if (dir->di_dirent == NULL) {		/* Top */
		dir->di_dirent_ofs = 0;
		dir->di_offset = 0;
		return fatReadCurDIR(fs, dir);
	}

	/* Next entry */
	dir->di_dirent_ofs++;

	if (dir->di_dirent_ofs >= dir->di_nent) {
		/* Map next cluster & read directory entry */
		err = fatMapNextDIR(fs, dir, mode);
	} else {
		/* Already mapped, read directory entry */
		err = fatReadCurDIR(fs, dir);
	}
	if (err >= E_OK) {
		if (dir->di_dirent != NULL) {
			dir->di_offset += DIRENTSZ;
		}
		/* di_dirent == NULL when last entry */
	}
	return err;
}

/*
 *  Get the directory entry just before
 */
LOCAL	ER	fatPrevDIR(FATFS *fs, DIRINF *dir, UW mode)
{
	ER	err;

	if (dir->di_dirent == NULL) {	/* End */
		err = EX_INVAL;		/* Not occur */
		//dir->di_dirent_ofs = dir->di_nent - 1;
		//dir->di_offset = dir->di_nent * DIRENTSZ;
	} else {			/* Previous */
		if (dir->di_dirent_ofs <= 0) {
			if (dir->di_offset > 0) {
				/* Map the previous cluster */
				err = fatMapPrevDIR(fs, dir, mode);
				if (err >= E_OK) {
					dir->di_offset -= DIRENTSZ;
				}
			} else {
				err = EX_INVAL;
			}
		} else {
			dir->di_dirent_ofs--;
			err = fatReadCurDIR(fs, dir);
			if (err >= E_OK) {
				dir->di_offset -= DIRENTSZ;
			}
		}
	}
	return err;
}

/*
 *  Set the terminated mark next to directory's present location.
 */
LOCAL	ER	fatSetEndDIR(FATFS *fs, DIRINF *dir)
{
	MapInfo		map;
	UW		clno;
	W		len, bufofs;
	CLAD		clad;
	FAT_DIRENT	*de;
	ER		err, err2;

	if (dir->di_dirent == NULL) {
		err = EX_INVAL;
		goto exit0;
	}

	if (dir->di_dirent_ofs + 1 >= dir->di_nent) {

		if (dir->di_dircl == 0) {	/* FAT12, 16 root */
			err = E_OK;
			goto exit0;		/* do nothing */
		}

		/* Get a next cluster */
		clno = dir->di_clno[dir->di_len - 1].ca_no
			+ dir->di_clno[dir->di_len - 1].ca_len - 1;
		err = fatGetClusterList(fs, &clad, 1, clno, OFSNEXT, &len);
		if (err < E_OK) goto exit0;

		if (len <= 0) goto exit0;	/* Last entry, do nothing */

		/* Map a cluster */
		err = fatMapDisk(fs, &clad, 1, &map,
					MAP_C | MAP_READ, (VW)dir->di_inode);
		if (err >= E_OK) {
			/* Set terminated entry */
			bufofs = OFFSETinPAGE(fs, map.m_page,
						CNtoBN(clad.ca_no), 0);
			de = (FAT_DIRENT*)&map.m_page->dcp_buf[bufofs];
			de->de_fname[0] = DE_B0_END;
			err = fatDCacheUpdate(fs,map.m_page, (VW)dir->di_inode);
			(void)fatMapEnd(&map);
		}
		fatUnmapDisk(&map);
	} else {
		dir->di_dirent_ofs++;
		err = fatReadCurDIR(fs, dir);
		if (err >= E_OK) {
			/* Set terminated entry */
			dir->di_dirent->de_fname[0] = DE_B0_END;
			err = fatWriteCurDIR(fs, dir, 0);

			dir->di_dirent_ofs--;
			err2 = fatReadCurDIR(fs, dir);
			SetErr2(err, err2);
		}
	}
exit0:
	return err;
}

/*
 *  Search the directory entry corresponding with the file started
 *	cluster "fclno".
 */
LOCAL	ER	fatSearchDIR_cl(FATFS *fs, DIRINF *dir, UW fclno, UW mode)
{
	FAT_DIRENT	*dirent;
	ER		err;

	for (;;) {
		/* Get next directory entry */
		err = fatNextDIR(fs, dir, mode);
		if (err < E_OK) break;

		dirent = dir->di_dirent;
		if (dirent == NULL ||		/* Last entry */
			isDE_TERM(dirent)) {	/* Terminated entry */
			dir->di_dirent = NULL;
			err = EX_NOENT;
			break;
		}
		if (isDE_FREE(dirent) ||	/* Free entry */
			isDE_LFN(dirent)) {	/* LFN entry */
			continue;
		}
		if (isFAT_SUBDIR(dirent->de_ftype)) {
			/* Directory, comparison of the started cluster */
			if (dirent_to_fcl(fs, dirent) == fclno) break;
		}
	}
	return err;
}

/*
 *  Search the directory entry whose file name is identical with "utf8nm".
 */
LOCAL	ER	fatSearchDIR_nm(FATFS *fs, DIRINF *dir, UB *utf8nm, W len,
						UW mode, UH *realname)
{
	FAT_DIRENT	*dirent;
	UB		sfname[BASE_EXTLEN];
	W		utf16len, enc, cks;
	UB		smallcaps;
	ER		err;

	/* Convert UTF8 name to UTF16 name (result in ff_nmwk) */
	utf16len = fatEncUtf8strToUtf16str(utf8nm, len,
					fs->ff_nmwk, LFN_MAXBUF, TRUE);
	if (utf16len <= 0) {
		err = EX_INVAL;
		goto exit0;
	}
	if (utf16len > LFN_MAXLEN) {
		err = EX_NAMETOOLONG;
		goto exit0;
	}

	/* Set SFN from UTF8 long name */
	enc = fatSetSFN(utf8nm, len, sfname, &smallcaps, 0);

	/* Search directory entry */
	for (cks = CKSVOID; ; ) {

		/* Get next directory entry */
		err = fatNextDIR(fs, dir, mode);
		if (err < E_OK) break;

		dirent = dir->di_dirent;
		if (dirent == NULL ||		/* Last entry */
			isDE_TERM(dirent)) {	/* Terminated entry */
			dir->di_dirent = NULL;
			err = EX_NOENT;
			break;
		}
		if (isDE_FREE(dirent)) {	/* Free entry */
			goto next0;
		}
		if (isDE_LFN(dirent)) {	/* LFN entry */
			/* Check LFN and calcurate chksum */
			cks = fatChkLFN(fs->ff_nmwk, utf16len,
				(FAT_DIRENT_LFN *)dirent, cks, realname);
			goto next1;
		}
		if (isDE_VOL(dirent)) {	/* Volume label */
			goto next0;
		}

		/* SFN entry */
		if (fatCalcChksum(dirent->de_fname) == cks) {
			/* Checksum is matched, LFN found */
			break;
		}
		if (enc != FAT_CHG_WITH_N &&
			memcmp((B*)dirent->de_fname,
					(B*)sfname, BASE_EXTLEN) == 0) {
			/* sfname is matched, SFN found */
			if (realname != NULL) {
				(void)fatGetSFN(dirent->de_fname,
					dirent->de_smallcaps, realname);
			}
			break;
		}
next0:
		cks = CKSVOID;		/* Initilize LFN search */
next1:
		CHK_BREAK_AND_SET_ERR(fs, &err);
		if (err < E_OK) break;
	}
exit0:
	return err;
}

/*
 *  Get file number from directory entry
 */
LOCAL	UD	fatGetInoDIR(FATFS *fs, DIRINF *dir)
{
	W	i, ofs, offset;

	if (isFAT_SUBDIR(dir->di_dirent->de_ftype)) {	/* Directory */
		return dirINO(fs, dirent_to_fcl(fs, dir->di_dirent));
	}

	if (dir->di_dircl > 0) {			/* File */
		for (i = 0, ofs = dir->di_dirent_ofs; ;) {
			offset = ofs;
			ofs -= (W)(dir->di_clno[i].ca_len * DEpCL);
			if (++i >= dir->di_len || ofs <= 0) break;
		}
		return fileINO(fs, dir->di_clno[i - 1].ca_no, offset);
	}
	/* FAT12,16 root directory */
	return rootINO(fs, dir->di_dirent_ofs);
}

/*
 *  Get root directory file reference node
 */
LOCAL	ER	fatGetRootINODE(FATFS *fs, FATNODE ** inodep)
{
	FATNODE	*inode;
	ER	err;

	err = E_OK;

	inode = &fs->ff_rootnode;
	if (inode->fino_ftype != FAT_SUBDIR) {
		inode->fino_c.ino_ino = fs->ff_c.fs_rootino;
		inode->fino_c.ino_path = (UB*)inode_root_path;
		inode->fino_c.ino_type = DT_DIR;
		inode->fino_d_ronly = TRUE;
		inode->fino_fclno = fs->ff_rootcl;
		inode->fino_ftype = FAT_SUBDIR;

		if (! isFAT32(fs->ff_fstype)) {
			inode->fino_filsz = ROOTSZ;
		} else {
			err = fatGetClusterCount(fs, inode->fino_fclno,
								inode, NULL);
			if (err < E_OK) inode = NULL;
		}
	}
	*inodep = inode;
	return err;
}

/*
 *  Get file reference node
 */
LOCAL	ER	fatGetINODE(FATFS *fs, DIRINF *dir, UB dtype,
					UB *abspath, W len, FATNODE ** inodep)
{
	FATNODE		*inode;
	UD		ino;
	UB		*path;
	FAT_DIRENT	*dirent;
	ER		err;

	err = E_OK;
	if ((dirent = dir->di_dirent) != NULL) {	/* Exist */
		ino = fatGetInoDIR(fs, dir);
		inode = (FATNODE *)fatInodeSearch((FS *)fs, ino);
	} else {					/* Not exist */
		inode = NULL;
		ino = NOEXS_INO;
	}
	if (inode == NULL) {	/* Create new inode */
		inode = fimp_calloc(1, sizeof(FATNODE));
		if (inode == NULL) {
			err = EX_NOMEM;
			goto exit0;
		}
		if (dirent != NULL) {	/* Exists */
			inode->fino_dirofs = dir->di_offset;
			inode->fino_fclno = dirent_to_fcl(fs, dirent);
			inode->fino_ftype = dirent->de_ftype;

			if (isFAT_SUBDIR(dirent->de_ftype)) {
				inode->fino_c.ino_type = DT_DIR;
				/* Size of directory */
				err = fatGetClusterCount(fs, inode->fino_fclno,
								inode, NULL);
				if (err < E_OK) {
					fimp_free(inode);
					goto exit0;
				}
			} else {	/* Usual file size */
				inode->fino_c.ino_type = DT_REG;
				inode->fino_filsz = (D)
						((UW)CEW(dirent->de_fsize));
			}
		}
		inode->fino_dircl = dir->di_dircl;
		inode->fino_d_ronly = isFAT_RDONLY(dtype) ? TRUE : FALSE;
		inode->fino_c.ino_ino = ino;
	}

	/* Set abs path name when directory */
	if (inode->fino_c.ino_type == DT_DIR &&
					inode->fino_c.ino_path == NULL) {
		path = fimp_malloc(len + 1);
		if (path == NULL) {
			fimp_free(inode);
			err = EX_NOMEM;
			goto exit0;
		}
		inode->fino_c.ino_path = path;
		while (--len >= 0) {
			*path++ = *abspath++;
		}
		*path = '\0';
	}
	if (inode != NULL) {
		*inodep = inode;
	}
exit0:
	return err;
}

/*----------------------------------------------------------------------------
	Time operations
----------------------------------------------------------------------------*/

/*
 *  Get date & time
 */
LOCAL	void	fatGetDateTime(FAT_DIRENT *dirent, struct stat64_us *sb)
{
	struct tm	mtim, atim;

	memset(&mtim, 0, sizeof(struct tm));
	mtim.tm_sec = (CEH(dirent->de_mtime) & FAT_TIME_MASK_SEC) << 1;
	mtim.tm_min = (CEH(dirent->de_mtime) & FAT_TIME_MASK_MIN) >>
						FAT_TIME_SHIFT_MIN;
	mtim.tm_hour = (CEH(dirent->de_mtime) & FAT_TIME_MASK_HOUR) >>
						FAT_TIME_SHIFT_HOUR;
	mtim.tm_mday = CEH(dirent->de_mdate) & FAT_TIME_MASK_SEC;
	mtim.tm_mon = ((CEH(dirent->de_mdate) & FAT_DATE_MASK_MON) >>
						FAT_DATE_SHIFT_MON) - 1;
	mtim.tm_year = ((CEH(dirent->de_mdate) & FAT_DATE_MAS_YEAR) >>
				FAT_DATE_SHIFT_YEAR) + FATYEAR_TO_TMYEAR;

	memset(&atim, 0, sizeof(struct tm));
	atim.tm_mday = CEH(dirent->de_adate) & FAT_TIME_MASK_SEC;
	atim.tm_mon = ((CEH(dirent->de_adate) & FAT_DATE_MASK_MON) >>
				FAT_DATE_SHIFT_MON) -1;
	atim.tm_year = ((CEH(dirent->de_adate) & FAT_DATE_MAS_YEAR) >>
				FAT_DATE_SHIFT_YEAR) + FATYEAR_TO_TMYEAR;

	/* There is no "last status change time" in FAT file system.
		Then, use "last modified time" as "last status change time".
		Note: Create date/time in FAT is not used.	*/

	(void)dt_mktime_us(&mtim, NULL, &sb->st_mtime_u);
	(void)dt_mktime_us(&mtim, NULL, &sb->st_ctime_u);
	(void)dt_mktime_us(&atim, NULL, &sb->st_atime_u);
}

/*
 *  Set date & time
 */
LOCAL	ER	fatSetDateTime(UH *date, UH *time, const SYSTIM_U *stime_u)
{
	SYSTIM_U	tim_u;
	struct tm	tm;
	UH		de_date, de_time;
	ER		err;

	if (stime_u == NULL) {	/* Set system time */
		(void)tk_get_tim_u(&tim_u, 0);
	} else {
		tim_u = *stime_u;
	}
	err = dt_localtime_us(tim_u, NULL, &tm);
	if (err >= E_OK) {
		/* Convert into calendar format */
		de_date = (UH)(	((tm.tm_year - FATYEAR_TO_TMYEAR) <<
							FAT_DATE_SHIFT_YEAR) |
				((tm.tm_mon + 1) << FAT_DATE_SHIFT_MON) |
				tm.tm_mday );
		de_time = (UH)(	(tm.tm_hour << FAT_TIME_SHIFT_HOUR) |
				(tm.tm_min << FAT_TIME_SHIFT_MIN) |
				(tm.tm_sec >> 1));
		*date = CEH(de_date);
		*time = CEH(de_time);
	}
	return err;
}

/*----------------------------------------------------------------------------
	File operarions
----------------------------------------------------------------------------*/

/*
 *  Check if the SFN exists in the directory.
 */
LOCAL	ER	fatCheckDirent(FATFS *fs, UW dircl, UB *sfname, UW nent)
{
	DIRINF		dir;
	FAT_DIRENT	*dirent;
	UB		nm[BASE_EXTLEN];
	W		i, exist, existall, gnpos;
	ER		err;

	/* Directory open */
	err = fatOpenDIR(fs, &dir, dircl, OFSVOID, AC_RDONLY, NULL);
	if (err < E_OK) goto exit0;

	/* Last digit position of generation number */
	for (gnpos = BASELEN; sfname[--gnpos] == ' '; );

	/* Directory search */
	existall = (1 << nent) - 1;		/* All exist pattern */
	for (exist = 0; exist != existall; ) {

		/* Get next directory entry */
		err = fatNextDIR(fs, &dir, AC_RDONLY);
		if (err < E_OK) break;

		dirent = dir.di_dirent;
		if (dirent == NULL ||		/* Last entry */
			isDE_TERM(dirent) ) {	/* Terminated entry */
			break;
		}
		if (isDE_FREE(dirent) ||	/* Free entry */
			isDE_LFN(dirent)  ||	/* LFN entry */
			isDE_VOL(dirent) ) {	/* Volume label */
			continue;
		}

		/* SFN entry, check name by increasing generation number */
		memcpy(nm, sfname, BASE_EXTLEN);
		for (i = 0; i < nent; i++, nm[gnpos] += 1) {
			if (isSET(exist, (1 << i)) ||
				memcmp(dirent->de_fname, nm, BASE_EXTLEN) != 0)
					continue;
			exist |= (1 << i);	/* Exists */
			break;
		}
	}

	/* Directory close */
	err = fatCloseDIR(fs, &dir, err);
	if (err < E_OK) goto exit0;

	err = E_OK;
	if (exist != existall) {
		for (i = 0; i < nent && isSET(exist, 1); i++, exist >>= 1);
		sfname[gnpos] += i;	/* Set unique generation number */
		err = E_OK + 1;
	}
exit0:
	return err;
}

/*
 *  Search the "nent" (>= 1) pieces of empty entries successive from
 *  the directory "dircl".
 *    1. Use entries after the termination entry.
 *    2. Use successive free entry.
 *    3. Expands directory.
 */
LOCAL	ER	fatSearchFreeDirent(FATFS *fs, UW dircl, UW nent, W *ofs)
{
	DIRINF		dir;
	FAT_DIRENT	*dirent;
	W		end, free, offset, freeofs, num, nbyte;
	FATNODE		*inode;
	BOOL		found;
	CLAD		clad;
	MapInfo		map;
	ER		err;

	/* Directory open */
	err = fatOpenDIR(fs, &dir, dircl, 0, AC_WRITE, NULL);
	if (err < E_OK) goto exit0;

	for (end = free = 0, freeofs = OFSVOID, found = FALSE;;) {
		offset = (W)dir.di_offset;

		dirent = dir.di_dirent;
		if (dirent == NULL) {			/* Last entry */
			if (freeofs != OFSVOID) {
				offset = freeofs;
				found = TRUE;
			}
			break;
		}
		if (end > 0 || isDE_TERM(dirent)) {	/* Terminated entry */
			if (++end >= nent) {
				err = fatSetEndDIR(fs, &dir);
				if (err >= E_OK) found = TRUE;
				break;
			}
		} else if (isDE_FREE(dirent) && freeofs == OFSVOID) {
			/* Free entry */
			if (++free >= nent) {
				freeofs = offset;
#ifdef	USE_1ST_FREEENT
				found = TRUE;
				break;
#endif
			}
		} else {
			/* Reset of the number of successive free entries */
			free = 0;
		}
		/* Get next directory entry */
		err = fatNextDIR(fs, &dir, AC_WRITE);
		if (err < E_OK) break;
	}

	/* Directory close */
	err = fatCloseDIR(fs, &dir, err);
	if (err < E_OK) goto exit0;

	if (found != FALSE) goto exit1;

	/* Not found */
	inode = dir.di_inode;
	if (dircl > 0) {	/* FAT32 or sub directory */
		nbyte = ((W)nent - end) * DIRENTSZ;

		/* Empty cluster search. Note nbyte <= ff_clsz */
		err = fatGetNewCluster(fs, &clad, fs->ff_nextfree, nbyte);
		if (err < E_OK) goto exit0;

		/* Directory size */
		err = fatGetClusterCount(fs, dircl, NULL, &num);
		if (err < E_OK) goto exit0;

		if (((num + clad.ca_len) * fs->ff_clsz) > DIR_SIZE_MAX) {
			err = EX_FBIG;
			goto exit0;
		}

		/* Directory expansion */
		if (inode != NULL) {
			err = fatExtendClusterList(fs, &clad, inode);
			if (err < E_OK) goto exit0;

			inode->fino_filsz += (fs->ff_clsz * clad.ca_len);
			if (inode->fino_filsz > FILE_SIZE_MAX) {
				inode->fino_filsz = FILE_SIZE_MAX;
			}
		} else {
			err = fatChainCluster(fs, dircl, &clad, NULL);
		}

		/* Zero clear */
		err = fatMapDisk(fs, &clad, 1, &map, MAP_C | MAP_CLR,(VW)inode);
		fatUnmapDisk(&map);

		/* Entry location */
		offset += nbyte;

	} else {	/* FAT12, FAT16 root directory */
		/* Note: !! do not support arrangement of root directory
			entries to make successive free entries !! */
		err = EX_NOSPC;
		goto exit0;
	}
exit1:
	*ofs = offset;
exit0:
	return err;
}

/*
 *  File initialization
 */
LOCAL	ER	fatInitDirent(FATFS *fs, FAT_DIRENT *dirent, FATNODE *inode)
{
	UH		date, time;
	MapInfo		map;
	CLAD		clad;
	FAT_DIRENT	*de;
	ER		err;

	/* File time */
	err = fatSetDateTime(&date, &time, NULL);
	if (err < E_OK) goto exit0;

	/* Entry initialization */
	memset(dirent, 0, sizeof(FAT_DIRENT));
	dirent->de_ftype = inode->fino_ftype | FAT_ARCHIV;
	//dirent->de_smallcaps = 0;
	dirent->de_adate = date;
	dirent->de_ctime = time;
	dirent->de_cdate = date;
	dirent->de_mdate = date;
	dirent->de_mtime = time;

	/* Directory */
	if (! isFAT_SUBDIR(inode->fino_ftype)) goto exit0;

	/* Append new cluster */
	err = fatApdNewCluster(fs, &clad, fs->ff_clsz, inode);
	if (err < E_OK) goto exit0;

	inode->fino_filsz = fs->ff_clsz;
	inode->fino_fclno = clad.ca_no;
	fcl_to_dirent(fs, dirent, inode->fino_fclno);

	/* Map and initialization of cluster */
	err = fatMapDisk(fs, &clad, 1, &map, MAP_C | MAP_CLR, (VW)inode);
	if (err < E_OK) goto exit1;

	/* Create "." entry  */
	de = (FAT_DIRENT *)&map.m_page->dcp_buf[
			OFFSETinPAGE(fs, map.m_page, CNtoBN(clad.ca_no), 0)];
	memcpy(de->de_fname, nmDOT, BASE_EXTLEN);
	de->de_ftype = FAT_SUBDIR;
	de->de_smallcaps = 0;
	de->de_rsv[0] = 0;
	de->de_adate = date;
	de->de_ctime = time;
	de->de_cdate = date;
	de->de_mtime = time;
	de->de_mdate = date;
	de->de_fsize = 0;
	fcl_to_dirent(fs, de, inode->fino_fclno);

	/* Create ".." entry  */
	de++;
	memcpy(de->de_fname, nmDOT2, BASE_EXTLEN);
	de->de_ftype = FAT_SUBDIR;
	de->de_smallcaps = 0;
	de->de_rsv[0] = 0;
	de->de_adate = date;
	de->de_ctime = time;
	de->de_cdate = date;
	de->de_mtime = time;
	de->de_mdate = date;
	de->de_fsize = 0;
	fcl_to_dirent(fs, de, inode->fino_dircl);

	err = fatDCacheUpdate(fs, map.m_page, (VW)inode);
	(void)fatMapEnd(&map);
exit1:
	fatUnmapDisk(&map);
exit0:
	return err;
}

/*
 *  Copy a directory entry
 */
LOCAL	ER	fatCopyDirent(FATFS *fs, FAT_DIRENT *from, FAT_DIRENT *to,
						FATNODE *inode)
{
	DIRINF	dir;
	ER	err;

	err = E_OK;

	/* Initilization of entry */
	memset(to, 0, sizeof(FAT_DIRENT));
	to->de_ftype = from->de_ftype | FAT_ARCHIV;
	to->de_adate = from->de_adate;
	to->de_ctime = from->de_ctime;
	to->de_cdate = from->de_cdate;
	to->de_mtime = from->de_mtime;
	to->de_mdate = from->de_mdate;
	to->de_clno_hi = from->de_clno_hi;
	to->de_clno_lo = from->de_clno_lo;
	to->de_fsize = from->de_fsize;

	/* Directory */
	if (! isFAT_SUBDIR(inode->fino_ftype)) goto exit0;

	/* Directory open */
	err = fatOpenDIR(fs, &dir, inode->fino_fclno, OFSVOID, AC_WRITE, inode);
	if (err < E_OK) goto exit0;

	/*  ".." entry search */
	err = fatSearchDIR_nm(fs, &dir, (UB*)nmDOT2, 2, AC_WRITE, NULL);
	if (err >= E_OK) {
		fcl_to_dirent(fs, dir.di_dirent, inode->fino_dircl);
		err = fatWriteCurDIR(fs, &dir, 0);
	}

	/* Directory close */
	err = fatCloseDIR(fs, &dir, err);
exit0:
	return err;
}

/*
 *  Update file system information
 */
LOCAL	ER	fatUpdateFileSystem(FATFS *fs)
{
	FAT_FSINFO	*fsi;
	DCachePage	*page;
	ER		err;

	err = E_OK;

	/* Confirm file system information */
	if (fs->ff_fsinfo > 0) {

		/* Map the file system information sector */
		err = fatDCacheStart(fs, fs->ff_fsinfo, &page);
		if (err < E_OK) goto exit0;

		fsi = (FAT_FSINFO *)&page->dcp_buf[
			OFFSETinPAGE(fs, page, fs->ff_fsinfo, 0)];

		/* Update file system information only when changed */
		if ( fsi->fsi_freecl != CEW(fs->ff_freecl) ||
			fsi->fsi_nextfree != CEW(fs->ff_nextfree) ) {

			/* Update the free cluster count */
			fsi->fsi_freecl = CEW(fs->ff_freecl);

			/* Record the start position to search free clusters */
			fsi->fsi_nextfree = CEW(fs->ff_nextfree);

			(void)fatDCacheUpdate(fs, page, 0);
			err = fatDCacheEnd(fs, page);
		}
	}
exit0:
	return err;
}

/*
 *  Update directory entry to the file "inode" content.
 */
LOCAL	ER	fatUpdateFile(FATFS *fs, FATNODE *inode)
{
	DIRINF	dir;
	UH	date, time;
	ER	err;

	/* File time */
	err = fatSetDateTime(&date, &time, NULL);
	if (err < E_OK) goto exit0;

	/* Directory open */
	err = fatOpenDIR(fs, &dir, inode->fino_dircl,
				(W)inode->fino_dirofs, AC_WRITE, NULL);
	if (err < E_OK) goto exit0;

	if (dir.di_dirent == NULL) {
		err = EX_NOENT;
	} else {
		dir.di_dirent->de_fsize = (W)CEW(inode->fino_filsz);
		dir.di_dirent->de_ftype |= FAT_ARCHIV;
		fcl_to_dirent(fs, dir.di_dirent, inode->fino_fclno);
		dir.di_dirent->de_mdate = date;
		dir.di_dirent->de_mtime = time;
		err = fatWriteCurDIR(fs, &dir, (VW)inode);
	}
	/* Directory close */
	err = fatCloseDIR(fs, &dir, err);
	if (err >= E_OK) {
		/* Update the file system information : ignore error */
		(void)fatUpdateFileSystem(fs);
	}
exit0:
	return err;
}

/*
 *  Get parent directory
 */
LOCAL	ER	fatGetParentDirectory(FATFS *fs, W *dircl, FATNODE *inode)
{
	ER	err;
	DIRINF	dir;

	/* Directory open */
	err = fatOpenDIR(fs, &dir, *dircl, OFSVOID, AC_RDONLY, inode);
	if (err < E_OK) return err;

	/* Search of directory ".."  */
	err = fatSearchDIR_nm(fs, &dir, (UB*)nmDOT2, 2, AC_RDONLY, NULL);
	if (err >= E_OK) {
		/* Parent directory started cluster */
		*dircl = (W)dirent_to_fcl(fs, dir.di_dirent);
	}
	/* Directory close */
	return fatCloseDIR(fs, &dir, err);
}

/*
 *  Update the directory entry of the parent diretory of the file "inode".
 */
LOCAL	ER	fatUpdateDirectory(FATFS *fs, FATNODE *inode)
{
	DIRINF	dir;
	FATNODE	*din;
	W	dircl;
	UH	date, time;
	ER	err;

	dircl = 0;

	/* Root directory */
	if (inode->fino_dircl == fs->ff_rootcl) goto exit0;

	/* File reference node search */
	din = (FATNODE *)fatInodeSearch((FS*)fs, dirINO(fs, inode->fino_dircl));

	/* File modification time */
	err = fatSetDateTime(&date, &time, NULL);
	if (err < E_OK) goto exit0;

	/* Get Parent directory started cluster */
	dircl = inode->fino_dircl;
	err = fatGetParentDirectory(fs, &dircl, din);
	if (err < E_OK) goto exit0;

	/* Parent directory open */
	err = fatOpenDIR(fs, &dir, (UW)dircl, OFSVOID, AC_WRITE, NULL);
	if (err < E_OK) goto exit0;

	/* Search of directory entry  */
	err = fatSearchDIR_cl(fs, &dir, inode->fino_dircl, AC_WRITE);
	if (err >= E_OK) {
		dir.di_dirent->de_mdate = date;
		dir.di_dirent->de_mtime = time;
		err = fatWriteCurDIR(fs, &dir, (VW)inode);
	}
	/* Parent directory close */
	err = fatCloseDIR(fs, &dir, err);
exit0:
	/* Update the file system information : ignore error */
	(void)fatUpdateFileSystem(fs);
	return E_OK;
}

/*
 *  Check if directory "to" is not the subdirectory of directory "from".
 */
LOCAL	ER	fatCheckSubDirectory(FATFS *fs, FATNODE *from, FATNODE *to)
{
	W	dircl;
	ER	err;

	err = E_OK;

	if (isFAT_SUBDIR(from->fino_ftype)) {
		for (dircl = to->fino_dircl; ;) {
			/* Error by the consistence of subdirectory */
			if (dircl == from->fino_fclno) {
				err = EX_INVAL;
				break;
			}
			/* Root directory */
			if (dircl == fs->ff_rootcl) break;

			/* Get Parent directory started cluster */
			err = fatGetParentDirectory(fs, &dircl, NULL);
			if (err < E_OK) break;
		}
	}
	return err;
}

/*
 *  Create a new file from the file "inode" directory and the file type.
 *  If copy file "rename" is designated, copy "rename" to new file.
 */
LOCAL	ER	fatCreateFile(FATFS *fs, FATNODE *inode, UB *utf8nm,
							FAT_DIRENT *rename)
{
	UB		sfname[BASE_EXTLEN];
	W		utf8len, utf16len, ofs;
	W		gen, idx, nent, enc;
	UB		smallcaps, chksum;
	DIRINF		dir;
	FAT_DIRENT	*dirent;
	ER		err;

	err = E_OK;

	/* Remove trailing ' ' and '.' of pathname */
	utf8len = strlen((B*)utf8nm);
	while (utf8nm[utf8len - 1] == ' ' ||
				utf8nm[utf8len - 1] == '.') utf8len--;

	/* Convert to UTF16 LFN */
	utf16len = fatEncUtf8strToUtf16str(utf8nm, utf8len,
					fs->ff_nmbf, LFN_MAXBUF, FALSE);
	if (utf16len <= 0) {
		err = EX_INVAL;
		goto exit1;
	}
	if (utf16len > LFN_MAXLEN) {
		err = EX_NAMETOOLONG;
		goto exit1;
	}

	/* Generate SFN which is unique in the directory */
	smallcaps = 0x00;
	for (nent = 1, gen = 1; ; ) {

		/* Generation numbered file name conversion */
		enc = fatSetSFN(utf8nm, utf8len, sfname, &smallcaps, gen);
		if (enc == FAT_SAME_NAME) break;	/* LFN not required */

		smallcaps = 0x00;

		/* Check file name increasing last digit of generation number
			from '1' to '9' */
		err = fatCheckDirent(fs, inode->fino_dircl, sfname,
					(enc == FAT_CHG_WITH_N) ? 9 : 1);
		if (err < E_OK) break;
		if (err > E_OK) {	/* Check OK */
			nent += (utf16len + LFN_CHARS - 1) / LFN_CHARS;
			break;
		}
		/* Check again with new generation number : ..N1 */
		gen += 10;
	}

	/* Get empty directory entry */
	err = fatSearchFreeDirent(fs, inode->fino_dircl, (UW)nent, &ofs);
	if (err < E_OK) goto exit1;

	/* Entry location */
	inode->fino_dirofs = (UW)ofs;

	/* Directory open */
	err = fatOpenDIR(fs, &dir, inode->fino_dircl, (W)inode->fino_dirofs,
						AC_WRITE, NULL);
	if (err < E_OK) goto exit1;

	if (dir.di_dirent == NULL) {
		err = EX_NOENT;
		goto exit2;
	}

	/* Set SFN entry */
	dirent = dir.di_dirent;
	if (rename != NULL) {	/* Entry copy */
		err = fatCopyDirent(fs, rename, dirent, inode);
	} else {		/* Entry initialization */
		err = fatInitDirent(fs, dirent, inode);
	}
	if (err < E_OK) goto exit2;

	dirent->de_smallcaps = smallcaps;
	memcpy(dirent->de_fname, sfname, BASE_EXTLEN);
	inode->fino_c.ino_ino = fatGetInoDIR(fs, &dir);
	inode->fino_c.ino_type =
			isFAT_SUBDIR(inode->fino_ftype) ? DT_DIR : DT_REG;
	err = fatWriteCurDIR(fs, &dir, (VW)inode);
	if (err < E_OK) goto exit2;

	/* Set LFN entry */
	if (nent > 1)  {
		/* Checksum */
		chksum = fatCalcChksum(sfname);

		for (idx = 1; idx < nent; idx++) {
			/* Previous */
			err = fatPrevDIR(fs, &dir, AC_WRITE);
			if (err < E_OK) break;

			fatSetLFN(fs->ff_nmbf, utf16len,
				(FAT_DIRENT_LFN *)dir.di_dirent, idx, chksum);
			err = fatWriteCurDIR(fs, &dir, (VW)inode);
			if (err < E_OK) break;
		}
	}
exit2:
	/* Directory close */
	err = fatCloseDIR(fs, &dir, err);
exit1:
	return err;
}

/*
 *  Deletion of file
 */
LOCAL	ER	fatRemoveFile(FATFS *fs, FATNODE *inode)
{
	DIRINF	dir;
	ER	err;

	/* Directory open */
	err = fatOpenDIR(fs, &dir, inode->fino_dircl, (W)inode->fino_dirofs,
						AC_WRITE, NULL);
	if (err < E_OK) goto exit0;

	if (dir.di_dirent == NULL) {
		err = EX_NOENT;
		goto exit0;
	}

	/* Delete directory entry */
	dir.di_dirent->de_fname[0] = DE_B0_FREE;
	err = fatWriteCurDIR(fs, &dir, (VW)inode);
	if (err < E_OK) goto exit0;

	/* Deletion of LFN entry */
	for (;;) {
		if (dir.di_offset == 0) break;	/* Top entry */

		/* Previous */
		err = fatPrevDIR(fs, &dir, AC_WRITE);
		if (err < E_OK) break;

		if (! isDE_LFN(dir.di_dirent)) break;

		/* Delete LFN entry */
		dir.di_dirent->de_fname[0] = DE_B0_FREE;
		err = fatWriteCurDIR(fs, &dir, (VW)inode);
		if (err < E_OK) break;
	}

	/* Directory close */
	err = fatCloseDIR(fs, &dir, err);
	if (err >= E_OK) {
		/* Delete clusters */
		err = fatTruncClusterList(fs, 0, inode);
	}
exit0:
	return err;
}

/*
 *  Extension of file size
 */
LOCAL	ER	fatExtendFile(FATFS *fs, FATNODE *inode, D size)
{
	CLAD	clad[MAX_CLEN];
	MapInfo	map;
	D	fsz, incsz;
	W	cllen, ofs, i;
	UW	clsz;
	ER	err;

	err = E_OK;

	/* File size */
	fsz = (UW)inode->fino_filsz;
	if (fsz >= size) goto exit0;

	/* Check free clusters */
	if (BYTEtoCL(size) - BYTEtoCL(fsz) > fs->ff_freecl) {
		err = EX_NOSPC;
		goto exit0;
	}

	/* Extend file size */
	for (ofs = fsz % fs->ff_clsz; (incsz = size - fsz) > 0; ofs = 0) {

		/* Get next cluster list */
		err = fatCopyClusterList(fs, clad, fsz, incsz, inode, &cllen);
		if (err < E_OK) goto exit0;

		/* Note; if ofs > 0, cllen == 1, and clad[0].ca_len = 1 */

		if (cllen <= 0) {	/* Append new cluster */
			err = fatApdNewCluster(fs, &clad[0], incsz, inode);
			if (err < E_OK) goto exit0;
			cllen = 1;
		}

		/* Zero clear */
		err = fatMapDisk(fs, clad, cllen, &map, MAP_C |
					((ofs > 0) ? 0 : MAP_CLR), (VW)inode);
		if (err >= E_OK && ofs > 0) {
			/* Clear a cluster after ofs : 1st time only */
			err = fatMapWriteBytes(&map, ofs, NULL,
					(incsz < fs->ff_clsz - ofs) ?
						incsz : fs->ff_clsz - ofs);
			(void)fatMapEnd(&map);
		}
		fatUnmapDisk(&map);
		if (err < E_OK) break;

		/* Calc new file cluster size */
		for (clsz = 0, i = 0; i < cllen; i++) clsz += clad[i].ca_len;
		fsz += clsz * fs->ff_clsz - ofs;
	}

	/* File update */
	inode->fino_filsz = size;
	err = fatUpdateFile(fs, inode);
exit0:
	return err;
}

/*
 *  Reduction of file size
 */
LOCAL	ER	fatTruncateFile(FATFS *fs, FATNODE *inode, D size)
{
	ER	err;

	err = E_OK;
	if (size < inode->fino_filsz) {
		/* File reduction */
		err = fatTruncClusterList(fs, size, inode);
		if (err >= E_OK) {
			/* File update */
			err = fatUpdateFile(fs, inode);
		}
	}
	return err;
}

/*
 *  Check if the directory "inode" includes the active entry or not.
 */
LOCAL	ER	fatChkEmptyDirectory(FATFS *fs, FATNODE *inode)
{
	DIRINF		dir;
	FAT_DIRENT	*dirent;
	ER		err;

	/* Directory open */
	err = fatOpenDIR(fs, &dir, inode->fino_fclno, OFSVOID,
							AC_RDONLY, inode);
	if (err < E_OK) goto exit0;

	for (;;) {
		/* Ge next dirent */
		err = fatNextDIR(fs, &dir, AC_RDONLY);
		if (err < E_OK) break;

		dirent = dir.di_dirent;
		if (dirent == NULL ||		/* Last entry */
			isDE_TERM(dirent)) {	/* Terminated entry */
			break;
		}
		if (isDE_FREE(dirent) ||	/* Free entry */
			isDE_LFN(dirent) ||	/* LFN entry */
			isDE_VOL(dirent) ) {	/* VOLUME label */
			continue;
		}
		/* SFN entry, ignore "." or ".." entry */
		if (memcmp(dirent->de_fname, nmDOT, BASE_EXTLEN) != 0 &&
		    memcmp(dirent->de_fname, nmDOT2, BASE_EXTLEN) != 0) {
			err = EX_NOTEMPTY;
			break;			/* Active entry */
		}
	}

	/* Directory close */
	err = fatCloseDIR(fs, &dir, err);
exit0:
	return err;
}

/*
 *  Get file information
 */
LOCAL	ER	fatGetAttr64us(FATFS *fs, FATNODE *inode,
						struct stat64_us *sb)
{
	ER	err;
	UB	ftyp;

	err = E_OK;
	memset(sb, 0, sizeof(struct stat64_us));

	/* Device ID */
	sb->st_dev = (dev_t)fs->ff_c.fs_devnum;

	/* File number:
		Note: It may not be unique, because it ignores upper word */
	sb->st_ino = (ino_t)inode->fino_c.ino_ino;

	/* File mode */
	ftyp = inode->fino_ftype;
	sb->st_mode = DEFAULT_FILMODE;
	if (! isFAT_RDONLY(ftyp)) {
		sb->st_mode |= FILMODE_WRITE;
	}
	sb->st_mode |= isFAT_SUBDIR(ftyp) ? S_IFDIR : S_IFREG;

	/* The number of links */
	sb->st_nlink = 1;

	/* user ID : sb->st_uid = 0 */
	/* group ID : sb->st_gid = 0 */

	/* File size */
	sb->st_size = inode->fino_filsz;

	/* File time */
	if (! IsRootINODE(fs, inode)) {
		DIRINF dir;

		err = fatOpenDIR(fs, &dir, inode->fino_dircl,
				(W)inode->fino_dirofs, AC_RDONLY, NULL);
		if (err >= E_OK) {
			if (dir.di_dirent == NULL) {
				err = EX_NOENT;
			} else {
				fatGetDateTime(dir.di_dirent, sb);
			}
			/* Directory close */
			err = fatCloseDIR(fs, &dir, err);
		}
	}

	/* Device ID : sb->st_rdev = 0 */

	/* A file system-specific preferred I/O block size */
	sb->st_blksize = fs->ff_clsz;

	/* Number of blocks allocated */
	if (inode->fino_fclno > 0) {
		W	num = 0;

		if (isFAT_SUBDIR(ftyp)) {
			err = fatGetClusterCount(fs, inode->fino_fclno,
								NULL, &num);
		} else {
			num = BYTEtoCL(inode->fino_filsz);
		}
		sb->st_blocks = (blkcnt_t)num;
	}
	return err;
}

/*
 *  Confirmation of the disk partition information
 */
LOCAL	ER	fatCheckPartitionID(FATFS *fs, FATFSType *fstype)
{
	DiskPartInfo	inf;
	ER		err;

	*fstype = 0;		/* Unknown */

	/* Reading of partition information */
	err = fatDiskRead(fs, DN_DISKPARTINFO, (B*)&inf, sizeof(inf));
	if (err < E_OK) {
		if (err == E_NOSPT || err == E_PAR) err = E_OK;
	} else {
		switch (inf.systemid) {
		case DSID_DOS1:	/* 0x01: FAT12 */
			//*fstype = 0;	/* judged by the number of clusters */
			break;
		case DSID_DOS2:	/* 0x04: FAT16 */
			//*fstype = 0;	/* judged by the number of clusters */
			break;
		case DSID_DOS3:	/* 0x06: FAT16 */
			//*fstype = 0;	/* judged by the number of clusters */
			break;
		case DSID_DOS3L:	/* 0x0e: FAT16 */
			//*fstype = 0;	/* judged by the number of clusters */
			break;
		case DSID_WIN95:	/* 0x0b: FAT32 */
			*fstype = FAT32;
			break;
		case DSID_WIN95L:	/* 0x0c: FAT32 */
			*fstype = FAT32;
			break;
		default:
			err = EX_NOTSUP;
			break;
		}
	}
	return err;
}

/*
 *  Confirmation of file system
 */
LOCAL	ER	fatCheckFileSystem(FATFS *fs)
{
	FAT_BOOTSEC	*boot;
	FAT_FSINFO	*fsi;
	FATFSType	fstype;
	UB		*blkbuf;
	UW		dsksz, secsz, rsvsc, nfat;
	UH		fsinfo;
	ER		err;

	/* Allocate block read buffer */
	if ((blkbuf = fimp_malloc(DSECSZ(fs))) == NULL) {
		err = EX_NOMEM;
		goto exit0;
	}

	/* Confirmation of the disk parttion information */
	err = fatCheckPartitionID(fs, &fstype);
	if (err < E_OK) goto exit1;

	/* Check boot sector */
	boot = (FAT_BOOTSEC *)blkbuf;
	err = fatDiskRead(fs, 0, boot, BPB_READ_SIZE);
	if (err < E_OK) goto exit1;

	/* Confirmation of boot sector */
	err = EX_NOTSUP;
	if (	(((UB *)boot)[BPB_TAILSIGN_OFS1] != BPB_TAILSIGN_VAL1) ||
		(((UB *)boot)[BPB_TAILSIGN_OFS2] != BPB_TAILSIGN_VAL2)) {
		goto exit1;
	}

	/* File system information */
	secsz = CEH(GetMisalignH(boot->bo_scsz));
	fs->ff_ratio = boot->bo_clsz;
	fs->ff_clsz = (UH)(secsz * fs->ff_ratio);
	rsvsc = CEH(GetMisalignH(boot->bo_rsvsc));
	nfat = boot->bo_nfat;
	fs->ff_fatsz = CEH(boot->bo_fatsz);
	if (fs->ff_fatsz == 0) {
		fs->ff_fatsz = ((UW)CEH(boot-> bo_fatsz_hi) << BPB_SHIFT_FATSZ)
				| (UW)CEH(boot->bo_fatsz_lo);
	}
	dsksz = CEH(GetMisalignH(boot->bo_dsksz));
	if (dsksz == 0) {
		dsksz = ((UW)CEH(boot->bo_dsksz_hi) << BPB_SHIFT_DSKSZ) |
				(UW)CEH(boot->bo_dsksz_lo);
	}

	if (secsz <= 0 || (W)secsz != DSECSZ(fs) ||
	    fs->ff_ratio <= 0 || (W)dsksz > fs->ff_c.fs_diskinfo.blockcont) {
		goto exit1;
	}

	fs->ff_rootent = CEH(GetMisalignH(boot->bo_rootent));
	fs->ff_fat = rsvsc;

CHK_AGAIN:
	/* FAT 32 root directory is also inside the cluster area */
	if (isFAT32(fstype)) {
		fs->ff_rootcl = ((UW)CEH(boot->bo_root_hi) <<
				BPB_SHIFT_ROOT_HI) | (UW)CEH(boot->bo_root_lo);
		if ((boot->bo_extflg & FAT_NoFATMirror) != 0) {
			fs->ff_fat += (boot->bo_extflg & FAT_ActiveFAT) *
						fs->ff_fatsz;
			fs->ff_nfat = 1;
		} else {
			fs->ff_nfat = (UB)nfat;
		}
		fsinfo = CEH(boot->bo_fsinfo);
	}
	/* FAT12, 16 root directories are outside the cluster area */
	else {
		fs->ff_rootcl = 0;
		fs->ff_nfat = (UB)nfat;
		fsinfo = 0;
	}

	/* Root directory and cluster area */
	if (isFAT32(fstype)) {
		fs->ff_rootsc = 0;	/* FAT32 is meaningless */
		fs->ff_clstart = rsvsc + (fs->ff_fatsz * nfat);
	} else {
		fs->ff_rootsc = rsvsc + (fs->ff_fatsz * nfat);
		fs->ff_clstart = (UW)(fs->ff_rootsc +
					(ROOTSZ + secsz - 1) / secsz);
	}

	/* Last cluster number */
	fs->ff_lastcl = ((dsksz - fs->ff_clstart) / fs->ff_ratio) + 1;

	/* Re-judgment of file system */
	if (fstype == 0) {
		if (fs->ff_lastcl < FAT12_EOC) {
			fstype = FAT12;
		} else if (fs->ff_lastcl < FAT16_EOC) {
			fstype = FAT16;
		} else {
			fstype = FAT32;
			goto CHK_AGAIN;
		}
	}

	fs->ff_fstype = fstype;
	fs->ff_fsinfo = 0;		/* Invalid */

	/* Check file system information (only FAT32) */
	if (fsinfo > 0) {
		/* Map of file system information sector */
		fsi = (FAT_FSINFO *)blkbuf;
		err = fatDiskRead(fs, fsinfo, fsi, FSINFO_READ_SIZE);
		if (err < E_OK) goto exit1;

		/* Check signature */
		if (	fsi->fsi_leadsig == CEW(FSINFO_LEADSIGN_VAL) &&
			fsi->fsi_structsig == CEW(FSINFO_STRUCTSIGN_VAL) &&
			fsi->fsi_trailsig == CEW(FSINFO_TAILSIGN_VAL)) {
			fs->ff_fsinfo = fsinfo;		/* Active */

			/* Note: Do not use the free cluster count
				(fsi->fsi_freecl), but update it. */

			/* Use the next free cluster number */ 
			fs->ff_nextfree = fsi->fsi_nextfree;
		}
	}
	err = E_OK;
exit1:
	fimp_free(blkbuf);
exit0:
	return err;
}

/*
 *  Reset current directory to root directory
 */
LOCAL	void	fatResetCurdir(FS *fs)
{
	CDINF	*curdir;
	INODE	*rootinode;

	curdir = &fs->fs_uxinfo.uxi_curdir;

	if (! IsRootINODE(fs, curdir->cd_inode)) {
		/* Set root as curdir */
		(void)fatGetRootINODE((FATFS *)fs, (FATNODE **)&rootinode);
		fatInodeRegister(fs, rootinode, O_RDONLY);
		/* Regist curdir->cd_inode for release after current
				proccessing using this inode has done. */
		fs->fs_uxinfo.uxi_relino = curdir->cd_inode;
		curdir->cd_inode = rootinode;
	}
}

/*
 *  Check inode is busy or not
 */
LOCAL	BOOL	fatCheckIsInodeBusy(FS *fs, INODE *inode)
{
	if (inode->ino_refcnt <= 0) return FALSE;

	if (inode == fs->fs_uxinfo.uxi_curdir.cd_inode &&
						inode->ino_refcnt == 1) {
		/* Reset current directory to make inode free */
		fatResetCurdir(fs);
		return FALSE;
	}
	return TRUE;		/* Busy */
}

/*
 *  Update access date
 */
LOCAL	ER	fatUpdateAccessDate(FATFS *fs, FATNODE *inode)
{
	DIRINF	dir;
	UH	date, time;
	ER	err;

	err = E_OK;
	if (LastAccess != FALSE && fs->ff_c.fs_rdonly == FALSE &&
						! IsRootINODE(fs, inode)) {
		/* Get date & time */
		err = fatSetDateTime(&date, &time, NULL);
		if (err < E_OK) goto exit0;

		/* Directpory open */
		err = fatOpenDIR(fs, &dir, inode->fino_dircl,
				(W)inode->fino_dirofs, AC_WRITE, NULL);
		if (err < E_OK) goto exit0;

		/* File access date */
		if (dir.di_dirent != NULL && dir.di_dirent->de_adate != date){
			dir.di_dirent->de_adate = date;
			err = fatWriteCurDIR(fs, &dir, (VW)inode);
		}

		/* Directory close */
		err = fatCloseDIR(fs, &dir, err);
	}
exit0:
	return err;
}

/*----------------------------------------------------------------------------
	FATFS API functions
----------------------------------------------------------------------------*/

/*
 *  Analize path name and get inode
 */
LOCAL	ER	fat_lookup(FATFS *fs, LPINF *lp, const B *r_path)
{
	CDINF	*curdir;
	FATNODE	*inode;
	DIRINF	dir;
	UB	*path, *cpath, *npath;
	UW	len, dcl;
	UB	dtp;
	ER	err;

	err = E_OK;
	cpath = path = (UB*)&r_path[fs->ff_c.fs_conlen];

	/* Check root directory */
	if (path[0] == '\0') {		/* Root directory */
		(void)fatGetRootINODE(fs, &inode);
		goto exit1;
	}

	/* Check current directory */
	curdir = &fs->ff_c.fs_uxinfo.uxi_curdir;
	len = strlen((B*)curdir->cd_inode->ino_path);

	if (fat_strncasecmp(curdir->cd_inode->ino_path, path, len) == 0
			&& (path[len] == '\0' || path[len] == '/')) {
		/* Under current directory */
		if (path[len] == '\0') {
			/* Current directory */
			if ((cpath = (UB*)strrchr((B*)path, '/')) != NULL) cpath++;
			else	cpath = path;
			inode = (FATNODE*)curdir->cd_inode;
			goto exit1;
		}
		/* Search from current directory */
		dcl = dirCLNO(fs, curdir->cd_inode->ino_ino);
		cpath += len + 1;
	} else {
		/* Search from top */
		dcl = fs->ff_rootcl;
		cpath += 1;
	}

	/* Search parent directory */
	dtp = FAT_SUBDIR;
	dir.di_dirent = NULL;
	for ( ; (npath = (UB*)strchr((B*)cpath, '/')) != NULL; cpath = npath + 1) {

		/* Open directory */
		err = fatOpenDIR(fs, &dir, dcl, OFSVOID, AC_RDONLY, NULL);
		if (err < E_OK) break;

		/* Search name */
		err = fatSearchDIR_nm(fs, &dir, cpath,
					npath - cpath, AC_RDONLY, NULL);
		if (err >= E_OK) {
			if (isFAT_SUBDIR(dir.di_dirent->de_ftype)) {
				/* Parent directory */
				dcl = dirent_to_fcl(fs, dir.di_dirent);
				dtp = dir.di_dirent->de_ftype;
			} else {
				dir.di_dirent = NULL;
				err = EX_NOTDIR;
			}
		}
		/* Close directory */
		err = fatCloseDIR(fs, &dir, err);

		CHK_BREAK_AND_SET_ERR(fs, &err);
		if (err < E_OK) break;
	}
	if (err < E_OK) goto exit0;

	if (dir.di_dirent != NULL) {
		err = fatGetINODE(fs, &dir, dtp, path, cpath - path -1, &inode);
		if (err >= E_OK && curdir->cd_inode != (INODE*)inode) {
			/* Set parent directory as a current directory */
			fatInodeRegister((FS*)fs, (INODE*)inode, O_RDONLY);
			fatInodeRelease((FS*)fs, curdir->cd_inode, O_RDONLY);
			curdir->cd_inode = (INODE*)inode;
		}
		CHK_BREAK_AND_SET_ERR(fs, &err);
	}
	if (err < E_OK) goto exit0;

	/* Open parent directory */
	err = fatOpenDIR(fs, &dir, dcl, OFSVOID, AC_RDONLY, NULL);
	if (err < E_OK) goto exit0;

	/* Search file */
	inode = NULL;
	err = fatSearchDIR_nm(fs, &dir, cpath, strlen((B*)cpath), AC_RDONLY, NULL);
	if (err >= E_OK || err == EX_NOENT) {
		err = fatGetINODE(fs, &dir, dtp, path, strlen((B*)path), &inode);
	}

	/* Close parent directory */
	err = fatCloseDIR(fs, &dir, err);
	if (err < E_OK) {
		/* Free memory that is allocated by fatGetINODE() */
		if (inode != NULL && inode->fino_c.ino_refcnt == 0) {
			fatInodeFree((FS*)fs, (INODE*)inode);
		}
	} else {
exit1:		/* Set result */
		lp->lp_inode = (INODE*)inode;
		lp->lp_path = cpath;
	}
	CHK_BREAK_AND_SET_ERR(fs, &err);
exit0:
	return err;
}

/*
 *  Truncate or extend size of file specified by path name
 */
LOCAL	ER	fat_truncate64(FATFS *fs, FATNODE *inode, D size)
{
	ER	err;

	if (isFAT_RDONLY(inode->fino_ftype)) {
		err = EX_ACCES;
	} else if (size > FILE_SIZE_MAX) {
		err = EX_FBIG;
	} else if (size < 0) {
		err = EX_INVAL;
	} else if (size > inode->fino_filsz) {
		err = fatExtendFile(fs, inode, size);
	} else {
		err = fatTruncateFile(fs, inode, size);
	}
	return err;
}

/*
 *  Truncate or extend size of file specified by descriptor
 */
LOCAL	ER	fat_ftruncate64(FD *fd, D size)
{
	return fat_truncate64((FATFS*)fd->fd_fs, (FATNODE*)fd->fd_inode, size);
}

/*
 *  Change access and modification time of file specified by path name
 */
LOCAL	ER	fat_utimes_us(FATFS *fs, FATNODE *inode,
						const SYSTIM_U times_u[2])
{
	DIRINF	dir;
	UH	date[2], time[2];
	ER	err;

	err = E_OK;

	if (IsRootINODE(fs, inode)) goto exit0;

	/* Set date & time */
	err = fatSetDateTime(&date[0], &time[0], &times_u[0]);
	if (err >= E_OK) {
		err = fatSetDateTime(&date[1], &time[1], &times_u[1]);
	}
	if (err < E_OK) goto exit0;

	/* Open directory */
	err = fatOpenDIR(fs, &dir, inode->fino_dircl,
				(W)inode->fino_dirofs, AC_WRITE, NULL);
	if (err >= E_OK) {
		if (dir.di_dirent == NULL) {
			err = EX_NOENT;
		} else {
			/* Set times */
			dir.di_dirent->de_adate = date[0];
			dir.di_dirent->de_mdate = date[1];
			dir.di_dirent->de_mtime = time[1];
			err = fatWriteCurDIR(fs, &dir, (VW)inode);
		}
		/* Directory close */
		err = fatCloseDIR(fs, &dir, err);
	}
exit0:
	return err;
}

/*
 *  Change mode of file specified by path name
 */
LOCAL	ER	fat_chmod(FATFS *fs, FATNODE *inode, mode_t mode)
{
	DIRINF	dir;
	ER	err;

	if (IsRootINODE(fs, inode)) {
		err = isSET(mode, FILMODE_WRITE) ? E_OK : EX_INVAL;
	} else {
		/* Open directory */
		err = fatOpenDIR(fs, &dir, inode->fino_dircl,
				(W)inode->fino_dirofs, AC_WRITE, NULL);
		if (err >= E_OK) {
			if (dir.di_dirent == NULL) {
				err = EX_NOENT;
			} else {
				/* Change file mode */
				if (isSET(mode, FILMODE_WRITE)) {
					dir.di_dirent->de_ftype &= ~FAT_RDONLY;
				} else {
					dir.di_dirent->de_ftype |= FAT_RDONLY;
				}
				err = fatWriteCurDIR(fs, &dir, (VW) inode);
				inode->fino_ftype = dir.di_dirent->de_ftype;
			}
			/* Close directory */
			err = fatCloseDIR(fs, &dir, err);
		}
	}
	return err;
}

/*
 *  Change mode of file specified by descriptor
 */
LOCAL	ER	fat_fchmod(FD *fd, mode_t mode)
{
	return fat_chmod((FATFS*)fd->fd_fs, (FATNODE*)fd->fd_inode, mode);
}

/*
 *  Rename file name
 */
LOCAL	ER	fat_rename(FATFS *fs, FATNODE *from,
						FATNODE *to, const UB *fname)
{
	DIRINF		dir;
	FAT_DIRENT	de;
	UW		dircl, dirofs;
	ER		err;

	/* Check readonly */
	if (from->fino_d_ronly || to->fino_d_ronly) {
		err = EX_ACCES;
		goto exit0;
	}

	/* Check directory "to" is not the subdirectory of "from" */
	err = fatCheckSubDirectory(fs, from, to);
	if (err < E_OK) goto exit0;

	/* Check rename of directory */
	if (isFAT_SUBDIR(from->fino_ftype) && isFAT_RDONLY(from->fino_ftype)
				&& to->fino_dircl != from->fino_dircl) {
		err = EX_ACCES;
		goto exit0;
	}

	/* Open "from" directory */
	err = fatOpenDIR(fs, &dir, from->fino_dircl,
				(W)from->fino_dirofs, AC_RDONLY, NULL);
	if (err >= E_OK) {
		/* Save "from" entry save */
		memcpy(&de, dir.di_dirent, DIRENTSZ);

		/* Close "from" directory */
		err = fatCloseDIR(fs, &dir, E_OK);
	}
	if (err < E_OK) goto exit0;

	if (to->fino_c.ino_ino != NOEXS_INO) {	/* "to" exists */

		if (isFAT_SUBDIR(to->fino_ftype)) {
			/* Rename directory, check "to" is empty */
			err = fatChkEmptyDirectory(fs, to);
			if (err == EX_NOTEMPTY) err = EX_EXIST;
		}
		if (err < E_OK) goto exit0;

		/* Open "to" directory */
		err = fatOpenDIR(fs, &dir, to->fino_dircl,
					(W)to->fino_dirofs, AC_WRITE, NULL);
		if (err < E_OK) goto exit0;

		if (dir.di_dirent == NULL) {
			err = EX_NOENT;
		} else {
			/* Copy directory entry except name */
			dir.di_dirent->de_ftype = de.de_ftype | FAT_ARCHIV;
			dir.di_dirent->de_ctime = de.de_ctime;
			dir.di_dirent->de_cdate = de.de_cdate;
			dir.di_dirent->de_adate = de.de_adate;
			dir.di_dirent->de_clno_hi = de.de_clno_hi;
			dir.di_dirent->de_mtime = de.de_mtime;
			dir.di_dirent->de_mdate = de.de_mdate;
			dir.di_dirent->de_clno_lo = de.de_clno_lo;
			dir.di_dirent->de_fsize = de.de_fsize;

			err = fatWriteCurDIR(fs, &dir, (VW)to);
		}
		/* Close "to" directory */
		err = fatCloseDIR(fs, &dir, err);
		if (err < E_OK) goto exit0;

		/* Delete original "to" file and "from" directory entry */
		dircl = to->fino_dircl;
		dirofs = to->fino_dirofs;
		to->fino_dircl = from->fino_dircl;
		to->fino_dirofs = from->fino_dirofs;

		err = fatRemoveFile(fs, to);
		if (err < E_OK) goto exit0;

		/* Update "from" directory */
		err = fatUpdateDirectory(fs, to);
		if (err < E_OK) goto exit0;

		/* Transfer file reference node */
		if (! isFAT_SUBDIR(to->fino_ftype)) {
			from->fino_c.ino_ino = to->fino_c.ino_ino;
		}
		from->fino_dircl = dircl;
		from->fino_dirofs = dirofs;

	} else {	/* "to" does not exist */
		dircl = from->fino_dircl;
		dirofs = from->fino_dirofs;
		from->fino_dircl = to->fino_dircl;

		/* Create new file in "to" directory */
		err = fatCreateFile(fs, from, (UB*)fname, &de);
		if (err < E_OK) goto exit0;

		/* Delete original "from" file */
		to->fino_dircl = dircl;
		to->fino_dirofs = dirofs;

		err = fatRemoveFile(fs, to);
		if (err < E_OK) goto exit0;

		/* Update "to" directory */
		err = fatUpdateDirectory(fs, from);
		if (err < E_OK) goto exit0;

		/* Update "from" directory */
		if (from->fino_dircl != to->fino_dircl) {
			err = fatUpdateDirectory(fs, to);
		}
	}
	if (err >= E_OK) {
		/* Change inode queue entry */
		fatInodeChangeQue((FS *)fs, (INODE *)from);
	}
exit0:
	return err;
}

/*
 *  Delete directory entry and file itself
 */
LOCAL	ER	fat_unlink(FATFS *fs, FATNODE *inode)
{
	ER	err;

	if (inode->fino_d_ronly != FALSE) {
		err = EX_ACCES;
	} else {
		/* Delete file */
		err = fatRemoveFile(fs, inode);
		if (err >= E_OK) {
			err = fatUpdateDirectory(fs, inode);
		}
	}
	return err;
}

/*
 *  Create directory
 */
LOCAL	ER	fat_mkdir(FATFS *fs, FATNODE *inode,
						const UB *fname, mode_t mode)
{
	ER	err;

	if (inode->fino_d_ronly != FALSE) {
		err = EX_ACCES;
	} else {
		/* Set file type */
		inode->fino_ftype = FAT_SUBDIR;
		if (! isSET(mode, FILMODE_WRITE)) {
			inode->fino_ftype |= FAT_RDONLY;
		}

		/* Create new directory */
		err = fatCreateFile(fs, inode, (UB*)fname, NULL);
		if (err >= E_OK) {
			err = fatUpdateDirectory(fs, inode);
		}
	}
	return err;
}

/*
 *  Delete directory
 */
LOCAL	ER	fat_rmdir(FATFS *fs, CDINF *curdir, FATNODE *inode)
{
	ER	err;

	if (inode->fino_d_ronly != FALSE) {
		err = EX_ACCES;
		goto exit0;
	}

	if (inode == (FATNODE*)curdir->cd_inode) {
		/* Reset current directory */
		fatResetCurdir((FS*)fs);
	}

	/* Check directory is empty */
	err = fatChkEmptyDirectory(fs, inode);
	if (err >= E_OK) {
		err = fatRemoveFile(fs, inode);
		if (err >= E_OK) {
			err = fatUpdateDirectory(fs, inode);
		}
	}
exit0:
	return err;
}

/*
 *  Get file status - stat64_us
 */
LOCAL	ER	fat_stat64_us(FATFS *fs, FATNODE *inode, struct stat64_us *sb)
{
	return fatGetAttr64us(fs, inode, sb);
}

/*
 *  Get file status - fstat64_us
 */
LOCAL	ER	fat_fstat64_us(FD *fd, struct stat64_us *sb)
{
	return fatGetAttr64us((FATFS*)fd->fd_fs, (FATNODE*)fd->fd_inode, sb);
}

/*
 *  Read file data
 */
LOCAL	ER	fat_read64(FD *fd, UB *buf, UW len, UW *alen, D *aoffset)
{
	FATFS	*fs;
	FATNODE	*inode;
	CLAD	clad[MAX_CLEN];
	D	offset;
	UW	actual, ofs, sz;
	W	cllen, i;
	MapInfo	map;
	ER	err;

	fs = (FATFS *)fd->fd_fs;
	inode = (FATNODE *)fd->fd_inode;
	offset = fd->fd_fpos;
	err = E_OK;

	*alen = 0;
	*aoffset = offset;

	/* Check read offset */
	if (offset >= inode->fino_filsz) {
		if (offset > inode->fino_filsz) {
			err = EX_OVERFLOW;
		}
		goto exit0;
	}

	/* Check read size */
	if (offset + len > inode->fino_filsz) {
		len = inode->fino_filsz - offset;
	}

	/* Read offset within 1st cluster */
	ofs = offset % fs->ff_clsz;

	for (actual = 0; len > 0; actual += sz, len -= sz, ofs = 0) {

		/* Get next cluster list */
		err = fatCopyClusterList(fs, clad, offset + actual,
							len, inode, &cllen);
		if (err < E_OK) break;

		/* Map area size */
		for (sz = 0, i = 0; i < cllen; i++) {
			sz += clad[i].ca_len;
		}
		sz *= fs->ff_clsz;

		/* Read size */
		if ((sz -= ofs) > len) sz = len;

		/* Map data area */
		err = fatMapDisk(fs, clad, cllen, &map, MAP_C, (VW)inode);
		if (err < E_OK) break;

		/* Read data */
		err = fatMapReadBytes(&map, ofs, &buf[actual], sz);

		(void)fatMapEnd(&map);
		fatUnmapDisk(&map);
		if (err < E_OK) break;
	}
	if (err == EX_INTR && actual > 0) err = E_OK;

	/* Update access date */
	if (err >= E_OK) {
		err = fatUpdateAccessDate(fs, inode);
	}

	if (err >= E_OK) {
		*aoffset = (fd->fd_fpos += actual);
		*alen = actual;
	}
exit0:
	return err;
}

/*
 *  Write file data
 */
LOCAL	ER	fat_write64(FD *fd, const UB *buf, UW len,
						UW *alen, D *aoffset)
{
	FATFS	*fs;
	FATNODE	*inode;
	CLAD	clad[MAX_CLEN];
	D	offset;
	UW	actual, gapsz, ofs, sz, gsz, dsz;
	W	i, cllen;
	MapInfo	map;
	ER	err;

	fs = (FATFS *)fd->fd_fs;
	inode = (FATNODE *)fd->fd_inode;
	err = E_OK;

	/* If O_APPEND, force offset to end of file */
	if ((fd->fd_omode & O_APPEND) != 0) {
		fd->fd_fpos = inode->fino_filsz;
	}
	offset = fd->fd_fpos;

	*alen = 0;
	*aoffset = offset;

	if (len <= 0) goto exit0;

	/* Check gap area */
	gapsz = 0;
	if (offset > inode->fino_filsz) {
		gapsz = offset - inode->fino_filsz;
		offset -= gapsz;
	}

	/* Check writing size */
	len += gapsz;
	if (offset + len > FILE_SIZE_MAX) {
		err = EX_FBIG;
		goto exit0;
	}

	/* Write offset within 1st cluster */
	ofs = offset % fs->ff_clsz;

	for (actual = 0; len > 0; len -= sz, offset += sz, gapsz -= gsz,
						actual += dsz, ofs = 0) {

		/* Get next cluster list */
		err = fatCopyClusterList(fs, clad, offset, len, inode, &cllen);
		if (err >= E_OK && cllen == 0) {
			/* Append new cluster */
			err = fatApdNewCluster(fs, &clad[0], (W)len, inode);
			if (err >= E_OK) cllen = 1;
		}
		if (err < E_OK) break;

		/* Map area size */
		for (sz = i = 0; i < cllen; i++) {
			sz += clad[i].ca_len;
		}
		sz *= fs->ff_clsz;

		/* Write total size : gap + data */
		if ((sz -= ofs) > len) sz = len;

		/* Write gap size */
		if ((gsz = gapsz) > sz) gsz = sz;

		/* Write data size */
		dsz = sz - gsz;

		/* Map data area */
		err = fatMapDisk(fs, clad, cllen, &map, MAP_C, (VW)inode);
		if (err < E_OK) break;

		/* Write gap - zero clear */
		if (gsz > 0) {
			err = fatMapWriteBytes(&map, ofs, NULL, gsz);
		}

		/* Write data */
		if (err >= E_OK && dsz > 0) {
			err = fatMapWriteBytes(&map, ofs + gsz,
							&buf[actual], dsz);
		}
		(void)fatMapEnd(&map);
		fatUnmapDisk(&map);
		if (err < E_OK) break;
	}

	if (err == EX_INTR && actual > 0) err = E_OK;

	/* Update file size */
	if (inode->fino_filsz < fd->fd_fpos + actual) {
		inode->fino_filsz = fd->fd_fpos + actual;
	}

	/* Update */
	err = fatUpdateFile(fs, inode);
	if (err >= E_OK) {
		*aoffset = (fd->fd_fpos += actual);
		*alen = actual;
	}
exit0:
	return err;
}

/*
 *  Set a direcory entry to buffer
 */
LOCAL	ER	set_dirent(FATFS *fs, UD ino, void *nm,
				struct dirent *ent, W *alen, W bflen)
{
	W	nmlen, nmsz;
	struct dirent	*e;

	/* Check buffer length */
	bflen -= *alen;
	nmsz  = bflen - offsetof(struct dirent, d_name) - 1;
	if (nmsz <= 0) goto nospace;			/* No space */
	if (nmsz > NAME_MAX) nmsz = NAME_MAX;

	/* Get actual name length */
	e = (struct dirent *)((UB*)ent + *alen);
	if (ino <= CLSTART) {	/* Special for root "." and ".." */
		nmlen = strlen((B*)nm);
		if (nmlen <= nmsz) strcpy(e->d_name, (B*)nm);
	} else {
		nmlen = fatEncUtf16strToUtf8str((UH*)nm,
				fat_strlen16((UH*)nm), (UB*)e->d_name, nmsz + 1);
	}
	if (bflen < RECLEN_DIRENT(nmlen)) goto nospace;	/* No space */

	/* Set entry:
		Note: d_ino may not be unique, because it ignores upper word */
	e->d_ino = (ino_t)ino;
	e->d_reclen = RECLEN_DIRENT(nmlen);
	*alen += e->d_reclen;
	return E_OK + 1;
nospace:
	return (nmsz == NAME_MAX) ? EX_NAMETOOLONG :
				((*alen == 0) ? EX_INVAL : E_OK);
}

/*
 *  Get direcory entries
 */
LOCAL	ER	fat_getdents(FD *fd, struct dirent *ent, W *len, W *retofs)
{
	FATFS		*fs;
	FATNODE		*inode;
	DIRINF		dir;
	FAT_DIRENT	*dirent;
	W		ofs, ofsbias, ldenum, cks, bflen, alen;
	ER		err;

/*
 *  Special specification : Get only one entry when buffer size is just
 *    same as the sizeof(struct dirent).
 */
#define	GetOnlyOneEnt	(bflen == sizeof(struct dirent))

	fs = (FATFS *)fd->fd_fs;
	inode = (FATNODE *)fd->fd_inode;
	ofs = (W)fd->fd_fpos;

	bflen = *len;
	*len = alen = 0;
	ofsbias = 0;
	err = E_OK;

	/* Check offset */
	if ((ofs % DIRENTSZ) != 0) {
		err = EX_NOENT;
		goto exit0;
	}

	/* Special handling for "." and ".." of root directory */
	if (IsRootINODE(fs, inode)) {
		/* Root of "." directory */
		if (ofs == 0) {
			err = set_dirent(fs, CLSTART, ".", ent, &alen, bflen);
			if (err <= E_OK) goto exit1;
			ofs = DIRENTSZ;
			if (GetOnlyOneEnt) goto exit1;
		}
		/* Root of ".." entry  */
		if (ofs == DIRENTSZ) {
			err = set_dirent(fs, 0, "..", ent, &alen, bflen);
			if (err <= 0) goto exit1;
			ofs = DIRENTSZ * 2;
			if (GetOnlyOneEnt) goto exit1;
		}
		ofs -= (ofsbias = DIRENTSZ * 2);
	}

	/* Open directory */
	err = fatOpenDIR(fs, &dir, inode->fino_fclno, ofs, AC_RDONLY, inode);
	if (err < E_OK) goto exit0;

	/* Get directory entries */
	for (ldenum = 0, cks = CKSVOID; err >= E_OK;
				err = fatNextDIR(fs, &dir, AC_RDONLY) ) {
		/* Check directory entry */
		dirent = dir.di_dirent;
		if (dirent == NULL ||		/* Last entry */
			isDE_TERM(dirent)) {	/* Terminated entry */
			break;
		}
		if (isDE_FREE(dirent)) {	/* Free entry */
			;
		} else if (isDE_LFN(dirent)) {	/* LFN entry */
			if (fatGetLFN((FAT_DIRENT_LFN *)dirent,
				&ldenum, &cks, fs->ff_nmbf) >= 0) continue;

		} else if (isDE_VOL(dirent)) {	/* Volume label */
			;
		} else if (cks == fatCalcChksum(dirent->de_fname) ||
							cks == CKSVOID ) {
			/* SFN entry, check checksum  */
			if (cks == CKSVOID) {
				/* Get SFN */
				(void)fatGetSFN(dirent->de_fname,
					dirent->de_smallcaps, fs->ff_nmbf);
			}
			/* Set entry */
			err = set_dirent(fs, fatGetInoDIR(fs, &dir),
					fs->ff_nmbf, ent, &alen, bflen);
			if (err <= E_OK) break;

			ofs = dir.di_offset + DIRENTSZ;
			if (GetOnlyOneEnt) break;
		}
		cks = CKSVOID;
		ldenum = 0;
	}

	/* Close directory */
	err = fatCloseDIR(fs, &dir, err);
	if (err >= E_OK) {
		/* Update access date */
		err = fatUpdateAccessDate(fs, inode);
	}
exit1:
	if (err >= E_OK) {
		*retofs = fd->fd_fpos = ofs + ofsbias;
		*len = alen;
	}
exit0:
	return err;
}

/*
 *  Open file/directory
 */
LOCAL	ER	fat_open(FD *fd, const UB *fname, W oflag, mode_t mode)
{
	FATFS	*fs;
	FATNODE	*inode;
	ER	err;

	fs = (FATFS *)fd->fd_fs;
	inode = (FATNODE *)fd->fd_inode;
	err = E_OK;

	fd->fd_omode = (UB)oflag;			/* Open mode */

	if (inode->fino_c.ino_ino == NOEXS_INO) {	/* Create open */

		/* Check writable */
		if (inode->fino_d_ronly != FALSE) {
			err = EX_ACCES;
			goto exit0;
		}
		/* File type */
		if (! isSET(mode, FILMODE_WRITE)) {
			inode->fino_ftype = FAT_RDONLY;
		}
		/* Create a file */
		err = fatCreateFile(fs, inode, (UB*)fname, NULL);
		if (err < E_OK) goto exit0;

		err = fatUpdateDirectory(fs, inode);

	} else {	/* Normal open */
		/* Check writable */
		if (isFAT_RDONLY(inode->fino_ftype) &&
					isNOT_RDONLY(fd->fd_omode)) {
			err = EX_ACCES;
			goto exit0;
		}

		/* If O_TRUNC, delete file data */
		if (isSET(oflag, O_TRUNC) &&
					! isFAT_SUBDIR(inode->fino_ftype)) {
			/* Check writable */
			if (isFAT_RDONLY(inode->fino_ftype)) {
				err = EX_ACCES;
				goto exit0;
			}
			/* Check open mode */
			if (isNOT_RDONLY(fd->fd_omode)) {
				/* Delete file data */
				err = fatTruncateFile(fs, inode, 0);
			}
		}
	}
	if (err >= E_OK) {
		/* Make cluster list of inode */
		err = fatMakeClusterList(fs, inode);
	}
exit0:
	return err;
}

/*
 *  Close file/directory
 */
LOCAL	ER	fat_close(FD *fd)
{
	/* do nothing */
	(void)fd;
	return E_OK;
}

/*
 *  Get file sysatem status - statvfs
 */
LOCAL	ER	fat_statvfs(FATFS *fs, struct statvfs *buf)
{
	buf->f_bsize = DSECSZ(fs);
	buf->f_frsize = fs->ff_clsz;
	buf->f_blocks = fs->ff_lastcl;
	buf->f_bfree = fs->ff_freecl;
	buf->f_bavail = buf->f_bfree;
	buf->f_files = 0;		/* Not support */
	buf->f_ffree = 0;		/* Not support */
	buf->f_favail = buf->f_ffree;
	buf->f_fsid = 0;		/* Not support */
	buf->f_flag = ST_NOSUID;
	if (fs->ff_c.fs_rdonly != FALSE) {
		buf->f_flag |= ST_RDONLY;
	}
	if (fs->ff_c.fs_diskinfo.removable != 0) {
		buf->f_flag |= ST_REMOVABLE;
	}
	buf->f_namemax = LFN_MAXLEN;
	return E_OK;
}

/*
 *  ioctl function
 */
LOCAL	ER	fat_ioctl(FD *fd, UW dcmd, void *data)
{
	/* No I/O control function supported  */

	(void)fd;
	(void)dcmd;
	(void)data;
	return EX_NOTSUP;
}

/*
 *  Change current diretory
 */
LOCAL	ER	fat_fchdir(FD *fd, UB *utf8nm, W utf8len)
{
	FATFS	*fs;
#ifndef	FAT_CURDIR_CASE_NOCARE
	DIRINF	dir;
	UB	*npath, *cpath;
	UW	len, dcl;
#endif	/* ~ FAT_CURDIR_CASE_NOCARE */
	UB	*path, *e_utf8nm;
	ER	err;

	fs = (FATFS *)fd->fd_fs;

	/* Check inode's path name */
	path = ((FATNODE *)fd->fd_inode)->fino_c.ino_path;
	if (path == NULL) {
		err = EX_INVAL;
		goto exit0;
	}
	if (path[0] != '/') {
		err = EX_IO;
		goto exit0;
	}

	/* Set connection name */
	e_utf8nm = utf8nm + utf8len;
	*utf8nm++ = '/';
	(void)strcpy((B*)utf8nm, fs->ff_c.fs_coninf->connm);
	if (path[1] == '\0') goto exit0;	/* root directory */

	utf8nm += strlen((B*)utf8nm);

#ifdef	FAT_CURDIR_CASE_NOCARE

	if (utf8nm + strlen(path) >= e_utf8nm) {
		err = EX_NAMETOOLONG;
	} else {
		strcpy((B*)utf8nm, path);
		err = E_OK;
	}

#else	/* FAT_CURDIR_CASE_NOCARE */

	/* Convert inode's path name to real "case-sensitve" path name */
	cpath = &path[1];
	for (dcl = fs->ff_rootcl; ; cpath = npath + 1) {

		if ((npath = (UB*)strchr((B*)cpath, '/')) == NULL) {
			len = strlen((B*)cpath);
		} else {
			len = npath - cpath;
		}

		err = fatOpenDIR(fs, &dir, dcl, OFSVOID, AC_RDONLY, NULL);
		if (err < E_OK) break;

		err = fatSearchDIR_nm(fs, &dir, cpath, len,
						AC_RDONLY, fs->ff_nmbf);
		if (err >= E_OK) {
			if (isFAT_SUBDIR(dir.di_dirent->de_ftype)) {
				*utf8nm++ = '/';
				len = fatEncUtf16strToUtf8str(
					fs->ff_nmbf, fat_strlen16(fs->ff_nmbf),
					utf8nm, e_utf8nm - utf8nm);
				if ((utf8nm += len) >= e_utf8nm) {
					err = EX_NAMETOOLONG;
				}
				/* Go parent directory */
				dcl = dirent_to_fcl(fs, dir.di_dirent);
			} else {
				dir.di_dirent = NULL;
				err = EX_NOTDIR;
			}
		}
		/* Close directory */
		err = fatCloseDIR(fs, &dir, err);

		CHK_BREAK_AND_SET_ERR(fs, &err);
		if (err < E_OK) break;
		if (npath == NULL) break;
	}
#endif	/* FAT_CURDIR_CASE_NOCARE */
exit0:
	return err;
}

/*----------------------------------------------------------------------------
	FATFS FIMP services
----------------------------------------------------------------------------*/

/*
 *  FATFS FIMP service - FIMP_OPEN
 */
LOCAL	ER	fatfs_open(FS *fs, fimp_t *req)
{
	LPINF	lp;
	FD	*fd;
	W	oflag, fdno;
	mode_t	mode;
	ER	err;

	/* Analize path */
	err = fat_lookup((FATFS*)fs, &lp, req->r_open.path);
	if (err < E_OK) goto exit0;

	mode = 0;
	oflag = (W)req->r_open.oflags;

	/* Check the file system writing access */
	if (isNOT_RDONLY(oflag) && fs->fs_rdonly != FALSE) {
		err = EX_ROFS;
	} else if (lp.lp_inode->ino_ino != NOEXS_INO) {
		/* Existent file */
		if (isSET(oflag, O_CREAT) && isSET(oflag, O_EXCL)) {
			err = EX_EXIST;
		} else if (lp.lp_inode->ino_type == DT_DIR) {
			if (isNOT_RDONLY(oflag)) {
				/* Directory is reading open */
				err = EX_ISDIR;
			} else if (isSET(oflag, O_TRUNC)) {
				/* Directory is not truncated */
				err = EX_ISDIR;
			}
		} else if (lp.lp_inode->ino_type == DT_REG &&
					isSET(oflag, O_DIRECTORY)) {
			err = EX_NOTDIR;
		}
	} else {
		/* Nonnexisitent file */
		if (! isSET(oflag, O_CREAT)) {
			err = EX_NOENT;
		} else if (fs->fs_rdonly != FALSE) {
			/* Writing access of file system */
			err = EX_ROFS;
		} else {
			/* File creation mode */
			mode = req->r_open.mode & ~(fs->fs_uxinfo.uxi_cmask);
		}
	}
	if (err < E_OK) goto exit1;

	/* Creation of file descriptor */
	err = fatNewFileDesc(fs, &fd, &fdno);
	if (err < E_OK) goto exit1;

	/* Open */
	fd->fd_inode = lp.lp_inode;
	err = fat_open(fd, lp.lp_path, oflag, mode);
	if (err < E_OK) goto exit2;

	err = fatDCacheSyncFS((FATFS*)fs, 0);
	if (err >= E_OK) {
		/* Registration of file ref. node */
		fatInodeRegister(fs, lp.lp_inode, fd->fd_omode);
		*req->r_open.fid = (fid_t)fdno;
	}
exit2:
	if (err < E_OK) {
		fd->fd_inode = NULL;
		fatDelFileDesc(fs, fd, fdno);
	}
exit1:
	if (err < E_OK) {
		fatInodeFree(fs, lp.lp_inode);
	}
exit0:
	return err;
}

/*
 *  FATFS FIMP service - FIMP_CLOSE
 */
LOCAL	ER	fatfs_close(FS *fs, fimp_t *req)
{
	FD	*fd;
	ER	err;

	err = fatSearchFileDesc(fs, &fd, (W)req->r_close.fid);
	if (err >= E_OK) {
		if (fd->fd_refcnt <= 1) {
			fd->fd_omode = req->r_close.oflags;
			err = fat_close(fd);
		}
		if (err >= E_OK) {
			err = fatFDSync(fd);
			fatDelFileDesc(fs, fd, (W)req->r_close.fid);
		}
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_READ64
 */
LOCAL	ER	fatfs_read64(FS *fs, fimp_t *req)
{
	FD	*fd;
	ER	err;
	UW	alen;

	err = fatSearchFileDesc(fs, &fd, (W)req->r_read64.fid);
	if (err >= E_OK) {
		if (fd->fd_inode == NULL) {
			err = EX_BADF;
		} else if (fd->fd_inode->ino_type != DT_REG) {
			err = EX_ISDIR;
		} else {
			fd->fd_fpos = (D)*(req->r_read64.off);
			fd->fd_omode = (W)req->r_read64.oflags;
			err = fat_read64(fd, (UB*)req->r_read64.buf,
					(UW)*req->r_read64.len, (UW*)&alen,
						(D*)req->r_read64.retoff);
			if (err >= E_OK) {
				*req->r_read64.len = alen;
				err = fatFDSync(fd);
			}
		}
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_WRITE64
 */
LOCAL	ER	fatfs_write64(FS *fs, fimp_t *req)
{
	FD	*fd;
	ER	err;
	UW	alen;

	err = fatSearchFileDesc(fs, &fd, (W)req->r_write64.fid);
	if (err >= E_OK) {
		if (fd->fd_inode == NULL) {
			err = EX_BADF;
		} else if (fd->fd_inode->ino_type != DT_REG) {
			err = EX_ISDIR;
		} else {
			fd->fd_fpos = (D)*(req->r_write64.off);
			fd->fd_omode = (W)req->r_write64.oflags;
			err = fat_write64(fd, (const UB*)req->r_write64.buf,
					(UW)*req->r_write64.len, (UW*)&alen,
						(D*)req->r_write64.retoff);
			if (err >= E_OK) {
				*req->r_write64.len = alen;
				err = fatFDSync(fd);
			}
		}
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_IOCTL
 */
LOCAL	ER	fatfs_ioctl(FS *fs, fimp_t *req)
{
	FD	*fd;
	ER	err;

	err = fatSearchFileDesc(fs, &fd, (W)req->r_ioctl.fid);
	if (err >= E_OK) {
		fd->fd_omode = (W)req->r_ioctl.oflags;
		*req->r_ioctl.retval = (INT)
			fat_ioctl(fd, (UW)req->r_ioctl.dcmd,req->r_ioctl.arg);
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_FSYNC
 */
LOCAL	ER	fatfs_fsync(FS *fs, fimp_t *req)
{
	FD	*fd;
	ER	err;

	err = fatSearchFileDesc(fs, &fd, (W)req->r_fsync.fid);
	if (err >= E_OK) {
		fd->fd_omode = (W)req->r_fsync.oflags;
		err = fatDCacheSyncFS((FATFS*)fs, (VW)fd->fd_inode);
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_TRUNCATE64
 */
LOCAL	ER	fatfs_truncate64(FS *fs, fimp_t *req)
{
	LPINF	lp;
	ER	err;

	err = fat_lookup((FATFS*)fs, &lp, req->r_truncate64.path);
	if (err >= E_OK) {
		if (fs->fs_rdonly != FALSE) {
			err = EX_ROFS;
		} else {
			err = fat_truncate64((FATFS*)fs, (FATNODE*)lp.lp_inode,
						(D)req->r_truncate64.len);
			if (err >= E_OK) {
				err = fatDCacheSyncFS((FATFS*)fs, 0);
			}
		}
		fatInodeFree(fs, lp.lp_inode);
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_FTRUNCATE64
 */
LOCAL	ER	fatfs_ftruncate64(FS *fs, fimp_t *req)
{
	ER	err;
	FD	*fd;

	err = fatSearchFileDesc(fs, &fd, (W)req->r_ftruncate64.fid);
	if (err >= E_OK) {
		err = fat_ftruncate64(fd, (D)req->r_ftruncate64.len);
		if (err >= E_OK) {
			err = fatFDSync(fd);
		}
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_UNLINK
 */
LOCAL	ER	fatfs_unlink(FS *fs, fimp_t *req)
{
	LPINF	lp;
	ER	err;

	err = fat_lookup((FATFS*)fs, &lp, req->r_unlink.path);
	if (err >= E_OK) {
		if (lp.lp_inode->ino_ino == NOEXS_INO) {
			err = EX_NOENT;
		} else if (lp.lp_inode->ino_refcnt > 0) {
			err = EX_BUSY;
		} else if (fs->fs_rdonly != FALSE) {
			err = EX_ROFS;
		} else {
			err = fat_unlink((FATFS*)fs, (FATNODE*)lp.lp_inode);
			if (err >= E_OK) {
				err = fatDCacheSyncFS((FATFS*)fs, 0);
			}
		}
		fatInodeFree(fs, lp.lp_inode);
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_RENAME
 */
LOCAL	ER	fatfs_rename(FS *fs, fimp_t *req)
{
	LPINF	olp, nlp;
	INODE	*from, *to;
	ER	err;

	err = fat_lookup((FATFS*)fs, &olp, req->r_rename.oldpath);
	if (err < E_OK) goto exit0;

	from = olp.lp_inode;
	if (from->ino_ino == NOEXS_INO) {
		err = EX_NOENT;
		goto exit1;
	}
	if (IsRootINODE(fs, from)) {
		err = EX_ACCES;
		goto exit1;
	}

	err = fat_lookup((FATFS*)fs, &nlp, req->r_rename.newpath);
	if (err < E_OK) goto exit1;

	to = nlp.lp_inode;
	if (IsRootINODE(fs, to)) {
		err = EX_ACCES;
		goto exit2;
	}

	/* Existence of "to" */
	if (to->ino_ino != NOEXS_INO) {
		/* Check of the number of "to" references */
		if (fatCheckIsInodeBusy(fs, to) != FALSE) {
			err = EX_BUSY;
			goto exit2;
		}
		/* Check of the accordance of file type */
		if (from->ino_type != to->ino_type) {
			err = (to->ino_type == DT_DIR) ? EX_ISDIR : EX_NOTDIR;
			goto exit2;
		}
		/* Stop if it is an same file */
		if (from->ino_ino == to->ino_ino) {
			goto exit2;
		}
	}
	if (fs->fs_rdonly != FALSE) {
		err = EX_ROFS;
		goto exit2;
	}
	err = fat_rename((FATFS*)fs, (FATNODE*)from, (FATNODE*)to, nlp.lp_path);
	if (err >= E_OK) {
		err = fatDCacheSyncFS((FATFS*)fs, 0);
	}
exit2:
	fatInodeFree(fs, to);
exit1:
	fatInodeFree(fs, from);
exit0:
	return err;
}

/*
 *  FATFS FIMP service - FIMP_CHMOD
 */
LOCAL	ER	fatfs_chmod(FS *fs, fimp_t *req)
{
	LPINF	lp;
	ER	err;

	err = fat_lookup((FATFS*)fs, &lp, req->r_chmod.path);
	if (err >= E_OK) {
		if (lp.lp_inode->ino_ino == NOEXS_INO) {
			err = EX_NOENT;
		} else if (fs->fs_rdonly != FALSE) {
			err = EX_ROFS;
		} else {
			err = fat_chmod((FATFS*)fs, (FATNODE*)lp.lp_inode,
							req->r_chmod.mode);
			if (err >= E_OK) {
				err = fatDCacheSyncFS((FATFS*)fs, 0);
			}
		}
		fatInodeFree(fs, lp.lp_inode);
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_FCHMOD
 */
LOCAL	ER	fatfs_fchmod(FS *fs, fimp_t *req)
{
	FD	*fd;
	ER	err;

	err = fatSearchFileDesc(fs, &fd, (W)req->r_fchmod.fid);
	if (err >= E_OK) {
		if (fs->fs_rdonly != FALSE) {
			err = EX_ROFS;
		} else {
			err = fat_fchmod(fd, req->r_fchmod.mode);
			if (err >= E_OK) {
				err = fatFDSync(fd);
			}
		}
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_MKDIR
 */
LOCAL	ER	fatfs_mkdir(FS *fs, fimp_t *req)
{
	LPINF	lp;
	ER	err;

	err = fat_lookup((FATFS*)fs, &lp, req->r_mkdir.path);
	if (err >= E_OK) {
		if (lp.lp_inode->ino_ino != NOEXS_INO) {
			err = EX_EXIST;
		} else if (fs->fs_rdonly != FALSE) {
			err = EX_ROFS;
		} else {
			err = fat_mkdir((FATFS*)fs, (FATNODE*)lp.lp_inode,
					lp.lp_path, req->r_mkdir.mode);
			if (err >= E_OK) {
				err = fatDCacheSyncFS((FATFS*)fs, 0);
			}
		}
		fatInodeFree(fs, lp.lp_inode);
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_RMDIR
 */
LOCAL	ER	fatfs_rmdir(FS *fs, fimp_t *req)
{
	LPINF	lp;
	ER	err;

	err = fat_lookup((FATFS*)fs, &lp, req->r_rmdir.path);
	if (err >= E_OK) {
		if (lp.lp_inode->ino_ino == NOEXS_INO) {
			err = EX_NOENT;
		} else if (IsRootINODE(fs, lp.lp_inode)) {
			err = EX_ACCES;
		} else if (fatCheckIsInodeBusy(fs, lp.lp_inode) != FALSE) {
			err = EX_BUSY;
		} else if (fs->fs_rdonly != FALSE) {
			err = EX_ROFS;
		} else {
			err = fat_rmdir((FATFS*)fs, &fs->fs_uxinfo.uxi_curdir,
						(FATNODE*)lp.lp_inode);
			if (err >= E_OK) {
				err = fatDCacheSyncFS((FATFS*)fs, 0);
			}
		}
		fatInodeFree(fs, lp.lp_inode);
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_CHDIR
 */
LOCAL	ER	fatfs_chdir(FS *fs, fimp_t *req)
{
	(void)fs;
	(void)req;
	/* Nothing to do */
	return E_OK;
}

/*
 *  FATFS FIMP service - FIMP_FCHDIR
 */
LOCAL	ER	fatfs_fchdir(FS *fs, fimp_t *req)
{
	FD	*fd;
	ER	err;

	err = fatSearchFileDesc(fs, &fd, (W)req->r_fchdir.fid);
	if (err >= E_OK) {
		err = fat_fchdir(fd, (UB*)req->r_fchdir.buf,
						(W)req->r_fchdir.len);
		if (err >= E_OK) {
			err = fatFDSync(fd);
		}
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_READDIR
 */
LOCAL	ER	fatfs_getdents(FS *fs, fimp_t *req)
{
	FD	*fd;
	W	retofs;
	ER	err;

	err = fatSearchFileDesc(fs, &fd, (W)req->r_getdents.fid);
	if (err >= E_OK) {
		fd->fd_fpos = (D)*(req->r_getdents.off);
		fd->fd_omode = (W)req->r_getdents.oflags;
		err = fat_getdents(fd, req->r_getdents.buf,
					(W*)req->r_getdents.len, &retofs);
		if (err >= E_OK) {
			err = fatFDSync(fd);
			if (err >= E_OK) {
				*(req->r_getdents.retoff) = (off64_t)retofs;
			}
		}
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_FSTATVFS
 */
LOCAL	ER	fatfs_fstatvfs(FS *fs, fimp_t *req)
{
	return fat_statvfs((FATFS*)fs, req->r_fstatvfs.buf);
}

/*
 *  FATFS FIMP service - FIMP_STATVFS
 */
LOCAL	ER	fatfs_statvfs(FS *fs, fimp_t *req)
{
	LPINF	lp;
	ER	err;

	err = fat_lookup((FATFS*)fs, &lp, req->r_statvfs.path);
	if (err >= E_OK) {
		if (lp.lp_inode->ino_ino == NOEXS_INO) {
			err = EX_NOENT;
		} else {
			err = fat_statvfs((FATFS*)fs, req->r_statvfs.buf);
			//if (err >= E_OK) {
			//	err = fatDCacheSyncFS((FATFS*)fs, 0);
			//}
		}
		fatInodeFree(fs, lp.lp_inode);
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_SYNC
 */
LOCAL	ER	fatfs_sync(FS *fs, fimp_t *req)
{
	(void)req;
	return fatDCacheSyncFS((FATFS*)fs, 0);
}

/*
 *  FATFS FIMP service - FIMP_UTIMES_US
 */
LOCAL	ER	fatfs_utimes_us(FS *fs, fimp_t *req)
{
	LPINF	lp;
	ER	err;

	err = fat_lookup((FATFS*)fs, &lp, req->r_utimes_us.path);
	if (err >= E_OK) {
		if (lp.lp_inode->ino_ino == NOEXS_INO) {
			err = EX_NOENT;
		} else if (fs->fs_rdonly != FALSE) {
			err = EX_ROFS;
		} else {
			err = fat_utimes_us((FATFS*)fs, (FATNODE*)lp.lp_inode,
						req->r_utimes_us.times_u);
			if (err >= E_OK) {
				err = fatDCacheSyncFS((FATFS*)fs, 0);
			}
		}
		fatInodeFree(fs, lp.lp_inode);
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_FCNTL64
 */
LOCAL	ER	fatfs_fcntl64(FS *fs, fimp_t *req)
{
	(void)fs;
	(void)req;
	return EX_NOTSUP;
}

/*
 *  FATFS FIMP service - FIMP_FSTAT64_US
 */
LOCAL	ER	fatfs_fstat64_us(FS *fs, fimp_t *req)
{
	FD	*fd;
	ER	err;

	err = fatSearchFileDesc(fs, &fd, (W)req->r_fstat64_us.fid);
	if (err >= E_OK) {
		err = fat_fstat64_us(fd, req->r_fstat64_us.buf);
		if (err >= E_OK) {
			err = fatFDSync(fd);
		}
	}
	return err;
}

/*
 *  FATFS FIMP service - FIMP_STAT64_US
 */
LOCAL	ER	fatfs_stat64_us(FS *fs, fimp_t *req)
{
	LPINF	lp;
	ER	err;

	err = fat_lookup((FATFS*)fs, &lp, req->r_stat64_us.path);
	if (err >= E_OK) {
		if (lp.lp_inode->ino_ino == NOEXS_INO) {
			err = EX_NOENT;
		} else {
			err = fat_stat64_us((FATFS*)fs, (FATNODE*)lp.lp_inode,
							req->r_stat64_us.buf);
			//if (err >= E_OK) {
			//	err = fatDCacheSyncFS((FATFS*)fs, 0);
			//}
		}
		fatInodeFree(fs, lp.lp_inode);
	}
	return err;
}

/*----------------------------------------------------------------------------
	FAT uxinfo operations
----------------------------------------------------------------------------*/

/*
 *  Initialize UXINFO
 */
LOCAL	ER	fatUxinfoInit(FATFS *fs)
{
	UXINFO	*uxinfo;
	FATNODE	*rootinode;
	UB	*buf;
	ER	err;

	uxinfo = &fs->ff_c.fs_uxinfo;
	rootinode = NULL;

	/* Current directory takeover */
	fatInodeQueInit(&fs->ff_c);
	err = fatGetRootINODE(fs, &rootinode);
	if (err < E_OK) goto exit0;

	/* Twice */
	fatInodeRegister((FS *)fs, (INODE *)rootinode, O_RDONLY);
	fatInodeRegister((FS *)fs, (INODE *)rootinode, O_RDONLY);
	uxinfo->uxi_curdir.cd_inode = (INODE *)rootinode;

	/* File creation mode takeover */
	uxinfo->uxi_cmask = 0;

	/* Released inode */
	uxinfo->uxi_relino = NULL;

	/* Creation of file descriptor table  */
	buf = (UB *)fimp_calloc(MaxOpenF, sizeof(FD *));
	if (buf == NULL) {
		err = EX_NOMEM;
		fatInodeRelease((FS *)fs,
				uxinfo->uxi_curdir.cd_inode, O_RDONLY);
		uxinfo->uxi_curdir.cd_inode = NULL;
		goto exit0;
	}
	uxinfo->uxi_fdtbl = (FD **)buf;
exit0:
	return err;
}

/*
 *  Cleanup UXINFO
 */
LOCAL	void	fatUxinfoCleanup(FATFS *fs)
{
	UXINFO	*uxinfo;
	FATNODE	*rootinode;
	FD	*fd;
	W	i;

	uxinfo = &fs->ff_c.fs_uxinfo;
	rootinode = NULL;

	/* Opened files releasee */
	if (uxinfo->uxi_fdtbl != NULL) {
		/* All files close */
		for (i = 0; i < MaxOpenF; i++) {
			fd = uxinfo->uxi_fdtbl[i];
			if (fd != NULL) {
				struct fimp_close	req;
				req.fid = i;
				(void)fatfs_close((FS *)fs, (fimp_t *)&req);
			}
		}
		/* Deletion of file descriptor table */
		fimp_free(uxinfo->uxi_fdtbl);
		uxinfo->uxi_fdtbl = NULL;
	}

	/* Release current directory */
	if (uxinfo->uxi_curdir.cd_inode != NULL) {
		fatInodeRelease((FS *)fs,
				uxinfo->uxi_curdir.cd_inode, O_RDONLY);
		uxinfo->uxi_curdir.cd_inode = NULL;
	}

	/* Release rootdir inode */
	(void)fatGetRootINODE(fs, &rootinode);
	fatInodeRelease((FS *)fs, (INODE *)rootinode, O_RDONLY);
}

/*----------------------------------------------------------------------------
	FIMP various functions
----------------------------------------------------------------------------*/

/*
 *  Error code conversion
 */
LOCAL	ER	toEXER(ER err)
{
	if (err < E_OK && MERCD(err) != EC_ERRNO) {
		switch (MERCD(err)) {
		case MERCD(E_SYS):	err = EX_IO;		break;
		case MERCD(E_NOCOP):	err = EX_IO;		break;
		case MERCD(E_NOSPT):	err = EX_NOTSUP;	break;
		case MERCD(E_RSFN):	err = EX_IO;		break;
		case MERCD(E_RSATR):	err = EX_INVAL;		break;
		case MERCD(E_PAR):	err = EX_INVAL;		break;
		case MERCD(E_ID):	err = EX_INVAL;		break;
		case MERCD(E_CTX):	err = EX_IO;		break;
		case MERCD(E_MACV):	err = EX_FAULT;		break;
		case MERCD(E_OACV):	err = EX_ACCES;		break;
		case MERCD(E_ILUSE):	err = EX_INVAL;		break;
		case MERCD(E_NOMEM):	err = EX_NOMEM;		break;
		case MERCD(E_LIMIT):	err = EX_NOSPC;		break;
		case MERCD(E_OBJ):	err = EX_INVAL;		break;
		case MERCD(E_NOEXS):	err = EX_INVAL;		break;
		case MERCD(E_QOVR):	err = EX_IO;		break;
		case MERCD(E_RLWAI):	err = EX_INVAL;		break;
		case MERCD(E_TMOUT):	err = EX_IO;		break;
		case MERCD(E_DLT):	err = EX_IO;		break;
		case MERCD(E_DISWAI):	err = EX_INTR;		break;
		case MERCD(E_IO):	err = EX_IO;		break;
		case MERCD(E_NOMDA):	err = EX_NODEV;		break;
		case MERCD(E_BUSY):	err = EX_BUSY;		break;
		case MERCD(E_ABORT):	err = EX_INTR;		break;
		case MERCD(E_RONLY):	err = EX_ROFS;		break;
		default:					break;
		}
	}
	return err;
}

/*
 *  Get request information
 */
LOCAL	void	*fatGetReqInf(fimp_t *req, UW *func)
{
static const	UW	functab[] = {
		(UW)((B*)fatfs_open + 0x01),	/* +0x01: Breakable	*/
		(UW)((B*)fatfs_close + 0x01),
		(UW)fatfs_ioctl,
		(UW)fatfs_getdents,
		(UW)((B*)fatfs_chdir + 0x01),
		(UW)((B*)fatfs_fchdir + 0x01),
		(UW)((B*)fatfs_fsync + 0x01),
		(UW)fatfs_unlink,
		(UW)fatfs_rename,
		(UW)fatfs_chmod,
		(UW)fatfs_fchmod,
		(UW)fatfs_mkdir,
		(UW)fatfs_rmdir,
		(UW)((B*)fatfs_statvfs + 0x01),
		(UW)((B*)fatfs_fstatvfs + 0x01),
		(UW)((B*)fatfs_sync + 0x01),
		(UW)((B*)fatfs_read64 + 0x01),
		(UW)((B*)fatfs_write64 + 0x01),
		(UW)((B*)fatfs_truncate64 + 0x01),
		(UW)((B*)fatfs_ftruncate64 + 0x01),
		(UW)fatfs_fcntl64,
		(UW)fatfs_stat64_us,
		(UW)fatfs_fstat64_us,
		(UW)fatfs_utimes_us,
};
	INT	i;

	if (req->com.coninf != NULL) {
		if ((i = req->com.r_code) >= 0 && i <= FIMP_COMMAND_MAX) {
			*func = functab[i];
			return req->com.coninf->consd;
		}
	}
	*func = 0;
	return NULL;
}

/*
 *  FIMP request function execute task
 */
LOCAL	void	FileSystemTask(INT stacd)
{
	FS	*fs;
	BOOL	fin;
	CMDPKT	*pkt;
	RNO	rdvno;
	UW	func;
	ER	err;

	fs = (FS *)stacd;
	for (fin = FALSE; fin == FALSE; ) {
		/* accept request */
		if (tk_acp_por(fs->fs_porid, 1, &rdvno,
					&pkt, TMO_FEVR) < E_OK) continue;

		/* Set task space */
		err = SetTaskSpace(pkt->cp_tskid);
		if (err < E_OK) goto reply;

		if (pkt->cp_req == NULL) {	/* terminate own task */
			fin = TRUE;
			err = E_OK;
			goto reply;
		}

		/* Check request */
		if (fatGetReqInf(pkt->cp_req, &func) != fs) {
			err = EX_IO;
			goto reply;
		}

		/* Here, check break_done flag again, note that
		   break_enb flag has been set in fatfs_service */
		fs->fs_tsksts = &tskStsTab[pkt->cp_tskid];
		if (fs->fs_tsksts->c.break_done != 0) {
			err = EX_INTR;
			goto reply;
		}

		/* Call service function */
		err = ((fatfs_api_t)(func & ~0x01))(fs, pkt->cp_req);

		if (fs->fs_uxinfo.uxi_relino != NULL) {
			/* Release inode registered in fatResetCurdir() */
			fatInodeRelease(fs, fs->fs_uxinfo.uxi_relino, O_RDONLY);
			fs->fs_uxinfo.uxi_relino = NULL;
		}
reply:
		/* Reply response */
		pkt->cp_err = err;
		(void)tk_rpl_rdv(rdvno, &pkt, sizeof(pkt));
	}
	/* Exit and delete own task */
	tk_exd_tsk();
}

/*
 *  Call rendezvous
 */
LOCAL	ER	CallFileSystemTask(FATFS *fs, CMDPKT *ppkt)
{
	ER	err;

	ppkt->cp_err = E_OK;
	err = tk_cal_por(fs->ff_c.fs_porid, 1, &ppkt,
						sizeof(CMDPKT *), TMO_FEVR);
	return (err < E_OK) ? err : ppkt->cp_err;
}

/*
 *  Create objects
 */
LOCAL	ER	fatCreateObjects(FATFS *fs)
{
	T_CPOR	cpor;
	T_CTSK	ctsk;
	ER	err;

	/* Creation of rendezvous port */
	SetOBJNAME(cpor.exinf, "bfat");
	cpor.poratr = TA_TPRI;
	cpor.maxcmsz = sizeof(CMDPKT *);
	cpor.maxrmsz = sizeof(CMDPKT *);
	err = tk_cre_por(&cpor);
	if (err >= E_OK) {
		fs->ff_c.fs_porid = (ID)err;

		/* Creation of file system task */
		SetOBJNAME(ctsk.exinf, "bfat");
		ctsk.tskatr = TA_HLNG | TA_RNG0;
		ctsk.task = (void*)FileSystemTask;
		ctsk.itskpri = TaskPriority;
		ctsk.stksz = TaskStackSize;
		err = tk_cre_tsk(&ctsk);
		if (err >= E_OK) {
			fs->ff_c.fs_tskid = (ID)err;
			err = tk_sta_tsk(fs->ff_c.fs_tskid, (INT)fs);
			if (err < E_OK) {
				(void)tk_del_tsk(fs->ff_c.fs_tskid);
			}
		}
		if (err < E_OK) {
			(void)tk_del_por(fs->ff_c.fs_porid);
		}
	}
	return err;
}

/*
 *  Delete objects
 */
LOCAL	void	fatDeleteObjects(FATFS *fs)
{
	CMDPKT	pkt;

	/* Delete task */
	pkt.cp_tskid = tk_get_tid();
	pkt.cp_req = NULL;

	(void)CallFileSystemTask(fs, &pkt);

	/* Delete rendezvous */
	(void)tk_del_por(fs->ff_c.fs_porid);
}

/*
 *  FATFS FIMP service function
 */
LOCAL	ER	fatfs_service(fimp_t *req)
{
	FATFS		*fs;
	UW		func;
	TaskSts		*tsksts;
	ER		err;
	CMDPKT		pkt;

	/* Check request */
	fs = (FATFS*)fatGetReqInf(req, &func);
	if (fs == NULL) {
		err = EX_NOTSUP;
	} else {
		err = E_OK;
		pkt.cp_tskid = tk_get_tid();
		tsksts = &tskStsTab[pkt.cp_tskid];

		/* Check break flag */
		if ((func & 0x01) != 0) {
			tsksts->c.break_enb = 1;
			if (tsksts->c.break_done != 0) {
				err = EX_INTR;
			} else if (tsksts->c.break_req != 0) {
				err = EX_INTR;
			}
		}

		/* Call FileSystemTask */
		if (err >= E_OK) {
			pkt.cp_req = req;
			err = CallFileSystemTask(fs, &pkt);
		}

		tsksts->c.break_enb = 0;
		if (tsksts->c.break_done != 0) {
			tsksts->c.break_done = 0;
		}
	}
	return toEXER(err);
}

/*
 *  Get system configuration parameter
 */
LOCAL	void	fatGetConfiguration(UB *name, INT *par)
{
	INT	val;

	if (tk_get_cfn(name, &val, 1) >= 1) *par = val;
}

/*
 *  FATFS FIMP registration function
 */
LOCAL	ER	fatfs_registfn(fimpinf_t *fimpinf, void *exinf)
{
	FAT_FSInfo	*fpi;

	(void)exinf;

	/* Alloc FAT FIMP informationn queue */
	if ((fpi = fimp_calloc(1, sizeof(FAT_FSInfo))) == NULL) {
		return EX_NOMEM;
	}

	/* Set task status tbale : allocated by upper fs */
	tskStsTab = (TaskSts*)fimpinf->fimpsd;

	QueInit(&fpi->fpi_attachque);
	fimpinf->fimpsd = (void *)fpi;

	/* Get FAT FS configuration parameters : global */
	fatGetConfiguration(SYSCONF_MaxOpenF, &MaxOpenF);
	fatGetConfiguration(SYSCONF_TaskStackSize, &TaskStackSize);
	fatGetConfiguration(SYSCONF_CacheFATMemorySize, &CacheFATMemorySize);
	fatGetConfiguration(SYSCONF_CacheFATRatio, &CacheFATRatio);
	fatGetConfiguration(SYSCONF_CacheRootMemorySize, &CacheRootMemorySize);
	fatGetConfiguration(SYSCONF_CacheRootRatio, &CacheRootRatio);
	fatGetConfiguration(SYSCONF_CacheDataMemorySize, &CacheDataMemorySize);
	fatGetConfiguration(SYSCONF_CacheDataRatio, &CacheDataRatio);
	fatGetConfiguration(SYSCONF_LastAccess, &LastAccess);

	return E_OK;
}

/*
 *  FATFS FIMP unregistration function
 */
LOCAL	ER	fatfs_unregistfn(fimpinf_t *fimpinf)
{
	/* Free FIMP Information memory */
	fimp_free(fimpinf->fimpsd);
	fimpinf->fimpsd = NULL;
	return E_OK;
}

/*
 *  FATFS FIMP attach function
 */
LOCAL	ER	fatfs_attachfn(coninf_t *coninf, void *exinf)
{
	FAT_FSInfo	*fpi;
	FATFS		*fs;
	ER		err;

	(void)exinf;

	fpi = (FAT_FSInfo *)coninf->fimpinf->fimpsd;
	
	/* Alloc FAT Information memory */
	if ((fs = fimp_calloc(1, sizeof(FATFS))) == NULL) {
		err = EX_NOMEM;
		goto exit0;
	}

	/* Open device */
	err = fatDiskOpen(coninf, fs);
	if (err < E_OK) goto exit1;

	/* Check boot sector and calc number of free cluster */
	err = fatCheckFileSystem(fs);
		printf("2:%d\n", err);
	if (err < E_OK) goto exit2;

	/* Initialize Disk cache */
	err = fatDCacheInit(fs);
		printf("3:%d\n", err);
	if (err < E_OK) goto exit2;

	/* Initialize FAT map */
	err = fatMapFATInit(fs);
		printf("4:%d\n", err);
	if (err < E_OK) goto exit3;

	/* Count of the number of empty clusters */
	err = fatCountFreeCluster(fs, &fs->ff_freecl);
	if (err < E_OK) goto exit4;

	/* Get file number of root directory */
	fs->ff_c.fs_rootino = dirINO(fs, fs->ff_rootcl);

	/* Get device number */
	err = fatGetDeviceNumber(coninf->devnm, &fs->ff_c.fs_devnum);
	if (err < E_OK) goto exit4;

	/* Initialize uxinfo */
	err = fatUxinfoInit(fs);
	if (err < E_OK) goto exit5;

	/* Create kernel objects */
	err = fatCreateObjects(fs);

exit5:
	if (err < E_OK) {
		fatUxinfoCleanup(fs);
	}
exit4:
	if (err < E_OK) {
		fatMapFATCleanup(fs);
	}
exit3:
	if (err < E_OK) {
		fatDCacheCleanup(fs);
	}
exit2:
	if (err < E_OK) {
		fatDiskClose(coninf, fs);
	}

	if (err >= E_OK) {
		/* Set dflags */
		if (fs->ff_c.fs_diskinfo.protect != 0) {
			coninf->dflags |= DEV_FLAG_READONLY;
		}
		if (fs->ff_c.fs_diskinfo.removable != 0) {
			coninf->dflags |= DEV_FLAG_REMOVABLE;
		}
		coninf->consd = (void *)fs;
		fs->ff_c.fs_coninf = coninf;
		fs->ff_c.fs_conlen = strlen(coninf->connm) + 1;
						/* + 1 : '/' <connm>  */
		/* Insert FAT attach queue */
		QueInsert(&fs->ff_c.fs_q, &fpi->fpi_attachque);
	}
exit1:
	if (err < E_OK) {
		fimp_free(fs);
	}
exit0:
	return toEXER(err);
}

/*
 *  FATFS FIMP detach function
 */
LOCAL	ER	fatfs_detachfn(coninf_t *coninf)
{
	FATFS	*fs;
	FATNODE	*rootinode;
	ER	err;

	fs = (FATFS *)coninf->consd;
	rootinode = NULL;

	(void)fatGetRootINODE(fs, &rootinode);
	fatInodeRegister((FS *)fs, (INODE *)rootinode, O_RDONLY);
	fatInodeRelease((FS *)fs, (INODE *)
			fs->ff_c.fs_uxinfo.uxi_curdir.cd_inode, O_RDONLY);
	fs->ff_c.fs_uxinfo.uxi_curdir.cd_inode = (INODE *)rootinode;

	err = fatInodeCheckEmpty(&fs->ff_c, (INODE *)rootinode);
	if (err >= E_OK) {
		/* Remove from queue */
		QueRemove(&fs->ff_c.fs_q);

		/* Cleanup */
		fatUxinfoCleanup(fs);
		fatDeleteObjects(fs);
		fatMapFATCleanup(fs);
		fatDCacheCleanup(fs);
		fatDiskClose(coninf, fs);
		fimp_free(fs);
		coninf->consd = NULL;
	}
	return err;
}

/*
 *  FATFS FIMP break function
 */
LOCAL	ER	fatfs_breakfn(coninf_t *coninf, ID tskid, BOOL set)
{
	FATFS	*fs;
	TaskSts	*tsksts;

	fs = (FATFS *)coninf->consd;
	tsksts = &tskStsTab[tskid];

	/* Disable dispatch to ensure atomic operations
					when called in task context */
	tk_dis_dsp();
	if (set != FALSE) {
		if (tsksts->c.break_enb != 0) {
			/* Already enabled, do break and set done flag */
			tsksts->c.break_done = 1;
		} else {
			/* Not yet enabled, set req flag */
			tsksts->c.break_req = 1;
		}
	} else {
		/* Clear break req, done and enb flag */
		tsksts->w = 0;
	}
	tk_ena_dsp();

	return E_OK;
}

