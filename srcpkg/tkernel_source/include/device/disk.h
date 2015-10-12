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
 *	disk.h
 *
 *	Disk driver
 */

#ifndef	__DEVICE_DISK_H__
#define	__DEVICE_DISK_H__

#include <basic.h>
#include <tk/devmgr.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Disk attribute data number */
typedef	enum {
	DN_DISKEVENT	= TDN_EVENT,	/* Message buffer for event notification
						data: ID		RW */
	DN_DISKINFO	= TDN_DISKINFO,	/* Disk information
						data: DiskInfo		R- */
	DN_DISKINFO_D	= TDN_DISKINFO_D, /* Disk information
						data: DiskInfo_D	R- */
	DN_DISKFORMAT	= -100,		/* Disk format
						data: DiskFormat	-W */
	DN_DISKINIT	= -101,		/* Disk initialization
						data: DiskInit		-W */
	DN_DISKCMD	= -102,		/* Disk command
						data: DiskCmd		-W */
	DN_DISKMEMADR	= -103,		/* First address of memory disk area
						data: void*		R- */
	DN_DISKPARTINFO	= -104,		/* Disk partition information
						data: DiskPartInfo	R- */
	DN_DISKCHSINFO	= -105,		/* Disk CHS information
						data: DiskCHSInfo	R- */
	DN_DISKIDINFO	= -106		/* Disk identifying information
						data: UB[]		R- */
} DiskDataNo;

/* DN_DISKFORMAT: Disk format (W) */
typedef enum {
	DiskFmt_MEMINIT	= -2,		/* Memory disk initialization	*/
	DiskFmt_MEM	= -1,		/* Memory disk		*/
	DiskFmt_STD	= 0,		/* Only this type of standard HD	*/
	DiskFmt_2DD	= 1,		/* 2DD 720KB			*/
	DiskFmt_2HD	= 2,		/* 2HD 1.44MB			*/
	DiskFmt_VHD	= 3,		/* Floptical 20MB		*/
	DiskFmt_CDROM	= 4,		/* CD-ROM 640MB			*/
	DiskFmt_2HD12	= 0x12		/* 2HD 1.2MB			*/
} DiskFormat;

/* DN_DISKINFO: Disk information (R) */
struct diskinfo {
	DiskFormat format;		/* Format type		*/
	unsigned int	protect:1;		/* Protect state		*/
	unsigned int	removable:1;		/* Removable or not			*/
	unsigned int	rsv:30;			/* Reserved (0)			*/
	W	blocksize;		/* Number of block bytes		*/
	W	blockcont;		/* Total number of blocks			*/
};
#ifndef __diskinfo__
#define __diskinfo__
typedef struct diskinfo		DiskInfo;
#endif

/* DN_DISKINFO_D: Disk information (R) */
struct diskinfo_d {
	DiskFormat format;		/* Format type		*/
	unsigned int	protect:1;		/* Protect state		*/
	unsigned int	removable:1;		/* Removable or not			*/
	unsigned int	rsv:30;			/* Reserved (0)			*/
	W	blocksize;		/* Number of block bytes		*/
	D	blockcont_d;		/* Total number of blocks			*/
};
#ifndef __diskinfo_d__
#define __diskinfo_d__
typedef struct diskinfo_d	DiskInfo_D;
#endif

/* DN_DISKINIT: Disk initialization (W) */
typedef enum {
	DISKINIT = 1
} DiskInit;

/* DN_DISKCMD:	Disk command (W) */
typedef	struct {
	B	clen;			/* Length of SCSI command		*/
	UB	cdb[12];		/* SCSI command		*/
	W	dlen;			/* Data length			*/
	UB	*data;			/* Data address		*/
} DiskCmd;

/* DN_DISKPARTINFO: Disk partition information (R) */
typedef enum {				/* Disk system ID		*/
	DSID_NONE	= 0x00,
	DSID_DOS1	= 0x01,
	DSID_STDFS_X	= 0x03,		/* Regard XENIX as STDFS.	*/
	DSID_DOS2	= 0x04,
	DSID_DOSE	= 0x05,
	DSID_DOS3	= 0x06,
	DSID_HPFS	= 0x07,
	DSID_FS		= 0x08,
	DSID_AIX	= 0x09,
	DSID_OS2	= 0x0A,
	DSID_WIN95	= 0x0B,
	DSID_WIN95L	= 0x0C,
	DSID_DOS3L	= 0x0E,
	DSID_DOS3E	= 0x0F,
	DSID_STDFS	= 0x13,
	DSID_VENIX	= 0x40,
	DSID_CPM1	= 0x52,
	DSID_UNIX	= 0x63,
	DSID_NOVELL1	= 0x64,
	DSID_NOVELL2	= 0x65,
	DSID_PCIX	= 0x75,
	DSID_MINIX1	= 0x80,
	DSID_MINIX2	= 0x81,
	DSID_LINUX1	= 0x82,
	DSID_LINUX2	= 0x83,
	DSID_AMOEBA	= 0x93,
	DSID_BSDI	= 0x9F,
	DSID_386BSD	= 0xA5,
	DSID_CPM2	= 0xDB,
	DSID_DOSSEC	= 0xF2
} DiskSystemId;

#define	isSTDFS_SID(id)		(((id) == DSID_STDFS )||( (id) == DSID_STDFS_X))

typedef struct {
	DiskSystemId	systemid;	/* System ID			*/
	W		startblock;	/* Start block number		*/
	W		endblock;	/* End block number		*/
} DiskPartInfo;

/* DN_DISKCHSINFO: Disk CHS information (R) */
typedef struct {
	W	cylinder;		/* Total number of cylinders			*/
	W	head;			/* Number of heads per cylinder	*/
	W	sector;			/* Number of sectors per head	*/
} DiskCHSInfo;

/* Event notification */
typedef struct {
	T_DEVEVT_ID	h;		/* Standard header (with device ID)	*/
	UW		info;		/* Additional information			*/
} DiskEvt;

/* DOS/V-format partition information */
typedef struct {
	UB	BootInd;		/* Boot indicator		*/
	UB	StartHead;		/* Start head number		*/
	UB	StartSec;		/* Start sector number		*/
	UB	StartCyl;		/* Start cylinder number		*/
	UB	SysInd;			/* System indicator	*/
	UB	EndHead;		/* End head number		*/
	UB	EndSec;			/* End sector number		*/
	UB	EndCyl;			/* End cylinder number		*/
	UH	StartBlock[2];		/* Relative start sector number		*/
	UH	BlockCnt[2];		/* Number of sectors			*/
} PartInfo;

#define	MAX_PARTITION	4		/* Number of partitions		*/

/* Structure of block 0 */
typedef struct {
	VB		boot_prog[0x1be];	/* Boot program	*/
	PartInfo	part[MAX_PARTITION];	/* Partition information	*/
	UH		signature;		/* Signature		*/
} DiskBlock0;

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_DISK_H__ */
