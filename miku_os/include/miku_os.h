/*
 * Miku OS - Advanced 64-bit Kernel
 * A modern, monolithic kernel with Linux-like capabilities
 * 
 * Features:
 * - Preemptive multitasking with CFS-like scheduler
 * - Virtual File System with multiple backends
 * - Advanced Memory Management (paging, slab allocator)
 * - Inter-Process Communication (pipes, shared memory, signals)
 * - Full POSIX API compatibility layer
 * - SMP support with spinlocks and atomic operations
 * - Dynamic module loading
 */

#ifndef MIKU_OS_H
#define MIKU_OS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

/* ============================================================================
 * VERSION & CONFIGURATION
 * ============================================================================ */
#define MIKU_VERSION_MAJOR      2
#define MIKU_VERSION_MINOR      0
#define MIKU_VERSION_PATCH      0
#define MIKU_VERSION_STRING     "2.0.0"
#define MIKU_CODENAME           "Hatsune Future"

#define MIKU_MAX_CPUS           256
#define MIKU_MAX_THREADS        65536
#define MIKU_MAX_PROCESSES      8192
#define MIKU_MAX_FILES          1048576
#define MIKU_MAX_PATH           4096
#define MIKU_MAX_MOUNT_POINTS   64

/* ============================================================================
 * BASIC TYPES
 * ============================================================================ */
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

typedef u32         pid_t;
typedef u64         tid_t;
typedef u64         uid_t;
typedef u64         gid_t;
typedef u64         dev_t;
typedef s64         ssize_t;
typedef u64         size_t;
typedef u64         off_t;
typedef u64         ino_t;
typedef u64         mode_t;
typedef u64         time_t;

/* ============================================================================
 * ERROR CODES (POSIX-compatible)
 * ============================================================================ */
#define EPERM       1      /* Operation not permitted */
#define ENOENT      2      /* No such file or directory */
#define ESRCH       3      /* No such process */
#define EINTR       4      /* Interrupted system call */
#define EIO         5      /* I/O error */
#define ENXIO       6      /* No such device or address */
#define E2BIG       7      /* Argument list too long */
#define ENOMEM      12     /* Out of memory */
#define EACCES      13     /* Permission denied */
#define EFAULT      14     /* Bad address */
#define EEXIST      17     /* File exists */
#define ENODEV      19     /* No such device */
#define EINVAL      22     /* Invalid argument */
#define EMFILE      24     /* Too many open files */
#define ENOSPC      28     /* No space left on device */
#define EPIPE       32     /* Broken pipe */
#define EAGAIN      35     /* Resource temporarily unavailable */
#define ENAMETOOLONG 36    /* File name too long */
#define ENOSYS      38     /* Function not implemented */
#define ERANGE      34     /* Range error */

/* ============================================================================
 * FILE MODE FLAGS
 * ============================================================================ */
#define S_IFMT      0170000    /* File type mask */
#define S_IFSOCK    0140000    /* Socket */
#define S_IFLNK     0120000    /* Symbolic link */
#define S_IFREG     0100000    /* Regular file */
#define S_IFBLK     0060000    /* Block device */
#define S_IFDIR     0040000    /* Directory */
#define S_IFCHR     0020000    /* Character device */
#define S_IFIFO     0010000    /* FIFO */

#define S_ISUID     0004000    /* Set UID bit */
#define S_ISGID     0002000    /* Set GID bit */
#define S_ISVTX     0001000    /* Sticky bit */

#define S_IRWXU     00700      /* Owner read/write/execute */
#define S_IRUSR     00400      /* Owner read */
#define S_IWUSR     00200      /* Owner write */
#define S_IXUSR     00100      /* Owner execute */
#define S_IRWXG     00070      /* Group read/write/execute */
#define S_IRGRP     00040      /* Group read */
#define S_IWGRP     00020      /* Group write */
#define S_IXGRP     00010      /* Group execute */
#define S_IRWXO     00007      /* Other read/write/execute */
#define S_IROTH     00004      /* Other read */
#define S_IWOTH     00002      /* Other write */
#define S_IXOTH     00001      /* Other execute */

/* ============================================================================
 * OPEN FLAGS
 * ============================================================================ */
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT     0x0040
#define O_EXCL      0x0080
#define O_NOCTTY    0x0100
#define O_TRUNC     0x0200
#define O_APPEND    0x0400
#define O_NONBLOCK  0x0800
#define O_SYNC      0x1000
#define O_ASYNC     0x2000
#define O_DIRECTORY 0x4000
#define O_NOFOLLOW  0x8000

/* ============================================================================
 * SEEK FLAGS
 * ============================================================================ */
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

/* ============================================================================
 * THREAD STATES
 * ============================================================================ */
typedef enum {
    THREAD_RUNNING,
    THREAD_RUNNABLE,
    THREAD_SLEEPING,
    THREAD_STOPPED,
    THREAD_ZOMBIE,
    THREAD_DEAD
} thread_state_t;

/* ============================================================================
 * THREAD PRIORITIES
 * ============================================================================ */
typedef enum {
    PRIO_IDLE     = 0,
    PRIO_LOW      = 1,
    PRIO_NORMAL   = 2,
    PRIO_HIGH     = 3,
    PRIO_REALTIME = 4
} thread_priority_t;

/* ============================================================================
 * SIGNAL DEFINITIONS (POSIX)
 * ============================================================================ */
#define SIGHUP      1
#define SIGINT      2
#define SIGQUIT     3
#define SIGILL      4
#define SIGTRAP     5
#define SIGABRT     6
#define SIGBUS      7
#define SIGFPE      8
#define SIGKILL     9
#define SIGUSR1     10
#define SIGSEGV     11
#define SIGUSR2     12
#define SIGPIPE     13
#define SIGALRM     14
#define SIGTERM     15
#define SIGCHLD     17
#define SIGCONT     18
#define SIGSTOP     19
#define SIGTSTP     20
#define SIGTTIN     21
#define SIGTTOU     22
#define SIGURG      23
#define SIGXCPU     24
#define SIGXFSZ     25
#define SIGVTALRM   26
#define SIGPROF     27
#define SIGWINCH    28
#define SIGIO       29
#define SIGSYS      31
#define NSIG        32

/* ============================================================================
 * CPU FEATURES
 * ============================================================================ */
typedef struct {
    bool has_sse;
    bool has_sse2;
    bool has_sse3;
    bool has_ssse3;
    bool has_sse4_1;
    bool has_sse4_2;
    bool has_avx;
    bool has_avx2;
    bool has_avx512;
    bool has_fpu;
    bool has_apic;
    bool has_x2apic;
    bool has_hypervisor;
    u32 cpu_family;
    u32 cpu_model;
    u32 cpu_stepping;
    char vendor[16];
    char brand[64];
} cpu_features_t;

/* ============================================================================
 * MEMORY ZONES
 * ============================================================================ */
typedef enum {
    ZONE_KERNEL,      /* Kernel code and data */
    ZONE_DMA,         /* DMA-capable memory (< 16MB) */
    ZONE_NORMAL,      /* Normal kernel allocations */
    ZONE_HIGHMEM,     /* High memory (user space) */
    ZONE_COUNT
} memory_zone_t;

/* ============================================================================
 * VFS STRUCTURES
 * ============================================================================ */
struct inode;
struct dentry;
struct file;
struct superblock;
struct vfsmount;

/* File operations */
typedef struct {
    ssize_t (*read)(struct file *, char *, size_t, off_t *);
    ssize_t (*write)(struct file *, const char *, size_t, off_t *);
    int (*open)(struct inode *, struct file *);
    int (*close)(struct file *);
    int (*ioctl)(struct file *, unsigned long, unsigned long);
    int (*mmap)(struct file *, void *, size_t, int, off_t);
    int (*fsync)(struct file *);
    int (*fasync)(struct file *, int, int);
    int (*lock)(struct file *, int, struct flock *);
} file_operations_t;

/* Inode operations */
typedef struct {
    struct dentry *(*lookup)(struct inode *, struct dentry *);
    int (*create)(struct inode *, struct dentry *, mode_t);
    int (*mkdir)(struct inode *, struct dentry *, mode_t);
    int (*rmdir)(struct inode *, struct dentry *);
    int (*unlink)(struct inode *, struct dentry *);
    int (*symlink)(struct inode *, struct dentry *, const char *);
    int (*link)(struct dentry *, struct inode *, struct dentry *);
    int (*rename)(struct inode *, struct dentry *, struct inode *, struct dentry *);
    int (*chmod)(struct inode *, mode_t);
    int (*chown)(struct inode *, uid_t, gid_t);
    int (*getattr)(struct dentry *, struct kstat *);
    int (*setattr)(struct dentry *, struct iattr *);
} inode_operations_t;

/* Superblock operations */
typedef struct {
    int (*mount)(struct superblock *, int, void *);
    void (*unmount)(struct superblock *);
    struct inode *(*alloc_inode)(struct superblock *);
    void (*destroy_inode)(struct inode *);
    int (*write_inode)(struct inode *, int);
    void (*put_inode)(struct inode *);
    void (*delete_inode)(struct inode *);
    int (*sync_fs)(struct superblock *, int);
    int (*statfs)(struct superblock *, struct kstatfs *);
} super_operations_t;

/* Inode structure */
struct inode {
    ino_t           i_ino;          /* Inode number */
    mode_t          i_mode;         /* File mode */
    uid_t           i_uid;          /* Owner UID */
    gid_t           i_gid;          /* Owner GID */
    dev_t           i_rdev;         /* Device ID */
    u64             i_size;         /* File size */
    time_t          i_atime;        /* Last access time */
    time_t          i_mtime;        /* Last modification time */
    time_t          i_ctime;        /* Last change time */
    u32             i_nlink;        /* Link count */
    u32             i_blksize;      /* Block size */
    u64             i_blocks;       /* Number of blocks */
    
    struct superblock *i_sb;        /* Superblock */
    struct dentry   *i_dentry;      /* Associated dentry */
    
    const inode_operations_t *i_op; /* Inode operations */
    const file_operations_t *i_fop; /* File operations */
    
    void            *i_private;     /* Filesystem private data */
    
    u32             i_refcount;     /* Reference count */
    spinlock_t      i_lock;         /* Inode lock */
};

/* Dentry structure */
struct dentry {
    char            *d_name;        /* Name */
    u32             d_len;          /* Name length */
    u32             d_hash;         /* Hash value */
    
    struct inode    *d_inode;       /* Associated inode */
    struct dentry   *d_parent;      /* Parent dentry */
    struct superblock *d_sb;        /* Superblock */
    
    struct list_head d_children;    /* Children list */
    struct list_head d_child;       /* Child node */
    
    u32             d_refcount;     /* Reference count */
    spinlock_t      d_lock;         /* Dentry lock */
    
    void            *d_fsdata;      /* Filesystem private data */
};

/* File structure */
struct file {
    struct dentry   *f_dentry;      /* Dentry */
    struct vfsmount *f_vfsmnt;      /* Mount point */
    struct inode    *f_inode;       /* Inode */
    
    const file_operations_t *f_op;  /* File operations */
    
    u64             f_pos;          /* Current position */
    u32             f_flags;        /* File flags */
    u32             f_mode;         /* File mode */
    u32             f_count;        /* Reference count */
    
    void            *f_private;     /* Private data */
    
    struct fown_struct f_owner;     /* Owner for async I/O */
    spinlock_t      f_lock;         /* File lock */
};

/* Superblock structure */
struct superblock {
    dev_t           s_dev;          /* Device ID */
    u32             s_magic;        /* Magic number */
    u32             s_blocksize;    /* Block size */
    u64             s_blocks;       /* Total blocks */
    u64             s_free_blocks;  /* Free blocks */
    u64             s_files;        /* Total inodes */
    u64             s_free_files;   /* Free inodes */
    
    struct vfsmount *s_root;        /* Root mount */
    struct inode    *s_root_inode;  /* Root inode */
    
    const super_operations_t *s_op; /* Superblock operations */
    
    void            *s_fs_info;     /* Filesystem private info */
    
    u32             s_refcount;     /* Reference count */
    spinlock_t      s_lock;         /* Superblock lock */
};

/* Mount point structure */
struct vfsmount {
    struct dentry   *mnt_root;      /* Root dentry */
    struct superblock *mnt_sb;      /* Superblock */
    struct vfsmount *mnt_parent;    /* Parent mount */
    struct dentry   *mnt_mountpoint;/* Mount point dentry */
    
    char            *mnt_devname;   /* Device name */
    char            *mnt_type;      /* Filesystem type */
    char            *mnt_opts;      /* Mount options */
    
    u32             mnt_flags;      /* Mount flags */
    u32             mnt_refcount;   /* Reference count */
};

/* Stat structures */
struct kstat {
    dev_t           st_dev;
    ino_t           st_ino;
    mode_t          st_mode;
    nlink_t         st_nlink;
    uid_t           st_uid;
    gid_t           st_gid;
    dev_t           st_rdev;
    off_t           st_size;
    blksize_t       st_blksize;
    blkcnt_t        st_blocks;
    time_t          st_atime;
    time_t          st_mtime;
    time_t          st_ctime;
};

/* ============================================================================
 * PROCESS & THREAD STRUCTURES
 * ============================================================================ */
typedef struct {
    u64 r15, r14, r13, r12, r11, r10, r9, r8;
    u64 rdi, rsi, rbp, rdx, rcx, rbx, rax;
    u64 vector, error_code;
    u64 rip, cs, rflags, rsp, ss;
} x86_64_regs_t;

typedef struct {
    u64 cr3;
    u64 rsp;
    u64 rip;
    u64 rflags;
    u64 fs_base;
    u64 gs_base;
    u64 user_cs;
    u64 user_ss;
    u64 kernel_cs;
    u64 kernel_ss;
} cpu_context_t;

typedef struct mm_struct {
    u64             pgd;            /* Page Global Directory */
    u64             start_code;
    u64             end_code;
    u64             start_data;
    u64             end_data;
    u64             start_brk;
    u64             brk;
    u64             start_stack;
    u64             arg_start;
    u64             arg_end;
    u64             env_start;
    u64             env_end;
    
    u64             total_vm;       /* Total virtual memory */
    u64             shared_vm;      /* Shared memory */
    u64             exec_vm;        /* Executable memory */
    u64             reserved_vm;    /* Reserved memory */
    
    u32             mm_users;       /* Users count */
    u32             mm_count;       /* Reference count */
    
    spinlock_t      mmap_lock;      /* MMAP lock */
    struct vm_area_struct *mmap;    /* Memory areas */
    
    struct file     **files;        /* Open files */
    u32             max_fds;        /* Max file descriptors */
    u32             num_fds;        /* Open file count */
} mm_struct_t;

typedef struct signal_struct {
    u32             sigcount;       /* Signal count */
    u64             sigpending;     /* Pending signals */
    u64             sigignore;      /* Ignored signals */
    u64             sigcatch;       /* Caught signals */
    
    void            (*handlers[NSIG])(int);
    u64             handler_stack[NSIG];
    
    spinlock_t      siglock;        /* Signal lock */
} signal_struct_t;

typedef struct task_struct {
    tid_t           tid;            /* Thread ID */
    pid_t           pid;            /* Process ID */
    pid_t           ppid;           /* Parent PID */
    pid_t           tgid;           /* Thread group ID */
    
    thread_state_t  state;          /* Thread state */
    thread_priority_t priority;     /* Priority */
    int             nice;           /* Nice value (-20 to 19) */
    
    cpu_context_t   context;        /* CPU context */
    u64             stack;          /* Stack pointer */
    u64             stack_size;     /* Stack size */
    
    char            name[64];       /* Thread name */
    
    mm_struct_t     *mm;            /* Memory descriptor */
    signal_struct_t *signal;        /* Signal handlers */
    
    struct file     *files[1024];   /* Open files */
    u32             max_files;      /* Max open files */
    
    uid_t           uid;            /* User ID */
    uid_t           euid;           /* Effective UID */
    gid_t           gid;            /* Group ID */
    gid_t           egid;           /* Effective GID */
    
    u64             start_time;     /* Start time */
    u64             utime;          /* User time */
    u64             stime;          /* System time */
    u64             cutime;         /* Children user time */
    u64             cstime;         /* Children system time */
    
    u64             timeout;        /* Sleep timeout */
    u64             alarm;          /* Alarm time */
    
    u32             cpu;            /* Current CPU */
    u32             processor_id;   /* Processor ID */
    
    u32             refcount;       /* Reference count */
    spinlock_t      lock;           /* Task lock */
    
    struct list_head run_list;      /* Run queue list */
    struct list_head sibling;       /* Sibling list */
    struct list_head children;      /* Children list */
    struct task_struct *parent;     /* Parent task */
    struct task_struct *group_leader; /* Group leader */
    struct task_struct *next_thread; /* Next thread in group */
    struct task_struct *prev_thread; /* Previous thread in group */
    
    void            *thread_specific; /* Thread-specific data */
} task_struct_t;

/* ============================================================================
 * SCHEDULER STRUCTURES
 * ============================================================================ */
typedef struct {
    u64             vruntime;       /* Virtual runtime */
    u64             min_vruntime;   /* Minimum vruntime */
    u64             load_weight;    /* Load weight */
    u64             nr_running;     /* Number of running tasks */
    u64             nr_switches;    /* Context switches */
    
    u64             runtime_left;   /* Remaining runtime */
    u64             deadline;       /* Deadline */
    u64             period;         /* Period */
    
    task_struct_t   *curr;          /* Current task */
    task_struct_t   *next;          /* Next task */
    task_struct_t   *idle;          /* Idle task */
    
    struct list_head queue;         /* Run queue */
    spinlock_t      lock;           /* Queue lock */
} cfs_rq_t;

typedef struct {
    cfs_rq_t        cfs[MIKU_MAX_CPUS]; /* Per-CPU run queues */
    u32             nr_cpus;            /* CPU count */
    u32             boot_cpu;           /* Boot CPU */
    
    u64             jiffies;            /* System ticks */
    u64             uptime;             /* System uptime */
    
    spinlock_t      global_lock;        /* Global scheduler lock */
} scheduler_t;

/* ============================================================================
 * MEMORY MANAGEMENT STRUCTURES
 * ============================================================================ */
typedef struct page {
    u64             flags;          /* Page flags */
    u32             count;          /* Reference count */
    u32             mapcount;       /* Map count */
    u64             index;          /* Index in mapping */
    
    struct list_head lru;           /* LRU list */
    struct list_head list;          /* Generic list */
    
    union {
        struct {
            struct page *next;      /* Next in slab */
            struct page *prev;      /* Prev in slab */
        };
        struct {
            u64 private;            /* Private data */
            struct address_space *mapping; /* Mapping */
        };
    };
    
    struct zone     *zone;          /* Memory zone */
} page_t;

typedef struct zone {
    char            *name;          /* Zone name */
    memory_zone_t   type;           /* Zone type */
    
    u64             zone_start_pfn; /* Starting PFN */
    u64             spanned_pages;  /* Spanned pages */
    u64             present_pages;  /* Present pages */
    u64             managed_pages;  /* Managed pages */
    
    u64             free_pages;     /* Free pages */
    u64             min_pages;      /* Minimum pages */
    u64             low_pages;      /* Low watermark */
    u64             high_pages;     /* High watermark */
    
    spinlock_t      lock;           /* Zone lock */
    struct list_head free_list;     /* Free list */
    struct list_head active_list;   /* Active list */
    struct list_head inactive_list; /* Inactive list */
} zone_t;

typedef struct slab_cache {
    char            name[32];       /* Cache name */
    size_t          size;           /* Object size */
    size_t          align;          /* Alignment */
    u32             objects;        /* Objects per slab */
    u32             flags;          /* Flags */
    
    struct list_head slabs_full;    /* Full slabs */
    struct list_head slabs_partial; /* Partial slabs */
    struct list_head slabs_free;    /* Free slabs */
    
    u32             active_objs;    /* Active objects */
    u32             num_objs;       /* Total objects */
    
    void            (*ctor)(void *); /* Constructor */
    void            (*dtor)(void *); /* Destructor */
    
    spinlock_t      lock;           /* Cache lock */
} slab_cache_t;

typedef struct vm_area_struct {
    u64             vm_start;       /* Start address */
    u64             vm_end;         /* End address */
    u64             vm_pgoff;       /* Page offset */
    u64             vm_flags;       /* Flags */
    
    struct mm_struct *vm_mm;        /* MM struct */
    
    struct file     *vm_file;       /* Backing file */
    void            *vm_private_data; /* Private data */
    
    struct vm_area_struct *vm_next; /* Next VMA */
    struct vm_area_struct *vm_prev; /* Previous VMA */
    
    struct rb_node  vm_rb;          /* RB-tree node */
} vm_area_struct_t;

/* ============================================================================
 * IPC STRUCTURES
 * ============================================================================ */
typedef struct ipc_perm {
    key_t           key;            /* Key */
    uid_t           uid;            /* Owner UID */
    gid_t           gid;            /* Owner GID */
    uid_t           cuid;           /* Creator UID */
    gid_t           cgid;           /* Creator GID */
    mode_t          mode;           /* Permissions */
    u16             seq;            /* Sequence number */
} ipc_perm_t;

typedef struct sem_array {
    ipc_perm_t      perm;           /* Permissions */
    u16             nsems;          /* Semaphore count */
    u64             otime;          /* Last operation time */
    u64             ctime;          /* Creation time */
    
    struct sem      *sem;           /* Semaphores */
    struct list_head pending_alter; /* Pending alterations */
    struct list_head pending_const; /* Pending constants */
    
    spinlock_t      lock;           /* Lock */
} sem_array_t;

typedef struct msg_queue {
    ipc_perm_t      perm;           /* Permissions */
    u64             qnum;           /* Message count */
    u64             qbytes;         /* Bytes in queue */
    pid_t           lspid;          /* Last sender PID */
    pid_t           lrpid;          /* Last receiver PID */
    u64             stime;          /* Last send time */
    u64             rtime;          /* Last receive time */
    u64             ctime;          /* Creation time */
    
    struct msg_msg  *messages;      /* Messages */
    struct list_head q_messages;    /* Message queue */
    
    spinlock_t      lock;           /* Lock */
} msg_queue_t;

typedef struct shmid_kernel {
    ipc_perm_t      perm;           /* Permissions */
    size_t          shm_segsz;      /* Segment size */
    pid_t           shm_lpid;       /* Last PID */
    pid_t           shm_cpid;       /* Creator PID */
    u32             shm_nattch;     /* Attach count */
    u64             shm_atim;       /* Attach time */
    u64             shm_dtim;       /* Detach time */
    u64             shm_ctim;       /* Change time */
    
    struct file     *shm_file;      /* Backing file */
    void            *shm_addr;      /* Address */
    
    spinlock_t      lock;           /* Lock */
} shmid_kernel_t;

/* ============================================================================
 * DEVICE STRUCTURES
 * ============================================================================ */
typedef struct device {
    char            *name;          /* Device name */
    char            *bus_id;        /* Bus ID */
    dev_t           devt;           /* Device number */
    u32             class;          /* Device class */
    
    struct device   *parent;        /* Parent device */
    struct device   *child;         /* Child device */
    struct device   *sibling;       /* Sibling device */
    
    void            *driver_data;   /* Driver data */
    struct driver   *driver;        /* Driver */
    
    struct bus_type *bus;           /* Bus type */
    
    spinlock_t      lock;           /* Device lock */
    u32             refcount;       /* Reference count */
} device_t;

typedef struct driver {
    char            *name;          /* Driver name */
    struct bus_type *bus;           /* Bus type */
    
    int             (*probe)(device_t *);
    int             (*remove)(device_t *);
    void            (*shutdown)(device_t *);
    
    struct list_head devices;       /* Devices */
    spinlock_t      lock;           /* Lock */
} driver_t;

typedef struct bus_type {
    char            *name;          /* Bus name */
    
    int             (*match)(device_t *, driver_t *);
    int             (*uevent)(device_t *, char *, int);
    
    struct list_head devices;       /* Devices */
    struct list_head drivers;       /* Drivers */
    
    spinlock_t      lock;           /* Lock */
} bus_type_t;

/* ============================================================================
 * KERNEL GLOBALS
 * ============================================================================ */
extern scheduler_t g_scheduler;
extern task_struct_t *g_current_task;
extern task_struct_t *g_init_task;
extern mm_struct_t g_kernel_mm;
extern struct superblock *g_rootfs;

/* ============================================================================
 * CORE FUNCTIONS
 * ============================================================================ */

/* Kernel initialization */
void miku_kernel_init(void);
void miku_early_init(void);
void miku_late_init(void);

/* Scheduler */
void scheduler_init(void);
task_struct_t *scheduler_get_current(void);
void scheduler_schedule(void);
void scheduler_add_task(task_struct_t *task);
void scheduler_remove_task(task_struct_t *task);
void scheduler_yield(void);
void scheduler_sleep(u64 jiffies);
void scheduler_wakeup(task_struct_t *task);

/* Process/Thread management */
task_struct_t *task_create(const char *name, void (*entry)(void *), void *arg, u32 flags);
void task_exit(int status);
int task_kill(pid_t pid, int sig);
task_struct_t *task_find(pid_t pid);
task_struct_t *task_find_tid(tid_t tid);
void task_wait(pid_t pid, int *status);

/* Memory management */
void mm_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
void *kzalloc(size_t size);
void *vmalloc(size_t size);
void vfree(void *ptr);

page_t *alloc_page(gfp_t flags);
void free_page(page_t *page);
u64 alloc_pages(u32 order, gfp_t flags);
void free_pages(u64 addr, u32 order);

slab_cache_t *slab_create(const char *name, size_t size, void (*ctor)(void *), void (*dtor)(void *));
void *slab_alloc(slab_cache_t *cache);
void slab_free(slab_cache_t *cache, void *obj);

/* VFS */
void vfs_init(void);
int vfs_mount(const char *dev, const char *path, const char *type, u32 flags, void *data);
int vfs_umount(const char *path);
struct file *vfs_open(const char *path, int flags, mode_t mode);
int vfs_close(struct file *file);
ssize_t vfs_read(struct file *file, char *buf, size_t count, off_t *pos);
ssize_t vfs_write(struct file *file, const char *buf, size_t count, off_t *pos);
int vfs_stat(const char *path, struct kstat *stat);
int vfs_unlink(const char *path);
int vfs_mkdir(const char *path, mode_t mode);
int vfs_rmdir(const char *path);
int vfs_rename(const char *oldpath, const char *newpath);

/* IPC */
void ipc_init(void);
int sys_pipe(int pipefd[2]);
int sys_fifo(const char *path, mode_t mode);
int sys_semget(key_t key, int nsems, int flags);
int sys_msgget(key_t key, int flags);
int sys_shmget(key_t key, size_t size, int flags);

/* Console/Printing */
void console_init(void);
void console_putchar(char c);
void console_putstr(const char *str);
void console_printf(const char *fmt, ...);
void console_clear(void);
void console_set_color(u8 fg, u8 bg);

/* Logging */
void log_init(void);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_debug(const char *fmt, ...);
void log_panic(const char *fmt, ...) __attribute__((noreturn));

/* Time */
void time_init(void);
u64 time_get_jiffies(void);
u64 time_get_uptime(void);
time_t time_get_unix_time(void);
void time_set_alarm(u64 jiffies);

/* CPU */
void cpu_init(void);
cpu_features_t *cpu_get_features(u32 cpu);
u32 cpu_get_id(void);
u32 cpu_get_count(void);
void cpu_halt(void);
void cpu_reboot(void);
void cpu_shutdown(void);

/* Interrupts */
void irq_init(void);
int irq_register(u32 irq, void (*handler)(u32, void *), void *data);
int irq_unregister(u32 irq);
void irq_enable(void);
void irq_disable(void);
bool irq_save_and_disable(u64 *flags);
void irq_restore(u64 flags);

/* Spinlocks */
void spinlock_init(spinlock_t *lock);
void spinlock_lock(spinlock_t *lock);
void spinlock_unlock(spinlock_t *lock);
bool spinlock_trylock(spinlock_t *lock);

/* Atomic operations */
u32 atomic_inc(u32 *value);
u32 atomic_dec(u32 *value);
u32 atomic_add(u32 *value, u32 add);
u32 atomic_sub(u32 *value, u32 sub);
u32 atomic_cmpxchg(u32 *ptr, u32 old, u32 new);
u32 atomic_xchg(u32 *ptr, u32 new);

/* String functions */
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
char *strtok(char *str, const char *delim);
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

/* Utility */
u32 crc32(const u8 *data, size_t len);
u32 hash_string(const char *str);
u64 random_get(void);
void random_seed(u64 seed);

#endif /* MIKU_OS_H */
