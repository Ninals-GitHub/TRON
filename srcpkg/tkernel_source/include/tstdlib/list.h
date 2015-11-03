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

#ifndef	__LIST_H__
#define	__LIST_H__

#include <compiler.h>
#include <typedef.h>

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
struct list;

struct list {
	struct list *prev;
	struct list *next;
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
 Funtion	:container_of
 Input		:pointer
		 < pointer to the member of the struct >
		 type
		 < type of the struct >
		 member
		 < member of the struct >
 Output		:void
 Return		:pointer to the struct entry which contains a specified member
 Description	:get the address of struct entry from its member
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	container_of(pointer, type, member)					\
(										\
{										\
	const typeof(((type*)0)->member) *_m_pointer = (pointer);		\
	(type*)((char*)_m_pointer - offsetof(type, member));			\
}										\
)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_entry
 Input		:pointer
		 < pointer to the member of the struct >
		 type
		 < type of the struct >
		 member
		 < member of the struct >
 Output		:void
 Return		:pointer to the struct entry which contains a specified member
 Description	:get the entry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	get_entry(pointer, type, member) container_of(pointer, type, member)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_first_entry
 Input		:list_ptr
		 < pointer of the list >
		 type
		 < type of the struct in which list is embedded
		 member
		 < the member of the struct >
 Output		:void
 Return		:address of the struct in which the first list is embedded
 Description	:get teh first entry of the list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	get_first_entry(list_ptr, type, member)					\
		get_entry((list_ptr)->next, type, member)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_list
 Input		:struct *list
		 < list to initialize >
 Output		:void
 Return		:void
 Description	:initialize a list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void init_list(struct list *list)
{
	list->next = list;
	list->prev = list;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:add_list_element
 Input		:struct list *new
		 < list element to be inserted >
		 struct list *prev
		 < previous list element to add it before >
		 struct list *next
		 < next list element to add it after >
 Output		:void
 Return		:void
 Description	:insert a new list element between pre and next
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE
void add_list_element(struct list *new, struct list *prev, struct list *next)
{
	new->next	= next;
	new->prev	= prev;
	prev->next	= new;
	next->prev	= new;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:add_list
 Input		:struct list *new
		 < list element to be added >
		 struct list *list
		 < list element to add it after >
 Output		:void
 Return		:void
 Description	:add a new list element to next
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void add_list(struct list *new, struct list *list)
{
	add_list_element(new, list, list->next);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:add_list_tail
 Input		:struct list *new
		 < list element to be added >
		 struct list *list
		 < list element to add it before >
 Output		:void
 Return		:void
 Description	:add a new list element to last element
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void add_list_tail(struct list *new, struct list *list)
{
	add_list_element(new, list->prev, list);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:del_list
 Input		:struct *list
		 < list element to be deleted >
 Output		:void
 Return		:void
 Description	:delete a list element between prev and next
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void del_list(struct list *list)
{
	struct list *prev;
	struct list *next;
	
	/* -------------------------------------------------------------------- */
	/* get previous element and nextt element				*/
	/* -------------------------------------------------------------------- */
	prev = list->prev;
	next = list->next;
	/* -------------------------------------------------------------------- */
	/* delete element chain information					*/
	/* -------------------------------------------------------------------- */
	prev->next = list->next;
	next->prev = list->prev;
	/* -------------------------------------------------------------------- */
	/* initialize deleted list element					*/
	/* -------------------------------------------------------------------- */
	init_list(list);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:replace_list
 Input		:struct list *old
		 < list element to be replaced >
		 struct list *new
		 < list element to replace it >
 Output		:void
 Return		:void
 Description	:replace a new list element and an old list element
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void replace_list(struct list *old, struct list *new)
{
	new->next	= old->next;
	new->next->prev	= new;
	new->prev	= old->prev;
	new->prev->next	= new;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:move_list
 Input		:struct list *move
		 < a list to move >
		 struct list *list
		 < a list to be moved >
 Output		:void
 Return		:void
 Description	:move a list to next element of the list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void move_list(struct list *move, struct list *list)
{
	del_list(move);
	add_list(move, list);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:move_list_tail
 Input		:struct list *move
		 < a list to move >
		 struct list *list
		 < a list to be moved >
 Output		:void
 Return		:void
 Description	:move a list to last element of the list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void move_list_tail(struct list *move, struct list *list)
{
	del_list(move);
	add_list_tail(move, list);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:is_empty_list
 Input		:struct list *list
		 < a list to be tested >
 Output		:void
 Return		:int
		 < 1:list is empty, 0:list is not empty >
 Description	:test whether a list is empty or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE int is_empty_list(struct list *list)
{
	return(list->next == list);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:is_singular_list
 Input		:struct list *list
		 < a list to be tested >
 Output		:void
 Return		:int
		 < 1:list is singular, 0:list is not singular >
 Description	:test whether a list is singular or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE int is_singular_list(struct list *list)
{
	return(!is_empty_list(list) && (list->next == list->prev));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:rotate_list_left
 Input		:struct list *list
		 < list to be rotated >
 Output		:void
 Return		:void
 Description	:roate a list to left
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void rotate_list_left(struct list *list)
{
	struct list *first;
	
	if (!is_empty_list(list)) {
		first = list->next;
		move_list_tail(first, list);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:cut_list_position
 Input		:struct list *list
		 < list (to be empty list) to all removed elements >
		 struct list *cut_list
		 < list to be cut >
		 struct list *position
		 < cut at the element(must be in the cut_list) >
 Output		:struct list *list
		 < removed list >
		 struct list *cut_list
		 < remains of list to be cut >
 Return		:void
 Description	:cut a list into two
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void 
cut_list_position(struct list *list,
			struct list *cut_list, struct list *position )
{
	/* -------------------------------------------------------------------- */
	/* no element								*/
	/* -------------------------------------------------------------------- */
	if (is_empty_list(cut_list)) {
		return;
	}
	/* -------------------------------------------------------------------- */
	/* just one element and cut_list does not have position			*/
	/* -------------------------------------------------------------------- */
	if ((is_singular_list(cut_list)) && (cut_list->next != position)
		&& (cut_list != position)) {
		return;
	}

	if (position == cut_list) {
		/* ------------------------------------------------------------ */
		/* nothing to do as to cutting					*/
		/* ------------------------------------------------------------ */
		init_list(list);
	} else {
		struct list *new_first;
		/* ------------------------------------------------------------ */
		/* cut a list							*/
		/* ------------------------------------------------------------ */
		new_first	= position->next;
		
		list->next	= cut_list->next;
		list->next->prev= list;
		list->prev	= position;
		position->next	= list;
		cut_list->next	= new_first;
		new_first->prev	= cut_list;
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:insert_list
 Input		:struct list *list
		 < list to insert between prev and next >
		 struct list *prev
		 < list to be added the list after >
		 struct list *next
		 < list to be added the list before >
 Output		:void
 Return		:void
 Description	:insert a list between prev and next
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void
insert_list(struct list *list, struct list *prev, struct list *next)
{
	if (!is_empty_list(list)) {
		struct list *first;
		struct list *last;
		
		first		= list->next;
		last		= list->prev;
		
		first->prev	= prev;
		prev->next	= first;
		
		last->next	= next;
		next->prev	= last;
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:splice_list
 Input		:struct list *insert
		 < list to add >
		 stuct list *list
		 < list to be inserted between head and next >
 Output		:void
 Return		:void
 Description	:splice two list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void splice_list(struct list *insert, struct list *list)
{
	if (!is_empty_list(insert)) {
		insert_list(insert, list, list->next);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:splice_list
 Input		:struct list *insert
		 < list to add >
		 stuct list *list
		 < list to be inserted between head and next >
 Output		:struct list *insert
		 < list is initialized >
 Return		:void
 Description	:splice two list and initiazlie inserttee
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void splice_list_init(struct list *insert, struct list *list)
{
	if (!is_empty_list(insert)) {
		insert_list(insert, list, list->next);
		init_list(insert);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:splice_list_tail
 Input		:struct list *insert
		 < list to add >
		 stuct list *list
		 < list to be inserted between head and prev >
 Output		:void
 Return		:void
 Description	:splice two list between head and prev
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void splice_list_tail(struct list *insert, struct list *list)
{
	if (!is_empty_list(insert)) {
		insert_list(insert, list->prev, list);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:splice_list_tail_init
 Input		:struct list *insert
		 < list to add >
		 stuct list *list
		 < list to be inserted between head and prev >
 Output		:struct list *insert
		 < list is initialized >
 Return		:void
 Description	:splice two list between head and prev, and initialize a insertee
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE void splice_list_tail_init(struct list *insert, struct list *list)
{
	if (!is_empty_list(insert)) {
		insert_list(insert, list->prev, list);
		init_list(insert);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each
 Input		:pos
		 < list loop cursor pointer >
		 list
		 < list to iterate over >
 Output		:void
 Return		:void
 Description	:iterate over a list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each(pos, list)						\
	for (pos = (list)->next;pos != (list);pos = (pos)->next)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_prev
 Input		:pos
		 < list loop cursor pointer >
		 list
		 < list to iterate over >
 Output		:void
 Return		:void
 Description	:iterate over a list backwards
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_prev(pos, list)						\
	for (pos = (list)->prev;pos != (list);pos = (pos)->prev)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_safe
 Input		:pos
		 < list loop cursor pointer >
		 temp
		 < temporary list pointer >
		 list
		 < list to iterate over >
 Output		:void
 Return		:void
 Description	:iterate over a list safe against removal of a list entry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_safe(pos, temp, list)					\
	for (pos = (list)->next, temp = (pos)->next;				\
		pos != (list);							\
		pos = (temp), temp = (pos)->next)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_prev_safe
 Input		:pos
		 < list loop cursor pointer >
		 temp
		 < temporary list pointer >
		 list
		 < list to iterate over >
 Output		:void
 Return		:void
 Description	:iterate over a list backwards safe against removal of
		 a list entry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_prev_safe(pos, temp, list)				\
	for (pos = (list)->prev, temp = (pos)->prev;				\
		pos != (list);							\
		pos = (temp), temp = (pos)->prev)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_entry
 Input		:pos
		 < list loop cursor pointer >
		 list
		 < list to iterate over >
		 member
		 < the member of the struct >
 Output		:void
 Return		:void
 Description	:iterate over a list of entries
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_entry(pos, list, member)					\
	for (pos = get_entry((list)->next, typeof(*pos), member);		\
		&pos->member != (list);						\
		pos = get_entry(pos->member.next, typeof(*pos), member))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_entry_reverse
 Input		:pos
		 < list loop cursor pointer >
		 list
		 < list to iterate over >
		 member
		 < the member of the struct >
 Output		:void
 Return		:void
 Description	:iterate over a list of entries backwards
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_entry_reverse(pos, list, member)				\
	for (pos = get_entry((list)->prev, typeof(*pos), member);		\
		&pos->member != (list);						\
		pos = get_entry(pos->member.prev, typeof(*pos), member))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_entry_continue
 Input		:pos
		 < list loop cursor pointer >
		 list
		 < list to iterate over >
		 member
		 < the member of the struct >
 Output		:void
 Return		:void
 Description	:iterate over a list of entries of specified type, continuing
		 after the current position
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_entry_continue(pos, list, member)				\
	for (pos = get_entry((pos)->member.next, typeof(*pos), member);		\
		&pos->member != (list);						\
		pos = get_entry((pos)->member.next, typeof(*pos), member))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_entry_continue_rev
 Input		:pos
		 < list loop cursor pointer >
		 list
		 < list to iterate over >
		 member
		 < the member of the struct >
 Output		:void
 Return		:void
 Description	:iterate over a list of entries of specified type backwards,
		 continuing after the current position
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_entry_continue_rev(pos, list, member)			\
	for (pos = get_entry((pos)->member.prev, typeof(*pos), member);		\
		&pos->member != (list);						\
		pos = get_entry((pos)->member.prev, typeof(*pos), member))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:prepare_list_entry
 Input		:pos
		 < list loop cursor to prepare as a start point >
		 list
		 < list to iterate over >
 Output		:void
 Return		:void
 Description	:prepare a start point for user in list_for_each_entry_from
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	prepare_list_entry(pos, list, member)					\
	((pos)?:get_entry(list, typeof(*pos), member))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_entry_from
 Input		:pos
		 < list loop cursor pointer >
		 list
		 < list to iterate over >
		 member
		 < the member of the struct >
 Output		:void
 Return		:void
 Description	:iterate over a list of entry of specified type, continuing
		 from the current position
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_entry_from(pos, list, member)				\
	for (;									\
		&pos->member != (list);						\
		pos = get_entry(pos->member.next, typeof(*pos), member))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_entry_safe
 Input		:pos
		 < list loop cursor pointer, must be pointer of struct >
		 temp
		 < temporary list pointer >
		 list
		 < list to iterate over >
		 member
		 < the member of the struct >
 Output		:void
 Return		:void
 Description	:iterate over a list of entry of specified type safe against
		 removal of list entry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_entry_safe(pos, temp, list, member)			\
	for (pos = get_entry((list)->next, typeof(*pos), member),		\
		temp = get_entry((pos)->member.next, typeof(*pos), member);	\
		&pos->member != (list);						\
		pos = temp,							\
		temp = get_entry((temp)->member.next, typeof(*temp), member))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_entry_safe_rev
 Input		:pos
		 < list loop cursor pointer, must be pointer of struct >
		 temp
		 < temporary list pointer >
		 list
		 < list to iterate over >
		 member
		 < the member of the struct >
 Output		:void
 Return		:void
 Description	:iterate over a list of entry of specified type backwards 
		 safe against removal of list entry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_entry_safe_rev(pos, temp, list, member)			\
	for (pos = get_entry((list)->prev, typeof(*pos), member),		\
		temp = get_entry((pos)->member.prev, typeof(*pos), member);	\
		&pos->member != (list);						\
		pos = temp,							\
		temp = get_entry((temp)->member.prev, typeof(*temp), member))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_entry_safe_continue
 Input		:pos
		 < list loop cursor pointer, must be pointer of struct >
		 temp
		 < temporary list pointer >
		 list
		 < list to iterate over >
		 member
		 < the member of the struct >
 Output		:void
 Return		:void
 Description	:iterate over a list of entry of specified type, continuing after
		 current point, safe against removval of list entry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_entry_safe_continue(pos, temp, list, member)		\
	for (pos = get_entry((pos)->next, typeof(*pos), member),		\
		temp = get_entry((pos)->member.next, typeof(*pos), member);	\
		&pos->member != (list);						\
		pos = temp,							\
		temp = get_entry((temp)->member.next, typeof(*temp), meber))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_for_each_entry_safe_from
 Input		:pos
		 < list loop cursor pointer, must be pointer of struct >
		 temp
		 < temporary list pointer >
		 list
		 < list to iterate over >
		 member
		 < the member of the struct >
 Output		:void
 Return		:void
 Description	:iterate over a list of entry of specified type from current
		 point, safe against removal of list entry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_for_each_entry_safe_from(pos, temp, list, member)			\
	for (temp = get_entry((pos)->member.next, typeof(*pos), member);	\
		&pos->member != (list);						\
		pos = temp,							\
		temp = get_entry((temp)->member.next, typeof(*temp), member))

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:list_safe_reset_next
 Input		::pos
		 < list loop cursor pointer, must be pointer of struct >
		 temp
		 < temporary list pointer >
		 member
		 < the member of the struct >
 Output		:void
 Return		:void
 Description	:reset safe loop state
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	list_safe_reset_next(pos, temp, member)					\
	temp = get_entry((pos)->member.next, typeof(*pos), member)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	// __LIST_H__
