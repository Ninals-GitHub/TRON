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

#include <machine.h>

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
.macro _SVC_ENTRY name
	.extern \name
	.long \name
.endm

/*
==================================================================================

	Management 

==================================================================================
*/
	_SVC_ENTRY restart_syscall 		/* 0000 *//* restart_syscall	*/
	_SVC_ENTRY exit		 		/* 0001 *//* exit		*/
	_SVC_ENTRY syscall_test 		/* 0002 *//* fork		*/
	_SVC_ENTRY read				/* 0003 *//* read		*/
	_SVC_ENTRY write	 		/* 0004 *//* write		*/
	_SVC_ENTRY open		 		/* 0005 *//* open		*/
	_SVC_ENTRY close	 		/* 0006 *//* close		*/
	_SVC_ENTRY waitpid	 		/* 0007 *//* waitpid		*/
	_SVC_ENTRY syscall_test 		/* 0008 *//* creat		*/
	_SVC_ENTRY syscall_test 		/* 0009 *//* link		*/
	_SVC_ENTRY syscall_test 		/* 0010 *//* unlink		*/
	_SVC_ENTRY execve	 		/* 0011 *//* execve		*/
	_SVC_ENTRY chdir	 		/* 0012 *//* chdir		*/
	_SVC_ENTRY syscall_test 		/* 0013 *//* time		*/
	_SVC_ENTRY syscall_test 		/* 0014 *//* mknod		*/
	_SVC_ENTRY syscall_test 		/* 0015 *//* chmod		*/
	_SVC_ENTRY syscall_test 		/* 0016 *//* lchown		*/
	_SVC_ENTRY syscall_test 		/* 0017 *//* break		*/
	_SVC_ENTRY syscall_test 		/* 0018 *//* oldstat		*/
	_SVC_ENTRY syscall_test 		/* 0019 *//* lseek		*/
	_SVC_ENTRY getpid	 		/* 0020 *//* getpid		*/
	_SVC_ENTRY syscall_test 		/* 0021 *//* mount		*/
	_SVC_ENTRY syscall_test 		/* 0022 *//* umount		*/
	_SVC_ENTRY syscall_test 		/* 0023 *//* setuid		*/
	_SVC_ENTRY syscall_test 		/* 0024 *//* getuid		*/
	_SVC_ENTRY syscall_test 		/* 0025 *//* stime		*/
	_SVC_ENTRY syscall_test 		/* 0026 *//* ptrace		*/
	_SVC_ENTRY syscall_test 		/* 0027 *//* alarm		*/
	_SVC_ENTRY syscall_test 		/* 0028 *//* oldfstat		*/
	_SVC_ENTRY syscall_test 		/* 0029 *//* pause		*/
	_SVC_ENTRY syscall_test 		/* 0030 *//* utime		*/
	_SVC_ENTRY syscall_test 		/* 0031 *//* stty		*/
	_SVC_ENTRY syscall_test 		/* 0032 *//* gtty		*/
	_SVC_ENTRY access	 		/* 0033 *//* access		*/
	_SVC_ENTRY syscall_test 		/* 0034 *//* nice		*/
	_SVC_ENTRY syscall_test 		/* 0035 *//* ftime		*/
	_SVC_ENTRY syscall_test 		/* 0036 *//* sync		*/
	_SVC_ENTRY syscall_test 		/* 0037 *//* kill		*/
	_SVC_ENTRY syscall_test 		/* 0038 *//* rename		*/
	_SVC_ENTRY mkdir	 		/* 0039 *//* mkdir		*/
	_SVC_ENTRY syscall_test 		/* 0040 *//* rmdir		*/
	_SVC_ENTRY syscall_test 		/* 0041 *//* dup		*/
	_SVC_ENTRY syscall_test 		/* 0042 *//* pipe		*/
	_SVC_ENTRY syscall_test 		/* 0043 *//* times		*/
	_SVC_ENTRY syscall_test 		/* 0044 *//* prof		*/
	_SVC_ENTRY brk		 		/* 0045 *//* brk		*/
	_SVC_ENTRY syscall_test 		/* 0046 *//* setgid		*/
	_SVC_ENTRY syscall_test 		/* 0047 *//* getgid		*/
	_SVC_ENTRY syscall_test 		/* 0048 *//* signal		*/
	_SVC_ENTRY syscall_test 		/* 0049 *//* geteuid		*/
	_SVC_ENTRY syscall_test 		/* 0050 *//* getegid		*/
	_SVC_ENTRY syscall_test 		/* 0051 *//* acct		*/
	_SVC_ENTRY syscall_test 		/* 0052 *//* umount2		*/
	_SVC_ENTRY syscall_test 		/* 0053 *//* lock		*/
	_SVC_ENTRY ioctl	 		/* 0054 *//* ioctl		*/
	_SVC_ENTRY syscall_test 		/* 0055 *//* fcntl		*/
	_SVC_ENTRY syscall_test 		/* 0056 *//* mpx		*/
	_SVC_ENTRY syscall_test 		/* 0057 *//* setpgid		*/
	_SVC_ENTRY syscall_test 		/* 0058 *//* ulimit		*/
//	_SVC_ENTRY oldolduname			/* 0059 *//* oldolduname	*/
	_SVC_ENTRY syscall_test		/* 0059 *//* oldolduname	*/
	_SVC_ENTRY syscall_test 		/* 0060 *//* umask		*/
	_SVC_ENTRY syscall_test 		/* 0061 *//* chroot		*/
	_SVC_ENTRY syscall_test 		/* 0062 *//* ustat		*/
	_SVC_ENTRY syscall_test 		/* 0063 *//* dup2		*/
	_SVC_ENTRY getppid	 		/* 0064 *//* getppid		*/
	_SVC_ENTRY syscall_test 		/* 0065 *//* getpgrp		*/
	_SVC_ENTRY syscall_test 		/* 0066 *//* setsid		*/
	_SVC_ENTRY syscall_test 		/* 0067 *//* sigaction		*/
	_SVC_ENTRY syscall_test 		/* 0068 *//* sgetmask		*/
	_SVC_ENTRY syscall_test 		/* 0069 *//* ssetmask		*/
	_SVC_ENTRY syscall_test 		/* 0070 *//* setreuid		*/
	_SVC_ENTRY syscall_test 		/* 0071 *//* setregid		*/
	_SVC_ENTRY syscall_test 		/* 0072 *//* sigsuspend		*/
	_SVC_ENTRY syscall_test 		/* 0073 *//* sigpending		*/
	_SVC_ENTRY syscall_test 		/* 0074 *//* sethostname	*/
	_SVC_ENTRY syscall_test 		/* 0075 *//* setrlimit		*/
	_SVC_ENTRY syscall_test 		/* 0076 *//* getrlimit		*/
	_SVC_ENTRY syscall_test 		/* 0077 *//* getrusage		*/
	_SVC_ENTRY syscall_test 		/* 0078 *//* gettimeofday	*/
	_SVC_ENTRY syscall_test 		/* 0079 *//* settimeofday	*/
	_SVC_ENTRY syscall_test 		/* 0080 *//* getgroups		*/
	_SVC_ENTRY syscall_test 		/* 0081 *//* setgroups		*/
	_SVC_ENTRY syscall_test 		/* 0082 *//* select		*/
	_SVC_ENTRY symlink	 		/* 0083 *//* symlink		*/
	_SVC_ENTRY syscall_test 		/* 0084 *//* oldlstat		*/
	_SVC_ENTRY readlink	 		/* 0085 *//* readlink		*/
	_SVC_ENTRY syscall_test 		/* 0086 *//* uselib		*/
	_SVC_ENTRY syscall_test 		/* 0087 *//* swapon		*/
	_SVC_ENTRY syscall_test 		/* 0088 *//* reboot		*/
	_SVC_ENTRY syscall_test 		/* 0089 *//* readdir		*/
//	_SVC_ENTRY mmap		 		/* 0090 *//* mmap		*/
	_SVC_ENTRY syscall_test		 		/* 0090 *//* mmap		*/
	_SVC_ENTRY munmap	 		/* 0091 *//* munmap		*/
	_SVC_ENTRY syscall_test 		/* 0092 *//* truncate		*/
	_SVC_ENTRY syscall_test 		/* 0093 *//* ftruncate		*/
	_SVC_ENTRY syscall_test 		/* 0094 *//* fchmod		*/
	_SVC_ENTRY syscall_test 		/* 0095 *//* fchown		*/
	_SVC_ENTRY syscall_test 		/* 0096 *//* getpriority	*/
	_SVC_ENTRY syscall_test 		/* 0097 *//* setpriority	*/
	_SVC_ENTRY syscall_test 		/* 0098 *//* profil		*/
	_SVC_ENTRY syscall_test 		/* 0099 *//* statfs		*/
	_SVC_ENTRY syscall_test 		/* 0100 *//* fstatfs		*/
	_SVC_ENTRY syscall_test 		/* 0101 *//* ioperm		*/
	_SVC_ENTRY syscall_test 		/* 0102 *//* socketcall		*/
	_SVC_ENTRY syscall_test 		/* 0103 *//* syslog		*/
	_SVC_ENTRY syscall_test 		/* 0104 *//* setitimer		*/
	_SVC_ENTRY syscall_test 		/* 0105 *//* getitimer		*/
	_SVC_ENTRY syscall_test 		/* 0106 *//* stat		*/
	_SVC_ENTRY syscall_test 		/* 0107 *//* lstat		*/
	_SVC_ENTRY syscall_test 		/* 0108 *//* fstat		*/
//	_SVC_ENTRY olduname		/* 0109 *//* olduname		*/
	_SVC_ENTRY syscall_test			/* 0109 *//* olduname		*/
	_SVC_ENTRY syscall_test			/* 0110 *//* iopl		*/
	_SVC_ENTRY syscall_test			/* 0111 *//* vhangup		*/
	_SVC_ENTRY syscall_test			/* 0112 *//* idle		*/
	_SVC_ENTRY syscall_test			/* 0113 *//* vm86old		*/
//	_SVC_ENTRY wait4			/* 0114 *//* wait4		*/
	_SVC_ENTRY syscall_test		/* 0114 *//* wait4		*/
	_SVC_ENTRY syscall_test			/* 0115 *//* swapoff		*/
	_SVC_ENTRY syscall_test			/* 0116 *//* sysinfo		*/
	_SVC_ENTRY syscall_test			/* 0117 *//* ipc		*/
	_SVC_ENTRY syscall_test			/* 0118 *//* fsync		*/
	_SVC_ENTRY syscall_test			/* 0119 *//* sigreturn		*/
	_SVC_ENTRY clone			/* 0120 *//* clone		*/
	_SVC_ENTRY syscall_test			/* 0121 *//* setdomainname	*/
	_SVC_ENTRY uname			/* 0122 *//* uname		*/
	_SVC_ENTRY syscall_test			/* 0123 *//* modify_ldt		*/
	_SVC_ENTRY syscall_test			/* 0124 *//* adjtimex		*/
	_SVC_ENTRY mprotect			/* 0125 *//* mprotect		*/
	_SVC_ENTRY syscall_test			/* 0126 *//* sigprocmask	*/
	_SVC_ENTRY syscall_test			/* 0127 *//* create_module	*/
	_SVC_ENTRY syscall_test			/* 0128 *//* init_module	*/
	_SVC_ENTRY syscall_test			/* 0129 *//* delete_module	*/
	_SVC_ENTRY syscall_test			/* 0130 *//* get_kernel_syms	*/
	_SVC_ENTRY syscall_test			/* 0131 *//* quotactl		*/
	_SVC_ENTRY syscall_test			/* 0132 *//* getpgid		*/
	_SVC_ENTRY syscall_test			/* 0133 *//* fchdir		*/
	_SVC_ENTRY syscall_test			/* 0134 *//* bdflush		*/
	_SVC_ENTRY syscall_test			/* 0135 *//* sysfs		*/
	_SVC_ENTRY syscall_test			/* 0136 *//* personality	*/
	_SVC_ENTRY syscall_test			/* 0137 *//* afs_syscall	*/
	_SVC_ENTRY syscall_test			/* 0138 *//* setfsuid		*/
	_SVC_ENTRY syscall_test			/* 0139 *//* setfsgid		*/
	_SVC_ENTRY syscall_test			/* 0140 *//* _llseek		*/
	_SVC_ENTRY syscall_test			/* 0141 *//* getdents		*/
	_SVC_ENTRY syscall_test			/* 0142 *//* _newselect		*/
	_SVC_ENTRY syscall_test			/* 0143 *//* flock		*/
	_SVC_ENTRY syscall_test			/* 0144 *//* msync		*/
	_SVC_ENTRY syscall_test			/* 0145 *//* readv		*/
	_SVC_ENTRY writev			/* 0146 *//* writev		*/
	_SVC_ENTRY syscall_test			/* 0147 *//* getsid		*/
	_SVC_ENTRY syscall_test			/* 0148 *//* fdatasync		*/
	_SVC_ENTRY syscall_test			/* 0149 *//* _sysctl		*/
	_SVC_ENTRY syscall_test			/* 0150 *//* mlock		*/
	_SVC_ENTRY syscall_test			/* 0151 *//* munlock		*/
	_SVC_ENTRY syscall_test			/* 0152 *//* mlockall		*/
	_SVC_ENTRY syscall_test			/* 0153 *//* munlockall		*/
	_SVC_ENTRY syscall_test			/* 0154 *//* sched_setparam	*/
	_SVC_ENTRY syscall_test			/* 0155 *//* sched_getparam	*/
	_SVC_ENTRY syscall_test			/* 0156 *//* sched_setscheduler	*/
	_SVC_ENTRY syscall_test			/* 0157 *//* sched_getscheduler	*/
	_SVC_ENTRY syscall_test			/* 0158 *//* sched_yield	*/
	_SVC_ENTRY syscall_test			/* 0159 *//* sched_get_priority_max	*/
	_SVC_ENTRY syscall_test			/* 0160 *//* sched_get_priority_min	*/
	_SVC_ENTRY syscall_test			/* 0161 *//* sched_rr_get_interval	*/
	_SVC_ENTRY nanosleep			/* 0162 *//* nanosleep		*/
	_SVC_ENTRY syscall_test			/* 0163 *//* mremap		*/
	_SVC_ENTRY syscall_test			/* 0164 *//* setresuid		*/
	_SVC_ENTRY syscall_test			/* 0165 *//* getresuid		*/
	_SVC_ENTRY syscall_test			/* 0166 *//* vm86		*/
	_SVC_ENTRY syscall_test			/* 0167 *//* query_module	*/
	_SVC_ENTRY poll				/* 0168 *//* poll		*/
	_SVC_ENTRY syscall_test			/* 0169 *//* nfsservctl		*/
	_SVC_ENTRY syscall_test			/* 0170 *//* setresgid		*/
	_SVC_ENTRY syscall_test			/* 0171 *//* getresgid		*/
	_SVC_ENTRY syscall_test			/* 0172 *//* prctl		*/
	_SVC_ENTRY syscall_test			/* 0173 *//* rt_sigreturn	*/
	_SVC_ENTRY rt_sigaction			/* 0174 *//* rt_sigaction	*/
//	_SVC_ENTRY rt_sigprocmask		/* 0175 *//* rt_sigprocmask	*/
	_SVC_ENTRY syscall_test		/* 0175 *//* rt_sigprocmask	*/
	_SVC_ENTRY syscall_test			/* 0176 *//* rt_sigpending	*/
	_SVC_ENTRY syscall_test			/* 0177 *//* rt_sigtimedwait	*/
	_SVC_ENTRY syscall_test			/* 0178 *//* rt_sigqueueinfo	*/
	_SVC_ENTRY syscall_test			/* 0179 *//* rt_sigsuspend	*/
	_SVC_ENTRY syscall_test			/* 0180 *//* pread64		*/
	_SVC_ENTRY syscall_test			/* 0181 *//* pwrite64		*/
	_SVC_ENTRY syscall_test			/* 0182 *//* chown		*/
	_SVC_ENTRY getcwd			/* 0183 *//* getcwd		*/
	_SVC_ENTRY syscall_test			/* 0184 *//* capget		*/
	_SVC_ENTRY syscall_test			/* 0185 *//* capset		*/
	_SVC_ENTRY syscall_test			/* 0186 *//* sigaltstack	*/
	_SVC_ENTRY syscall_test			/* 0187 *//* sendfile		*/
	_SVC_ENTRY syscall_test			/* 0188 *//* getpmsg		*/
	_SVC_ENTRY syscall_test			/* 0189 *//* putpmsg		*/
	_SVC_ENTRY syscall_test			/* 0190 *//* vfork		*/
	_SVC_ENTRY syscall_test			/* 0191 *//* ugetrlimit		*/
	_SVC_ENTRY mmap2			/* 0192 *//* mmap2		*/
	_SVC_ENTRY syscall_test			/* 0193 *//* truncate64		*/
	_SVC_ENTRY syscall_test			/* 0194 *//* ftruncate64	*/
	_SVC_ENTRY stat64			/* 0195 *//* stat64		*/
	_SVC_ENTRY lstat64			/* 0196 *//* lstat64		*/
	_SVC_ENTRY fstat64			/* 0197 *//* fstat64		*/
	_SVC_ENTRY syscall_test			/* 0198 *//* lchown32		*/
	_SVC_ENTRY getuid32			/* 0199 *//* getuid32		*/
	_SVC_ENTRY getgid32			/* 0200 *//* getgid32		*/
	_SVC_ENTRY geteuid32			/* 0201 *//* geteuid32		*/
//	_SVC_ENTRY getegid32			/* 0202 *//* getegid32		*/
	_SVC_ENTRY syscall_test			/* 0202 *//* getegid32		*/
	_SVC_ENTRY syscall_test			/* 0203 *//* setreuid32		*/
	_SVC_ENTRY syscall_test			/* 0204 *//* setregid32		*/
	_SVC_ENTRY syscall_test			/* 0205 *//* getgroups32	*/
	_SVC_ENTRY syscall_test			/* 0206 *//* setgroups32	*/
	_SVC_ENTRY syscall_test			/* 0207 *//* fchown32		*/
	_SVC_ENTRY syscall_test			/* 0208 *//* setresuid32	*/
	_SVC_ENTRY syscall_test			/* 0209 *//* getresuid32	*/
	_SVC_ENTRY syscall_test			/* 0210 *//* setresgid32	*/
	_SVC_ENTRY syscall_test			/* 0211 *//* getresgid32	*/
	_SVC_ENTRY syscall_test			/* 0212 *//* chown32		*/
	_SVC_ENTRY setuid32			/* 0213 *//* setuid32		*/
	_SVC_ENTRY setgid32			/* 0214 *//* setgid32		*/
	_SVC_ENTRY syscall_test			/* 0215 *//* setfsuid32		*/
	_SVC_ENTRY syscall_test			/* 0216 *//* setfsgid32		*/
	_SVC_ENTRY syscall_test			/* 0217 *//* pivot_root		*/
	_SVC_ENTRY syscall_test			/* 0218 *//* mincore		*/
	_SVC_ENTRY syscall_test			/* 0219 *//* madvise		*/
	_SVC_ENTRY getdents64			/* 0220 *//* getdents64		*/
	_SVC_ENTRY syscall_test			/* 0221 *//* fcntl64		*/
	_SVC_ENTRY syscall_test			/* 0222 *//* reserved		*/
	_SVC_ENTRY syscall_test			/* 0223 *//* reserved		*/
//	_SVC_ENTRY gettid			/* 0224 *//* gettid		*/
	_SVC_ENTRY syscall_test		/* 0224 *//* gettid		*/
	_SVC_ENTRY syscall_test			/* 0225 *//* readahead		*/
	_SVC_ENTRY syscall_test			/* 0226 *//* setxattr		*/
	_SVC_ENTRY syscall_test			/* 0227 *//* lsetxattr		*/
	_SVC_ENTRY syscall_test			/* 0228 *//* fsetxattr		*/
	_SVC_ENTRY syscall_test			/* 0229 *//* getxattr		*/
	_SVC_ENTRY syscall_test			/* 0230 *//* lgetxattr		*/
	_SVC_ENTRY syscall_test			/* 0231 *//* fgetxattr		*/
	_SVC_ENTRY syscall_test			/* 0232 *//* listxattr		*/
	_SVC_ENTRY syscall_test			/* 0233 *//* llistxattr		*/
	_SVC_ENTRY syscall_test			/* 0234 *//* flistxattr		*/
	_SVC_ENTRY syscall_test			/* 0235 *//* removexattr	*/
	_SVC_ENTRY syscall_test			/* 0236 *//* lremovexattr	*/
	_SVC_ENTRY syscall_test			/* 0237 *//* fremovexattr	*/
	_SVC_ENTRY syscall_test			/* 0238 *//* tkill		*/
	_SVC_ENTRY syscall_test			/* 0239 *//* sendfile64		*/
	_SVC_ENTRY futex			/* 0240 *//* futex		*/
	_SVC_ENTRY syscall_test			/* 0241 *//* sched_setaffinity	*/
	_SVC_ENTRY syscall_test			/* 0242 *//* sched_getaffinity	*/
	_SVC_ENTRY set_thread_area		/* 0243 *//* set_thread_area	*/
//	_SVC_ENTRY get_thread_area		/* 0244 *//* get_thread_area	*/
	_SVC_ENTRY syscall_test		/* 0244 *//* get_thread_area	*/
	_SVC_ENTRY syscall_test			/* 0245 *//* io_setup		*/
	_SVC_ENTRY syscall_test			/* 0246 *//* io_destroy		*/
	_SVC_ENTRY syscall_test			/* 0247 *//* io_getevents	*/
	_SVC_ENTRY syscall_test			/* 0248 *//* io_submit		*/
	_SVC_ENTRY syscall_test			/* 0249 *//* io_cancel		*/
	_SVC_ENTRY syscall_test			/* 0250 *//* fadvise64		*/
	_SVC_ENTRY syscall_test			/* 0251 *//* reserved		*/
//	_SVC_ENTRY exit_group			/* 0252 *//* exit_group		*/
	_SVC_ENTRY exit_group		/* 0252 *//* exit_group		*/
	_SVC_ENTRY syscall_test			/* 0253 *//* lookup_dcookie	*/
	_SVC_ENTRY syscall_test			/* 0254 *//* epoll_create	*/
	_SVC_ENTRY syscall_test			/* 0255 *//* epoll_ctl		*/
	_SVC_ENTRY syscall_test			/* 0256 *//* epoll_wait		*/
	_SVC_ENTRY syscall_test			/* 0257 *//* remap_file_pages	*/
	_SVC_ENTRY syscall_test			/* 0258 *//* set_tid_address	*/
	_SVC_ENTRY syscall_test			/* 0259 *//* timer_create	*/
	_SVC_ENTRY syscall_test			/* 0260 *//* timer_settime	*/
	_SVC_ENTRY syscall_test			/* 0261 *//* timer_gettime	*/
	_SVC_ENTRY syscall_test			/* 0262 *//* timer_getoverrun	*/
	_SVC_ENTRY syscall_test			/* 0263 *//* timer_delete	*/
	_SVC_ENTRY syscall_test			/* 0264 *//* clock_settime	*/
	_SVC_ENTRY syscall_test			/* 0265 *//* clock_gettime	*/
	_SVC_ENTRY syscall_test			/* 0266 *//* clock_getres	*/
	_SVC_ENTRY syscall_test			/* 0267 *//* clock_nanosleep	*/
	_SVC_ENTRY syscall_test			/* 0268 *//* statfs64		*/
	_SVC_ENTRY syscall_test			/* 0269 *//* fstatfs64		*/
//	_SVC_ENTRY tgkill			/* 0270 *//* tgkill		*/
	_SVC_ENTRY syscall_test			/* 0270 *//* tgkill		*/
	_SVC_ENTRY syscall_test			/* 0271 *//* utimes		*/
	_SVC_ENTRY syscall_test			/* 0272 *//* fadvise64_64	*/
	_SVC_ENTRY syscall_test			/* 0273 *//* vserver		*/
	_SVC_ENTRY syscall_test			/* 0274 *//* mbind		*/
	_SVC_ENTRY syscall_test			/* 0275 *//* get_mempolicy	*/
	_SVC_ENTRY syscall_test			/* 0276 *//* set_mempolicy	*/
	_SVC_ENTRY syscall_test			/* 0277 *//* mq_open		*/
	_SVC_ENTRY syscall_test			/* 0278 *//* mq_unlink		*/
	_SVC_ENTRY syscall_test			/* 0279 *//* mq_timedsend	*/
	_SVC_ENTRY syscall_test			/* 0280 *//* mq_timedreceive	*/
	_SVC_ENTRY syscall_test			/* 0281 *//* mq_notify		*/
	_SVC_ENTRY syscall_test			/* 0282 *//* mq_getsetaddr	*/
	_SVC_ENTRY syscall_test			/* 0283 *//* kexec_load		*/
	_SVC_ENTRY syscall_test			/* 0284 *//* waitid		*/
	_SVC_ENTRY syscall_test			/* 0285 *//* reserved		*/
	_SVC_ENTRY syscall_test			/* 0286 *//* add_key		*/
	_SVC_ENTRY syscall_test			/* 0287 *//* request_key	*/
	_SVC_ENTRY syscall_test			/* 0288 *//* keyctl		*/
	_SVC_ENTRY syscall_test			/* 0289 *//* ioprio_set		*/
	_SVC_ENTRY syscall_test			/* 0290 *//* ioprio_get		*/
	_SVC_ENTRY syscall_test			/* 0291 *//* inotify_init	*/
	_SVC_ENTRY syscall_test			/* 0292 *//* inotify_add_watch	*/
	_SVC_ENTRY syscall_test			/* 0293 *//* inotify_rm_watch	*/
	_SVC_ENTRY syscall_test			/* 0294 *//* migrate_pages	*/
	_SVC_ENTRY openat			/* 0295 *//* openat		*/
	_SVC_ENTRY syscall_test			/* 0296 *//* mkdirat		*/
	_SVC_ENTRY syscall_test			/* 0297 *//* mknodat		*/
	_SVC_ENTRY syscall_test			/* 0298 *//* fchownat		*/
	_SVC_ENTRY syscall_test			/* 0299 *//* futimesat		*/
	_SVC_ENTRY syscall_test			/* 0300 *//* fstatat64		*/
	_SVC_ENTRY syscall_test			/* 0301 *//* unlinkat		*/
	_SVC_ENTRY syscall_test			/* 0302 *//* renameat		*/
	_SVC_ENTRY syscall_test			/* 0303 *//* linkat		*/
	_SVC_ENTRY syscall_test			/* 0304 *//* symlinkat		*/
	_SVC_ENTRY syscall_test			/* 0305 *//* readlinkat		*/
	_SVC_ENTRY syscall_test			/* 0306 *//* fchmodat		*/
	_SVC_ENTRY syscall_test			/* 0307 *//* faccessat		*/
	_SVC_ENTRY syscall_test			/* 0308 *//* pselect6		*/
	_SVC_ENTRY syscall_test			/* 0309 *//* ppoll		*/
	_SVC_ENTRY syscall_test			/* 0310 *//* unshare		*/
	_SVC_ENTRY syscall_test			/* 0311 *//* set_robust_list	*/
	_SVC_ENTRY syscall_test			/* 0312 *//* get_robust_list	*/
	_SVC_ENTRY syscall_test			/* 0313 *//* splice		*/
	_SVC_ENTRY syscall_test			/* 0314 *//* sync_file_range	*/
	_SVC_ENTRY syscall_test			/* 0315 *//* tee		*/
	_SVC_ENTRY syscall_test			/* 0316 *//* vmsplice		*/
	_SVC_ENTRY syscall_test			/* 0317 *//* move_pages		*/
	_SVC_ENTRY syscall_test			/* 0318 *//* getcpu		*/
	_SVC_ENTRY syscall_test			/* 0319 *//* epoll_pwait	*/
	_SVC_ENTRY syscall_test			/* 0320 *//* utimensat		*/
	_SVC_ENTRY syscall_test			/* 0321 *//* signalfd		*/
	_SVC_ENTRY syscall_test			/* 0322 *//* timerfd_create	*/
	_SVC_ENTRY syscall_test			/* 0323 *//* eventfd		*/
	_SVC_ENTRY syscall_test			/* 0324 *//* fallocate		*/
	_SVC_ENTRY syscall_test			/* 0325 *//* timerfd_settime	*/
	_SVC_ENTRY syscall_test			/* 0326 *//* timerfd_gettime	*/
	_SVC_ENTRY syscall_test			/* 0327 *//* signalfd4		*/
	_SVC_ENTRY syscall_test			/* 0328 *//* eventfd2		*/
	_SVC_ENTRY syscall_test			/* 0329 *//* epoll_create1	*/
	_SVC_ENTRY syscall_test			/* 0330 *//* dup3		*/
	_SVC_ENTRY syscall_test			/* 0331 *//* pipe2		*/
	_SVC_ENTRY syscall_test			/* 0332 *//* inotify_init1	*/
	_SVC_ENTRY syscall_test			/* 0333 *//* preadv		*/
	_SVC_ENTRY syscall_test			/* 0334 *//* pwritev		*/
	_SVC_ENTRY syscall_test			/* 0335 *//* rt_tgsigqueueinfo	*/
	_SVC_ENTRY syscall_test			/* 0336 *//* perf_event_open	*/
	_SVC_ENTRY syscall_test			/* 0337 *//* recvmmsg		*/
	_SVC_ENTRY syscall_test			/* 0338 *//* fanotify_init	*/
	_SVC_ENTRY syscall_test			/* 0339 *//* fanotify_mark	*/
	_SVC_ENTRY syscall_test			/* 0340 *//* prlimit64		*/
	_SVC_ENTRY syscall_test			/* 0341 *//* name_to_handle_at	*/
	_SVC_ENTRY syscall_test			/* 0342 *//* open_by_handle_at	*/
	_SVC_ENTRY syscall_test			/* 0343 *//* clock_adjtime	*/
	_SVC_ENTRY syscall_test			/* 0344 *//* syncfs		*/
	_SVC_ENTRY syscall_test			/* 0345 *//* sendmmsg		*/
	_SVC_ENTRY syscall_test			/* 0346 *//* setns		*/
	_SVC_ENTRY syscall_test			/* 0347 *//* process_vm_readv	*/
	_SVC_ENTRY syscall_test			/* 0348 *//* process_vm_writev	*/


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


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
