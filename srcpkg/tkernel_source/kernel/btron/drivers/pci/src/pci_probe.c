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


#include <bk/kernel.h>
#include <bk/memory/slab.h>
#include <tstdlib/list.h>

#include <bk/drivers/bus_type.h>
#include <bk/drivers/pci/pci_bus.h>
#include <cpu.h>
#include <device/port.h>

#include "pci.h"


/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct pci_bus;
struct pci_device;
struct bus_number;

LOCAL ALWAYS_INLINE struct pci_bus* pci_bus_alloc(void);
LOCAL ALWAYS_INLINE struct pci_device* pci_device_alloc(void);
LOCAL ALWAYS_INLINE
uint32_t read_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg);
LOCAL ALWAYS_INLINE void write_config(uint8_t bus, uint8_t
					device, uint8_t func, uint8_t
					reg, uint32_t data);
LOCAL ALWAYS_INLINE
uint16_t read_vendor_id(uint8_t bus, uint8_t device, uint8_t func);
LOCAL ALWAYS_INLINE
uint32_t read_class_code(uint8_t bus, uint8_t device, uint8_t func);
LOCAL ALWAYS_INLINE
uint8_t read_header_type(uint8_t bus, uint8_t device, uint8_t func);
LOCAL ALWAYS_INLINE
uint8_t read_secondary_bus(uint8_t bus, uint8_t device, uint8_t func);
LOCAL ALWAYS_INLINE int is_invalid(uint32_t header);
LOCAL ALWAYS_INLINE int is_type1_header(uint8_t header_type);
LOCAL void* read_header(uint8_t bus, uint8_t device, uint8_t func);
LOCAL int
scan_device(uint8_t bus, uint8_t device, uint8_t func, struct bus_number *bus_num);
LOCAL int
scan_bus(uint8_t bus, uint8_t device, uint8_t func, struct bus_number *bus_num);
LOCAL struct pci_bus* insert_pci_bus(uint8_t bus, uint8_t device, uint8_t func,
						struct bus_number *bus_num);
LOCAL int insert_pci_device(uint8_t bus, uint8_t device, uint8_t func,
						struct bus_number *bus_num);
LOCAL int probe_pci(void);
LOCAL void show_pci(struct pci_bus *pci_bus);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
struct base_address {
	unsigned long	start;
	unsigned long	size;
};

struct class_code {
	uint8_t		base_class;
	uint8_t		sub_class;
	uint8_t		device_if;
};

struct pci_bus {
	uint8_t			bus_number;
	uint8_t			device_number;
	uint8_t			func_number;
	struct class_code	class;
	uint8_t			interrupt_line;
	uint8_t			interrupt_pin;
	
	struct base_address	io[TYPE0_BASE_ADDR_REG];
	
	struct pci_bus		*parent;
	struct list		list_devices;		// list of devices on the bus
	struct list		list_buses;		// list of buses on the bus
	struct list		node_bus;
	int			has_cap_list:1;
	int			has_pci_express:1;
};

struct pci_device {
	uint8_t			bus_number;
	uint8_t			device_number;
	uint8_t			func_number;
	struct class_code	class;
	uint8_t			interrupt_line;
	uint8_t			interrupt_pin;
	
	struct base_address	io[TYPE1_BASE_ADDR_REG];
	
	struct list		node_device;		// node of the list_device
	int			has_cap_list:1;
	int			has_pci_express:1;
};

struct bus_number {
	uint8_t			primary;
	uint8_t			secondary;
	uint8_t			bus_number;
	uint8_t			depth;
	uint8_t			max_depth;
	struct pci_bus		*bus;
};

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct bus_type pci_bus_type = {
	.name = "pci bus",
};

LOCAL struct pci_bus *root_pci;		// root_pci is a dummy list node

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_pci_device
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize pci device
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_pci_device(void)
{
	root_pci = pci_bus_alloc();
	
	if (UNLIKELY(!root_pci)) {
		return(-ENOMEM);
	}
	
	return(probe_pci());
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:pci_bus_alloc
 Input		:void
 Output		:void
 Return		:struct pci_bus*
 		 < allocated memory address >
 Description	:allocate a memory for a pci_bus structure
==================================================================================
*/
LOCAL ALWAYS_INLINE struct pci_bus* pci_bus_alloc(void)
{
	struct pci_bus *bus;
	
	bus = (struct pci_bus*)kcalloc(1, sizeof(struct pci_bus), 0);
	
	init_list(&bus->list_devices);
	init_list(&bus->list_buses);
	init_list(&bus->node_bus);
	
	return(bus);
}

/*
==================================================================================
 Funtion	:pci_device_alloc
 Input		:void
 Output		:void
 Return		:struct pci_device*
 		 < allocated memory address >
 Description	:allocate a memory for a pci_device structure
==================================================================================
*/
LOCAL ALWAYS_INLINE struct pci_device* pci_device_alloc(void)
{
	struct pci_device *device;
	
	device = (struct pci_device*)kcalloc(1, sizeof(struct pci_device), 0);
	
	init_list(&device->node_device);
	
	return(device);
}

/*
==================================================================================
 Funtion	:read_config
 Input		:uint8_t bus
 		 < bus number >
 		 uint8_t device
 		 < device number >
 		 uint8_t func
 		 < function number >
 		 uint8_t reg
 		 < register number >
 Output		:void
 Return		:uint32_t
 		 < config data >
 Description	:write CONFIG_ADDRESS and read CONFIG_DATA
==================================================================================
*/
LOCAL ALWAYS_INLINE
uint32_t read_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg)
{
	uint32_t config;
	
	config = (uint32_t)reg & REGISTER_NUMBER;
	config |= (uint32_t)(func << FUNC_NUM_SHIFT) & FUNCTION_NUMBER;
	config |= (uint32_t)(device << DEV_NUM_SHIFT) & DEVICE_NUMBER;
	config |= (uint32_t)(bus << BUS_NUM_SHIFT) & BUS_NUMBER;
	config |= PCI_ENABLE;
	
	out_w(PORT_PCI_CONFIG_ADDR, config);
	
	return(in_w(PORT_PCI_CONFIG_DATA));
}

/*
==================================================================================
 Funtion	:write_config
 Input		:uint8_t bus
 		 < bus number >
 		 uint8_t device
 		 < device number >
 		 uint8_t func
 		 < function number >
 		 uint8_t reg
 		 < register number >
 		 uint32_t data
 		 < data to write >
 Output		:void
 Return		:void
 Description	:write CONFIG_ADDRESS and write CONFIG_DATA
==================================================================================
*/
LOCAL ALWAYS_INLINE void write_config(uint8_t bus, uint8_t
					device, uint8_t func, uint8_t
					reg, uint32_t data)
{
	uint32_t config;
	
	config = (uint32_t)reg & REGISTER_NUMBER;
	config |= (uint32_t)(func << FUNC_NUM_SHIFT) & FUNCTION_NUMBER;
	config |= (uint32_t)(device << DEV_NUM_SHIFT) & DEVICE_NUMBER;
	config |= (uint32_t)(bus << BUS_NUM_SHIFT) & BUS_NUMBER;
	config |= PCI_ENABLE;
	
	out_w(PORT_PCI_CONFIG_ADDR, config);
	
	out_w(PORT_PCI_CONFIG_DATA, data);
}

/*
==================================================================================
 Funtion	:read_vendor_id
 Input		:uint8_t bus
 		 < bus number >
 		 uint8_t device
 		 < device number >
 		 uint8_t func
 		 < function number >
 Output		:void
 Return		:uint16_t
 		 < vendor id >
 Description	:read vendor id in configuration space header
==================================================================================
*/
LOCAL ALWAYS_INLINE
uint16_t read_vendor_id(uint8_t bus, uint8_t device, uint8_t func)
{
	uint32_t header;
	
	header = read_config(bus, device, func, CONFIG_DATA1);
	
	return((uint16_t)(VENDOR_ID(header)));
}

/*
==================================================================================
 Funtion	:read_class_code
 Input		:uint8_t bus
 		 < bus number >
 		 uint8_t device
 		 < device number >
 		 uint8_t func
 		 < function number >
 Output		:void
 Return		:uint32_t
 		 < class code >
 Description	:read class code in configuration space header
==================================================================================
*/
LOCAL ALWAYS_INLINE
uint32_t read_class_code(uint8_t bus, uint8_t device, uint8_t func)
{
	uint32_t header;
	
	header = read_config(bus, device, func, CONFIG_DATA3);
	
	return(CLASS_CODE(header));
}

/*
==================================================================================
 Funtion	:read_header_type
 Input		:uint8_t bus
 		 < bus number >
 		 uint8_t device
 		 < device number >
 		 uint8_t func
 		 < function number >
 Output		:void
 Return		:uint8_t
 		 < header type >
 Description	:read header type in configuration space header
==================================================================================
*/
LOCAL ALWAYS_INLINE
uint8_t read_header_type(uint8_t bus, uint8_t device, uint8_t func)
{
	uint32_t header;
	
	header = read_config(bus, device, func, CONFIG_DATA4);
	
	return((uint8_t)GET_HEADER_TYPE(header));
}

/*
==================================================================================
 Funtion	:read_secondary_bus
 Input		:uint8_t bus
 		 < bus number >
 		 uint8_t device
 		 < device number >
 		 uint8_t func
 		 < function number >
 Output		:void
 Return		:uint8_t
 		 < secondary bus number >
 Description	:read secondary bus number
==================================================================================
*/
LOCAL ALWAYS_INLINE
uint8_t read_secondary_bus(uint8_t bus, uint8_t device, uint8_t func)
{
	uint32_t header;
	
	header = read_config(bus, device, func, CONFIG_DATA7);
	
	return((uint8_t)SECONDARY_BUS_NUMBER(header));
}

/*
==================================================================================
 Funtion	:is_invalid
 Input		:uint32_t header
 		 < header which includes vendor id >
 Output		:void
 Return		:int
 		 < boolean result >
 Description	:check whether invalid device or not
==================================================================================
*/
LOCAL ALWAYS_INLINE int is_invalid(uint32_t header)
{
	return((header & 0xFFFF) == VENDOR_INVALID);
}

/*
==================================================================================
 Funtion	:is_type1_header
 Input		:uint8_t header_type
 		 < header which includes type field >
 Output		:void
 Return		:int
 		 < boolean result >
 Description	:check whether typer 0 or not
==================================================================================
*/
LOCAL ALWAYS_INLINE int is_type1_header(uint8_t header_type)
{
	return(!!(header_type & MULTIPLE_FUNCTION));
}

/*
==================================================================================
 Funtion	:read_header
 Input		:uint8_t bus
 		 < bus number >
 		 uint8_t device
 		 < device number >
 		 uint8_t func
 		 < function number >
 Output		:void
 Return		:void*
 		 < read heaer address >
 Description	:read a header
==================================================================================
*/
LOCAL void* read_header(uint8_t bus, uint8_t device, uint8_t func)
{
	uint8_t *header;
	uint32_t *hdata;
	uint32_t data;
	uint32_t i;
	size_t size;
	
	data = read_config(bus, device, func, CONFIG_DATA14);
	
	if (CAP_POINTER(data)) {
		size = MAX_CONFIG_DATA_SIZE + 1;
	} else {
		if (is_type1_header(read_header_type(bus, device, func))) {
			size = sizeof(struct type1_header);
		} else {
			size = sizeof(struct type0_header);
		}
	}
	
	header = (uint8_t*)kmalloc(size, 0);
	
	if (UNLIKELY(!header)) {
		return(NULL);
	}
	
	hdata = (uint32_t*)header;
	
	for (i = 0;i <= CONFIG_DATA16;i += 0x04) {
		*(hdata++) = read_config(bus, device, func, i);
	}
	
	if (CAP_POINTER(data)) {
		for (i = CONFIG_DATA16 + 0x04;i < MAX_CONFIG_DATA_SIZE;i += 0x04) {
			*(hdata++) = read_config(bus, device, func, i);
		}
	}
	
	return(header);
}

/*
==================================================================================
 Funtion	:scan_device
 Input		:uint8_t bus
 		 < bus number >
 		 uint8_t device
 		 < device number >
 		 struct bus_number *bus_num
 		 < detected bus number >
 Output		:void
 Return		:int
 		 < boolean result >
 Description	:scan the devices on the bus
==================================================================================
*/
LOCAL int
scan_device(uint8_t bus, uint8_t device, uint8_t func, struct bus_number *bus_num)
{
	uint8_t k;
	int result;
	
	for (k = func;k < MAX_FUNC_NUM;k++) {
		uint32_t class;
		if (is_invalid(read_vendor_id(bus, device, k))) {
			break;
		}
		
		class = read_class_code(bus, device, k);
		
		if (is_type1_header(read_header_type(bus, device, k))) {
			uint32_t data;
			struct pci_bus *new_bus;
			
			if (k == 0) {
				bus_num->bus_number++;
				bus_num->secondary = bus_num->bus_number;
			}
			
			data = read_config(bus, device, k, CONFIG_DATA7);
			data = (data & ~MASK_PRIMARY_BUS_NUMBER) | bus_num->primary;
			data = (data & ~MASK_SECONDARY_BUS_NUMBER) |
					(bus_num->secondary << SHIFT_SECONDARY_BUS_NUMBER);
			
			write_config(bus, device, k, CONFIG_DATA7, data);
			
			new_bus = insert_pci_bus(bus, device, k, bus_num);
			
			if (UNLIKELY(!new_bus)) {
				return(-ENOMEM);
			}
			
			bus_num->bus = new_bus;
			
			result = scan_bus(bus_num->secondary, 0, 0, bus_num);
			
			if (UNLIKELY(result)) {
				return(result);
			}
			
			if (bus_num->bus->parent) {
				bus_num->bus = bus_num->bus->parent;
			}
			
			data = read_config(bus, device, k, CONFIG_DATA7);
			data = (data & ~MASK_SUBORDINATE_BUS_NUMBER) |
					(bus_num->bus_number << SHIFT_SUBORDINATE_BUS_NUMBER);
			
			write_config(bus, device, k, CONFIG_DATA7, data);
			
			//printf("<bus:%d device:%d func:%d class:0x%08X bus:0x%08X>\n", bus, device, k, class,read_config(bus, device, k, CONFIG_DATA7));
			
		} else {
			
			//printf("--bus:%d device:%d func:%d class:0x%08X\n", bus, device, k, read_class_code(bus, device, k));
			result = insert_pci_device(bus, device, k, bus_num);
			
			if (UNLIKELY(result)) {
				return(result);
			}
		}
	}
	
	return(0);
}

/*
==================================================================================
 Funtion	:scan_bus
 Input		:uint8_t bus
 		 < bus number >
 		 uint8_t device
 		 < device number >
 		 struct bus_number *bus_num
 		 < detected bus number >
 Output		:void
 Return		:int
 		 < boolean result >
 Description	:scan the buses exist
==================================================================================
*/
LOCAL int
scan_bus(uint8_t bus, uint8_t device, uint8_t func, struct bus_number *bus_num)
{
	uint8_t j;
	int result;
	uint8_t primary_back = bus_num->primary;
	
	bus_num->primary = bus;
	bus_num->depth++;
	
	if (bus_num->max_depth < bus_num->depth) {
		bus_num->max_depth = bus_num->depth;
	}
	
	for (j = device;j < MAX_DEV_NUM;j++) {
		result = scan_device(bus, j, 0, bus_num);
		
		if (UNLIKELY(result)) {
			return(result);
		}
	}
	
	bus_num->depth--;
	bus_num->primary = primary_back;
	
	return(0);
}

/*
==================================================================================
 Funtion	:insert_pci_bus
 Input		:uint8_t bus
 		 < bus number >
 		 uint8_t device
 		 < device number >
 		 uint8_t func
 		 <function number >
 		 struct bus_number *bus_num
 		 < detected bus number >
 Output		:void
 Return		:struct pci_bus*
 		 < new bus >
 Description	:insert a pci bus information to the list
==================================================================================
*/
LOCAL struct pci_bus* insert_pci_bus(uint8_t bus, uint8_t device, uint8_t func,
						struct bus_number *bus_num)
{
	struct pci_bus *pci_bus = pci_bus_alloc();
	struct pci_bus *parent = bus_num->bus;
	uint32_t data;
	int i;
	
	if (UNLIKELY(!pci_bus)) {
		return(NULL);
	}
	
	pci_bus->bus_number = bus;
	pci_bus->device_number = device;
	pci_bus->func_number = func;
	
	data = read_config(bus, device, func, CONFIG_DATA3);
	
	pci_bus->class.base_class = BASE_CLASS(data);
	pci_bus->class.sub_class = SUB_CLASS(data);
	pci_bus->class.device_if = DEV_IF(data);
	
	data = read_config(bus, device, func, CONFIG_DATA16);
	
	pci_bus->interrupt_line = INTERRUPT_LINE(data);
	pci_bus->interrupt_pin = INTERRUPT_PIN(data);
	
	for (i = 0;i < TYPE0_BASE_ADDR_REG;i++) {
		data = read_config(bus, device, func, CONFIG_DATA5 + i);
		pci_bus->io[i].start = data;
		data = REQUEST_MEM_SIZE;
		write_config(bus, device, func, CONFIG_DATA5 + i, data);
		data = read_config(bus, device, func, CONFIG_DATA5 + i);
		pci_bus->io[i].size = pci_bus->io[i].start + data;
	}
	
	pci_bus->parent = parent;
	
	data = read_config(bus, device, func, CONFIG_DATA14);
	
	pci_bus->has_cap_list = (CAP_POINTER(data) != 0);
	
	add_list(&pci_bus->node_bus, &parent->list_buses);
	
	return(pci_bus);
}


/*
==================================================================================
 Funtion	:insert_pci_device
 Input		:uint8_t bus
 		 < bus number >
 		 uint8_t device
 		 < device number >
 		 uint8_t func
 		 <function number >
 		 struct bus_number *bus_num
 		 < detected bus number >
 Output		:void
 Return		:int
 		 < result >
 Description	:insert a pci device information to the list
==================================================================================
*/
LOCAL int insert_pci_device(uint8_t bus, uint8_t device, uint8_t func,
						struct bus_number *bus_num)
{
	struct pci_device *pdevice = pci_device_alloc();
	struct pci_bus *parent = bus_num->bus;
	uint32_t data;
	int i;
	
	if (UNLIKELY(!pdevice)) {
		return(-ENOMEM);
	}
	
	pdevice->bus_number = bus;
	pdevice->device_number = device;
	pdevice->func_number = func;
	
	data = read_config(bus, device, func, CONFIG_DATA3);
	
	pdevice->class.base_class = BASE_CLASS(data);
	pdevice->class.sub_class = SUB_CLASS(data);
	pdevice->class.device_if = DEV_IF(data);
	
	data = read_config(bus, device, func, CONFIG_DATA16);
	
	pdevice->interrupt_line = INTERRUPT_LINE(data);
	pdevice->interrupt_pin = INTERRUPT_PIN(data);
	
	for (i = 0;i < TYPE1_BASE_ADDR_REG;i++) {
		data = read_config(bus, device, func, CONFIG_DATA5 + i);
		pdevice->io[i].start = data;
		data = REQUEST_MEM_SIZE;
		write_config(bus, device, func, CONFIG_DATA5 + i, data);
		data = read_config(bus, device, func, CONFIG_DATA5 + i);
		pdevice->io[i].size = pdevice->io[i].start + data;
	}
	
	data = read_config(bus, device, func, CONFIG_DATA14);
	
	pdevice->has_cap_list = (CAP_POINTER(data) != 0);
	
	add_list(&pdevice->node_device, &parent->list_devices);
	
	return(0);
}

/*
==================================================================================
 Funtion	:probe_pci
 Input		:void
 Output		:void
 Return		:void
 Description	:probe pci buses and devices
==================================================================================
*/
LOCAL int probe_pci(void)
{
	int ret;
	struct bus_number bus_num = {0, 0, 0, 0, 0, root_pci};
	
	ret = scan_bus(0, 0, 0, &bus_num);
	
	//printf("max depth:%d\n", bus_num.max_depth);
	
	//printf("---------------------show pci---------------------\n");
	
	//show_pci(root_pci);
	
	return(ret);
}

/*
==================================================================================
 Funtion	:show_pci
 Input		:void
 Output		:void
 Return		:void
 Description	:show pci buses and devices on the list
==================================================================================
*/
LOCAL void show_pci(struct pci_bus *pci_bus)
{
	struct pci_bus *bus;
	struct pci_device *device;
	
	if (is_empty_list(&pci_bus->list_buses)) {
		return;
	}
	
	if (!is_empty_list(&pci_bus->list_devices)) {
		list_for_each_entry(device, &pci_bus->list_devices, node_device) {
			printf("--<DEVICE>bus:%d device:%d func:%d class:0x%02X 0x%02X 0x%02X>\n",
				device->bus_number, device->device_number, device->func_number,
				device->class.base_class, device->class.sub_class, device->class.device_if);
		}
	}
	
	list_for_each_entry(bus, &pci_bus->list_buses, node_bus) {
		printf("<BRIDGE>bus:%d device:%d func:%d class:0x%02X 0x%02X 0x%02X\n",
			bus->bus_number, bus->device_number, bus->func_number,
			bus->class.base_class, bus->class.sub_class, bus->class.device_if);
		
		show_pci(bus);
	}
}

/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
