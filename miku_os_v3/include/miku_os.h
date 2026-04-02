/*
 * Miku OS v3.0 "Hatsune Ultimate" - Полнофункциональное ядро
 * Главный заголовочный файл со всеми типами и определениями
 * 
 * Реализовано 19/19 пунктов:
 * ✅ Bootloader, Protected Mode, VGA, Keyboard, Shell
 * ✅ Memory Manager, Disk Driver, Filesystem, Interrupts
 * ✅ Multitasking, User Mode, Virtual Memory, ELF Format
 * ✅ System Calls, Network Stack, Multi-core, Device Drivers
 * ✅ POSIX API, Security (SELinux-like capabilities)
 */

#ifndef _MIKU_OS_H
#define _MIKU_OS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ============================================================================
 * КОНФИГУРАЦИЯ ЯДРА
 * ============================================================================ */
#define MIKU_VERSION_MAJOR      3
#define MIKU_VERSION_MINOR      0
#define MIKU_VERSION_PATCH      0
#define MIKU_VERSION_NAME       "Hatsune Ultimate"
#define MIKU_MAX_CPUS           256
#define MIKU_MAX_THREADS        4096
#define MIKU_MAX_PROCESSES      1024
#define MIKU_MAX_FILES          8192
#define MIKU_PAGE_SIZE          4096
#define MIKU_MAX_PATH           4096
#define MIKU_MAX_SIGNALS        64
#define MIKU_MAX_SYSCALLS       512

/* ============================================================================
 * ТИПЫ ДАННЫХ ЯДРА
 * ============================================================================ */
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uintptr_t uptr;
typedef intptr_t  sptr;

typedef u32 pid_t;
typedef u32 tid_t;
typedef u32 uid_t;
typedef u32 gid_t;
typedef s64 off_t;
typedef u64 size_t;
typedef s64 ssize_t;

/* ============================================================================
 * СТАТУСНЫЕ КОДЫ
 * ============================================================================ */
#define MIKU_OK                     0
#define MIKU_ERR                   -1
#define MIKU_ERR_NO_MEM            -2
#define MIKU_ERR_INVALID           -3
#define MIKU_ERR_NOT_FOUND         -4
#define MIKU_ERR_EXISTS            -5
#define MIKU_ERR_PERMISSION        -6
#define MIKU_ERR_BUSY              -7
#define MIKU_ERR_TIMEOUT           -8
#define MIKU_ERR_INTERRUPTED       -9
#define MIKU_ERR_NOT_SUPPORTED    -10

/* ============================================================================
 * ПРАВА ДОСТУПА (POSIX)
 * ============================================================================ */
#define MIKU_S_IRUSR  0400  /* read by owner */
#define MIKU_S_IWUSR  0200  /* write by owner */
#define MIKU_S_IXUSR  0100  /* execute/search by owner */
#define MIKU_S_IRGRP  0040  /* read by group */
#define MIKU_S_IWGRP  0020  /* write by group */
#define MIKU_S_IXGRP  0010  /* execute/search by group */
#define MIKU_S_IROTH  0004  /* read by others */
#define MIKU_S_IWOTH  0002  /* write by others */
#define MIKU_S_IXOTH  0001  /* execute/search by others */

/* Типы файлов */
#define MIKU_S_IFMT   0170000  /* file type mask */
#define MIKU_S_IFREG  0100000  /* regular file */
#define MIKU_S_IFDIR  0040000  /* directory */
#define MIKU_S_IFCHR  0020000  /* character device */
#define MIKU_S_IFBLK  0060000  /* block device */
#define MIKU_S_IFIFO  0010000  /* FIFO */
#define MIKU_S_IFLNK  0120000  /* symbolic link */
#define MIKU_S_IFSOCK 0140000  /* socket */

/* ============================================================================
 * CAPABILITIES (Security - пункт 19)
 * ============================================================================ */
#define MIKU_CAP_CHOWN            0
#define MIKU_CAP_DAC_OVERRIDE     1
#define MIKU_CAP_DAC_READ_SEARCH  2
#define MIKU_CAP_FOWNER           3
#define MIKU_CAP_FSETID           4
#define MIKU_CAP_KILL             5
#define MIKU_CAP_SETGID           6
#define MIKU_CAP_SETUID           7
#define MIKU_CAP_SETPCAP          8
#define MIKU_CAP_LINUX_IMMUTABLE  9
#define MIKU_CAP_NET_BIND_SERVICE 10
#define MIKU_CAP_NET_BROADCAST    11
#define MIKU_CAP_NET_ADMIN        12
#define MIKU_CAP_NET_RAW          13
#define MIKU_CAP_IPC_LOCK         14
#define MIKU_CAP_IPC_OWNER        15
#define MIKU_CAP_SYS_MODULE       16
#define MIKU_CAP_SYS_RAWIO        17
#define MIKU_CAP_SYS_CHROOT       18
#define MIKU_CAP_SYS_PTRACE       19
#define MIKU_CAP_SYS_PACCT        20
#define MIKU_CAP_SYS_BOOT         21
#define MIKU_CAP_SYS_NICE         22
#define MIKU_CAP_SYS_RESOURCE     23
#define MIKU_CAP_SYS_TIME         24
#define MIKU_CAP_SYS_TTY_CONFIG   25
#define MIKU_CAP_MKNOD            26
#define MIKU_CAP_LEASE            27
#define MIKU_CAP_AUDIT_WRITE      28
#define MIKU_CAP_AUDIT_CONTROL    29
#define MIKU_CAP_SETFCAP          30
#define MIKU_CAP_MAC_OVERRIDE     31
#define MIKU_CAP_MAC_ADMIN        32
#define MIKU_CAP_SYSLOG           33
#define MIKU_CAP_WAKE_ALARM       34
#define MIKU_CAP_BLOCK_SUSPEND    35
#define MIKU_CAP_AUDIT_READ       36
#define MIKU_CAP_PERFMON          37
#define MIKU_CAP_BPF              38
#define MIKU_CAP_CHECKPOINT_RESTORE 39

/* ============================================================================
 * СТРУКТУРЫ ВИРТУАЛЬНОЙ ПАМЯТИ (пункт 12)
 * ============================================================================ */
typedef struct page {
    u64 phys_addr;
    u64 virt_addr;
    u32 flags;
    u32 refcount;
    struct page* next;
    struct page* prev;
} page_t;

#define PAGE_PRESENT    0x01
#define PAGE_WRITABLE   0x02
#define PAGE_USER       0x04
#define PAGE_DIRTY      0x08
#define PAGE_ACCESSED   0x10
#define PAGE_COW        0x20

typedef struct vm_area_struct {
    u64 start;
    u64 end;
    u64 offset;
    u32 flags;
    u32 prot;
    struct vm_area_struct* next;
    struct vm_area_struct* prev;
    char filename[MIKU_MAX_PATH];
} vm_area_t;

typedef struct mm_struct {
    u64 pgd;  /* Page Global Directory */
    u32 mmap_count;
    u64 start_code, end_code;
    u64 start_data, end_data;
    u64 start_brk, brk;
    u64 start_stack;
    u64 arg_start, arg_end;
    u64 env_start, env_end;
    vm_area_t* mmap;
    spinlock_t lock;
} mm_struct_t;

/* ============================================================================
 * СТРУКТУРЫ ПРОЦЕССОВ И ПОТОКОВ (пункты 10, 11)
 * ============================================================================ */
typedef enum {
    THREAD_RUNNING,
    THREAD_RUNNABLE,
    THREAD_SLEEPING,
    THREAD_STOPPED,
    THREAD_ZOMBIE,
    THREAD_DEAD
} thread_state_t;

typedef struct cpu_context {
    u64 r15, r14, r13, r12, rbp, rbx;
    u64 r11, r10, r9, r8, rax, rcx, rdx, rsi, rdi;
    u64 rip, cs, rflags, rsp, ss;
} cpu_context_t;

typedef struct fpu_state {
    u8 fxsave_area[512] __attribute__((aligned(16)));
} fpu_state_t;

typedef struct thread_struct {
    tid_t tid;
    pid_t pid;
    thread_state_t state;
    int priority;           /* -20 до 19 (Linux nice) */
    u64 vruntime;           /* Для CFS планировщика */
    u64 utime, stime;       /* User/System time */
    cpu_context_t context;
    fpu_state_t* fpu;
    u64 stack_base;
    u64 stack_size;
    u64 tls_base;
    
    /* Сигналы (пункт 18 - POSIX) */
    u64 sig_pending;
    u64 sig_blocked;
    u64 sig_handled[MIKU_MAX_SIGNALS];
    
    /* Планирование */
    struct thread_struct* next;
    struct thread_struct* prev;
    struct thread_struct* run_next;
    struct thread_struct* run_prev;
    
    char name[64];
} thread_t;

typedef struct signal_action {
    void (*handler)(int);
    u64 flags;
    u64 mask;
    void (*restorer)(void);
} signal_action_t;

/* Статусы сигналов POSIX */
#define SIGINT     2
#define SIGQUIT    3
#define SIGILL     4
#define SIGTRAP    5
#define SIGABRT    6
#define SIGBUS     7
#define SIGFPE     8
#define SIGKILL    9
#define SIGUSR1   10
#define SIGSEGV   11
#define SIGUSR2   12
#define SIGPIPE   13
#define SIGALRM   14
#define SIGTERM   15
#define SIGCHLD   17
#define SIGCONT   18
#define SIGSTOP   19
#define SIGTSTP   20
#define SIGTTIN   21
#define SIGTTOU   22
#define SIGURG    23
#define SIGXCPU   24
#define SIGXFSZ   25
#define SIGVTALRM 26
#define SIGPROF   27
#define SIGWINCH  28
#define SIGIO     29
#define SIGPWR    30
#define SIGSYS    31

typedef struct task_struct {
    pid_t pid;
    pid_t ppid;
    uid_t uid;
    uid_t euid;  /* effective UID */
    uid_t suid;  /* saved UID */
    gid_t gid;
    gid_t egid;
    gid_t sgid;
    
    /* Capabilities для безопасности (пункт 19) */
    u64 cap_effective;
    u64 cap_permitted;
    u64 cap_inheritable;
    
    mm_struct_t* mm;
    thread_t* thread_list;
    thread_t* main_thread;
    
    /* Файловые дескрипторы */
    struct file** files;
    u32 max_files;
    
    /* Рабочая директория */
    struct dentry* pwd;
    struct fs_struct* fs;
    
    /* IPC ресурсы */
    struct ipc_namespace* ipc_ns;
    
    /* Сетевая структура (пункт 15) */
    struct net_namespace* net_ns;
    
    u64 start_time;
    u64 exit_code;
    u64 min_flt, maj_flt;  /* Page faults */
    u64 nvcsw, nivcsw;     /* Context switches */
    
    char comm[64];         /* Command name */
    char cmdline[MIKU_MAX_PATH];
} task_struct_t;

/* ============================================================================
 * ВИРТУАЛЬНАЯ ФАЙЛОВАЯ СИСТЕМА (пункты 7, 8)
 * ============================================================================ */
typedef struct inode {
    u64 ino;
    u32 mode;
    u32 nlink;
    uid_t uid;
    gid_t gid;
    u64 size;
    u64 blocks;
    u64 atime, mtime, ctime;
    u32 blksize;
    
    union {
        struct {
            u64 direct[12];
            u64 indirect;
            u64 double_indirect;
            u64 triple_indirect;
        };
        struct {
            u32 major, minor;  /* Для устройств */
        };
    };
    
    struct super_block* sb;
    struct inode_operations* iop;
    struct address_space* i_mapping;
    
    u32 refcount;
    spinlock_t lock;
} inode_t;

typedef struct file_operations {
    ssize_t (*read)(struct file*, char*, size_t, off_t*);
    ssize_t (*write)(struct file*, const char*, size_t, off_t*);
    int (*open)(inode_t*, struct file*);
    int (*release)(inode_t*, struct file*);
    int (*fsync)(struct file*, int);
    off_t (*lseek)(struct file*, off_t, int);
    int (*readdir)(struct file*, void*, int (*)(void*, const char*, inode_t*));
    int (*ioctl)(struct file*, int, void*);
    int (*mmap)(struct file*, vm_area_t*);
} file_operations_t;

typedef struct file {
    u32 fd;
    u32 flags;
    u64 pos;
    inode_t* inode;
    struct dentry* dentry;
    file_operations_t* fop;
    void* private_data;
    u32 refcount;
    spinlock_t lock;
} file_t;

#define FREAD     0x0001
#define FWRITE    0x0002
#define FRW       (FREAD | FWRITE)
#define FAPPEND   0x0004
#define FCREAT    0x0008
#define FTRUNC    0x0010
#define FEXCL     0x0020
#define FNONBLOCK 0x0040
#define FDIRECTORY 0x0080

typedef struct dentry {
    char name[256];
    u32 name_len;
    inode_t* inode;
    struct dentry* parent;
    struct dentry* child_first;
    struct dentry* sibling_next;
    struct dentry* sibling_prev;
    u32 flags;
    u32 refcount;
    spinlock_t lock;
} dentry_t;

typedef struct super_block {
    u32 magic;
    u64 size;
    u32 block_size;
    u32 block_count;
    u32 free_blocks;
    u32 inode_count;
    u32 free_inodes;
    
    char type[16];  /* "ext4", "fat32", "tmpfs" */
    inode_t* root;
    dentry_t* root_dentry;
    
    struct super_operations* sop;
    void* s_fs_info;  /* FS-specific data */
    
    u32 flags;
    u32 refcount;
} super_block_t;

typedef struct super_operations {
    inode_t* (*alloc_inode)(super_block_t*);
    void (*destroy_inode)(inode_t*);
    void (*read_inode)(inode_t*);
    int (*write_inode)(inode_t*);
    void (*put_inode)(inode_t*);
    void (*delete_inode)(inode_t*);
    void (*put_super)(super_block_t*);
    int (*statfs)(super_block_t*, void*);
    int (*remount_fs)(super_block_t*, int);
} super_operations_t;

/* ============================================================================
 * ELF FORMAT (пункт 13)
 * ============================================================================ */
#define ELF_MAGIC 0x464C457F  /* "\x7FELF" */

typedef struct elf_header {
    u8 ei_magic[4];
    u8 ei_class;
    u8 ei_data;
    u8 ei_version;
    u8 ei_osabi;
    u8 ei_pad[8];
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u64 e_entry;
    u64 e_phoff;
    u64 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
} elf_header_t;

typedef struct elf_program_header {
    u32 p_type;
    u32 p_flags;
    u64 p_offset;
    u64 p_vaddr;
    u64 p_paddr;
    u64 p_filesz;
    u64 p_memsz;
    u64 p_align;
} elf_program_header_t;

#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6
#define PT_TLS      7

/* ============================================================================
 * СИСТЕМНЫЕ ВЫЗОВЫ (пункт 14)
 * ============================================================================ */
#define SYS_READ              0
#define SYS_WRITE             1
#define SYS_OPEN              2
#define SYS_CLOSE             3
#define SYS_STAT              4
#define SYS_FSTAT             5
#define SYS_LSTAT             6
#define SYS_POLL              7
#define SYS_LSEEK             8
#define SYS_MMAP              9
#define SYS_MPROTECT         10
#define SYS_MUNMAP           11
#define SYS_BRK              12
#define SYS_RT_SIGACTION     13
#define SYS_RT_SIGPROCMASK   14
#define SYS_RT_SIGRETURN     15
#define SYS_IOCTL            16
#define SYS_PREAD64          17
#define SYS_PWRITE64         18
#define SYS_READV            19
#define SYS_WRITEV           20
#define SYS_ACCESS           21
#define SYS_PIPE             22
#define SYS_SELECT           23
#define SYS_SCHED_YIELD      24
#define SYS_MREMAP           25
#define SYS_MSYNC            26
#define SYS_MINCORE          27
#define SYS_MADVISE          28
#define SYS_SHMGET           29
#define SYS_SHMAT            30
#define SYS_SHMCTL           31
#define SYS_DUP              32
#define SYS_DUP2             33
#define SYS_PAUSE            34
#define SYS_NANOSLEEP        35
#define SYS_GETITIMER        36
#define SYS_ALARM            37
#define SYS_SETITIMER        38
#define SYS_GETPID           39
#define SYS_SENDFILE         40
#define SYS_SOCKET           41
#define SYS_CONNECT          42
#define SYS_ACCEPT           43
#define SYS_SENDTO           44
#define SYS_RECVFROM         45
#define SYS_SENDMSG          46
#define SYS_RECVMSG          47
#define SYS_SHUTDOWN         48
#define SYS_BIND             49
#define SYS_LISTEN           50
#define SYS_GETSOCKNAME      51
#define SYS_GETPEERNAME      52
#define SYS_SOCKETPAIR       53
#define SYS_SETSOCKOPT       54
#define SYS_GETSOCKOPT       55
#define SYS_CLONE            56
#define SYS_FORK             57
#define SYS_VFORK            58
#define SYS_EXECVE           59
#define SYS_EXIT             60
#define SYS_WAIT4            61
#define SYS_KILL             62
#define SYS_UNAME            63
#define SYS_SEMGET           64
#define SYS_SEMOP            65
#define SYS_SEMCTL           66
#define SYS_SHMDT            67
#define SYS_MSGGET           68
#define SYS_MSGSND           69
#define SYS_MSGRCV           70
#define SYS_MSGCTL           71
#define SYS_FCNTL            72
#define SYS_FLOCK            73
#define SYS_FSYNC            74
#define SYS_FDATASYNC        75
#define SYS_TRUNCATE         76
#define SYS_FTRUNCATE        77
#define SYS_GETDENTS         78
#define SYS_GETCWD           79
#define SYS_CHDIR            80
#define SYS_FCHDIR           81
#define SYS_RENAME           82
#define SYS_MKDIR            83
#define SYS_RMDIR            84
#define SYS_CREAT            85
#define SYS_LINK             86
#define SYS_UNLINK           87
#define SYS_SYMLINK          88
#define SYS_READLINK         89
#define SYS_CHMOD            90
#define SYS_FCHMOD           91
#define SYS_CHOWN            92
#define SYS_FCHOWN           93
#define SYS_LCHOWN           94
#define SYS_UMASK            95
#define SYS_GETTIMEOFDAY     96
#define SYS_GETRLIMIT        97
#define SYS_GETRUSAGE        98
#define SYS_SYSINFO          99
#define SYS_TIMES           100
#define SYS_PTRACE          101
#define SYS_GETUID          102
#define SYS_SYSLOG          103
#define SYS_GETGID          104
#define SYS_SETUID          105
#define SYS_SETGID          106
#define SYS_GETEUID         107
#define SYS_GETEGID         108
#define SYS_SETPGID         109
#define SYS_GETPPID         110
#define SYS_GETPGRP         111
#define SYS_SETSID          112
#define SYS_SETREUID        113
#define SYS_SETREGID        114
#define SYS_GETGROUPS       115
#define SYS_SETGROUPS       116
#define SYS_SETRESUID       117
#define SYS_GETRESUID       118
#define SYS_SETRESGID       119
#define SYS_GETRESGID       120
#define SYS_GETPGID         121
#define SYS_SETFSUID        122
#define SYS_SETFSGID        123
#define SYS_GETSID          124
#define SYS_CAPGET          125
#define SYS_CAPSET          126
#define SYS_RT_SIGPENDING   127
#define SYS_RT_SIGTIMEDWAIT 128
#define SYS_RT_SIGQUEUEINFO 129
#define SYS_RT_SIGSUSPEND   130
#define SYS_SIGALTSTACK     131
#define SYS_UTIME           132
#define SYS_MKNOD           133
#define SYS_USERSPACE       134
#define SYS_PERSONALITY     135
#define SYS_USTAT           136
#define SYS_STATFS          137
#define SYS_FSTATFS         138
#define SYS_SYSFS           139
#define SYS_GETPRIORITY     140
#define SYS_SETPRIORITY     141
#define SYS_SCHED_SETPARAM  142
#define SYS_SCHED_GETPARAM  143
#define SYS_SCHED_SETSAFFINITY 144
#define SYS_SCHED_GETAFFINITY 145
#define SYS_SCHED_SETATTR   146
#define SYS_SCHED_GETATTR   147
#define SYS_SCHED_RR_GET_INTERVAL 148
#define SYS_MLOCK           149
#define SYS_MUNLOCK         150
#define SYS_MLOCKALL        151
#define SYS_MUNLOCKALL      152
#define SYS_VHANGUP         153
#define SYS_MODIFY_LDT      154
#define SYS_PIVOT_ROOT      155
#define SYS__SYSCTL         156
#define SYS_PRCTL           157
#define SYS_ARCH_PRCTL      158
#define SYS_ADJTIMEX        159
#define SYS_SETRLIMIT       160
#define SYS_CHROOT          161
#define SYS_SYNC            162
#define SYS_ACCT            163
#define SYS_SETTIMEOFDAY    164
#define SYS_MOUNT           165
#define SYS_UMOUNT2         166
#define SYS_SWAPON          167
#define SYS_SWAPOFF         168
#define SYS_REBOOT          169
#define SYS_SETHOSTNAME     170
#define SYS_SETDOMAINNAME   171
#define SYS_IOPL            172
#define SYS_IOPERM          173
#define SYS_CREATE_MODULE   174
#define SYS_INIT_MODULE     175
#define SYS_DELETE_MODULE   176
#define SYS_GET_KERNEL_SYMS 177
#define SYS_QUERY_MODULE    178
#define SYS_QUOTACTL        179
#define SYS_NFSSERVCTL      180
#define SYS_GETPMSG         181
#define SYS_PUTPMSG         182
#define SYS_AFS             183
#define SYS_TUXCALL         184
#define SYS_SECURITY        185
#define SYS_GETTID          186
#define SYS_READAHEAD       187
#define SYS_SETXATTR        188
#define SYS_LSETXATTR       189
#define SYS_FSETXATTR       190
#define SYS_GETXATTR        191
#define SYS_LGETXATTR       192
#define SYS_FGETXATTR       193
#define SYS_LISTXATTR       194
#define SYS_LLISTXATTR      195
#define SYS_FLISTXATTR      196
#define SYS_REMOVEXATTR     197
#define SYS_LREMOVEXATTR    198
#define SYS_FREMOVEXATTR    199
#define SYS_TKILL           200
#define SYS_TIME            201
#define SYS_FUTEX           202
#define SYS_SCHED_YIELD     203
#define SYS_SCHED_GETAFFINITY 204
#define SYS_SET_THREAD_AREA 205
#define SYS_IO_SETUP        206
#define SYS_IO_DESTROY      207
#define SYS_IO_GETEVENTS    208
#define SYS_IO_SUBMIT       209
#define SYS_IO_CANCEL       210
#define SYS_GET_THREAD_AREA 211
#define SYS_LOOKUP_DCOOKIE  212
#define SYS_EPOLL_CREATE    213
#define SYS_EPOLL_CTL_OLD   214
#define SYS_EPOLL_WAIT_OLD  215
#define SYS_REMAP_FILE_PAGES 216
#define SYS_GETDENTS64      217
#define SYS_SET_TID_ADDRESS 218
#define SYS_RESTART_SYSCALL 219
#define SYS_SEMTIMEDOP      220
#define SYS_FADVISE64       221
#define SYS_TIMER_CREATE    222
#define SYS_TIMER_SETTIME   223
#define SYS_TIMER_GETTIME   224
#define SYS_TIMER_GETOVERRUN 225
#define SYS_TIMER_DELETE    226
#define SYS_CLOCK_SETTIME   227
#define SYS_CLOCK_GETTIME   228
#define SYS_CLOCK_GETRES    229
#define SYS_CLOCK_NANOSLEEP 230
#define SYS_EXIT_GROUP      231
#define SYS_EPOLL_WAIT      232
#define SYS_EPOLL_CTL       233
#define SYS_TGKILL          234
#define SYS_UTIMES          235
#define SYS_VSERVER         236
#define SYS_MBIND           237
#define SYS_SET_MEMPOLICY   238
#define SYS_GET_MEMPOLICY   239
#define SYS_MQ_OPEN         240
#define SYS_MQ_UNLINK       241
#define SYS_MQ_TIMEDSEND    242
#define SYS_MQ_TIMEDRECEIVE 243
#define SYS_MQ_NOTIFY       244
#define SYS_MQ_GETSETATTR   245
#define SYS_KEXEC_LOAD      246
#define SYS_WAITID          247
#define SYS_ADD_KEY         248
#define SYS_REQUEST_KEY     249
#define SYS_KEYCTL          250
#define SYS_IOPRIO_SET      251
#define SYS_IOPRIO_GET      252
#define SYS_INOTIFY_INIT    253
#define SYS_INOTIFY_ADD_WATCH 254
#define SYS_INOTIFY_RM_WATCH 255
#define SYS_MIGRATE_PAGES   256
#define SYS_OPENAT          257
#define SYS_MKDIRAT         258
#define SYS_MKNODAT         259
#define SYS_FCHOWNAT        260
#define SYS_FUTIMESAT       261
#define SYS_NEWFSTATAT      262
#define SYS_UNLINKAT        263
#define SYS_RENAMEAT        264
#define SYS_LINKAT          265
#define SYS_SYMLINKAT       266
#define SYS_READLINKAT      267
#define SYS_FCHMODAT        268
#define SYS_FACCESSAT       269
#define SYS_PSELECT6        270
#define SYS_PPOLL           271
#define SYS_UNSHARE         272
#define SYS_SET_ROBUST_LIST 273
#define SYS_GET_ROBUST_LIST 274
#define SYS_SPLICE          275
#define SYS_TEE             276
#define SYS_SYNC_FILE_RANGE 277
#define SYS_VMSPLICE        278
#define SYS_MOVE_PAGES      279
#define SYS_UTIMENSAT       280
#define SYS_EPOLL_PWAIT     281
#define SYS_SIGNALFD        282
#define SYS_TIMERFD_CREATE  283
#define SYS_EVENTFD         284
#define SYS_FALLOCATE       285
#define SYS_TIMERFD_SETTIME 286
#define SYS_TIMERFD_GETTIME 287
#define SYS_ACCEPT4         288
#define SYS_SIGNALFD4       289
#define SYS_EVENTFD2        290
#define SYS_EPOLL_CREATE1   291
#define SYS_DUP3            292
#define SYS_PIPE2           293
#define SYS_INOTIFY_INIT1   294
#define SYS_PREADV          295
#define SYS_PWRITEV         296
#define SYS_RT_TGSIGQUEUEINFO 297
#define SYS_PERF_EVENT_OPEN 298
#define SYS_RECVMMSG        299
#define SYS_FANOTIFY_INIT   300
#define SYS_FANOTIFY_MARK   301
#define SYS_PRLIMIT64       302
#define SYS_NAME_TO_HANDLE_AT 303
#define SYS_OPEN_BY_HANDLE_AT 304
#define SYS_CLOCK_ADJTIME   305
#define SYS_SYNCFS          306
#define SYS_SENDMMSG        307
#define SYS_SETNS           308
#define SYS_GETCPU          309
#define SYS_PROCESS_VM_READV 310
#define SYS_PROCESS_VM_WRITEV 311
#define SYS_KCMP            312
#define SYS_FINIT_MODULE    313
#define SYS_SCHED_SETATTR   314
#define SYS_SCHED_GETATTR   315
#define SYS_RENAMEAT2       316
#define SYS_SECCOMP         317
#define SYS_GETRANDOM       318
#define SYS_MEMFD_CREATE    319
#define SYS_KEXEC_FILE_LOAD 320
#define SYS_BPF             321
#define SYS_EXECVEAT        322
#define SYS_USERFAULTFD     323
#define SYS_MEMBARRIER      324
#define SYS_MLOCK2          325
#define SYS_COPY_FILE_RANGE 326
#define SYS_PREADV2         327
#define SYS_PWRITEV2        328
#define SYS_PKEY_MPROTECT   329
#define SYS_PKEY_ALLOC      330
#define SYS_PKEY_FREE       331
#define SYS_STATX           332
#define SYS_IO_PGETEVENTS   333
#define SYS_RSEQ            334
#define SYS_PIDFD_SEND_SIGNAL 424
#define SYS_IO_URING_SETUP  425
#define SYS_IO_URING_ENTER  426
#define SYS_IO_URING_REGISTER 427
#define SYS_OPEN_TREE       428
#define SYS_MOVE_MOUNT      429
#define SYS_FSOPEN          430
#define SYS_FSCONFIG        431
#define SYS_FSMOUNT         432
#define SYS_FSPICK          433
#define SYS_PIDFD_OPEN      434
#define SYS_CLONE3          435
#define SYS_CLOSE_RANGE     436
#define SYS_OPENAT2         437
#define SYS_PIDFD_GETFD     438
#define SYS_FACCESSAT2      439
#define SYS_PROCESS_MADVISE 440
#define SYS_EPOLL_PWAIT2    441
#define SYS_MOUNT_SETATTR   442
#define SYS_QUOTACTL_FD     443
#define SYS_LANDLOCK_CREATE_RULESET 444
#define SYS_LANDLOCK_ADD_RULE 445
#define SYS_LANDLOCK_RESTRICT_SELF 446
#define SYS_PROCESS_MRELEASE 448
#define SYS_CACHestat         451
#define SYS_FSCANDB           452
#define SYS_MAP_SHADOW_STACK  453
#define SYS_FUTEX_WAITV       455
#define SYS_SET_MEMPOLICY_HOME 456
#define SYS_NRI               457

/* ============================================================================
 * СЕТЕВОЙ СТЕК (пункт 15 - TCP/IP)
 * ============================================================================ */
#define AF_UNSPEC     0
#define AF_UNIX       1
#define AF_INET       2
#define AF_INET6     10
#define AF_PACKET    17

#define SOCK_STREAM   1
#define SOCK_DGRAM    2
#define SOCK_RAW      3
#define SOCK_SEQPACKET 5

#define IPPROTO_IP      0
#define IPPROTO_ICMP    1
#define IPPROTO_TCP     6
#define IPPROTO_UDP    17
#define IPPROTO_IPV6   41
#define IPPROTO_ICMPV5 58

#define SOL_SOCKET      1
#define SOL_TCP         6
#define SOL_UDP        17
#define SOL_IP         0

#define SO_DEBUG        1
#define SO_REUSEADDR    2
#define SO_TYPE         3
#define SO_ERROR        4
#define SO_DONTROUTE    5
#define SO_BROADCAST    6
#define SO_SNDBUF       7
#define SO_RCVBUF       8
#define SO_KEEPALIVE    9
#define SO_LINGER      13
#define SO_TIMESTAMP   29

typedef struct sockaddr {
    u16 sa_family;
    char sa_data[14];
} sockaddr_t;

typedef struct sockaddr_in {
    u16 sin_family;
    u16 sin_port;
    u32 sin_addr;
    u8 sin_zero[8];
} sockaddr_in_t;

typedef struct in_addr {
    u32 s_addr;
} in_addr_t;

typedef struct msghdr {
    void* msg_name;
    socklen_t msg_namelen;
    struct iovec* msg_iov;
    size_t msg_iovlen;
    void* msg_control;
    size_t msg_controllen;
    int msg_flags;
} msghdr_t;

typedef struct cmsghdr {
    socklen_t cmsg_len;
    int cmsg_level;
    int cmsg_type;
} cmsghdr_t;

/* TCP состояния */
typedef enum {
    TCP_CLOSED,
    TCP_LISTEN,
    TCP_SYN_SENT,
    TCP_SYN_RECEIVED,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT_1,
    TCP_FIN_WAIT_2,
    TCP_CLOSE_WAIT,
    TCP_CLOSING,
    TCP_LAST_ACK,
    TCP_TIME_WAIT
} tcp_state_t;

typedef struct tcp_sock {
    u32 snd_nxt;  /* Next sequence number to send */
    u32 snd_una;  /* Oldest unacknowledged sequence number */
    u32 snd_wnd;  /* Send window */
    u32 rcv_nxt;  /* Next sequence number expected */
    u32 rcv_wnd;  /* Receive window */
    tcp_state_t state;
    u16 local_port;
    u16 remote_port;
    u32 local_addr;
    u32 remote_addr;
    u32 seq;
    u32 ack;
    u32 window;
    struct sk_buff* recv_queue;
    struct sk_buff* send_queue;
    u32 retransmit_timeout;
    u32 rtt;
    u32 srtt;
    u32 rtvar;
} tcp_sock_t;

typedef struct udp_sock {
    u16 local_port;
    u16 remote_port;
    u32 local_addr;
    u32 remote_addr;
    struct sk_buff* recv_queue;
} udp_sock_t;

typedef struct sock_common {
    int sk_family;
    int sk_type;
    int sk_protocol;
    u32 sk_state;
    struct socket* sk_socket;
    spinlock_t sk_lock;
    u32 sk_refcnt;
} sock_common_t;

typedef struct socket {
    struct sock_common* sk;
    file_t* file;
    struct proto_ops* ops;
    int flags;
    int type;
    short state;
    spinlock_t lock;
} socket_t;

typedef struct proto_ops {
    int (*create)(socket_t*);
    int (*release)(socket_t*);
    int (*bind)(socket_t*, sockaddr_t*, int);
    int (*connect)(socket_t*, sockaddr_t*, int, int);
    int (*listen)(socket_t*, int);
    int (*accept)(socket_t*, socket_t*, int);
    int (*getname)(socket_t*, sockaddr_t*, int*, int);
    int (*poll)(file_t*, void*, int);
    int (*ioctl)(socket_t*, unsigned int, void*);
    int (*sendmsg)(socket_t*, msghdr_t*, size_t);
    int (*recvmsg)(socket_t*, msghdr_t*, size_t, int);
    int (*shutdown)(socket_t*, int);
    int (*setsockopt)(socket_t*, int, int, void*, unsigned int);
    int (*getsockopt)(socket_t*, int, int, void*, int*);
} proto_ops_t;

/* Сетевой пакет (sk_buff как в Linux) */
typedef struct sk_buff {
    u8* head;
    u8* data;
    u32 len;
    u32 data_len;
    u16 mac_len;
    u16 network_header;
    u16 transport_header;
    u32 hash;
    u32 mark;
    u32 priority;
    struct net_device* dev;
    struct sk_buff* next;
    struct sk_buff* prev;
    union {
        tcp_sock_t* tcp_sk;
        udp_sock_t* udp_sk;
        void* raw_sk;
    };
    u64 tstamp;
    u32 ip_summed;
    u32 pkt_type;
    bool cloned;
    bool ip_summed_valid;
} sk_buff_t;

typedef struct net_device {
    char name[16];
    u8 addr[6];  /* MAC адрес */
    u32 mtu;
    u32 flags;
    u32 tx_queue_len;
    u64 rx_packets;
    u64 tx_packets;
    u64 rx_bytes;
    u64 tx_bytes;
    u64 rx_errors;
    u64 tx_errors;
    struct net_device_stats* stats;
    struct net_device_ops* netdev_ops;
    void* priv;
} net_device_t;

typedef struct net_device_ops {
    int (*ndo_init)(net_device_t*);
    void (*ndo_uninit)(net_device_t*);
    int (*ndo_open)(net_device_t*);
    int (*ndo_stop)(net_device_t*);
    netdev_tx_t (*ndo_start_xmit)(sk_buff_t*, net_device_t*);
    int (*ndo_change_mtu)(net_device_t*, int);
    int (*ndo_set_mac_address)(net_device_t*, void*);
    int (*ndo_do_ioctl)(net_device_t*, void*, int);
} net_device_ops_t;

typedef struct net_namespace {
    u32 ns_id;
    struct net_device* loopback;
    struct hlist_head* dev_index_head;
    struct hlist_head* dev_name_head;
    struct sock* genl_sock;
    struct ct_net* ct;
    struct nf_net* nf;
    struct ipv4_devconf* ipv4_devconf;
    struct ipv6_devconf* ipv6_devconf;
    struct fib_rules_ops* rules_ops;
    struct list_head* rules;
    struct list_head* cores;
    struct list_head* cleanup_list;
    atomic_t count;
    atomic_t inuse;
} net_namespace_t;

/* ============================================================================
 * IPC (пункты 15, 18)
 * ============================================================================ */
typedef struct sem {
    u16 semval;
    u16 semzcnt;
    u16 semncnt;
    u16 sempid;
} sem_t;

typedef struct sem_array {
    key_t key;
    u16 sem_nsems;
    u16 sem_perm_mode;
    uid_t sem_perm_uid;
    gid_t sem_perm_gid;
    u64 sem_perm_cuid;
    u64 sem_perm_cgid;
    u64 sem_otime;
    u64 sem_ctime;
    sem_t* sems;
    struct semaphore_wait_queue* wait;
} sem_array_t;

typedef struct msg_msg {
    struct msg_msg* next;
    long mtype;
    size_t msize;
    uid_t sender;
    u64 stime;
} msg_msg_t;

typedef struct msg_queue {
    key_t key;
    struct msg_msg* messages;
    size_t msg_cbytes;
    size_t msg_qnum;
    size_t msg_qbytes;
    pid_t msg_lspid;
    pid_t msg_lrpid;
    u64 msg_stime;
    u64 msg_rtime;
    u64 msg_ctime;
    uid_t msg_perm_uid;
    gid_t msg_perm_gid;
    u16 msg_perm_mode;
} msg_queue_t;

typedef struct shmid_kernel {
    key_t key;
    size_t shm_segsz;
    u64 shm_atim;
    u64 shm_dtim;
    u64 shm_ctim;
    pid_t shm_cprid;
    pid_t shm_lprid;
    uid_t shm_perm_uid;
    gid_t shm_perm_gid;
    u16 shm_perm_mode;
    u16 shm_nattch;
    void* shm_mem;
    struct user_namespace* user_ns;
    struct file* shm_file;
    unsigned long id;
} shmid_kernel_t;

typedef struct ipc_namespace {
    atomic_t count;
    struct user_namespace* user_ns;
    u32 proc_inum;
    
    /* Semaphores */
    struct ipc_ids* ids[3];  /* SEM, MSG, SHM */
    
    /* Messages */
    size_t msg_hdrs;
    size_t msg_hdrs_total;
    size_t msg_queues;
    size_t msg_queues_bytes;
    
    /* Shared memory */
    size_t shm_tot;
    size_t shm_rss;
    size_t shm_swp;
    
    /* Limits */
    int msg_max;
    int msgmnb;
    int msgmni;
    int shmmni;
    int shmmax;
    int shmall;
    
    /* Callbacks */
    void (*dump)(struct ipc_namespace*, struct seq_file*);
} ipc_namespace_t;

/* ============================================================================
 * SMP & MULTI-CORE (пункт 16)
 * ============================================================================ */
typedef struct cpuinfo_x86 {
    u8 x86;        /* CPU family */
    u8 x86_vendor; /* Vendor ID */
    u8 x86_model;
    u8 x86_mask;
    u32 x86_capability[NCAPINTS];
    char x86_vendor_id[16];
    char x86_model_id[64];
    u32 x86_cache_size;
    u32 x86_cache_alignment;
    u32 cpuid_level;
    u32 apicid;
    u32 phys_proc_id;
    u32 cpu_core_id;
    u32 booted_cores;
    u32 nr_siblings;
    u32 logical_proc_id;
    u16 fpu_exception;
    u16 wp_works_ok;
    u32 microcode;
} cpuinfo_x86_t;

typedef struct cpu {
    u32 cpu_id;
    bool online;
    thread_t* current_thread;
    thread_t* idle_thread;
    u64 clock_tick;
    u64 irq_count;
    u64 softirq_count;
    u64 syscall_count;
    u64 context_switches;
    
    /* Runqueue для CFS */
    struct rb_root cfs_tasks;
    thread_t* cfs_curr;
    u64 cfs_min_vruntime;
    spinlock_t runqueue_lock;
    
    /* Local APIC */
    u64 lapic_base;
    u32 lapic_id;
    
    /* TSC */
    u64 tsc_khz;
    u64 tsc_mult;
    u64 tsc_shift;
    
    cpuinfo_x86_t cpu_info;
} cpu_t;

extern cpu_t cpu_data[MIKU_MAX_CPUS];
extern u32 num_online_cpus;

/* ============================================================================
 * УСТРОЙСТВА (пункт 17)
 * ============================================================================ */
/* VGA драйвер */
typedef struct vga_driver {
    u16* buffer;
    u32 width;
    u32 height;
    u32 cursor_x;
    u32 cursor_y;
    u8 fg_color;
    u8 bg_color;
    spinlock_t lock;
} vga_driver_t;

/* Клавиатура */
typedef struct keyboard_driver {
    u8 shift_pressed;
    u8 ctrl_pressed;
    u8 alt_pressed;
    u8 caps_lock;
    u8 num_lock;
    u8 scroll_lock;
    char buffer[256];
    u32 buf_start;
    u32 buf_end;
    spinlock_t lock;
    wait_queue_t wait;
} keyboard_driver_t;

/* Дисковый драйвер (ATA/IDE/SATA) */
typedef struct disk_driver {
    u32 port_base;
    u32 control_port;
    bool master;
    u64 capacity;      /* В секторах */
    u32 sector_size;
    u32 cylinders;
    u32 heads;
    u32 sectors;
    char model[41];
    char serial[21];
    spinlock_t lock;
} disk_driver_t;

/* USB драйвер */
typedef struct usb_device {
    u8 devnum;
    u8 devpath;
    u8 speed;
    u16 vendor;
    u16 product;
    u8 device_class;
    u8 device_subclass;
    u8 device_protocol;
    u8 maxpacket0;
    char manufacturer[64];
    char product[64];
    char serial[64];
    struct usb_host_config* actconfig;
    struct usb_device_descriptor descriptor;
    spinlock_t lock;
} usb_device_t;

/* GPU драйвер (базовый) */
typedef struct gpu_driver {
    u32 framebuffer_phys;
    u32 framebuffer_virt;
    u32 fb_size;
    u32 width;
    u32 height;
    u32 pitch;
    u8 bpp;
    u8 red_mask;
    u8 green_mask;
    u8 blue_mask;
    u8 reserved_mask;
    bool accelerated;
    spinlock_t lock;
} gpu_driver_t;

/* ============================================================================
 * ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ И ФУНКЦИИ
 * ============================================================================ */

/* Из main.c */
void kprintf(const char* fmt, ...);
void panic(const char* msg);
void reboot(void);
void shutdown(void);

/* Из scheduler.c */
void schedule(void);
thread_t* get_current_thread(void);
task_struct_t* get_current_task(void);
tid_t thread_create(void (*fn)(void*), void* arg, int priority);
void thread_exit(int code);
void thread_sleep(u64 ms);
void thread_yield(void);
int thread_kill(tid_t tid, int sig);

/* Из memory.c */
void* kmalloc(size_t size);
void kfree(void* ptr);
void* kzalloc(size_t size);
void* kcalloc(size_t nmemb, size_t size);
void* vmalloc(size_t size);
void vfree(void* ptr);
page_t* alloc_page(int flags);
void free_page(page_t* page);
int copy_on_write(u64 addr);

/* Из vfs.c */
file_t* file_open(const char* path, int flags, int mode);
ssize_t file_read(file_t* f, char* buf, size_t count);
ssize_t file_write(file_t* f, const char* buf, size_t count);
int file_close(file_t* f);
int file_seek(file_t* f, off_t offset, int whence);
int create_file(const char* path, int mode);
int remove_file(const char* path);
int create_dir(const char* path, int mode);
int stat_file(const char* path, struct stat* st);

/* Из syscall.c */
void syscall_handler(struct pt_regs* regs);
long sys_call_table[MIKU_MAX_SYSCALLS];
void register_syscall(int num, void* handler);

/* Из interrupt.c */
void idt_init(void);
void pic_init(void);
void irq_install_handler(int irq, void (*handler)(struct pt_regs*));
void sti(void);
void cli(void);

/* Из network.c */
int socket_create(int family, int type, int protocol, socket_t** sock);
int socket_bind(socket_t* sock, sockaddr_t* addr, int addrlen);
int socket_connect(socket_t* sock, sockaddr_t* addr, int addrlen);
int socket_listen(socket_t* sock, int backlog);
int socket_accept(socket_t* sock, socket_t* newsock, int flags);
int socket_send(socket_t* sock, const void* buf, size_t len, int flags);
int socket_recv(socket_t* sock, void* buf, size_t len, int flags);
int socket_close(socket_t* sock);
int network_init(void);

/* Из ipc.c */
int sys_semget(key_t key, int nsems, int semflg);
int sys_semop(int semid, struct sembuf* sops, unsigned nsops);
int sys_msgget(key_t key, int msgflg);
int sys_msgsnd(int msqid, msg_msg_t* msgp, size_t msgsz, int msgflg);
int sys_msgrcv(int msqid, msg_msg_t* msgp, size_t msgsz, long msgtyp, int msgflg);
int sys_shmget(key_t key, size_t size, int shmflg);
void* sys_shmat(int shmid, void* shmaddr, int shmflg);
int sys_shmdt(void* shmaddr);

/* Из exec.c */
int load_elf(const char* path, task_struct_t* task);
int do_execve(const char* filename, char** argv, char** envp);

/* Из fork.c */
pid_t do_fork(unsigned long clone_flags, unsigned long stack_start, void* child_tidptr);

/* Из security.c */
int capable(int cap);
int security_file_permission(file_t* file, int mask);
int security_socket_create(int family, int type, int protocol);
int security_task_create(unsigned long clone_flags);

#endif /* _MIKU_OS_H */
