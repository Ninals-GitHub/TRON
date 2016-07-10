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
#include <bk/elf.h>
#include <bk/sys.h>
#include <bk/memory/mmap.h>
#include <bk/fs/vfs.h>
#include <bk/fs/vdso.h>
#include <bk/fs/load_elf.h>
#include <bk/fs/load/auxvec.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL unsigned long
make_user_stack(char *filename, char *arg_filename, struct elf_info *elf_info,
			struct vdso_load_info *vdso_info,
			char *const argv[], char *const envp[]);
LOCAL INLINE unsigned long stack_up(unsigned long stack, unsigned long value);
LOCAL INLINE unsigned long
stack_up_auxvec(unsigned long stack, unsigned long id, unsigned long value);
LOCAL INLINE unsigned long stack_up_string(unsigned long stack, const char *str);
LOCAL INLINE unsigned long
setup_aux_vector(struct process *proc, unsigned long stack_top,
			struct elf_info *elf_info,
			struct vdso_load_info *vdso_info,
			unsigned long filename, unsigned long machine_name);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	HAS_VDSO	0

#define MAX_ARGC	1024
#define MAX_ENVC	1024

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
 Funtion	:execve
 Input		:const char *filename
 		 < file name to execute >
 		 char *const argv[]
 		 < arguments for a new program >
 		 char *const envp[]
 		 < environment variables for a new program >
 Output		:void
 Return		:int
 		 < result >
 Description	:execute a program
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int execve(const char *filename, char *const argv[], char *const envp[])
{
	struct elf_info elf_info;
	struct vdso_load_info vdso_load_info;
	int fd;
	int err = 0;
	char *interp_name = '\0';
	int interp_exist = 0;
	unsigned long entry_point;
	unsigned long stack_top;
	
	init_elf_info(&elf_info);
	
	//printf("execute:%s\n", filename);
	
	fd = open_exe_file(filename);
	
	if (UNLIKELY(fd < 0)) {
		vd_printf("error:%s:%d\n", __func__, fd);
		return(fd);
	}
	
	interp_exist = read_elf(fd, &elf_info);
	
	if (interp_exist < 0) {
		err = interp_exist;
		vd_printf("error read_elf[%d]\n", -err);
		goto error;
	}
	
	/* -------------------------------------------------------------------- */
	/* interpreter is needed						*/
	/* -------------------------------------------------------------------- */
	if (interp_exist == ELF_HAS_INTERP) {
		interp_name = read_interp_name(fd, &elf_info);
		
		if (UNLIKELY(!interp_name)) {
			err = -ENOMEM;
			panic("unexepected error at interp %s\n", __func__);
			goto error;
		}
		
		err = close(fd);
		
		if (UNLIKELY(err < 0)) {
			panic("unexpected error %s[%d]\n", __func__, err);
			goto error_close;
		}
		
		/* ------------------------------------------------------------ */
		/* free load information before loading interpreter		*/
		/* ------------------------------------------------------------ */
		free_elf_info(&elf_info);
		
		fd = open_exe_file(interp_name);
		
		if (UNLIKELY(fd < 0)) {
			printf("error open_exe_file:%d\n", -fd);
			return(fd);
		}
		
		err = read_elf(fd, &elf_info);
		
		if (err < 0) {
			vd_printf("error interp read_elf[%d]\n", -err);
			goto error_read_elf;
		}
	}
	
#if HAS_VDSO
	printf("load_vdso\n");
	
	err = load_vdso(&vdso_load_info);
	
	if (UNLIKELY(err)) {
		goto error_load_vdso;
	}
#endif
	
	if (interp_exist) {
		stack_top = make_user_stack(interp_name, (char*)filename,
					&elf_info, &vdso_load_info, argv, envp);
	} else {
		stack_top = make_user_stack((char*)filename, NULL,
					&elf_info, &vdso_load_info, argv, envp);
	}
	
	if (UNLIKELY(PROCESS_SIZE < stack_top)) {
		err = (int)stack_top;
		vd_printf("error:invalid stack_top:%d\n", -err);
		goto error_make_user_stack;
	}
	
	//printf("vdso->vdso_base:0x%08X vdso->vdso_phdr:0x%08X\n", vdso_load_info.vdso_base, vdso_load_info.vdso_entry);
	
	//printf("load elf ");
	err = load_elf(fd, &elf_info);
	//printf("->loaded!\n");
	
	if (UNLIKELY(err)) {
		vd_printf("error:load_elf:%d\n", -err);
		goto error_load_elf;
	}
	
	close(fd);
	
	//printf("stack_top:0x%08X\n", stack_top);
	
	entry_point = (unsigned long)elf_info.hdr.e_entry;
	
	//printf("entry_point:0x%08X\n", entry_point);
	
	kfree(interp_name);
	free_elf_info(&elf_info);
	
	return(start_task(entry_point, stack_top));

error_load_elf:
	free_vm(get_current());
#if HAS_VDSO
error_load_vdso:
#endif
error_make_user_stack:
error_read_elf:
error_close:
	kfree(interp_name);
error:
	free_elf_info(&elf_info);
	close(fd);
	vd_printf("%s:error=%d\n", __func__, -err);
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
 Funtion	:make_user_stack
 Input		:char *filename
 		 < program name >
 		 char *arg_filename
 		 < if load interpreter, its 2nd argument is an executable filename >
 		 struct elf_info *elf_info
 		 < elf information >
 		 struct vdso_load_info *vdso_info
 		 < vdso loaded information >
 		 char *const argv[]
 		 < passing arguments >
 		 char *const envp[]
 		 < passing environment values >
 Output		:void
 Return		:int
 		 < result >
 Description	:establish a user stack
==================================================================================
*/
LOCAL unsigned long
make_user_stack(char *filename, char *arg_filename, struct elf_info *elf_info,
			struct vdso_load_info *vdso_info,
			char *const argv[], char *const envp[])
{
	struct process *proc = get_current();
	struct memory_space *mspace = proc->mspace;
	struct page **pages;
	struct page *page;
	unsigned long kstack_top;
	unsigned long kstack_org;
	unsigned long ustack;
	unsigned long *new_envp;
	unsigned long *new_argvp;
	int nr_pages;
	int i;
	int err;
	int argc = 0;
	int envc = 0;
	int start_argc = 0;
	
	nr_pages = PageCount(PROC_STACK_SIZE);
	
	pages = (struct page**)kmalloc(sizeof(struct page*) * nr_pages, 0);
	
	if (UNLIKELY(!pages)) {
		return((unsigned long)-ENOMEM);
	}
	
	page = alloc_pages(nr_pages);
	
	if (UNLIKELY(!page)) {
		err = -ENOMEM;
		goto failed_alloc_page;
	}
	
	for (i = 0;i < nr_pages;i++) {
		pages[i] = page + i;
	}
	
	kstack_org = page_to_address(pages[0]);
	kstack_top = kstack_org + PROC_STACK_SIZE;
	
	get_current()->mspace->start_stack = PROC_STACK_START;
	get_current()->mspace->end_stack = PROC_STACK_TOP;
	
	memset((void*)kstack_org, 0x00, PROC_STACK_SIZE);
	
	
	ustack = mspace->start_stack;
	
	/* -------------------------------------------------------------------- */
	/* count strings							*/
	/* -------------------------------------------------------------------- */
	if (UNLIKELY(vm_check_accessR((void*)argv, PAGESIZE))) {
		printf("unexpected argv:0x%08X\n", argv);
	}
	
	while (argv[argc]) {
		//printf("argv[%d]:%s\n", argc, argv[argc]);
		argc++;
		
		if (UNLIKELY(MAX_ARGC < argc)) {
			panic("unexcepted argc:%d\n", argc);
		}
	}
	
	if (UNLIKELY(vm_check_accessR((void*)envp, PAGESIZE))) {
		printf("unexpected envp:0x%08X\n", envp);
	}
	
	while (envp[envc]) {
		//printf("envp[%d]:%s\n", envc, envp[envc]);
		envc++;
		if (UNLIKELY(MAX_ENVC < envc)) {
			panic("unexcepted argc:%d\n", envc);
		}
	}
	
	argc++;		// for a program file name
	start_argc++;
	
	if (arg_filename) {
		argc++;	// 2nd argument is for arg_filename
		start_argc++;
	}
	
	envc++;		// for a machine name
	
	
	if (MAX_ARGC < argc) {
		err = -EINVAL;
		goto error_max_argc;
	}
	
	if (MAX_ENVC < envc) {
		err = -EINVAL;
		goto error_max_envc;
	}
	
	/* -------------------------------------------------------------------- */
	/* end marker								*/
	/* -------------------------------------------------------------------- */
	kstack_top = stack_up(kstack_top, 0);
	
	/* -------------------------------------------------------------------- */
	/* copy environment values						*/
	/* -------------------------------------------------------------------- */
	new_envp = (unsigned long*)kmalloc(sizeof(unsigned long) * envc, 0);
	
	if (UNLIKELY(!new_envp)) {
		err = -ENOMEM;
		goto failed_copy_env;
	}
	
	kstack_top = stack_up_string(kstack_top, get_machine_name());
	new_envp[0] = ustack + (kstack_top - kstack_org);

	
	for (i = 1;i < envc;i++) {
		kstack_top = stack_up_string(kstack_top, envp[i - 1]);
		new_envp[i] = ustack + (kstack_top - kstack_org);
	}
	
	/* -------------------------------------------------------------------- */
	/* end marker								*/
	/* -------------------------------------------------------------------- */
	kstack_top = stack_up(kstack_top, 0);
	
	/* -------------------------------------------------------------------- */
	/* copy arguments							*/
	/* -------------------------------------------------------------------- */
	new_argvp = (unsigned long*)kmalloc(sizeof(unsigned long) * argc, 0);
	
	if (UNLIKELY(!new_argvp)) {
		err = -ENOMEM;
		goto failed_copy_argv;
	}
	
	
	kstack_top = stack_up_string(kstack_top, filename);
	new_argvp[0] = ustack + (kstack_top - kstack_org);
	
	if (arg_filename) {
		kstack_top = stack_up_string(kstack_top, arg_filename);
		new_argvp[1] = ustack + (kstack_top - kstack_org);
	}
	
	for (i = start_argc;i < argc;i++) {
		kstack_top = stack_up_string(kstack_top, argv[i - start_argc]);
		new_argvp[i] = ustack + (kstack_top - kstack_org);
	}
	/* -------------------------------------------------------------------- */
	/* end marker								*/
	/* -------------------------------------------------------------------- */
	kstack_top = stack_up(kstack_top, 0);
	
	/* -------------------------------------------------------------------- */
	/* set up padding							*/
	/* -------------------------------------------------------------------- */
	if (kstack_top & 0xF) {
		char *top = (char*)kstack_top;
		size_t padding_len = kstack_top & 0xF;
		
		top--;
		padding_len--;
		
		for (;padding_len;padding_len--) {
			*(top--) = 0x00;
		}
		kstack_top = (unsigned long)top;
	}
	
	/* -------------------------------------------------------------------- */
	/* update user stack top						*/
	/* -------------------------------------------------------------------- */
	ustack = ustack + (kstack_top - kstack_org);
	
	/* -------------------------------------------------------------------- */
	/* free user virtual memory space					*/
	/* -------------------------------------------------------------------- */
	free_vm(get_current());
	
	/* -------------------------------------------------------------------- */
	/* allocate vm for an initial stack					*/
	/* -------------------------------------------------------------------- */
	err = vm_initial_stack(get_current(), nr_pages, pages);
	
	if (UNLIKELY(err)) {
		goto failed_vm_initial_stack;
	}
	
	
	kfree(pages);
	
	/* -------------------------------------------------------------------- */
	/* set up auxv vector							*/
	/* -------------------------------------------------------------------- */
	ustack = setup_aux_vector(proc, ustack, elf_info, vdso_info,
					new_argvp[0], new_envp[0]);
	/* -------------------------------------------------------------------- */
	/* set up envp vector							*/
	/* -------------------------------------------------------------------- */
	mspace->end_env = ustack;
	ustack = stack_up(ustack, 0);		// null terminator
	
	for (i = envc - 1;0 < i;i--) {		// index 0 for a machine name
		ustack = stack_up(ustack, new_envp[i]);
		//printf("new_env[%d]:%s\n", i, new_envp[i]);
	}
	
	mspace->start_env = ustack;
	
	kfree(new_envp);
	/* -------------------------------------------------------------------- */
	/* set up argv vector							*/
	/* -------------------------------------------------------------------- */
	mspace->end_arg = ustack;
	ustack = stack_up(ustack, 0);		// null terminator
	
	for (i = argc - 1;0 <= i;i--) {
		ustack = stack_up(ustack, new_argvp[i]);
		//printf("new_arg[%d]:%s\n", i, new_argvp[i]);
	}
	
	mspace->start_arg = ustack;
	
	kfree(new_argvp);
	
	//printf("argc = %d\n", argc);
	/* -------------------------------------------------------------------- */
	/* set up argc								*/
	/* -------------------------------------------------------------------- */
	ustack = stack_up(ustack, argc);
	
	return(ustack);
	
failed_copy_argv:
	kfree(new_envp);
failed_copy_env:
error_max_envc:
error_max_argc:
failed_vm_initial_stack:
	free_pages(page, nr_pages);
failed_alloc_page:
	kfree(pages);
	
	return((unsigned long)err);
}

/*
==================================================================================
 Funtion	:stack_up
 Input		:unsigned long stack
 		 < address of stack >
 		 unsigned long value
 		 < value to stack up >
 Output		:void
 Return		:unsigned long
 		 < updated stack top address >
 Description	:stack up the value
==================================================================================
*/
LOCAL INLINE unsigned long stack_up(unsigned long stack, unsigned long value)
{
	stack -= sizeof(unsigned long);
	
	*(unsigned long*)stack = value;
	
	return(stack);
}

/*
==================================================================================
 Funtion	:stack_up_auxvec
 Input		:unsigned long stack
 		 < address of stack >
 		 unsigned long id
 		 < id of aux vector >
 		 unsigned long value
 		 < value of aux information >
 Output		:void
 Return		:unsigned long
 		 < updated stack top address >
 Description	:stack up the aux vector
==================================================================================
*/
LOCAL INLINE unsigned long
stack_up_auxvec(unsigned long stack, unsigned long id, unsigned long value)
{
	stack -= sizeof(unsigned long);
	
	*(unsigned long*)stack = value;
	
	stack -= sizeof(unsigned long);
	
	*(unsigned long*)stack = id;
	
	return(stack);
}

/*
==================================================================================
 Funtion	:stack_up_string
 Input		:unsigned long stack
 		 < address of stack >
 		 const char *str
 		 < string to stack up >
 Output		:void
 Return		:unsigned long
 		 < updated stack top address >
 Description	:stack up the string
==================================================================================
*/
LOCAL INLINE unsigned long stack_up_string(unsigned long stack, const char *str)
{
	char *stack_top = (char*)(stack);
	size_t len = strnlen(str, PAGESIZE);
	
	stack_top--;
	
	*(stack_top--) = '\0';
	
	while (len--) {
		*(stack_top--) = *(str + len);
	}
	
	stack_top++;
	
	return((unsigned long)stack_top);
}

/*
==================================================================================
 Funtion	:setup_aux_vector
 Input		:struct process *proc
 		 < process to set up its aux vector >
 		 unsigned long stack_top
 		 < stack top of a task >
 		 struct elf_info *elf_info
 		 < elf information for a program >
 		 struct vdso_load_info *vdso_info
 		 < vdso loaded information >
 		 unsigned long filename
 		 < address of a executable file name >
 		 unsigned long machine_name
 		 < address of a machine name >
 Output		:void
 Return		:unsigned long
 		 < new stack top >
 Description	:set up aux vector
==================================================================================
*/
LOCAL INLINE unsigned long
setup_aux_vector(struct process *proc, unsigned long stack_top,
			struct elf_info *elf_info,
			struct vdso_load_info *vdso_info,
			unsigned long filename, unsigned long machine_name)
{
	Elf32_Ehdr *hdr = &elf_info->hdr;
	unsigned long ph_addr;
	unsigned long entry;
	
	ph_addr = proc->mspace->start_code + hdr->e_phoff;
	entry = (unsigned long)hdr->e_entry;
	
	/* as for now do nothing. future work					*/
	stack_top = stack_up_auxvec(stack_top, 0, 0);	// null terminator
	
//	stack_top = stack_up_auxvec(stack_top, AT_EXECFD, 	fd);
	stack_top = stack_up_auxvec(stack_top, AT_PHDR,		ph_addr);
	stack_top = stack_up_auxvec(stack_top, AT_PHENT,	hdr->e_phentsize);
	stack_top = stack_up_auxvec(stack_top, AT_PHNUM,	hdr->e_phnum);
	stack_top = stack_up_auxvec(stack_top, AT_PAGESZ,	PAGESIZE);
	stack_top = stack_up_auxvec(stack_top, AT_BASE,		0);
	stack_top = stack_up_auxvec(stack_top, AT_FLAGS,	0);
	stack_top = stack_up_auxvec(stack_top, AT_ENTRY,	entry);
	stack_top = stack_up_auxvec(stack_top, AT_UID,		proc->uid);
	stack_top = stack_up_auxvec(stack_top, AT_EUID,		proc->euid);
	stack_top = stack_up_auxvec(stack_top, AT_GID,		proc->gid);
	stack_top = stack_up_auxvec(stack_top, AT_EGID,		proc->egid);
	stack_top = stack_up_auxvec(stack_top, AT_PLATFORM,	machine_name);
	//stack_top = stack_up_auxvec(stack_top, AT_HWCAP,	0xFFFFFFFF);
	stack_top = stack_up_auxvec(stack_top, AT_HWCAP,	0);
	stack_top = stack_up_auxvec(stack_top, AT_CLKTCK,	100);
	
	stack_top = stack_up_auxvec(stack_top, AT_BASE_PLATFORM,machine_name);
	//stack_top = stack_up_auxvec(stack_top, AT_RANDOM,	0);
	stack_top = stack_up_auxvec(stack_top, AT_HWCAP2,	0);
	stack_top = stack_up_auxvec(stack_top, AT_EXECFN,	filename);
	
#if HAS_VDSO
	stack_top = stack_up_auxvec(stack_top, AT_SYSINFO,
						vdso_info->vdso_entry);
	stack_top = stack_up_auxvec(stack_top, AT_SYSINFO_EHDR,
						vdso_info->vdso_base);
#endif
	stack_top = stack_up_auxvec(stack_top, AT_IGNORE,	0);
	
	return(stack_top);
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
