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

#include <typedef.h>
#include <cpu/x86/syscall_table.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
EXPORT int
syscall_test(int ebx, int ecx, int edx, int esi, int edi, int ebp, int eax);

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


unsigned long syscall_table[SYSCALL_NUM] =
{
	(unsigned long)&syscall_test,		/* 0000 *//* restart_syscall	*/
	(unsigned long)&syscall_test,		/* 0001 *//* exit		*/
	(unsigned long)&syscall_test,		/* 0002 *//* fork		*/
	(unsigned long)&syscall_test,		/* 0003 *//* read		*/
	(unsigned long)&syscall_test,		/* 0004 *//* write		*/
	(unsigned long)&syscall_test,		/* 0005 *//* open		*/
	(unsigned long)&syscall_test,		/* 0006 *//* close		*/
	(unsigned long)&syscall_test,		/* 0007 *//* waitpid		*/
	(unsigned long)&syscall_test,		/* 0008 *//* creat		*/
	(unsigned long)&syscall_test,		/* 0009 *//* link		*/
	(unsigned long)&syscall_test,		/* 0010 *//* unlink		*/
	(unsigned long)&syscall_test,		/* 0011 *//* execve		*/
	(unsigned long)&syscall_test,		/* 0012 *//* chdir		*/
	(unsigned long)&syscall_test,		/* 0013 *//* time		*/
	(unsigned long)&syscall_test,		/* 0014 *//* mknod		*/
	(unsigned long)&syscall_test,		/* 0015 *//* chmod		*/
	(unsigned long)&syscall_test,		/* 0016 *//* lchown		*/
	(unsigned long)&syscall_test,		/* 0017 *//* break		*/
	(unsigned long)&syscall_test,		/* 0018 *//* oldstat		*/
	(unsigned long)&syscall_test,		/* 0019 *//* lseek		*/
	(unsigned long)&syscall_test,		/* 0020 *//* getpid		*/
	(unsigned long)&syscall_test,		/* 0021 *//* mount		*/
	(unsigned long)&syscall_test,		/* 0022 *//* umount		*/
	(unsigned long)&syscall_test,		/* 0023 *//* setuid		*/
	(unsigned long)&syscall_test,		/* 0024 *//* getuid		*/
	(unsigned long)&syscall_test,		/* 0025 *//* stime		*/
	(unsigned long)&syscall_test,		/* 0026 *//* ptrace		*/
	(unsigned long)&syscall_test,		/* 0027 *//* alarm		*/
	(unsigned long)&syscall_test,		/* 0028 *//* oldfstat		*/
	(unsigned long)&syscall_test,		/* 0029 *//* pause		*/
	(unsigned long)&syscall_test,		/* 0030 *//* utime		*/
	(unsigned long)&syscall_test,		/* 0031 *//* stty		*/
	(unsigned long)&syscall_test,		/* 0032 *//* gtty		*/
	(unsigned long)&syscall_test,		/* 0033 *//* access		*/
	(unsigned long)&syscall_test,		/* 0034 *//* nice		*/
	(unsigned long)&syscall_test,		/* 0035 *//* ftime		*/
	(unsigned long)&syscall_test,		/* 0036 *//* sync		*/
	(unsigned long)&syscall_test,		/* 0037 *//* kill		*/
	(unsigned long)&syscall_test,		/* 0038 *//* rename		*/
	(unsigned long)&syscall_test,		/* 0039 *//* mkdir		*/
	(unsigned long)&syscall_test,		/* 0040 *//* rmdir		*/
	(unsigned long)&syscall_test,		/* 0041 *//* dup		*/
	(unsigned long)&syscall_test,		/* 0042 *//* pipe		*/
	(unsigned long)&syscall_test,		/* 0043 *//* times		*/
	(unsigned long)&syscall_test,		/* 0044 *//* prof		*/
	(unsigned long)&syscall_test,		/* 0045 *//* brk		*/
	(unsigned long)&syscall_test,		/* 0046 *//* setgid		*/
	(unsigned long)&syscall_test,		/* 0047 *//* getgid		*/
	(unsigned long)&syscall_test,		/* 0048 *//* signal		*/
	(unsigned long)&syscall_test,		/* 0049 *//* geteuid		*/
	(unsigned long)&syscall_test,		/* 0050 *//* getegid		*/
	(unsigned long)&syscall_test,		/* 0051 *//* acct		*/
	(unsigned long)&syscall_test,		/* 0052 *//* umount2		*/
	(unsigned long)&syscall_test,		/* 0053 *//* lock		*/
	(unsigned long)&syscall_test,		/* 0054 *//* ioctl		*/
	(unsigned long)&syscall_test,		/* 0055 *//* fcntl		*/
	(unsigned long)&syscall_test,		/* 0056 *//* mpx		*/
	(unsigned long)&syscall_test,		/* 0057 *//* setpgid		*/
	(unsigned long)&syscall_test,		/* 0058 *//* ulimit		*/
	(unsigned long)&syscall_test,		/* 0059 *//* oldolduname	*/
	(unsigned long)&syscall_test,		/* 0060 *//* umask		*/
	(unsigned long)&syscall_test,		/* 0061 *//* chroot		*/
	(unsigned long)&syscall_test,		/* 0062 *//* ustat		*/
	(unsigned long)&syscall_test,		/* 0063 *//* dup2		*/
	(unsigned long)&syscall_test,		/* 0064 *//* getppid		*/
	(unsigned long)&syscall_test,		/* 0065 *//* getpgrp		*/
	(unsigned long)&syscall_test,		/* 0066 *//* setsid		*/
	(unsigned long)&syscall_test,		/* 0067 *//* sigaction		*/
	(unsigned long)&syscall_test,		/* 0068 *//* sgetmask		*/
	(unsigned long)&syscall_test,		/* 0069 *//* ssetmask		*/
	(unsigned long)&syscall_test,		/* 0070 *//* setreuid		*/
	(unsigned long)&syscall_test,		/* 0071 *//* setregid		*/
	(unsigned long)&syscall_test,		/* 0072 *//* sigsuspend		*/
	(unsigned long)&syscall_test,		/* 0073 *//* sigpending		*/
	(unsigned long)&syscall_test,		/* 0074 *//* sethostname	*/
	(unsigned long)&syscall_test,		/* 0075 *//* setrlimit		*/
	(unsigned long)&syscall_test,		/* 0076 *//* getrlimit		*/
	(unsigned long)&syscall_test,		/* 0077 *//* getrusage		*/
	(unsigned long)&syscall_test,		/* 0078 *//* gettimeofday	*/
	(unsigned long)&syscall_test,		/* 0079 *//* settimeofday	*/
	(unsigned long)&syscall_test,		/* 0080 *//* getgroups		*/
	(unsigned long)&syscall_test,		/* 0081 *//* setgroups		*/
	(unsigned long)&syscall_test,		/* 0082 *//* select		*/
	(unsigned long)&syscall_test,		/* 0083 *//* symlink		*/
	(unsigned long)&syscall_test,		/* 0084 *//* oldlstat		*/
	(unsigned long)&syscall_test,		/* 0085 *//* readlink		*/
	(unsigned long)&syscall_test,		/* 0086 *//* uselib		*/
	(unsigned long)&syscall_test,		/* 0087 *//* swapon		*/
	(unsigned long)&syscall_test,		/* 0088 *//* reboot		*/
	(unsigned long)&syscall_test,		/* 0089 *//* readdir		*/
	(unsigned long)&syscall_test,		/* 0090 *//* mmap		*/
	(unsigned long)&syscall_test,		/* 0091 *//* munmap		*/
	(unsigned long)&syscall_test,		/* 0092 *//* truncate		*/
	(unsigned long)&syscall_test,		/* 0093 *//* ftruncate		*/
	(unsigned long)&syscall_test,		/* 0094 *//* fchmod		*/
	(unsigned long)&syscall_test,		/* 0095 *//* fchown		*/
	(unsigned long)&syscall_test,		/* 0096 *//* getpriority	*/
	(unsigned long)&syscall_test,		/* 0097 *//* setpriority	*/
	(unsigned long)&syscall_test,		/* 0098 *//* profil		*/
	(unsigned long)&syscall_test,		/* 0099 *//* statfs		*/
	(unsigned long)&syscall_test,		/* 0100 *//* fstatfs		*/
	(unsigned long)&syscall_test,		/* 0101 *//* ioperm		*/
	(unsigned long)&syscall_test,		/* 0102 *//* socketcall		*/
	(unsigned long)&syscall_test,		/* 0103 *//* syslog		*/
	(unsigned long)&syscall_test,		/* 0104 *//* setitimer		*/
	(unsigned long)&syscall_test,		/* 0105 *//* getitimer		*/
	(unsigned long)&syscall_test,		/* 0106 *//* stat		*/
	(unsigned long)&syscall_test,		/* 0107 *//* lstat		*/
	(unsigned long)&syscall_test,		/* 0108 *//* fstat		*/
	(unsigned long)&syscall_test,		/* 0109 *//* olduname		*/
	(unsigned long)&syscall_test,		/* 0110 *//* iopl		*/
	(unsigned long)&syscall_test,		/* 0111 *//* vhangup		*/
	(unsigned long)&syscall_test,		/* 0112 *//* idle		*/
	(unsigned long)&syscall_test,		/* 0113 *//* vm86old		*/
	(unsigned long)&syscall_test,		/* 0114 *//* wait4		*/
	(unsigned long)&syscall_test,		/* 0115 *//* swapoff		*/
	(unsigned long)&syscall_test,		/* 0116 *//* sysinfo		*/
	(unsigned long)&syscall_test,		/* 0117 *//* ipc		*/
	(unsigned long)&syscall_test,		/* 0118 *//* fsync		*/
	(unsigned long)&syscall_test,		/* 0119 *//* sigreturn		*/
	(unsigned long)&syscall_test,		/* 0120 *//* clone		*/
	(unsigned long)&syscall_test,		/* 0121 *//* setdomainname	*/
	(unsigned long)&syscall_test,		/* 0122 *//* uname		*/
	(unsigned long)&syscall_test,		/* 0123 *//* modify_ldt		*/
	(unsigned long)&syscall_test,		/* 0124 *//* adjtimex		*/
	(unsigned long)&syscall_test,		/* 0125 *//* mprotect		*/
	(unsigned long)&syscall_test,		/* 0126 *//* sigprocmask	*/
	(unsigned long)&syscall_test,		/* 0127 *//* create_module	*/
	(unsigned long)&syscall_test,		/* 0128 *//* init_module	*/
	(unsigned long)&syscall_test,		/* 0129 *//* delete_module	*/
	(unsigned long)&syscall_test,		/* 0130 *//* get_kernel_syms	*/
	(unsigned long)&syscall_test,		/* 0131 *//* quotactl		*/
	(unsigned long)&syscall_test,		/* 0132 *//* getpgid		*/
	(unsigned long)&syscall_test,		/* 0133 *//* fchdir		*/
	(unsigned long)&syscall_test,		/* 0134 *//* bdflush		*/
	(unsigned long)&syscall_test,		/* 0135 *//* sysfs		*/
	(unsigned long)&syscall_test,		/* 0136 *//* personality	*/
	(unsigned long)&syscall_test,		/* 0137 *//* afs_syscall	*/
	(unsigned long)&syscall_test,		/* 0138 *//* setfsuid		*/
	(unsigned long)&syscall_test,		/* 0139 *//* setfsgid		*/
	(unsigned long)&syscall_test,		/* 0140 *//* _llseek		*/
	(unsigned long)&syscall_test,		/* 0141 *//* getdents		*/
	(unsigned long)&syscall_test,		/* 0142 *//* _newselect		*/
	(unsigned long)&syscall_test,		/* 0143 *//* flock		*/
	(unsigned long)&syscall_test,		/* 0144 *//* msync		*/
	(unsigned long)&syscall_test,		/* 0145 *//* readv		*/
	(unsigned long)&syscall_test,		/* 0146 *//* writev		*/
	(unsigned long)&syscall_test,		/* 0147 *//* getsid		*/
	(unsigned long)&syscall_test,		/* 0148 *//* fdatasync		*/
	(unsigned long)&syscall_test,		/* 0149 *//* _sysctl		*/
	(unsigned long)&syscall_test,		/* 0150 *//* mlock		*/
	(unsigned long)&syscall_test,		/* 0151 *//* munlock		*/
	(unsigned long)&syscall_test,		/* 0152 *//* mlockall		*/
	(unsigned long)&syscall_test,		/* 0153 *//* munlockall		*/
	(unsigned long)&syscall_test,		/* 0154 *//* sched_setparam	*/
	(unsigned long)&syscall_test,		/* 0155 *//* sched_getparam	*/
	(unsigned long)&syscall_test,		/* 0156 *//* sched_setscheduler	*/
	(unsigned long)&syscall_test,		/* 0157 *//* sched_getscheduler	*/
	(unsigned long)&syscall_test,		/* 0158 *//* sched_yield	*/
	(unsigned long)&syscall_test,		/* 0159 *//* sched_get_priority_max	*/
	(unsigned long)&syscall_test,		/* 0160 *//* sched_get_priority_min	*/
	(unsigned long)&syscall_test,		/* 0161 *//* sched_rr_get_interval	*/
	(unsigned long)&syscall_test,		/* 0162 *//* nanosleep		*/
	(unsigned long)&syscall_test,		/* 0163 *//* mremap		*/
	(unsigned long)&syscall_test,		/* 0164 *//* setresuid		*/
	(unsigned long)&syscall_test,		/* 0165 *//* getresuid		*/
	(unsigned long)&syscall_test,		/* 0166 *//* vm86		*/
	(unsigned long)&syscall_test,		/* 0167 *//* query_module	*/
	(unsigned long)&syscall_test,		/* 0168 *//* poll		*/
	(unsigned long)&syscall_test,		/* 0169 *//* nfsservctl		*/
	(unsigned long)&syscall_test,		/* 0170 *//* setresgid		*/
	(unsigned long)&syscall_test,		/* 0171 *//* getresgid		*/
	(unsigned long)&syscall_test,		/* 0172 *//* prctl		*/
	(unsigned long)&syscall_test,		/* 0173 *//* rt_sigreturn	*/
	(unsigned long)&syscall_test,		/* 0174 *//* rt_sigaction	*/
	(unsigned long)&syscall_test,		/* 0175 *//* rt_sigprocmask	*/
	(unsigned long)&syscall_test,		/* 0176 *//* rt_sigpending	*/
	(unsigned long)&syscall_test,		/* 0177 *//* rt_sigtimedwait	*/
	(unsigned long)&syscall_test,		/* 0178 *//* rt_sigqueueinfo	*/
	(unsigned long)&syscall_test,		/* 0179 *//* rt_sigsuspend	*/
	(unsigned long)&syscall_test,		/* 0180 *//* pread64		*/
	(unsigned long)&syscall_test,		/* 0181 *//* pwrite64		*/
	(unsigned long)&syscall_test,		/* 0182 *//* chown		*/
	(unsigned long)&syscall_test,		/* 0183 *//* getcwd		*/
	(unsigned long)&syscall_test,		/* 0184 *//* capget		*/
	(unsigned long)&syscall_test,		/* 0185 *//* capset		*/
	(unsigned long)&syscall_test,		/* 0186 *//* sigaltstack	*/
	(unsigned long)&syscall_test,		/* 0187 *//* sendfile		*/
	(unsigned long)&syscall_test,		/* 0188 *//* getpmsg		*/
	(unsigned long)&syscall_test,		/* 0189 *//* putpmsg		*/
	(unsigned long)&syscall_test,		/* 0190 *//* vfork		*/
	(unsigned long)&syscall_test,		/* 0191 *//* ugetrlimit		*/
	(unsigned long)&syscall_test,		/* 0192 *//* mmap2		*/
	(unsigned long)&syscall_test,		/* 0193 *//* truncate64		*/
	(unsigned long)&syscall_test,		/* 0194 *//* ftruncate64	*/
	(unsigned long)&syscall_test,		/* 0195 *//* stat64		*/
	(unsigned long)&syscall_test,		/* 0196 *//* lstat64		*/
	(unsigned long)&syscall_test,		/* 0197 *//* fstat64		*/
	(unsigned long)&syscall_test,		/* 0198 *//* lchown32		*/
	(unsigned long)&syscall_test,		/* 0199 *//* getuid32		*/
	(unsigned long)&syscall_test,		/* 0200 *//* getgid32		*/
	(unsigned long)&syscall_test,		/* 0201 *//* geteuid32		*/
	(unsigned long)&syscall_test,		/* 0202 *//* getegid32		*/
	(unsigned long)&syscall_test,		/* 0203 *//* setreuid32		*/
	(unsigned long)&syscall_test,		/* 0204 *//* setregid32		*/
	(unsigned long)&syscall_test,		/* 0205 *//* getgroups32	*/
	(unsigned long)&syscall_test,		/* 0206 *//* setgroups32	*/
	(unsigned long)&syscall_test,		/* 0207 *//* fchown32		*/
	(unsigned long)&syscall_test,		/* 0208 *//* setresuid32	*/
	(unsigned long)&syscall_test,		/* 0209 *//* getresuid32	*/
	(unsigned long)&syscall_test,		/* 0210 *//* setresgid32	*/
	(unsigned long)&syscall_test,		/* 0211 *//* getresgid32	*/
	(unsigned long)&syscall_test,		/* 0212 *//* chown32		*/
	(unsigned long)&syscall_test,		/* 0213 *//* setuid32		*/
	(unsigned long)&syscall_test,		/* 0214 *//* setgid32		*/
	(unsigned long)&syscall_test,		/* 0215 *//* setfsuid32		*/
	(unsigned long)&syscall_test,		/* 0216 *//* setfsgid32		*/
	(unsigned long)&syscall_test,		/* 0217 *//* pivot_root		*/
	(unsigned long)&syscall_test,		/* 0218 *//* mincore		*/
	(unsigned long)&syscall_test,		/* 0219 *//* madvise		*/
	(unsigned long)&syscall_test,		/* 0220 *//* getdents64		*/
	(unsigned long)&syscall_test,		/* 0221 *//* fcntl64		*/
	(unsigned long)&syscall_test,		/* 0222 *//* reserved		*/
	(unsigned long)&syscall_test,		/* 0223 *//* reserved		*/
	(unsigned long)&syscall_test,		/* 0224 *//* gettid		*/
	(unsigned long)&syscall_test,		/* 0225 *//* readahead		*/
	(unsigned long)&syscall_test,		/* 0226 *//* setxattr		*/
	(unsigned long)&syscall_test,		/* 0227 *//* lsetxattr		*/
	(unsigned long)&syscall_test,		/* 0228 *//* fsetxattr		*/
	(unsigned long)&syscall_test,		/* 0229 *//* getxattr		*/
	(unsigned long)&syscall_test,		/* 0230 *//* lgetxattr		*/
	(unsigned long)&syscall_test,		/* 0231 *//* fgetxattr		*/
	(unsigned long)&syscall_test,		/* 0232 *//* listxattr		*/
	(unsigned long)&syscall_test,		/* 0233 *//* llistxattr		*/
	(unsigned long)&syscall_test,		/* 0234 *//* flistxattr		*/
	(unsigned long)&syscall_test,		/* 0235 *//* removexattr	*/
	(unsigned long)&syscall_test,		/* 0236 *//* lremovexattr	*/
	(unsigned long)&syscall_test,		/* 0237 *//* fremovexattr	*/
	(unsigned long)&syscall_test,		/* 0238 *//* tkill		*/
	(unsigned long)&syscall_test,		/* 0239 *//* sendfile64		*/
	(unsigned long)&syscall_test,		/* 0240 *//* futex		*/
	(unsigned long)&syscall_test,		/* 0241 *//* sched_setaffinity	*/
	(unsigned long)&syscall_test,		/* 0242 *//* sched_getaffinity	*/
	(unsigned long)&syscall_test,		/* 0243 *//* set_thread_area	*/
	(unsigned long)&syscall_test,		/* 0244 *//* get_thread_area	*/
	(unsigned long)&syscall_test,		/* 0245 *//* io_setup		*/
	(unsigned long)&syscall_test,		/* 0246 *//* io_destroy		*/
	(unsigned long)&syscall_test,		/* 0247 *//* io_getevents	*/
	(unsigned long)&syscall_test,		/* 0248 *//* io_submit		*/
	(unsigned long)&syscall_test,		/* 0249 *//* io_cancel		*/
	(unsigned long)&syscall_test,		/* 0250 *//* fadvise64		*/
	(unsigned long)&syscall_test,		/* 0251 *//* reserved		*/
	(unsigned long)&syscall_test,		/* 0252 *//* exit_group		*/
	(unsigned long)&syscall_test,		/* 0253 *//* lookup_dcookie	*/
	(unsigned long)&syscall_test,		/* 0254 *//* epoll_create	*/
	(unsigned long)&syscall_test,		/* 0255 *//* epoll_ctl		*/
	(unsigned long)&syscall_test,		/* 0256 *//* epoll_wait		*/
	(unsigned long)&syscall_test,		/* 0257 *//* remap_file_pages	*/
	(unsigned long)&syscall_test,		/* 0258 *//* set_tid_address	*/
	(unsigned long)&syscall_test,		/* 0259 *//* timer_create	*/
	(unsigned long)&syscall_test,		/* 0260 *//* timer_settime	*/
	(unsigned long)&syscall_test,		/* 0261 *//* timer_gettime	*/
	(unsigned long)&syscall_test,		/* 0262 *//* timer_getoverrun	*/
	(unsigned long)&syscall_test,		/* 0263 *//* timer_delete	*/
	(unsigned long)&syscall_test,		/* 0264 *//* clock_settime	*/
	(unsigned long)&syscall_test,		/* 0265 *//* clock_gettime	*/
	(unsigned long)&syscall_test,		/* 0266 *//* clock_getres	*/
	(unsigned long)&syscall_test,		/* 0267 *//* clock_nanosleep	*/
	(unsigned long)&syscall_test,		/* 0268 *//* statfs64		*/
	(unsigned long)&syscall_test,		/* 0269 *//* fstatfs64		*/
	(unsigned long)&syscall_test,		/* 0270 *//* tgkill		*/
	(unsigned long)&syscall_test,		/* 0271 *//* utimes		*/
	(unsigned long)&syscall_test,		/* 0272 *//* fadvise64_64	*/
	(unsigned long)&syscall_test,		/* 0273 *//* vserver		*/
	(unsigned long)&syscall_test,		/* 0274 *//* mbind		*/
	(unsigned long)&syscall_test,		/* 0275 *//* get_mempolicy	*/
	(unsigned long)&syscall_test,		/* 0276 *//* set_mempolicy	*/
	(unsigned long)&syscall_test,		/* 0277 *//* mq_open		*/
	(unsigned long)&syscall_test,		/* 0278 *//* mq_unlink		*/
	(unsigned long)&syscall_test,		/* 0279 *//* mq_timedsend	*/
	(unsigned long)&syscall_test,		/* 0280 *//* mq_timedreceive	*/
	(unsigned long)&syscall_test,		/* 0281 *//* mq_notify		*/
	(unsigned long)&syscall_test,		/* 0282 *//* mq_getsetaddr	*/
	(unsigned long)&syscall_test,		/* 0283 *//* kexec_load		*/
	(unsigned long)&syscall_test,		/* 0284 *//* waitid		*/
	(unsigned long)&syscall_test,		/* 0285 *//* reserved		*/
	(unsigned long)&syscall_test,		/* 0286 *//* add_key		*/
	(unsigned long)&syscall_test,		/* 0287 *//* request_key	*/
	(unsigned long)&syscall_test,		/* 0288 *//* keyctl		*/
	(unsigned long)&syscall_test,		/* 0289 *//* ioprio_set		*/
	(unsigned long)&syscall_test,		/* 0290 *//* ioprio_get		*/
	(unsigned long)&syscall_test,		/* 0291 *//* inotify_init	*/
	(unsigned long)&syscall_test,		/* 0292 *//* inotify_add_watch	*/
	(unsigned long)&syscall_test,		/* 0293 *//* inotify_rm_watch	*/
	(unsigned long)&syscall_test,		/* 0294 *//* migrate_pages	*/
	(unsigned long)&syscall_test,		/* 0295 *//* openat		*/
	(unsigned long)&syscall_test,		/* 0296 *//* mkdirat		*/
	(unsigned long)&syscall_test,		/* 0297 *//* mknodat		*/
	(unsigned long)&syscall_test,		/* 0298 *//* fchownat		*/
	(unsigned long)&syscall_test,		/* 0299 *//* futimesat		*/
	(unsigned long)&syscall_test,		/* 0300 *//* fstatat64		*/
	(unsigned long)&syscall_test,		/* 0301 *//* unlinkat		*/
	(unsigned long)&syscall_test,		/* 0302 *//* renameat		*/
	(unsigned long)&syscall_test,		/* 0303 *//* linkat		*/
	(unsigned long)&syscall_test,		/* 0304 *//* symlinkat		*/
	(unsigned long)&syscall_test,		/* 0305 *//* readlinkat		*/
	(unsigned long)&syscall_test,		/* 0306 *//* fchmodat		*/
	(unsigned long)&syscall_test,		/* 0307 *//* faccessat		*/
	(unsigned long)&syscall_test,		/* 0308 *//* pselect6		*/
	(unsigned long)&syscall_test,		/* 0309 *//* ppoll		*/
	(unsigned long)&syscall_test,		/* 0310 *//* unshare		*/
	(unsigned long)&syscall_test,		/* 0311 *//* set_robust_list	*/
	(unsigned long)&syscall_test,		/* 0312 *//* get_robust_list	*/
	(unsigned long)&syscall_test,		/* 0313 *//* splice		*/
	(unsigned long)&syscall_test,		/* 0314 *//* sync_file_range	*/
	(unsigned long)&syscall_test,		/* 0315 *//* tee		*/
	(unsigned long)&syscall_test,		/* 0316 *//* vmsplice		*/
	(unsigned long)&syscall_test,		/* 0317 *//* move_pages		*/
	(unsigned long)&syscall_test,		/* 0318 *//* getcpu		*/
	(unsigned long)&syscall_test,		/* 0319 *//* epoll_pwait	*/
	(unsigned long)&syscall_test,		/* 0320 *//* utimensat		*/
	(unsigned long)&syscall_test,		/* 0321 *//* signalfd		*/
	(unsigned long)&syscall_test,		/* 0322 *//* timerfd_create	*/
	(unsigned long)&syscall_test,		/* 0323 *//* eventfd		*/
	(unsigned long)&syscall_test,		/* 0324 *//* fallocate		*/
	(unsigned long)&syscall_test,		/* 0325 *//* timerfd_settime	*/
	(unsigned long)&syscall_test,		/* 0326 *//* timerfd_gettime	*/
	(unsigned long)&syscall_test,		/* 0327 *//* signalfd4		*/
	(unsigned long)&syscall_test,		/* 0328 *//* eventfd2		*/
	(unsigned long)&syscall_test,		/* 0329 *//* epoll_create1	*/
	(unsigned long)&syscall_test,		/* 0330 *//* dup3		*/
	(unsigned long)&syscall_test,		/* 0331 *//* pipe2		*/
	(unsigned long)&syscall_test,		/* 0332 *//* inotify_init1	*/
	(unsigned long)&syscall_test,		/* 0333 *//* preadv		*/
	(unsigned long)&syscall_test,		/* 0334 *//* pwritev		*/
	(unsigned long)&syscall_test,		/* 0335 *//* rt_tgsigqueueinfo	*/
	(unsigned long)&syscall_test,		/* 0336 *//* perf_event_open	*/
	(unsigned long)&syscall_test,		/* 0337 *//* recvmmsg		*/
	(unsigned long)&syscall_test,		/* 0338 *//* fanotify_init	*/
	(unsigned long)&syscall_test,		/* 0339 *//* fanotify_mark	*/
	(unsigned long)&syscall_test,		/* 0340 *//* prlimit64		*/
	(unsigned long)&syscall_test,		/* 0341 *//* name_to_handle_at	*/
	(unsigned long)&syscall_test,		/* 0342 *//* open_by_handle_at	*/
	(unsigned long)&syscall_test,		/* 0343 *//* clock_adjtime	*/
	(unsigned long)&syscall_test,		/* 0344 *//* syncfs		*/
	(unsigned long)&syscall_test,		/* 0345 *//* sendmmsg		*/
	(unsigned long)&syscall_test,		/* 0346 *//* setns		*/
	(unsigned long)&syscall_test,		/* 0347 *//* process_vm_readv	*/
	(unsigned long)&syscall_test,		/* 0348 *//* process_vm_writev	*/
};


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
EXPORT int
syscall_test(int ebx, int ecx, int edx, int esi, int edi, int ebp, int eax)
{
	printf("\nsyscall!!!!!!!!!!!\n");
	printf("eax = %d\n", eax);
	for(;;);
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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
