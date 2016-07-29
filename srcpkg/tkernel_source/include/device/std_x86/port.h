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

#ifndef	__STD_X86_PORT_H__
#define	__STD_X86_PORT_H__


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
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :8237 High Performance Programmable DMA Controllerr
				 ( DMAC 0 ) 

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
/*
==================================================================================

	ISA DMAC Channel Ports ( Channel Register )
	
	DMAC 0

==================================================================================
*/
#define	PORT_DMAC_CH0_ADDRESS	0x0000	/* Channel 0 Address Register		*/
#define	PORT_DMAC_CH0_COUNTER	0x0001	/* Channel 0 Counter Register		*/
#define	PORT_DMAC_CH1_ADDRESS	0x0002	/* Channel 1 Address Register		*/
#define	PORT_DMAC_CH1_COUNTER	0x0003	/* Channel 1 Counter Register		*/
#define	PORT_DMAC_CH2_ADDRESS	0x0004	/* Channel 2 Address Register		*/
#define	PORT_DMAC_CH2_COUNTER	0x0005	/* Channel 2 Counter Register		*/
#define	PORT_DMAC_CH3_ADDRESS	0x0006	/* Channel 3 Address Register		*/
#define	PORT_DMAC_CH3_COUNTER	0x0007	/* Channel 3 Counter Register		*/

#define	PORT_DMAC0_CH0_ADDRESS	0x0000	/* Channel 0 Address Register		*/
#define	PORT_DMAC0_CH0_COUNTER	0x0001	/* Channel 0 Counter Register		*/
#define	PORT_DMAC0_CH1_ADDRESS	0x0002	/* Channel 1 Address Register		*/
#define	PORT_DMAC0_CH1_COUNTER	0x0003	/* Channel 1 Counter Register		*/
#define	PORT_DMAC0_CH2_ADDRESS	0x0004	/* Channel 2 Address Register		*/
#define	PORT_DMAC0_CH2_COUNTER	0x0005	/* Channel 2 Counter Register		*/
#define	PORT_DMAC0_CH3_ADDRESS	0x0006	/* Channel 3 Address Register		*/
#define	PORT_DMAC0_CH3_COUNTER	0x0007	/* Channel 3 Counter Register		*/


/*
==================================================================================

	ISA DMAC Ports ( Generic Register )
	
	DMAC 0

==================================================================================
*/
/* DMAC0 Ports( Slave ) */
#define	PORT_DMAC0_STS		0x0008	/* Status Register ( Read )		*/
#define	PORT_DMAC0_COM		0x0008	/* Command Register ( Write )		*/
#define	PORT_DMAC0_REQ		0x0009	/* Request Register ( Write )		*/
#define	PORT_DMAC0_SINGLE_MSK	0x000A	/* Single Mask Register ( Write )	*/
#define	PORT_DMAC0_MODE		0x000B	/* Mode Register ( Write )		*/
#define	PORT_DMAC0_CLR_FF	0x000C	/* Clear Byte Pointer Flip-Flop ( Write )*/
#define	PORT_DMAC0_INTERMEDIATE	0x000D	/* Intermediate Register ( Read )	*/
#define	PORT_DMAC0_MST_CLR	0x000D	/* Master Clear ( Write )		*/
#define	PORT_DMAC0_RESET	0x000D	/* Reset (Write )			*/
#define	PORT_DMAC0_CLR_MSK	0x000E	/* Clear Mask Register ( Write )	*/
#define	PORT_DMAC0_WRITE_MSK	0x000F	/* Write Mask Register ( Write )	*/


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :8259A programmable interrupt master controller 

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	PIC_MST_COM_REG		0x0020
#define	PIC_MST_STS_REG		0x0020	/* ISR					*/
#define	PIC_MST_IMR_REG		0x0021
#define	PIC_MST_DAT_REG		0x0021

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :8253 programmable interval timer

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	PIT_REG_COUNTER0	0x0040
#define	PIT_REG_COUNTER1	0x0041
#define	PIT_REG_COUNTER2	0x0042
#define	PIT_REG_CONTROL		0x0043

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :keyboard encorder

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	KBD_ENC_READ_BUF	0x0060
#define	KBD_ENC_WRITE_BUF	0x0060

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :onboard keyboard controller

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	KBD_CTL_READ_STS	0x0064
#define	KBD_CTL_WRITE_REG	0x0064

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :8237 High Performance Programmable DMA Controllerr
				 
				 ISA DMAC Extended Page Address Registers

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	PORT_DMAC_PGADR_CH0_PC	0x0080	/* Channel 0 ( Original PC )		*/
#define	PORT_DMAC_EXTRA0	0x0080	/* Extra				*/
#define	PORT_DMAC_DIAG_PORT	0x0080	/* Diagnostic Port			*/
#define	PORT_DMAC_PGADR_CH1_ORG	0x0081	/* Channel 1 ( Original PC )		*/
#define	PORT_DMAC_PGADR_CH2	0x0081	/* Channel 2 ( AT )			*/
#define	PORT_DMAC_PGADR_CH2_ORG	0x0082	/* Channel 2 ( Original PC )		*/
#define	PORT_DMAC_PGADR_CH3	0x0082	/* Channel 3 ( AT )			*/
#define	PORT_DMAC_PGADR_CH3_ORG	0x0083	/* Channel 3 ( Original PC )		*/
#define	PORT_DMAC_PGADR_CH1	0x0083	/* Channel 1 ( AT )			*/
#define	PORT_DMAC_EXTRA1	0x0084	/* Extra				*/
#define	PORT_DMAC_EXTRA2	0x0085	/* Extra				*/
#define	PORT_DMAC_EXTRA3	0x0086	/* Extra				*/
#define	PORT_DMAC_PGADR_CH0	0x0087	/* Channel 0 ( AT )			*/
#define	PORT_DMAC_EXTRA4	0x0088	/* Extra				*/
#define	PORT_DMAC_PGADR_CH6	0x0089	/* Channel 6 ( AT )			*/
#define	PORT_DMAC_PGADR_CH7	0x008A	/* Channel 7 ( AT )			*/
#define	PORT_DMAC_PGADR_CH5	0x008B	/* Channel 5 ( AT )			*/
#define	PORT_DMAC_EXTRA5	0x008C	/* Extra				*/
#define	PORT_DMAC_EXTRA6	0x008D	/* Extra				*/
#define	PORT_DMAC_EXTRA7	0x008E	/* Extra				*/
#define	PORT_DMAC_PGADR_CH4	0x008F	/* Channel 4 ( AT )			*/
#define	PORT_DMAC_MEM_REFRESH	0x008F	/* Memory Refresh			*/
#define	PORT_DMAC_SLV_CONNECT	0x008F	/* Slave Connect			*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :8259A programmable interrupt slave controller 

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	PIC_SLV_COM_REG		0x00A0
#define	PIC_SLV_STS_REG		0x00A0
#define	PIC_SLV_IMR_REG		0x00A1
#define	PIC_SLV_DAT_REG		0x00A1

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :8237 High Performance Programmable DMA Controllerr
				 ( DMAC 1 ) 

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
/*
==================================================================================

	ISA DMAC Channel Ports ( Channel Register )
	
	DMAC 1

==================================================================================
*/
#define	PORT_DMAC_CH4_ADDRESS	0x00C0	/* Channel 4 Address Register		*/
#define	PORT_DMAC_CH4_COUNTER	0x00C2	/* Channel 4 Counter Register		*/
#define	PORT_DMAC_CH5_ADDRESS	0x00C4	/* Channel 5 Address Register		*/
#define	PORT_DMAC_CH5_COUNTER	0x00C6	/* Channel 5 Counter Register		*/
#define	PORT_DMAC_CH6_ADDRESS	0x00C8	/* Channel 6 Address Register		*/
#define	PORT_DMAC_CH6_COUNTER	0x00CA	/* Channel 6 Counter Register		*/
#define	PORT_DMAC_CH7_ADDRESS	0x00CC	/* Channel 7 Address Register		*/
#define	PORT_DMAC_CH7_COUNTER	0x00CE	/* Channel 7 Counter Register		*/

#define	PORT_DMAC1_CH0_ADDRESS	0x00C0	/* Channel 4 Address Register		*/
#define	PORT_DMAC1_CH0_COUNTER	0x00C2	/* Channel 4 Counter Register		*/
#define	PORT_DMAC1_CH1_ADDRESS	0x00C4	/* Channel 5 Address Register		*/
#define	PORT_DMAC1_CH1_COUNTER	0x00C6	/* Channel 5 Counter Register		*/
#define	PORT_DMAC1_CH2_ADDRESS	0x00C8	/* Channel 6 Address Register		*/
#define	PORT_DMAC1_CH2_COUNTER	0x00CA	/* Channel 6 Counter Register		*/
#define	PORT_DMAC1_CH3_ADDRESS	0x00CC	/* Channel 7 Address Register		*/
#define	PORT_DMAC1_CH3_COUNTER	0x00CE	/* Channel 7 Counter Register		*/

/*
==================================================================================

	ISA DMAC Ports ( Generic Register )
	
	DMAC 1

==================================================================================
*/
/* DMAC1 Ports( Master ) */
#define	PORT_DMAC1_STS		0x00D0	/* Status Register ( Read )		*/
#define	PORT_DMAC1_COM		0x00D0	/* Command Register ( Write )		*/
#define	PORT_DMAC1_REQ		0x00D2	/* Request Register ( Write )		*/
#define	PORT_DMAC1_SINGLE_MSK	0x00D4	/* Single Mask Register ( Write )	*/
#define	PORT_DMAC1_MODE		0x00D6	/* Mode Register ( Write )		*/
#define	PORT_DMAC1_CLR_FF	0x00D8	/* Clear Byte Pointer Flip-Flop ( Write )*/
#define	PORT_DMAC1_INTERMEDIATE	0x00DA	/* Intermediate Register ( Read )	*/
#define	PORT_DMAC1_MST_CLR	0x00DA	/* Master Clear ( Write )		*/
#define	PORT_DMAC1_RESET	0x00DA	/* Reset (Write )			*/
#define	PORT_DMAC1_CLR_MSK	0x00DC	/* Clear Mask Register ( Write )	*/
#define	PORT_DMAC1_WRITE_MSK	0x00DE	/* Write Mask Register ( Write )	*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :2nd Floppy Disc Controller 

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	PORT_FDC2_BASE		0x0370
#define	PORT_FDC2_SRA		0x0370	/* status register A			*/
#define	PORT_FDC2_SRB		0x0371	/* status register B			*/
#define	PORT_FDC2_DOR		0x0372	/* digital output register		*/
#define	PORT_FDC2_MSR		0x0374	/* main status register			*/
#define	PORT_FDC2_DSR		0x0374	/* datarate select register		*/
#define	PORT_FDC2_STS		0x0375	/* status registe ST0, 1, 2, 3		*/
#define	PORT_FDC2_CMD		0x0375	/* command register			*/
#define	PORT_FDC2_DATA		0x0375	/* data(status) register		*/
#define	PORT_FDC2_DATA_FIX	0x0376	/* 2nd fixed disc ctrler data register	*/
#define	PORT_FDC2_DIR		0x0377	/* Digital input register		*/
#define	PORT_FDC2_SR		0x0377	/* select regr for data transfer rate	*/
#define	PORT_FDC2_DIR		0x0377	/* digital input register		*/
#define	PORT_FDC2_CCR		0x0378	/* configuration control register	*/


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :VGA(Video Graphic Array)

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
/*
==================================================================================

	General Register

==================================================================================
*/
#define	PORT_VGA_ISR1_R0	0x03BA	/* Input Status Register 1(MOR0 = 0)	*/
#define	PORT_VGA_FCR_W0		0x03BA	/* Feature Control Register(MOR0 = 0)	*/
#define	PORT_VGA_MOR_W		0x03C2	/* Miscellaneous Output Register(Write)	*/
#define	PORT_VGA_ISR0_R		0x03C2	/* Input Status Register 0(Read Only)	*/
#define	PORT_VGA_VSER_R		0x03C3	/* Video Subsystem Enable Register(R)	*/
#define	PORT_VGA_VSER_W		0x03C3	/* Video Subsystem Enable Register(W)	*/
#define	PORT_VGA_FCR_R		0x03CA	/* Feature Control Register(Read)	*/
#define	PORT_VGA_MOR_R		0x03CC	/* Miscellaneous Output Register(Read)	*/
#define	PORT_VGA_ISR1_R1	0x03DA	/* Input Status Register 1(MOR0 = 1)	*/
#define	PORT_VGA_FCR_W1		0x03DA	/* Feature Control Register(Write)	*/

/*
==================================================================================

	Sequencer Register

==================================================================================
*/
#define	PORT_VGA_SEQ_ADDR	0x03C4	/* Address Register			*/
#define	PORT_VGA_SEQ_DATA	0x03C5	/* Data Register			*/

/*
==================================================================================

	CRT Controller Register

==================================================================================
*/
#define	PORT_VGA_CRT_ADDR0	0x03B4	/* Address Register(MOR0 = 0)		*/
#define	PORT_VGA_CRT_DATA0	0x03B5	/* Data Register(MOR0 = 0)		*/
#define	PORT_VGA_CRT_ADDR1	0x03D4	/* Address Register(MOR0 = 1)		*/
#define	PORT_VGA_CRT_DATA1	0x03D5	/* Data Register(MOR0 = 1)		*/

/*
==================================================================================

	Graphics Controller Register

==================================================================================
*/
#define	PORT_VGA_GF_ADDR	0x03CE	/* Address Register			*/
#define	PORT_VGA_GF_DATA	0x03CF	/* Data Register			*/

/*
==================================================================================

	Attribute Controller Register

==================================================================================
*/
#define	PORT_VGA_ATTR_ADDR	0x03C0	/* Address Register			*/
#define	PORT_VGA_ATTR_DAT_W	0x03C0	/* Data Registe(Write)			*/
#define	PORT_VGA_ATTR_DAT_R	0x03C1	/* Data Registe(Read)			*/


/*
==================================================================================

	Video DAC Register

==================================================================================
*/
#define	PORT_VGA_PEL_MASK	0x03C6	/* DAC PEL Mask Register(Read)		*/
#define	PORT_VGA_PALLET_ADDR_W	0x03C7	/* DAC Pallete Address(Write)		*/
#define	PORT_VGA_DAC_STATE	0x03C7	/* DAC State Register(Read)		*/
#define	PORT_VGA_PALLET_ADDR_R	0x03C8	/* DAC Pallete Address(Read)		*/
#define	PORT_VGA_PALLET_DATA	0x03C9	/* DAC Pallete Data(Read/Write)		*/


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :1st Floppy Disc Controller 

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	PORT_FDC1_BASE		0x03F0	
#define	PORT_FDC1_SRA		0x03F0	/* status register A			*/
#define	PORT_FDC1_SRB		0x03F1	/* status register B			*/
#define	PORT_FDC1_DOR		0x03F2	/* Digital output register		*/
#define	PORT_FDC1_TAPE		0x03F3	/* tapa drive register on the 82077AA	*/
#define	PORT_FDC1_MSR		0x03F4	/* main status register			*/
#define	PORT_FDC1_DRS		0x03F4	/* data rate select register		*/
#define	PORT_FDC1_STS		0x03F5	/* status register 0,1,2,3( ST0,1,2,3 )	*/
#define	PORT_FDC1_DATA		0x03F5	/* data( status ) register		*/
#define	PORT_FDC1_CMD		0x03F5	/* command register			*/
#define	PORT_FDC1_RESERVED	0x03F6	/* reserved				*/
#define	PORT_FDC1_DATA_FIX	0x03F7	/* FIXED disc controller data register	*/
#define	PORT_FDC1_DIR		0x03F7	/* digital input register		*/
#define	PORT_FDC1_CCR		0x03F8	/* configuration control register	*/

#define PORT_FDC_SRA		0x0000	/* relative address			*/
#define	PORT_FDC_SRB		0x0001	/* relative address			*/
#define	PORT_FDC_DOR		0x0002	/* relative address			*/
#define	PORT_FDC_MSR		0x0004
#define	PORT_FDC_STS		0x0005
#define	PORT_FDC_CMD		0x0005
#define	PORT_FDC_DATA		0x0005
#define	PORT_FDC_DIR		0x0007
#define	PORT_FDC_CCR		0x0008

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

	Description :PCI Bus

_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	PORT_PCI_CONFIG_ADDR	0x0CF8
#define	PORT_PCI_CONFIG_DATA	0x0CFC

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

#endif	// __FORMAT_H__
