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

#ifndef	__BK_DRIVERS_MAJOR_H__
#define	__BK_DRIVERS_MAJOR_H__

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
#define	UNNAMED_MAJOR		0
#define	MEM_MAJOR		1
#define	RAMDISK_MAJOR		MEM_MAJOR
#define	FLOPPY_MAJOR		2
#define	PTY_MASTER_MAJOR	2
#define	IDE0_MAJOR		3
#define	HD_MAJOR		IDE0_MAJOR
#define	PTY_SLAVE_MAJOR		3
#define	TTY_MAJOR		4
#define	TTYAUX_MAJOR		5
#define	LP_MAJOR		6
#define	VCS_MAJOR		7
#define	LOOP_MAJOR		7
#define	SCSI_DISK0_MAJOR	8
#define	SCSI_TAPE_MAJOR		9
#define	MD_MAJOR		9
#define	MISC_MAJOR		10
#define	SCSI_CDROM_MAJOR	11
#define	MUX_MAJOR		11
#define	XT_DISK_MAJOR		13
#define	INPUT_MAJOR		13
#define	SOUND_MAJOR		14
#define	CDU31A_CDROM_MAJOR	15
#define	JOYSTICK_MAJOR		15
#define	GOLDSTAR_CDROM_MAJOR	16
#define	OPTICS_CDROM_MAJOR	17
#define	SANYO_CDROM_MAJOR	18
#define	CYCLADES_MAJOR		19
#define	CYCLADESAUX_MAJOR	20
#define	MITSUMI_X_CDROM_MAJOR	20
#define	MFM_ACORN_MAJOR		21
#define	SCSI_GENERIC_MAJOR	21
#define	IDE1_MAJOR		22
#define	DIGICU_MAJOR		22
#define	DIGI_MAJOR		23
#define	MITSUMI_CDROM_MAJOR	23
#define	CDU535_CDROM_MAJOR	24
#define	STL_SERIAL_MAJOR	24
#define	MATSUSHITA_CDROM_MAJOR	25
#define	STL_CALLOUT_MAJOR	25
#define	MATSUSHITA_CDROM2_MAJOR	26
#define	QIC117_TAPE_MAJOR	27
#define	MATSUSHITA_CDROM3_MAJOR	27
#define	MATSUSHITA_CDROM4_MAJOR	28
#define	STL_SIOMEM_MAJOR	28
#define	ACSI_MAJOR		28
#define	AZTECH_CDROM_MAJOR	29
#define	FB_MAJOR		29
#define	MTD_BLOCK_MAJOR		31
#define	CM206_CDROM_MAJOR	32
#define	IDE2_MAJOR		33
#define	IDE3_MAJOR		34
#define	Z8530_MAJOR		34
#define	XPRAM_MAJOR		35
#define	NETLINK_MAJOR		36
#define	PS2ESDI_MAJOR		36
#define	IDETAPE_MAJOR		37
#define	Z2RAM_MAJOR		37
#define	APBLOCK_MAJOR		38
#define	DDV_MAJOR		39



#define	NBD_MAJOR		43




#define	RISCOM8_NORMAL_MAJOR	48
#define	DAC960_MAJOR		48
#define	RISCOM8_CALLOUT_MAJOR	49



#define	MKISS_MAJOR		55
#define	DSP56K_MAJOR		55
#define	IDE4_MAJOR		56
#define	IDE5_MAJOR		57







#define	SCSI_DISK1_MAJOR	65
#define	SCSI_DISK2_MAJOR	66
#define	SCSI_DISK3_MAJOR	67
#define	SCSI_DISK4_MAJOR	68
#define	SCSI_DISK5_MAJOR	69
#define	SCSI_DISK6_MAJOR	70
#define	SCSI_DISK7_MAJOR	71

#define	COMPAQ_SMART2_MAJOR	72
#define	COMPAQ_SMART2_MAJOR1	73
#define	COMPAQ_SMART2_MAJOR2	74
#define	COMPAQ_SMART2_MAJOR3	75
#define	COMPAQ_SMART2_MAJOR4	76
#define	COMPAQ_SMART2_MAJOR5	77
#define	COMPAQ_SMART2_MAJOR6	78
#define	COMPAQ_SMART2_MAJOR7	79

#define	SPECIALIX_NORMAL_MAJOR	75
#define	SPECIALIX_CALLOUT_MAJOR	76

#define	AURORA_MAJOR		79
#define	I2O_MAJOR		80




#define	SHMIQ_MAJOR		85
#define	SCSI_CHANGER_MAJOR	86

#define	IDE6_MAJOR		87
#define	IDE7_MAJOR		89
#define	IDE8_MAJOR		90
#define	MTD_CHAR_MAJOR		90
#define	IDE9_MAJOR		91


#define	DASD_MAJOR		94
#define	MDISK_MAJOR		95


#define	UBD_MAJOR		98
#define	PP_MAJOR		99
#define	JSFD_MAJOR		99
#define	PHONE_MAJOR		100


#define	COMPAQ_CISS_MAJOR	104
#define	COMPAQ_CISS_MAJOR1	105
#define	COMPAQ_CISS_MAJOR2	106
#define	COMPAQ_CISS_MAJOR3	107
#define	COMPAQ_CISS_MAJOR4	108
#define	COMPAQ_CISS_MAJOR5	109
#define	COMPAQ_CISS_MAJOR6	110
#define	COMPAQ_CISS_MAJOR7	111
#define	VIODASD_MAJOR		112
#define	VIOCD_MAJOR		113
#define	ATARAID_MAJOR		114













#define	SCSI_DISK8_MAJOR	128
#define	SCSI_DISK9_MAJOR	129
#define	SCSI_DISK10_MAJOR	130
#define	SCSI_DISK11_MAJOR	131
#define	SCSI_DISK12_MAJOR	132
#define	SCSI_DISK13_MAJOR	133
#define	SCSI_DISK14_MAJOR	134
#define	SCSI_DISK15_MAJOR	135

#define	UNIX98_PTY_MASTER_MAJOR	128
#define	UNIX98_PTY_MAJOR_COUNT	8
#define	UNIX98_PTY_SLAVE_MAJOR	(UNIX98_PTY_MASTER_MAJOR + UNIX98_PTY_MAJOR_COUNT)








#define	DRBD_MAJOR		147


#define	RTF_MAJOR		150











#define	RAW_MAJO		162



#define	USB_ACM_MAJOR		166
#define	USB_ACM_AUX_MAJOR	167











#define	MMC_BLOCK_MAJOR		179
#define	USB_CHAR_MAJOR		180




















#define	VXVM_MAJOR		199
#define	VXSPEC_MAJOR		200
#define	VXDMP_MAJOR		201
#define	XENVBD_MAJOR		201
#define	MSR_MAJOR		202
#define	CPUID_MAJOR		203


#define	OSST_MAJOR		206




















#define	IBM_TTY3270_MAJOR	227
#define	IBM_FS3270_MAJOR	228

#define	VIOTAPE_MAJOR		230





























#define	BLOCK_EXT_MAJOR		259
#define	SCSI_OSD_MAJOR		260

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

#endif	// __BK_DRIVERS_MAJOR_H__
