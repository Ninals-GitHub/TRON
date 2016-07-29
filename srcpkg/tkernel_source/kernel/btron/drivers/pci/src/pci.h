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

#ifndef	__BK_DRIVERS_PCI_H__
#define	__BK_DRIVERS_PCI_H__

#include <stdint.h>

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
----------------------------------------------------------------------------------
	CONFIG_ADDRESS
----------------------------------------------------------------------------------
*/
#define	REGISTER_NUMBER		0x000000FC
#define	FUNCTION_NUMBER		0x00000700
#define	DEVICE_NUMBER		0x0000F800
#define	BUS_NUMBER		0x00FF0000
#define	PCI_ENABLE		0x80000000

#define	FUNC_NUM_SHIFT		8
#define	DEV_NUM_SHIFT		11
#define	BUS_NUM_SHIFT		16

#define	MAX_REG_NUM		REGISTER_NUMBER
#define	MAX_FUNC_NUM		((FUNCTION_NUMBER >> FUNC_NUM_SHIFT) + 1)
#define	MAX_DEV_NUM		((DEVICE_NUMBER >> DEV_NUM_SHIFT) + 1)
#define	MAX_BUS_NUM		((BUS_NUMBER >> BUS_NUM_SHIFT) + 1)

#define	CONFIG_DATA1		0x00
#define	CONFIG_DATA2		0x04
#define	CONFIG_DATA3		0x08
#define	CONFIG_DATA4		0x0C
#define	CONFIG_DATA5		0x10
#define	CONFIG_DATA6		0x14
#define	CONFIG_DATA7		0x18
#define	CONFIG_DATA8		0x1C
#define	CONFIG_DATA9		0x20
#define	CONFIG_DATA10		0x24
#define	CONFIG_DATA11		0x28
#define	CONFIG_DATA12		0x2C
#define	CONFIG_DATA13		0x30
#define	CONFIG_DATA14		0x34
#define	CONFIG_DATA15		0x38
#define	CONFIG_DATA16		0x3C

#define	MAX_CONFIG_DATA_SIZE	0xFF
/*
----------------------------------------------------------------------------------
	type0 header
----------------------------------------------------------------------------------
*/
#define	TYPE0_BASE_ADDR_REG	6

struct type0_header {
	uint16_t	vendor_id;
	uint16_t	device_id;
	uint16_t	command;
	uint16_t	status;
	uint32_t	class_code;	// revision id at least significant byte
	uint8_t		cache_line_size;
	uint8_t		master_latency_timer;
	uint8_t		header_type;
	uint8_t		bist;
	uint32_t	base_address_register[TYPE0_BASE_ADDR_REG];
	uint32_t	cardbus_cis_pointer;
	uint16_t	subsystem_vendoer_id;
	uint16_t	subsystem_id;
	uint32_t	exrom_base_address;
	uint8_t		capability_pointer;
	uint8_t		reserved1[3];
	uint32_t	reserved2;
	uint8_t		interrupt_line;
	uint8_t		interrupt_pin;
	uint8_t		min_gnt;
	uint8_t		max_lat;
};

#define	MASK_VENDOR_ID			0x0000FFFF
#define	MASK_DEVICE_ID			0xFFFF0000
#define	SHIFT_DEVICE_ID			16
#define	MASK_COMMAND			0x0000FFFF
#define	MASK_STAUS			0xFFFF0000
#define	SHIFT_STATUS			16
#define	MASK_REVISION_ID		0x000000FF
#define	MASK_CLASS_CODE			0xFFFFFF00
#define	MASK_BASE_CLASS			0xFF000000
#define	MASK_SUB_CLASS			0x00FF0000
#define	MASK_DEV_IF			0x0000FF00
#define	MASK_CACHE_LINE_SIZE		0x000000FF
#define	MASK_LATENCY_TIMER		0x0000FF00
#define	SHIFT_LATENCY_TIMER		8
#define	MASK_HEADER_TYPE		0x00FF0000
#define	SHIFT_HEADER_TYPE		16
#define	MASK_BIST			0xFF000000
#define	SHIFT_BIST			24
#define	MASK_SUBSYSTEM_VENDOR_ID	0x0000FFFF
#define	MASK_SUBSYSTEM_ID		0xFFFF0000
#define	SHIFT_SUBSYSTEM_ID		16
#define	MASK_CAP_POINTER		0x000000FF
#define	MASK_INTERRUPT_LINE		0x000000FF
#define	MASK_INTERRUPT_PIN		0x0000FF00
#define	SHIFT_INTERRUPT_PIN		8
#define	MASK_MIN_GNT			0x00FF0000
#define	SHIFT_MIN_GNT			16
#define	MASK_MIN_LAT			0xFF000000
#define	SHIFT_MIN_LAT			24

#define	VENDOR_ID(id)			(id & MASK_VENDOR_ID)
#define	DEVICE_ID(id)			((id & MASK_DEVICE_ID) >> SHIFT_DEVICE_ID)
#define	COMMAND(data)			(data & MASK_COMMAND)
#define	STATUS(data)			((data & MASK_STAUS) >> SHIFT_STATUS)
#define	REVISION_ID(id)			(id & MASK_REVISION_ID)
#define	CLASS_CODE(code)		((code & MASK_CLASS_CODE) >> 8)
#define	BASE_CLASS(code)		((code & MASK_BASE_CLASS) >> 24)
#define	SUB_CLASS(code)			((code & MASK_SUB_CLASS) >> 16)
#define	DEV_IF(code)			((code & MASK_DEV_IF) >> 8)


#define	CACHE_LINE_SIZE(data)		(data & MASK_CACHE_LINE_SIZE)
#define	LATENCY_TIMER(data)		((data & MASK_LATENCY_TIMER) >> SHIFT_LATENCY_TIMER)
#define	GET_HEADER_TYPE(header)		((header & MASK_HEADER_TYPE) >> SHIFT_HEADER_TYPE)
#define	BIST(data)			((data & MASK_BIST) >> SHIFT_BIST)
#define	SUBSYSTEM_VENDOR_ID(data)	(data & MASK_SUBSYSTEM_VENDOR_ID)
#define	SUBSYSTEM_ID(data)		((data & MASK_SUBSYSTEM_ID) >> SHIFT_SUBSYSTEM_ID)
#define	CAP_POINTER(data)		(data & MASK_CAP_POINTER)
#define	INTERRUPT_LINE(data)		(data & MASK_INTERRUPT_LINE)
#define	INTERRUPT_PIN(data)		((data & MASK_INTERRUPT_PIN) >> SHIFT_INTERRUPT_PIN)
#define	MIN_GNT(data)			((data & MASK_MIN_GNT) >> SHIFT_MIN_GNT)
#define	MIN_LAT(data)			((data & MASK_MIN_LAT) >> SHIFT_MIN_LAT)

/* vendor id									*/
#define	VENDOR_INVALID			0xFFFF
/* device id									*/

/* command									*/
#define	IO_SPACE			0x0001
#define	MEMORY_SPACE			0x0002
#define	BUS_MASTER_ENABLE		0x0004		// RW
#define	SPECIAL_CYCLE_ENABLE		0x0008		// RO
#define	MEMORY_WRITE_INVALIDATE		0x0010		// RO
#define	VGA_PALETTE_SNOOP		0x0020		// RO
#define	PARITY_ERROR_RESPONSE		0x0040		// RW
#define	IDSEL_STEPPING			0x0080		// RO must be 0
#define	WAIT_CYCLE_CONTROL		IDSEL_STEPPING	// RO must be 0
#define	COM_SERR_ENABLE			0x0100		// RW
#define	FAST_B2B_TRANSACTION_EN		0x0200		// RO
#define	INTERRUPT_DISABLE		0x0400		// RW

/* status									*/

#define	INTERRUPT_STATUS		0x0008		// RO
#define	CAPABILITIES_LIST		0x0010		// RO
#define	CAPABLE_66MHZ			0x0020		// RO
#define	FAST_B2B_TRANSACTION_CAP	0x0080		// RO
#define	MASTER_DATA_PARITY_ERROR	0x0100		// RW1C
#define	DEVSEL_TIMING			0x0600		// RO
#define	DEVSEL_FAST			0x0000
#define	DEVSEL_MEDIUM			0x0200
#define	DEVSEL_SLOW			0x0400
#define	SIGNALED_TARGET_ABORT		0x0800		// RW1C
#define	RECEIVED_TARGET_ABORT		0x1000		// RW1C
#define	RECEIVED_MASTER_ABORT		0x2000		// RW1C
#define	SIGNALED_SYSTEM_ERROR		0x4000		// RW1C
#define	DETECTED_PARITY_ERROR		0x8000		// RW1C

/* class code									*/

/* cache line size								*/
/* master latency timer								*/
/* header type									*/
#define	MULTIPLE_FUNCTION		0x80
#define	HEADER_TYPE(type)		(type & ~MULTIPLE_FUNCTION)
#define	HEADER_TYPE0			0x00
#define	HEADER_TYPE1			0x01		// for pci-to-pci bridges
#define	HEADER_TYPE2			0x02		// for card bus bridges
/* bist - built-in self test							*/
#define	COMPLETION_CODE			0x0F
#define	COMPLETION_PASSED_TEST		0x00
#define	START_BIST			0x40
#define	BIST_CAPABLE			0x80
/* interrupt line								*/
/* interrupt pin								*/
#define	INTA				0x01
#define	INTB				0x02
#define	INTC				0x03
#define	INTD				0x04
/* base address									*/
#define	MEMORY_MAPPED_IO		0x00000000
#define	IO_SPACE_BIT			0x00000001
#define	BASE_ADDR_32_BIT		0x00000002
#define	BASE_ADDR_64_BIT		0x00000004
#define	PREFETCHABLE			0x00000008
#define	MASK_BASE_ADDRESS		0xFFFFFFF0
#define	REQUEST_MEM_SIZE		0xFFFFFFFF
/* expansion rom base addresss							*/
#define	EXPANSION_ROM_ENABLE		0x00000001
#define	MASK_ROM_BASE_ADDRESS		0xFFFFF800

/*
----------------------------------------------------------------------------------
	type1 header
----------------------------------------------------------------------------------
*/
#define	TYPE1_BASE_ADDR_REG	2

struct type1_header {
	uint16_t	vendor_id;
	uint16_t	device_id;
	uint16_t	command;
	uint16_t	status;
	uint32_t	class_code;	// revision id at least significant byte
	uint8_t		cache_line_size;
	uint8_t		master_latency_timer;
	uint8_t		header_type;
	uint8_t		bist;
	uint32_t	base_address_register[TYPE1_BASE_ADDR_REG];
	uint8_t		primary_bus_number;
	uint8_t		secondary_bus_number;
	uint8_t		subordinate_bus_number;
	uint8_t		secondary_latency_timer;
	uint8_t		io_base;
	uint8_t		io_limit;
	uint16_t	secondary_status;
	uint16_t	memory_base;
	uint16_t	memory_limit;
	uint16_t	prefetchable_memory_base;
	uint16_t	prefetchable_memory_limit;
	uint32_t	prefetchable_base_upper;
	uint32_t	prefetchable_limit_upper;
	uint16_t	io_base_upper;
	uint16_t	io_limit_upper;
	uint8_t		capability_pointer;
	uint8_t		reserved1[3];
	uint32_t	exrom_base_address;
	uint8_t		interrupt_line;
	uint8_t		interrupt_pin;
	uint16_t	bridge_control;
};


#define	MASK_PRIMARY_BUS_NUMBER		0x000000FF
#define	MASK_SECONDARY_BUS_NUMBER	0x0000FF00
#define	SHIFT_SECONDARY_BUS_NUMBER	8
#define	MASK_SUBORDINATE_BUS_NUMBER	0x00FF0000
#define	SHIFT_SUBORDINATE_BUS_NUMBER	16
#define	MASK_SECONDARY_LATENCY_TIMER	0xFF000000
#define	SHIFT_SECONDARY_LATENCY_TIMER	24
#define	MASK_IO_BASE			0x000000FF
#define	MASK_IO_LIMIT			0x0000FF00
#define	SHIFT_IO_LIMIT			8
#define	MASK_SECONDARY_STATUS		0xFFFF0000
#define	SHIFT_SECONDARY_STATUS		16
#define	MASK_PREF_MEMORY_BASE		0x0000FFFF
#define	MASK_PREF_MEMORY_LIMIT		0xFFFF0000
#define	SHIFT_PREF_MEMORY_LIMIT		16
#define	MASK_IO_BASE_UPPER		0x0000FFFF
#define	MASK_IO_LIMIT_UPPER		0xFFFF0000
#define	SHIFT_IO_LIMIT_UPPER		16
#define	MASK_BRIDGE_CONTROL		0xFFFF0000
#define	SHIFT_BRIDGE_CONTROL		16

#define	PRIMARY_LATENCY_TIMER(data)	LATENCY_TIMER(data)
#define	PRIMARY_BUS_NUMBER(data)	(data & MASK_PRIMARY_BUS_NUMBER)
#define	SECONDARY_BUS_NUMBER(data)	((data & MASK_SECONDARY_BUS_NUMBER) >> SHIFT_SECONDARY_BUS_NUMBER)
#define	SUBORDINATE_BUS_NUMBER(data)	((data & MASK_SUBORDINATE_BUS_NUMBER) >> SHIFT_SUBORDINATE_BUS_NUMBER)
#define	SECONDARY_LATENCY_TIMER(data)	((data & MASK_SECONDARY_LATENCY_TIMER) >> SHIFT_SECONDARY_LATENCY_TIMER)
#define	IO_BASE(data)			(data & MASK_IO_BASE)
#define	IO_LIMIT(data)			((data & MASK_IO_LIMIT) >> SHIFT_IO_LIMIT)
#define	SECONDARY_STATUS(data)		((data & MASK_SECONDARY_STATUS) >> SHIFT_SECONDARY_STATUS)
#define	PREF_MEMORY_BASE(data)		(data & MASK_PREF_MEMORY_BASE)
#define	PREF_MEMORY_LIMIT(data)		((data & MASK_PREF_MEMORY_LIMIT) >> SHIFT_PREF_MEMORY_LIMIT)
#define	IO_BASE_UPPER(data)		(data & MASK_IO_BASE_UPPER)
#define	IO_LIMIT_UPPER(data)		((data & MASK_IO_LIMIT_UPPER) >> SHIFT_IO_LIMIT_UPPER)
#define	BRIDGE_CONTROL(data)		((data & MASK_BRIDGE_CONTROL) >> SHIFT_BRIDGE_CONTROL)


/* secondary status								*/
// -> same as status register in type0 defined

/* bridege control								*/
#define	PARITY_ERROR_RESPONSE_ENABLE	0x0001		// RW
#define	SERR_ENABLE			0x0002		// RW
#define	MASTER_ABORT_MODE		0x0020		// RO
#define	SECONDARY_BUS_RESET		0x0040		// RW
#define	FAST_B2B_TRANSACTION_ENABLE	0x0080		// RO
#define	PRIMARY_DISCARD_TIMER		0x0100		// RO
#define	SECONDARY_DISCARD_TIMER		0x0200		// RO
#define	DISCARD_TIMER_STATUS		0x0400		// RO
#define	DISCARD_TIMER_SERR_ENABLE	0x0800		// RO

/*
----------------------------------------------------------------------------------
	class code
----------------------------------------------------------------------------------
*/
/* base class									*/
#define	CLASS_BEFORE_STD		0x00
#define	CLASS_MASS_STORAGE		0x01
#define	CLASS_NETWORK			0x02
#define	CLASS_DISPLAY			0x03
#define	CLASS_MULTIMEDIA		0x04
#define	CLASS_MEMORY			0x05	
#define	CLASS_BRIDGE			0x06
#define	CLASS_COMMUNICATION		0x07
#define	CLASS_PERIPHERALS		0x08
#define	CLASS_INPUT			0x09
#define	CLASS_DOCKING_STATION		0x0A
#define	CLASS_PROCESSOR			0x0B
#define	CLASS_SERIAL_BUS		0x0C
#define	CLASS_WIRELESS			0x0D
#define	CLASS_INTELLIGENCE_IO		0x0E
#define	CLASS_SATELLITE_COM		0x0F
#define	CLASS_ENC_DEC			0x10
#define	CLASS_NOT_FIT			0xFF

#if 0
#define	MAKE_CLASS_CODE(base, sub, ifa)						\
			((base & 0xFF) << 16 | (sub & 0xFF) << 8 | ifa & 0xFF)


#define	CLASS_VGA		MAKE_CLASS_CODE(CLASS_BEFORE_STD, 0x01, 0x00)
#define	CLASS_SCSI_BUS		MAKE_CLASS_CODE(CLASS_MASS_STORAGE, 0x00, 0x00)
#define	CLASS_IDE		MAKE_CLASS_CODE(CLASS_MASS_STORAGE, 0x01, 0x00)
#endif

/*
----------------------------------------------------------------------------------
	msi capability
----------------------------------------------------------------------------------
*/
struct msi_capability {
	uint8_t		capability_id;
	uint8_t		next_pointer;
	uint16_t	message_control;
	uint32_t	message_address;
	uint16_t	message_data;
};

struct msi_capability_64 {
	uint8_t		capability_id;
	uint8_t		next_pointer;
	uint16_t	message_control;
	uint32_t	message_address;
	uint32_t	message_upper_address;
	uint16_t	message_data;
};

struct msi_capability_per_vector_masking {
	uint8_t		capability_id;
	uint8_t		next_pointer;
	uint16_t	message_control;
	uint32_t	message_address;
	uint16_t	message_data;
	uint16_t	reserved;
	uint32_t	mask_bits;
	uint32_t	pendign_bits;
};

struct msi_capability_64_per_vector_masking {
	uint8_t		capability_id;
	uint8_t		next_pointer;
	uint16_t	message_control;
	uint32_t	message_address;
	uint32_t	message_upper_address;
	uint16_t	message_data;
	uint16_t	reserved;
	uint32_t	mask_bits;
	uint32_t	pending_bits;
};

/* message control								*/
#define	MSI_ENABLE			0x0001
#define	MULTI_MSG_CAPABLE		0x000E
#define	MULTI_MSG_CAPABLE_1		0x0000
#define	MULTI_MSG_CAPABLE_2		0x0002
#define	MULTI_MSG_CAPABLE_4		0x0004
#define	MULTI_MSG_CAPABLE_8		0x0006
#define	MULTI_MSG_CAPABLE_16		0x0008
#define	MULTI_MSG_CAPABLE_32		0x000A
#define	MULTI_MSG_ENABLE		0x0070
#define	MULTI_MSG_ENABLE_1		0x0000
#define	MULTI_MSG_ENABLE_2		0x0010
#define	MULTI_MSG_ENABLE_4		0x0020
#define	MULTI_MSG_ENABLE_8		0x0030
#define	MULTI_MSG_ENABLE_16		0x0040
#define	MULTI_MSG_ENABLE_32		0x0050
#define	ADDRESS_64_BIT_CAP		0x0080
#define	PER_VERCTOR_MASKING_CAP		0x0100
/* message address								*/
#define	MASK_MESSAGE_ADDRESS		0xFFFFFFFC
/* message upper address							*/
// nomask
/* message data 								*/
// nomask
/* mask bits									*/
// nomask
/* pending bits									*/
// nomask 

/*
----------------------------------------------------------------------------------
	msix capability
----------------------------------------------------------------------------------
*/
struct msix_capability {
	uint8_t		capability_id;
	uint8_t		next_pointer;
	uint16_t	message_control;
	uint32_t	table_offset;		// include table bir
	uint32_t	pba_offset;		// include pba bir
};

struct msix_table_entry {
	uint32_t	msg_addr;
	uint32_t	msg_upper_addr;
	uint32_t	msg_data;
	uint32_t	vector_control;
};

struct msix_pba_entry {
	uint64_t	pending_bits;
};

/* message control								*/
#define	MSIX_TABLE_SIZE			0x07FF
#define	MSIX_RESERVED			0x3800
#define	MSIX_FUNCTION_MASK		0x4000
#define	MSIX_ENABLE			0x8000

/* table offset									*/
#define	MSIX_TABLE_BIR			0x00000007
#define	MSIX_BASE_ADDRESS_REG(bir)	(bir * 0x4 + 0x10)
#define	MSIX_TABLE_OFFSET		0xFFFFFFF8
/* pba offset									*/
#define	MSIX_PBA_BIR			0x00000007
#define	MSIX_PBA_OFFSET			0xFFFFFFF8
/* message address								*/
#define	MSIX_BASE_ADDRESS		0xFFFFFFFC
#define	MSIX_MESSAGE_ADDRESS		0x00000003
/* message upper address							*/
// nomask
/* message data									*/
// nomask
/* vector control								*/
#define	MSIX_MASK_BIT			0x00000001
/* pending bits									*/
// nomask

/*
----------------------------------------------------------------------------------
	pci express capability
----------------------------------------------------------------------------------
*/
struct pci_exp_capability {
	uint8_t		pci_exp_cap_id;
	uint8_t		next_cap_pointer;
	uint16_t	pci_exp_cap_register;
	uint32_t	device_capability;
	uint16_t	device_control;
	uint16_t	device_status;
	uint32_t	link_capability;
	uint16_t	link_control;
	uint16_t	link_status;
	uint32_t	slot_capability;
	uint16_t	slot_control;
	uint16_t	slot_status;
	uint16_t	root_control;
	uint16_t	root_capability;
	uint32_t	root_status;
	uint32_t	device_capability2;
	uint16_t	device_control2;
	uint16_t	device_status2;
	uint32_t	link_capability2;
	uint16_t	link_control2;
	uint16_t	link_status2;
	uint32_t	slot_capability2;
	uint16_t	slot_control2;
	uint16_t	slot_status2;
};

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

#endif	// __BK_DRIVERS_PCI_H__
