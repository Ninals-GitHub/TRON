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

#include <bk/fs/vfs.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct char_dev_list;

LOCAL struct char_device* char_device_cache_alloc(void);
LOCAL void char_device_cache_free(struct char_device *cdev);
LOCAL struct char_dev_list* char_dev_list_alloc(const char *name);
LOCAL void char_dev_list_free(struct char_dev_list *list);
LOCAL struct char_dev_list** search_cdev_minor(struct char_dev_list **major_head,
						unsigned int minor,
						unsigned int count);


/*
==================================================================================

	DEFINE 

==================================================================================
*/

#define	MAX_CHAR_DEVICE		255
#define	MAX_CHAR_DEV_NAME	64

struct char_dev_list {
	struct char_dev_list	*minor_next;
	unsigned int		major;
	unsigned int		minor_base;
	unsigned int		count;
	char			*name;
	struct char_device	*cdev;
};

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct kmem_cache *char_device_cache;
LOCAL const char char_device_cache_name[] = "char_device_cache";

LOCAL struct kmem_cache *char_dev_list_cache;
LOCAL const char char_dev_list_cache_name[] = "char_dev_list_cache";

LOCAL struct char_dev_list *cdev_list[MAX_CHAR_DEVICE];

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_char_device
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize char device management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_char_device(void)
{
	char_device_cache = kmem_cache_create(char_device_cache_name,
				sizeof(struct char_device), 0, 0, NULL);
	
	if (UNLIKELY(!char_device_cache)) {
		vd_printf("error:char_device_cache\n");
		return(-ENOMEM);
	}
	
	char_dev_list_cache = kmem_cache_create(char_dev_list_cache_name,
				sizeof(struct char_dev_list), 0, 0, NULL);
	
	if (UNLIKELY(!char_dev_list_cache)) {
		vd_printf("error:char_dev_list_cache\n");
	}
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_char_device_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a char device cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void destroy_char_device_cache(void)
{
	kmem_cache_destroy(char_device_cache);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_char_device
 Input		:dev_t dev
 		 < device number >
 Output		:void
 Return		:struct char_device*
 		 < allocated object >
 Description	:allocate a char device object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct char_device* alloc_char_device(dev_t dev)
{
	struct char_device *cdev;
	
	cdev = char_device_cache_alloc();
	
	if (UNLIKELY(!cdev)) {
		return(NULL);
	}
	
	cdev->dev = dev;
	
	init_list(&cdev->cdev_list);
	
	return(cdev);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_char_device
 Input		:struct char_device *cdev
 		 < char device to free >
 Output		:void
 Return		:void
 Description	:free char device
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_char_device(struct char_device *cdev)
{
	char_device_cache_free(cdev);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_char_device_numbers
 Input		:dev_t *dev
 		 < device number to output result >
 		 unsinged int first_minor
 		 < first minor number to allocate >
 		 unsigned int count
 		 < minor count to allocate >
 		 const char *name
 		 < name of character device >
 Output		:dev_t *dev
 		 < device number to output result >
 Return		:int
 		 < result >
 Description	:allocate character device numbers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int alloc_char_device_numbers(dev_t *dev, unsigned int first_minor,
				unsigned int count, const char *name)
{
	unsigned int major = get_major(*dev);
	unsigned int minor = first_minor;
	struct char_dev_list *list;
	struct char_dev_list **clist;
	unsigned int i;
	int err;
	
	if (UNLIKELY(MAX_CHAR_DEVICE <= major)) {
		return(-EINVAL);
	}
	
	list = char_dev_list_alloc(name);
	
	if (UNLIKELY(!list)) {
		return(-ENOMEM);
	}
	
	if (!major) {
		major = 1;
	}
	
	for (i = major - 1;i < MAX_CHAR_DEVICE;i++) {
		if (!cdev_list[i]) {
			clist = &cdev_list[i];
			major = i + 1;
			break;
		}
		
		if (UNLIKELY(cdev_list[i]->major == major)) {
			clist = search_cdev_minor(&cdev_list[major - 1],
							minor, count);
			
			if (clist) {
				break;
			}
		}
	}
	
	if (MAX_CHAR_DEVICE < major) {
		err = -EBUSY;
		goto error_out;
	}
	
	list->major = major;
	list->minor_base = minor;
	list->count = count;
	
	if (!*clist) {
		*clist = list;
		//vd_printf("registered clist 0x%08X\n", *clist);
	} else {
		(*clist)->minor_next = list;
	}
	
	*dev = make_dev(major, minor);
	
	return(0);

error_out:
	char_dev_list_free(list);
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:register_char_device_numbers
 Input		:dev_t first
 		 < device number to register >
 		 unsigned int count
 		 < minor count to allocate >
 		 const char *name
 		 < name of character device >
 Output		:void
 Return		:int
 		 < result >
 Description	:register character device numbers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
register_char_device_numbers(dev_t first, unsigned int count, const char *name)
{
	unsigned int major = get_major(first);
	unsigned int minor = get_minor(first);
	struct char_dev_list *list;
	int i;
	int nr_free;
	int err;
	
	if (UNLIKELY((MAX_CHAR_DEVICE < major) ||
			(MAX_CHAR_DEVICE < (major + count)))) {
		return(-EINVAL);
	}
	
	for (i = 0;i < count;i++) {
		struct char_dev_list **clist;
		
		clist = &cdev_list[major - 1];
		
		if (*clist && ((*clist)->major != major)) {
			nr_free = i;
			major--;
			err = -EBUSY;
			goto error_out;
		}
		
		list = char_dev_list_alloc(name);
		
		if (UNLIKELY(!list)) {
			nr_free = i;
			major--;
			err = -ENOMEM;
			goto error_out;
		}
		
		if (*clist) {
			clist = search_cdev_minor(clist, minor, count);
			
			if (!*clist) {
				nr_free = i;
				major--;
				err = -EBUSY;
				goto error_out;
			}
		}
		
		list->major = major;
		list->minor_base = minor;
		list->count = count;
		
		if (*clist) {
			(*clist)->minor_next = list;
		} else {
			*clist = list;
		}
		
		major = major + 1;
	}
	
	return(0);

error_out:
	for (i = nr_free;0 <= i;i--) {
		char_dev_list_free(cdev_list[major - 1]);
		cdev_list[major - 1] = NULL;
		major--;
	}
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:register_char_device
 Input		:unsigned int major
 		 < major number to register >
 		 const char *name
 		 < name of a device >
 		 const struct file_operations *fops
 		 < file operations of a character device >
 Output		:void
 Return		:int
 		 < result >
 Description	:register a character device to the system
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int register_char_device(unsigned int major, const char *name,
				const struct file_operations *fops)
{
	int result;
	dev_t dev = make_dev(major, 0);
	struct char_device *cdev;
	int err;
	
	result = alloc_char_device_numbers(&dev, 0, 255, name);
	
	if (result) {
		return(result);
	}
	
	//vd_printf("allocated device numbers = %d\n", get_major(dev));
	
	cdev = char_device_cache_alloc();
	
	if (UNLIKELY(!cdev)) {
		err = -ENOMEM;
		goto error_out;
	}
	
	cdev->fops = fops;
	cdev->dev = dev;
	
	cdev_list[get_major(dev) - 1]->cdev = cdev;
	
	//vd_printf("regisetr done 0x%08X\n", cdev_list[get_major(dev) - 1]);
	
	return(get_major(dev));

error_out:
	unregister_char_device(dev, name);
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:add_char_device
 Input		:struct char_device *cdev
 		 < a character device to add to the system >
 		 dev_t dev
 		 < device number of a character device >
 		 unsinged int count
 		 < minor count >
 Output		:void
 Return		:int
 		 < result >
 Description	:add a character device to the system
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
add_char_device(struct char_device *cdev, dev_t dev, unsigned int count)
{
	unsigned int major = get_major(dev);
	unsigned int minor = get_minor(dev);
	struct char_dev_list *list = cdev_list[major - 1];
	
	if (UNLIKELY(!list)) {
		return(-ENXIO);
	}
	
	while (list) {
		if ((list->major == major) &&
			(list->minor_base == minor) &&
			(list->count == count)) {
			list->cdev = cdev;
			return(0);
		}
		
		list = list->minor_next;
	}
	
	return(-ENXIO);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:unregister_char_device
 Input		:unsigned int major
 		 < major number to unregister >
 		 const char *name
 		 < device name >
 Output		:void
 Return		:int
 		 < result >
 Description	:unregister character device numbers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int unregister_char_device(unsigned int major, const char *name)
{
	struct char_dev_list *list;
	struct char_dev_list *minor_next;
	
	list = cdev_list[major - 1];
	
	if (!list) {
		return(-ENXIO);
	}
	
	if (strncmp(list->name, name, MAX_CHAR_DEV_NAME) == 0) {
		return(-ENXIO);
	}
	
	while (list) {
		minor_next = list->minor_next;
		
		char_dev_list_free(list);
		
		list = minor_next;
	}
	
	cdev_list[major - 1] = NULL;
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:open_char_device
 Input		:struct vnode *vnode
 		 < vnode of a character device >
 		 struct file *filp
 		 < open file object >
 Output		:void
 Return		:int
 		 < result >
 Description	:open a character device
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int open_char_device(struct vnode *vnode, struct file *filp)
{
	unsigned int major = get_major(vnode->v_rdev);
	unsigned int minor = get_minor(vnode->v_rdev);
	struct char_dev_list *list;
	int err = 0;
	
	vd_printf("open_char_device:major(%d), minor(%d)\n", major, minor);
	
	if (UNLIKELY(MAX_CHAR_DEVICE < major)) {
		return(-ENXIO);
	}
	
	list = cdev_list[major - 1];
	
	if (!list) {
		return(-ENXIO);
	}
	
	while (list) {
		if (list->minor_base == minor) {
			break;
		}
		
		list = list->minor_next;
	}
	
	if (!list) {
		return(-ENXIO);
	}
	
	if (!list->cdev) {
		return(-ENOMEM);
	}
	
	if (!list->cdev->fops) {
		return(-ENXIO);
	}
	
	vnode->v_cdev = list->cdev;
	add_list(&vnode->v_devices, &list->cdev->cdev_list);
	filp->f_fops = list->cdev->fops;
	vnode->v_fops = list->cdev->fops;
	
	if (filp->f_fops->open) {
		err = filp->f_fops->open(vnode, filp);
	}
	
	return(err);
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
 Funtion	:char_device_cache_alloc
 Input		:void
 Output		:void
 Return		:struct char_device*
 		 < char device object >
 Description	:allocate a char device object
==================================================================================
*/
LOCAL struct char_device* char_device_cache_alloc(void)
{
	struct char_device *cdev;
	
	cdev = (struct char_device*)kmem_cache_alloc(char_device_cache, 0);
	
	if (UNLIKELY(!cdev)) {
		return(cdev);
	}
	
	memset((void*)cdev, 0x00, sizeof(struct char_device));
	
	init_list(&cdev->cdev_list);
	
	return(cdev);
}

/*
==================================================================================
 Funtion	:char_device_cache_free
 Input		:struct char_device *cdev
 		 < char device object to free >
 Output		:void
 Return		:void
 Description	:free a char device object
==================================================================================
*/
LOCAL ALWAYS_INLINE void char_device_cache_free(struct char_device *cdev)
{
	kmem_cache_free(char_device_cache, cdev);
}

/*
==================================================================================
 Funtion	:char_dev_list_alloc
 Input		:const char *name
 		 < name of device >
 Output		:void
 Return		:struct char_dev_list*
 		 < char device list >
 Description	:allocate a char device list
==================================================================================
*/
LOCAL struct char_dev_list* char_dev_list_alloc(const char *name)
{
	struct char_dev_list *list;
	
	list = (struct char_dev_list*)kmem_cache_alloc(char_dev_list_cache, 0);
	
	if (UNLIKELY(!list)) {
		return(list);
	}
	
	memset((void*)list, 0x00, sizeof(struct char_dev_list));
	
	list->name = kstrndup(name, MAX_CHAR_DEV_NAME, 0);
	
	if (UNLIKELY(!list->name)) {
		char_dev_list_free(list);
		return(NULL);
	}
	
	return(list);
}

/*
==================================================================================
 Funtion	:char_dev_list_free
 Input		:struct char_dev_list *list
 		 < char device list to free >
 Output		:void
 Return		:void
 Description	:free a char device list
==================================================================================
*/
LOCAL ALWAYS_INLINE void char_dev_list_free(struct char_dev_list *list)
{
	if (LIKELY(list->name)) {
		kfree(list->name);
	}
	
	if (list->cdev) {
		char_device_cache_free(list->cdev);
	}
	kmem_cache_free(char_dev_list_cache, list);
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
LOCAL struct char_dev_list** search_cdev_minor(struct char_dev_list **major_head,
						unsigned int minor,
						unsigned int count)
{
	struct char_dev_list **list = major_head;
	
	while (*list) {
		if (((*list)->minor_base + (*list)->count) < (minor + count)) {
			if (!(*list)->minor_next) {
				return(list);
			}
		}
		
		*list = (*list)->minor_next;
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
