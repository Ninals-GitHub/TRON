/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
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

#ifndef	__BTK_ERRCODE_H__
#define	__BTK_ERRCODE_H__

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	EC_OK		(  0)	/* Completed successfully			*/

#define	EC_ADR		(-1)	/* Invalid address				*/
#define	EC_PAR		(-2)	/* Invalid parameter				*/
#define	EC_NOSPT	(-3)	/* Not supported function			*/
#define	EC_NOSPC	(-4)	/* No memory space left on system		*/
#define	EC_ID		(-5)	/* Invalid id					*/
#define	EC_EXS		(-6)	/* Object exists				*/
#define	EC_NOEXS	(-7)	/* Object does not exist			*/
#define	EC_NOMEM	(-8)	/* Out of memory				*/
#define	EC_CTX		(-9)	/* Context error				*/
#define	EC_SYS		(-10)	/* System undefined error			*/
#define	EC_UNF		(-11)	/* Undifined system call			*/
#define	EC_SZOVR	(-12)	/* Exceeds length of system limit		*/
#define	EC_MPTR		(-13)	/* Invalid shared memory pointer		*/
#define	EC_AKEY		(-14)	/* Invalid shared memory access key		*/
#define	EC_PPRI		(-17)	/* Out of priority range of a process		*/
#define	EC_NOPRC	(-18)	/* Process does not exist			*/
#define	EC_SELF		(-19)	/* Specified self process			*/
#define	EC_LEVEL	(-20)	/* Specified higher level process		*/
#define	EC_MINTR	(-21)	/* Interrupted waiting in message handler	*/
#define	EC_DLT		(-22)	/* Semaphore is already deleted			*/
#define	EC_OBJ		(-23)	/* Invalid operation for a object		*/
#define	EC_NONE		(-24)	/* Cannot take a message/semaphore		*/
#define	EC_INIT		(-25)	/* Initialization handerl is abnormal terminated*/
#define	EC_FNAME	(-26)	/* Invalid path name				*/
#define	EC_FD		(-27)	/* Invalid file descriptor			*/
#define	EC_ACCES	(-28)	/* File access is denied			*/
#define	EC_PERM		(-29)	/* Immutable file				*/
#define	EC_RONLY	(-30)	/* File/media is protected against a write acces*/
#define	EC_PWD		(-31)	/* Invalid file password			*/
#define	EC_ENDR		(-32)	/* Current record is an end of record		*/
#define	EC_REC		(-33)	/* Invalid record type or record type not exist	*/
#define	EC_NOLNK	(-34)	/* Not a link file				*/
#define	EC_LOCK		(-35)	/* Record is locked				*/
#define	EC_LIMIT	(-36)	/* Exceed system limit				*/
#define	EC_XFS		(-37)	/* Object belongs to other file system		*/
#define	EC_NOFS		(-38)	/* File system is not mounted			*/
#define	EC_NODSK	(-39)	/* Out of disk space				*/
#define	EC_TRON		(-40)	/* Not a btron standard floppy disk format	*/
#define	EC_DD		(-41)	/* Device descriptor does not exist		*/
#define	EC_DEV		(-42)	/* The operation is forbidden for a device	*/
#define	EC_BUSY		(-43)	/* Device/file is busy				*/
#define	EC_NODEV	(-44)	/* Cannot access to device			*/
#define	EC_ERDEV	(-45)	/* Hardware error				*/
#define	EC_NOMDA	(-46)	/* Device is not inserted a media		*/
#define	EC_IO		(-47)	/* I/O error, aborted				*/

#define	EC_INNER	(-50)	/* System error of ITRON kenrnel		*/
#define	EC_UNIX		(-51)	/* Error of UNIX emmulator			*/


#define	ED_NONE		(0)		/* There is no error code		*/

/* EC_PAR:	Invalid parameter						*/
#define	ED_CMD		(0x8000)	/* Invalid command			*/
#define	ED_DEVID	(0x8001)	/* Invalid device id			*/
#define	ED_DATANO	(0x8002)	/* Invalid data number			*/
#define	ED_DATACNT	(0x8003)	/* Invalid number of data		*/
#define	ED_DATA		(0x8004)	/* Invalid data				*/

/* EC_INNER:	Error of ITRON kernel
			Error of ITRON kernel (<0)				*/

/* EC_UNIX:	Error of UNIX emulator
			UNIX error code(errno)					*/

/* EC_IO, EC_NOMDA, EC_ERDEV, etc.:
	SCSI device error： 0000 KKKK CCCC CCCC
				K:sense key, C：sense code			*/


/* Error code (ERR : ErrCode.err)
*/
#ifndef	E_OK
#define	E_OK		0	/* Completed successfully			*/
#endif

#define	ER_OK		(EC_OK	<< 16)

#define	ER_ADR		(EC_ADR << 16)
#define	ER_PAR		(EC_PAR << 16)
#define	ER_NOSPT	(EC_NOSPT << 16)
#define	ER_NOSPC	(EC_NOSPC << 16)
#define	ER_ID		(EC_ID << 16)
#define	ER_EXS		(EC_EXS << 16)
#define	ER_NOEXS	(EC_NOEXS << 16)
#define	ER_NOMEM	(EC_NOMEM << 16)
#define	ER_CTX		(EC_CTX << 16)
#define	ER_SYS		(EC_SYS << 16)
#define	ER_UNF		(EC_UNF << 16)
#define	ER_SZOVR	(EC_SZOVR << 16)
#define	ER_MPTR		(EC_MPTR << 16)
#define	ER_AKEY		(EC_AKEY << 16)
#define	ER_PPRI		(EC_PPRI << 16)
#define	ER_NOPRC	(EC_NOPRC << 16)
#define	ER_SELF		(EC_SELF << 16)
#define	ER_LEVEL	(EC_LEVEL << 16)
#define	ER_MINTR	(EC_MINTR << 16)
#define	ER_DLT		(EC_DLT << 16)
#define	ER_OBJ		(EC_OBJ << 16)
#define	ER_NONE		(EC_NONE << 16)
#define	ER_INIT		(EC_INIT << 16)
#define	ER_FNAME	(EC_FNAME << 16)
#define	ER_FD		(EC_FD << 16)
#define	ER_ACCES	(EC_ACCES << 16)
#define	ER_PERM		(EC_PERM << 16)
#define	ER_RONLY	(EC_RONLY << 16)
#define	ER_PWD		(EC_PWD << 16)
#define	ER_ENDR		(EC_ENDR << 16)
#define	ER_REC		(EC_REC << 16)
#define	ER_NOLNK	(EC_NOLNK << 16)
#define	ER_LOCK		(EC_LOCK << 16)
#define	ER_LIMIT	(EC_LIMIT << 16)
#define	ER_XFS		(EC_XFS << 16)
#define	ER_NOFS		(EC_NOFS << 16)
#define	ER_NODSK	(EC_NODSK << 16)
#define	ER_TRON		(EC_TRON << 16)
#define	ER_DD		(EC_DD << 16)
#define	ER_DEV		(EC_DEV << 16)
#define	ER_BUSY		(EC_BUSY << 16)
#define	ER_NODEV	(EC_NODEV << 16)
#define	ER_ERDEV	(EC_ERDEV << 16)
#define	ER_NOMDA	(EC_NOMDA << 16)
#define	ER_IO		(EC_IO << 16)

#define	ER_INNER	(EC_INNER << 16)
#define	ER_UNIX		(EC_UNIX << 16)


/* DP error code */
#define	EG_OK		(0)
#define EG_ADR		((-64) << 16)
#define EG_PAR		((-65) << 16)
#define EG_NOSPT	((-66) << 16)
#define EG_NOSPC	((-67) << 16)
#define EG_GID		((-68) << 16)
#define EG_LOCK 	((-69) << 16)
#define EG_LIMIT	((-70) << 16)
#define EG_NOEXS	((-71) << 16)
#define EG_DEV		((-72) << 16)
#define EG_ERDEV	((-73) << 16)

#define EG_FORM 	((-80) << 16)
#define EG_ENV		((-81) << 16)
#define EG_PTRID	((-82) << 16)

/* External kernel error */
#define	EX_OK		(0)
#define	EX_ADR		((-257) << 16)
#define	EX_AKEY		((-258) << 16)
#define	EX_DATA 	((-259) << 16)
#define	EX_CKEY 	((-260) << 16)
#define	EX_DFMT 	((-261) << 16)
#define	EX_DNUM 	((-262) << 16)
#define	EX_DRAG 	((-263) << 16)
#define	EX_EXS		((-264) << 16)
#define	EX_FD		((-265) << 16)
#define	EX_FONT 	((-266) << 16)
#define	EX_FTD		((-267) << 16)
#define	EX_FTFMT	((-268) << 16)
#define	EX_FTID 	((-269) << 16)
#define	EX_KEY		((-272) << 16)
#define	EX_LIMIT	((-273) << 16)
#define	EX_MID		((-274) << 16)
#define	EX_NAK		((-276) << 16)
#define	EX_NOSPC	((-277) << 16)
#define	EX_NOEXS	((-278) << 16)
#define	EX_PAR		((-279) << 16)
#define	EX_PID		((-280) << 16)
#define	EX_PNID 	((-281) << 16)
#define	EX_SAVE 	((-282) << 16)
#define	EX_SDATA	((-283) << 16)
#define	EX_TID		((-284) << 16)
#define	EX_NOSPT	((-285) << 16)
#define	EX_TRAY 	((-286) << 16)
#define	EX_WID		((-287) << 16)
#define	EX_WND		((-288) << 16)
#define	EX_WPRC 	((-289) << 16)
#define	EX_VID		((-290) << 16)
#define	EX_VOBJ 	((-291) << 16)

#define EX_SPOOL	((-300) << 16)
#define EX_PROC 	((-301) << 16)
#define EX_SPOBJ	((-302) << 16)
#define EX_SPID 	((-303) << 16)
#define EX_FILE 	((-304) << 16)

/* UNIX like error codes (TCPIP) */
#define EX_HOSTUNREACH	((-401) << 16)	/* No route to host				*/
#define EX_TIMEDOUT	((-402) << 16)	/* TCPIP timeout				*/
#define EX_CONNABORTED	((-403) << 16)	/* Connection is aborted			*/
#define EX_NOBUFS	((-404) << 16)	/* Out of TCPIP internal buffer			*/
#define EX_BADF		((-405) << 16)	/* Invalid socket descriptor			*/
#define EX_WOULDBLOCK	((-407) << 16)	/* Operation would block			*/
#define EX_MSGSIZE	((-408) << 16)	/* Message too long				*/
#define EX_DESTADDRREQ	((-409) << 16)	/* Destination address required			*/
#define EX_PROTOTYPE	((-410) << 16)	/* Protocol wrong type for socket		*/
#define EX_NOPROTOOPT	((-411) << 16)	 /* Protocol not supported			*/
#define EX_PROTONOSUPPORT ((-412) << 16) /* Protocol not supported			*/
#define EX_SOCKTNOSUPPORT ((-413) << 16) /* Socket type not supported			*/
#define EX_OPNOTSUPP	((-414) << 16)	/* Operation not supported on transport endpoint*/
#define EX_PFNOSUPPORT	((-415) << 16)	/* Protocol family not supported		*/
#define EX_AFNOSUPPORT	((-416) << 16)	/* Address family not supported by protocol	*/
#define EX_ADDRINUSE	((-417) << 16)	/* Address already in use			*/
#define EX_ADDRNOTAVAIL	((-418) << 16)	/* Cannot assign requested address		*/
#define EX_NETDOWN	((-419) << 16)	/* Network is down				*/
#define EX_NETUNREACH	((-420) << 16)	/* Network is unreachable			*/
#define EX_NETRESET	((-421) << 16)	/* Network dropped connection because of rest	*/
#define EX_CONNRESET	((-422) << 16)	/* Connection reset by peer			*/
#define EX_ISCONN	((-423) << 16)	/* Transport endpoint is already connected	*/
#define EX_NOTCONN	((-424) << 16)	/* Transport endpoint is not connected		*/
#define EX_SHUTDOWN	((-425) << 16)	/* Cannot send after transport endpoint shutdown*/
#define EX_CONNREFUSED	((-426) << 16)	/* Connection refused				*/
#define EX_HOSTDOWN	((-427) << 16)	/* Host is down					*/
#define EX_ALREADY	((-428) << 16)	/* Operation already in progress		*/
#define EX_INPROGRESS	((-429) << 16)	/* Operation now in progress			*/


/*
==================================================================================

	Management 

==================================================================================
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	// __BTK_ERRCODE_H__
