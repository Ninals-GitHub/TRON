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

#include <bk/drivers/bus_type.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL struct bus_type* find_get_last_bus_type(struct bus_type *bus);

/*
==================================================================================

	DEFINE 

==================================================================================
*/

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct bys_type *root_bus_type;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:register_bus
 Input		:struct bus_type *bus_type
 		 < bus type to register >
 Output		:void
 Return		:int
 		 < result >
 Description	:register a bus type to the system
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int register_bus(struct bus_type *bus_type)
{
	struct bus_type *last;
	
	if (UNLIKELY(!root_bus_type)) {
		root_bus_type = bus_type;
		root_bus_type->next = NULL;
	}
	
	last = find_get_last_bus_type(bus_type);
	
	if (UNLIKELY(!last)) {
		return(-EBUSY);
	}
	
	last->next = bus_type;
	bus_type->next = NULL;
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:unregister_bus
 Input		:struct bus_type *bus
 		 < bus to unregister from the system >
 Output		:void
 Return		:int
 		 < result >
 Description	:unregister a bus from the system
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int unregister_bus(struct bus_type *bus)
{
	struct bus_type *bus_element = root_bus_type;
	
	while (bus_element) {
		if (UNLIKELY(bus_element == bus)) {
			bus_element->next = bus->next;
			return(0);
		}
	}
	
	return(-EINVAL);
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
 Funtion	:find_get_last_bus_type
 Input		:struct bus_type *bus
 		 < bus type to find and get >
 Output		:void
 Return		:struct bus_type*
 		 < a last element >
 Description	:get a last element of a bus type list
==================================================================================
*/
LOCAL struct bus_type* find_get_last_bus_type(struct bus_type *bus)
{
	struct bus_type *bus_element = root_bus_type;
	
	while (bus_element) {
		if (UNLIKELY(bus == bus_element)) {
			return(NULL);
		}
		
		if (UNLIKELY(!bus_element->next)) {
			return(bus_element);
		}
		
		bus_element = bus_element->next;
	}
	
	return(NULL);
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
