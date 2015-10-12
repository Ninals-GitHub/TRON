/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
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
 *	@(#)fimp_fat_local.h
 *
 */

#ifndef __FIMP_FAT_LOCAL_H__
#define __FIMP_FAT_LOCAL_H__

#include "fimp_fat.h"
#include "fimp_fat_enc.h"
#include <device/disk.h>
#include <sys/queue.h>
#include <sys/util.h>
#include <stdlib.h>
#include <string.h>
#include <t2ex/datetime.h>

/* Memory allocation */
#define	fimp_malloc(n)		Vmalloc(n)
#define	fimp_calloc(n, i)	Vcalloc(n, i)
#define	fimp_realloc(p, n)	Vrealloc(p, n)
#define	fimp_free(p)		Vfree(p)

/* Type definitions */
typedef struct FileSystem	FS;
typedef struct FAT_FileSystem	FATFS;

/* Service function prototype */
typedef ER	(*fatfs_api_t)(FS *fs, fimp_t *_req);

/* FIMP attach information */
typedef struct {
	QUEUE	fpi_attachque;
} FAT_FSInfo;

/* Disk cache */
typedef struct DiskCachePage	DCachePage;

#define DCI_NHSQUE	(0x20)		/* Hash search size (= 2 ** N)	*/
#define DCI_HSQUEMSK	(DCI_NHSQUE - 1)
typedef struct {
	D	dci_secno;		/* Start sector number		*/
	D	dci_endsec;		/* End sector numbern + 1	*/
	UW	dci_ratio;		/* Sectors per page		*/
	W	dci_pagelen;		/* Total pages			*/
	W	dci_updatecnt;		/* Updated page count		*/
	DCachePage  *dci_pages;		/* Cache pages array		*/
	UB	*dci_bufs;		/* Cache buffers array		*/
	QUEUE	dci_hsque[DCI_NHSQUE];	/* Hash search entries
						for useque and actque	*/
	QUEUE	dci_freeque;		/* Free page queue		*/
	QUEUE	dci_useque;		/* Cached page queue		*/
	QUEUE	dci_actque;		/* Active page queue		*/
} DCacheInfo;

struct DiskCachePage {
	QUEUE	dcp_q;			/* Link of hsque or freeque	*/
	QUEUE	dcp_uq;			/* Link of useque or actque	*/
	DCacheInfo *dcp_dcinf;		/* Disk cache information	*/
	UB	*dcp_buf;		/* Buffer pointer		*/
	D	dcp_secno;		/* Start sector number		*/
	UW	dcp_seclen;		/* Number of sectors		*/
	H	dcp_accnt;		/* Access counter		*/
	BOOL	dcp_read:8;		/* Read flag			*/
	BOOL	dcp_update:8;		/* Dirty flag			*/
	VW	dcp_info;		/* Information (used for inode)	*/
};

/* Get the head address of struct DiskCachePage from the queue */
#define PageFromHsque(q)	((DCachePage*)(q))
#define PageFromFreeque(q)	((DCachePage*)(q))
#define PageFromUseque(q)	((DCachePage*)((UW)(q) - \
					offsetof(DCachePage, dcp_uq)))
#define PageFromActque(q)	((DCachePage*)((UW)(q) - \
					offsetof(DCachePage, dcp_uq)))

/* File reference node */
typedef struct FileIndexNode {
	QUEUE	ino_q;		/* For queue connection 		*/
	UD	ino_ino;	/* File number				*/
	UB	*ino_path;	/* Abs path (for directory only)	*/
	UB	ino_type;	/* File type				*/
	H	ino_wrefcnt;	/* Writing reference count		*/
	W	ino_refcnt;	/* Reference count			*/
	VP	ino_info;	/* Informatiuon (used for cluster list)	*/
} INODE;

/* File type */
#define DT_UNKNOWN	(0)	/* Unknown				*/
#define DT_DIR		(4)	/* Directory				*/
#define DT_REG		(8)	/* Normal file				*/

/* Special file number "ino" */
#define NOEXS_INO	(0)	/* Not exist				*/

/* File descriptor */
struct FileDescriptor {
	FS	*fd_fs;		/* File system connection information	*/
	INODE	*fd_inode;	/* File reference node			*/
	D	fd_fpos;	/* Location of file pointer		*/
	W	fd_omode;	/* Open mode				*/
	W	fd_refcnt;	/* Reference count			*/
};
typedef struct FileDescriptor	FD;

/* Current directory information */
typedef struct {
	INODE	*cd_inode;	/* Current directory			*/
} CDINF;

/* Path name analysis parameter */
typedef struct {
	UB	*lp_path;	/* Absolute path (FIMP origin)		*/
	INODE	*lp_inode;	/* File information			*/
} LPINF;

/* Analysis result flag */
#define LOOKUP_DONE	(0x01)	/* The analysis end			*/

/* Common information */
typedef struct {
	FD	**uxi_fdtbl;	/* Ptr to open file table		*/
	CDINF	uxi_curdir;	/* Last access directory		*/
	mode_t	uxi_cmask;	/* File creation mode mask		*/
	INODE	*uxi_relino;	/* The inode released later		*/
} UXINFO;

/* Task specific data */
typedef union {
	struct {
	_UB	break_enb;	/* Break enabled for fs_break()		*/
	_UB	break_req;	/* Break rquested by fs_break()		*/
	_UB	break_done;	/* Break done by fs_break()		*/
	_UB	rsv1;		/* Reserved				*/
	} c;
	_UW	w;
} TaskSts;

/* Common file system informations */
#define FS_NIQUE	(8)	/* Inode hash queue size (= 2 ** N)	*/
#define FS_IQUEMSK	(FS_NIQUE - 1)
struct FileSystem {
	QUEUE	fs_q;		/* For queue connection 		*/
	coninf_t  *fs_coninf;	/* Connection information		*/
	UW	fs_conlen;	/* Ignore bytes of the path		*/
	W	fs_devnum;	/* Device number			*/
	UD	fs_rootino;	/* Root file number			*/

	QUEUE	fs_ique[FS_NIQUE];	/* INODE connection hash queue	*/

	ID	fs_tskid;	/* File system task			*/
	ID	fs_porid;	/* Rendezvous port for file system task	*/
	TaskSts	*fs_tsksts;	/* Current task status			*/

	ID	fs_devid;	/* Device descriptor			*/
	DiskInfo  fs_diskinfo;	/* Disk information			*/
	BOOL	fs_rdonly;	/* True when connecting reading only	*/

	UXINFO	fs_uxinfo;	/* Common information			*/
};

/* Cluster address:
	!! This is also used for sector address of FAT, Rootdir and FSInf
	   those must be located within 32 bits sector number.
*/
typedef struct {
	UW	ca_no;		/* Cluster (/ sector) number		*/
	UW	ca_len;		/* The number of clusters(/ sectors)	*/
} CLAD;

/* Disk map information */
typedef struct {
	FATFS	*m_fs;		/* Related FATFS			*/
	DCacheInfo  *m_dcinf;	/* Cache info				*/
	UW	m_flags;	/* Flags MAP_C/S, MAP_XXX		*/
	VW	m_info;		/* Information (inode)			*/
	CLAD	*m_clad;	/* CLAD	chain (array)			*/
	UW	m_clen;		/* Number of m_clad[]			*/
	UW	m_bpp;		/* Bytes per cache page 		*/
	W	m_lastix;	/* Last index to m_clad		(*1)	*/
	UW	m_lastoff;	/* Last offset of m_clad[m_idx]	(*1)	*/
	DCachePage  *m_page;	/* Last disk cache page			*/
	UW	m_btop, m_bend;	/* Top & End data ix of in page buffer (*1)
				   data ix =	cluster (FAT map) or
						dirent offset (DIR map)	*/
				/*  (*1) Used for fast search		*/
} MapInfo;

/* m_flags */
#define	MAP_S		(0x00)	/* Map sector				*/
#define	MAP_C		(0x40)	/* Map cluster				*/

#define MAP_CLR 	(0x01)	/* Clear				*/
#define MAP_READ	(0x10)	/* Read at initailize			*/

/* FAT fils system */
typedef enum {
	FAT12 = 1,
	FAT16,
	FAT32
} FATFSType;

/* FAT File reference node (inode) */
typedef struct FAT_FileIndexNode {
	INODE	fino_c;		/* File reference node common portion	*/
	UW	fino_dircl;	/* Directory beginning cluster		*/
	UW	fino_dirofs;	/* Location from the top of directory	*/
	D	fino_filsz;	/* File size				*/
	UW	fino_fclno;	/* File top cluster			*/
	UW	fino_cllen;	/* Number of cluster list array elements */
	BOOL	fino_d_ronly:8;	/* Directory file type			*/
	UB	fino_ftype;	/* File type				*/
	UH	fino_maxcl;	/* Maximum number of cluster list array */
} FATNODE;

#define NUM_FATMAP_S	(1)

#define LFN_MAXLEN	(255)	/* LFN max characters			*/
#define LFN_MAXBUF	(LFN_MAXLEN + 1 + 1)	/* LFN Max buf size	*/

struct FAT_FileSystem {
	FS	ff_c;		/* File system common portion		*/

	/* Disk cache info */
	DCacheInfo	ff_dc_fat;	/* FAT Sector			*/
	DCacheInfo	ff_dc_rootdir;	/* Root directory(FAT12/16 only)*/
	DCacheInfo	ff_dc_data;	/* Data (Cluster unit)		*/
	DCacheInfo	ff_dc_other;	/* FSInfo, reserved sectors	*/

	FATFSType	ff_fstype;	/* File system format		*/
	UH	ff_clsz;	/* Cluster size (Byte count)		*/
	UB	ff_ratio;	/* Cluster/Sector			*/
	UB	ff_nfat;	/* Number of FAT mirrorings		*/
	UW	ff_lastcl;	/* Last cluster number			*/

	UW	ff_fat;		/* FAT beginning sector 		*/
	UW	ff_fatsz;	/* Number of FAT sectors		*/

	UW	ff_clstart;	/* Cluster area beginning sector number */

	/* Root directory */
	UW	ff_rootcl;	/* Beginning cluster FAT32		*/
	UW	ff_rootsc;	/* Beginning sector  FAT12,16		*/
	UH	ff_rootent;	/* Number of entries FAT12,16		*/

	UH	ff_fsinfo;	/* File system information sector number */
	UW	ff_freecl;	/* Number of free clusters		*/
	UW	ff_nextfree;	/* Final allocation cluster number	*/

	UH	ff_nmbf[LFN_MAXBUF];	/* File name buffer (utf16)	*/
	UH	ff_nmwk[LFN_MAXBUF];	/* File name work (utf16)	*/

	FATNODE	ff_rootnode;	/* Root Inode				*/

	/* Disk cache body */
	MapInfo	ff_fatmap;	/* FAT sector map			*/
	CLAD	ff_fatmap_s[NUM_FATMAP_S];
};

/* Service command packet */
typedef struct {
	ID	cp_tskid;	/* Task ID				*/
	fimp_t	*cp_req;	/* Service request			*/
	ER	cp_err;		/* Return error code			*/
} CMDPKT;

/*-------------- FAT file system format definitions --------------------*/
#pragma pack(1)			/* GCC dependent			*/

/* Boot sector */
typedef struct {
	UB	bo_jmp[3];	/* Jump command 			*/
	UB	bo_oem[8];	/* OEM name				*/
	UB	bo_scsz[2];	/* Sector size (byte count)		*/
	UB	bo_clsz;	/* Cluster size (number of sectors)	*/
	UB	bo_rsvsc[2];	/* Number of reserved sectors		*/
	UB	bo_nfat;	/* The number of FAT			*/
	UB	bo_rootent[2];	/* The number of route directory entries */
	UB	bo_dsksz[2];	/* Disk size(The number of sectors)	*/
	UB	bo_media;	/* Media identifier			*/
	UH	bo_fatsz;	/* FAT size(the number of sectors)	*/
	UH	bo_tracksz;	/* Track size(the number of sectors)	*/
	UH	bo_head;	/* Head count				*/
	UH	bo_hidsc_lo;	/* The number of hidden sectors: Lower	*/
	UH	bo_hidsc_hi;	/* The number of hidden sectors: Upper	*/
	UH	bo_dsksz_lo;	/* Disc size(the number of sectors): Lower */
	UH	bo_dsksz_hi;	/* Disc size(the number of sectors): Upper */
	UH	bo_fatsz_lo;	/* FAT size(the number of sectors): Lower */
	UH	bo_fatsz_hi;	/* FAT size(the number of sectors): Upper */
	UH	bo_extflg;	/* Extended flag			*/
	UH	bo_fsver;	/* File system version			*/
	UH	bo_root_lo;	/* Root directory start cluster number:Lower */
	UH	bo_root_hi;	/* Root directory start cluster number:Upper */
	UH	bo_fsinfo;	/* Sector number of file system information */
	UH	bo_backupboot;	/* Sector number of backup boot sector	*/
	UH	bo_resv[6];	/* Reserve				*/
} FAT_BOOTSEC;

/* Expanded flag(extflg) */
#define FAT_ActiveFAT	(0x000f)	/* Active FAT number		*/
#define FAT_NoFATMirror	(0x0080)	/* FAT mirroring		*/

/* FSInfo (FAT32) */
typedef struct {
	UW	fsi_leadsig;	/* Signature (0x41615252)		*/
	UB	fsi_resv1[480];	/* (Reserve)				*/
	UW	fsi_structsig;	/* Signature (0x61417272)		*/
	UW	fsi_freecl;	/* Number of empty clusters		*/
	UW	fsi_nextfree;	/* Final allocation cluster number	*/
	UB	fsi_resv2[12];	/* (Reserve)				*/
	UW	fsi_trailsig;	/* Signature (0xAA550000)		*/
} FAT_FSINFO;

/* Directory entry */
typedef struct {
	UB	de_fname[8];	/* File name				*/
	UB	de_extra[3];	/* Extension				*/
	UB	de_ftype;	/* File type				*/
	UB	de_smallcaps;	/* Small character flag 		*/
	UB	de_rsv[1];	/* Reserve				*/
	UH	de_ctime;	/* Creation time			*/
	UH	de_cdate;	/* Creation date			*/
	UH	de_adate;	/* Last access date			*/
	UH	de_clno_hi;	/* Starting cluster(Upper)		*/
	UH	de_mtime;	/* Update time				*/
	UH	de_mdate;	/* Update date				*/
	UH	de_clno_lo;	/* Staring cluster(Lower)		*/
	W	de_fsize;	/* File size				*/
} FAT_DIRENT;

/* File type(ftype) */
#define FAT_RDONLY	(0x01)	/* Read only				*/
#define FAT_HIDDEN	(0x02)	/* Hidden file				*/
#define FAT_SYSTEM	(0x04)	/* System file				*/
#define FAT_VOLUME	(0x08)	/* Volume label				*/
#define FAT_SUBDIR	(0x10)	/* Subdirectory				*/
#define FAT_ARCHIV	(0x20)	/* Archive file (entry modified)	*/
#define FAT_LONGNAME	(0x0f)	/* Long file name (multi bits)		*/

/* Small character flag */
#define SMALL_BASE	(0x08)	/* Base name is small caps		*/
#define SMALL_EXT	(0x10)	/* Extension is small caps		*/

/* Directory entry of long file name */
typedef struct {
	UB	lde_index;	/* Index number 			*/
	UB	lde_fname1[10];	/* File name 1 (Unicode)		*/
	UB	lde_ftype;	/* Emptiness:File type (0x0f Fixed)	*/
	UB	lde_smallcaps;	/* Emptiness:Small character flag (0 Fixed) */
	UB	lde_checksum;	/* Checksum				*/
	UB	lde_fname2[12];	/* File name2 (Unicode)			*/
	UH	lde_clno;	/* Emptiness:Cluster number (0 Fixed)	*/
	UB	lde_fname3[4];	/* File name3 (Unicode)			*/
} FAT_DIRENT_LFN;

#pragma pack()			/* GCC dependent			*/
/*----------------------------------------------------------------------*/

/* Directory/acccess handle */
#define ROOT_SLEN	(1)	/* Number of Root sector chain		*/
#define MAX_CLEN	(10)	/* Number of Cluster chain unit		*/

typedef struct {
	/* Directory identification information */
	UW	di_dircl;	/* Started cluster of directory 	*/
	FATNODE	*di_inode;	/* File reference node			*/
	/* Cache */
	CLAD	di_clno[MAX_CLEN];	/* Cluster chain		*/
	CLAD	di_secno[ROOT_SLEN];	/* Sectors chain of FAT12/16 rootdir */
	W	di_len;		/* The number of di_clno elements	*/
	MapInfo	di_map;		/* Disk map				*/
	UW	di_access;	/* Access condition			*/
	W	di_nent;	/* The number of diret entry inc. di_clno */
	/* Entry of current position */
	FAT_DIRENT	*di_dirent;	/* Directory entry		*/
	W	di_dirent_ofs;	/* Offset count from the di_clno[0].ca_clno */
	UW	di_offset;	/* Offset bytes from the top of directory */
} DIRINF;

/* Access condition */
#define AC_RDONLY	(0x0000)	/* Read only			*/
#define AC_WRITE	(0x0001)	/* Read & write			*/

/* Special values */
#define CLSTART 	(2)		/* Start cluster number		*/
#define CLEND		(0xffffffffUL)	/* Terminated cluster mark	*/
#define CLVOID		(0xffffffffUL)	/* Invalid cluster number	*/
#define SECVOID 	((D)(-1))	/* Invalid sector number	*/
#define OFSVOID 	(-1)		/* Invalid offset		*/
#define CKSVOID		(-1)		/* Invalid checksum		*/
#define OFSNEXT 	(0xffffffffUL)	/* Offset to specify next	*/

/* File name length */
#define BASELEN 	(8)		/* SFN: base name lenght	*/
#define EXTLEN		(3)		/* SFN: extension name length	*/
#define BASE_EXTLEN	(BASELEN + EXTLEN)

#define FILE_SIZE_MAX	(0xffffffffUL)		/* 4 G bytes		*/
#define DIR_SIZE_MAX	(2 * 1024 * 1024)	/* 2 M bytes		*/

/* BIOS Parameter Block */
#define BPB_READ_SIZE		(1)
#define BPB_SHIFT_FATSZ 	(16)
#define BPB_SHIFT_DSKSZ 	(16)
#define BPB_SHIFT_ROOT_HI	(16)
#define BPB_TAILSIGN_OFS1	(510)
#define BPB_TAILSIGN_OFS2	(511)
#define BPB_TAILSIGN_VAL1	(0x55)
#define BPB_TAILSIGN_VAL2	(0xaa)

/* FSinfo block */
#define FSINFO_READ_SIZE	(1)
#define FSINFO_LEADSIGN_VAL	(0x41615252)
#define FSINFO_STRUCTSIGN_VAL	(0x61417272)
#define FSINFO_TAILSIGN_VAL	(0xAA550000)

/* Directory LFN entry */
#define LFN_LAST		(0x40)	/* LFN last flag		*/
#define LFN_CNT 		(0x3f)	/* LFN order number mask	*/
#define LFN_CHARS		(13)	/* LFN characters per dirent	*/
#define LFN_MAXENT		(20)	/* LFN order number maximum	*/

/* Directory LFN entry */
#define DE_B0_END		(0x00)	/* The entry is free, and last	*/
#define DE_B0_FREE		(0xe5)	/* The entry is free.		*/
#define DE_B0_KANJI_ESC 	(0x05)	/* Substitution of 0xe5		*/
#define DE_SHIFT_CLNO_HI	(16)	/* Cluster number shift count	*/

/* FAT cluster number */
#define FAT12_EOC		(0x00000ff7)
#define FAT12_MASK_VALID_CLST	(0x00000fff)
#define FAT16_EOC		(0x0000fff7)
#define FAT32_EOC		(0x0ffffff7)
#define FAT32_MASK_VALID_CLST	(0x0fffffff)

/* Date/Time mask & shift counts */
#define FATYEAR_TO_TMYEAR	(80)		/* 1990 <-> 1980 */
#define FAT_DATE_MASK_MON	(0x01e0)
#define FAT_DATE_MAS_YEAR	(0xfe00)
#define FAT_DATE_SHIFT_YEAR	(9)
#define FAT_DATE_SHIFT_MON	(5)
#define FAT_TIME_MASK_HOUR	(0xf800)
#define FAT_TIME_MASK_MIN	(0x07e0)
#define FAT_TIME_MASK_SEC	(0x001f)
#define FAT_TIME_SHIFT_HOUR	(11)
#define FAT_TIME_SHIFT_MIN	(5)

/* File mode */
#define DEFAULT_FILMODE		(S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP \
						   | S_IROTH | S_IXOTH)
#define FILMODE_WRITE		(S_IWUSR | S_IWGRP | S_IWOTH)

/* SFN conversion result */
#define FAT_SAME_NAME		(0)	/* Conversion results is identical */
#define FAT_CHG_WITHOUT_N	(1)	/* Different conversion result	*/
#define FAT_CHG_WITH_N		(2)	/* Genertation numbered conversion */

#endif /* __FIMP_FAT_LOCAL_H__ */

