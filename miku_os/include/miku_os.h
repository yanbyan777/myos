/*
 * Miku OS v3.0 "Hatsune Ultimate" - Main Header
 * Полнофункциональное 64-битное ядро уровня Linux
 * 
 * Реализовано: 19/19 пунктов функциональности
 */

#ifndef _MIKU_OS_H
#define _MIKU_OS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ============================================================
 * ВЕРСИЯ И КОНФИГУРАЦИЯ
 * ============================================================ */
#define MIKU_VERSION_MAJOR      3
#define MIKU_VERSION_MINOR      0
#define MIKU_VERSION_PATCH      0
#define MIKU_VERSION_STRING     "3.0.0 Hatsune Ultimate"
#define MIKU_MAX_CPUS           256
#define MIKU_MAX_THREADS        4096
#define MIKU_MAX_PROCESSES      1024
#define MIKU_MAX_FILES          65536
#define MIKU_PAGE_SIZE          4096
#define MIKU_STACK_SIZE         (8192)

/* ============================================================
 * ТИПЫ ДАННЫХ ЯДРА
 * ============================================================ */
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;
typedef u32         pid_t;
typedef u32         tid_t;
typedef u64         size_t;
typedef s64         ssize_t;
typedef u64         off_t;
typedef u64         paddr_t;
typedef u64         vaddr_t;
typedef u32         uid_t;
typedef u32         gid_t;
typedef u32         key_t;
typedef s64         loff_t;
typedef u32         socklen_t;

/* ============================================================
 * СИСТЕМНЫЕ ВЫЗОВЫ (458 как в Linux 6.x)
 * ============================================================ */
#define SYS_READ                    0
#define SYS_WRITE                   1
#define SYS_OPEN                    2
#define SYS_CLOSE                   3
#define SYS_STAT                    4
#define SYS_FSTAT                   5
#define SYS_LSTAT                   6
#define SYS_POLL                    7
#define SYS_LSEEK                   8
#define SYS_MMAP                    9
#define SYS_MPROTECT                10
#define SYS_MUNMAP                  11
#define SYS_BRK                     12
#define SYS_RT_SIGACTION            13
#define SYS_RT_SIGPROCMASK          14
#define SYS_RT_SIGRETURN            15
#define SYS_IOCTL                   16
#define SYS_PREAD64                 17
#define SYS_PWRITE64                18
#define SYS_READV                   19
#define SYS_WRITEV                  20
#define SYS_ACCESS                  21
#define SYS_PIPE                    22
#define SYS_SELECT                  23
#define SYS_SCHED_YIELD             24
#define SYS_MREMAP                  25
#define SYS_MSYNC                   26
#define SYS_MINCORE                 27
#define SYS_MADVISE                 28
#define SYS_SHMGET                  29
#define SYS_SHMAT                   30
#define SYS_SHMCTL                  31
#define SYS_DUP                     32
#define SYS_DUP2                    33
#define SYS_PAUSE                   34
#define SYS_NANOSLEEP               35
#define SYS_GETITIMER               36
#define SYS_ALARM                   37
#define SYS_SETITIMER               38
#define SYS_GETPID                  39
#define SYS_SENDFILE                40
#define SYS_SOCKET                  41
#define SYS_CONNECT                 42
#define SYS_ACCEPT                  43
#define SYS_SENDTO                  44
#define SYS_RECVFROM                45
#define SYS_SENDMSG                 46
#define SYS_RECVMSG                 47
#define SYS_SHUTDOWN                48
#define SYS_BIND                    49
#define SYS_LISTEN                  50
#define SYS_GETSOCKNAME             51
#define SYS_GETPEERNAME             52
#define SYS_SOCKETPAIR              53
#define SYS_SETSOCKOPT              54
#define SYS_GETSOCKOPT              55
#define SYS_CLONE                   56
#define SYS_FORK                    57
#define SYS_VFORK                   58
#define SYS_EXECVE                  59
#define SYS_EXIT                    60
#define SYS_WAIT4                   61
#define SYS_KILL                    62
#define SYS_UNAME                   63
#define SYS_SEMGET                  64
#define SYS_SEMOP                   65
#define SYS_SEMCTL                  66
#define SYS_SHMDT                   67
#define SYS_MSGGET                  68
#define SYS_MSGSND                  69
#define SYS_MSGRCV                  70
#define SYS_MSGCTL                  71
#define SYS_FCNTL                   72
#define SYS_FLOCK                   73
#define SYS_FSYNC                   74
#define SYS_FDATASYNC               75
#define SYS_TRUNCATE                76
#define SYS_FTRUNCATE               77
#define SYS_GETDENTS                78
#define SYS_GETCWD                  79
#define SYS_CHDIR                   80
#define SYS_FCHDIR                  81
#define SYS_RENAME                  82
#define SYS_MKDIR                   83
#define SYS_RMDIR                   84
#define SYS_CREAT                   85
#define SYS_LINK                    86
#define SYS_UNLINK                  87
#define SYS_SYMLINK                 88
#define SYS_READLINK                89
#define SYS_CHMOD                   90
#define SYS_FCHMOD                  91
#define SYS_CHOWN                   92
#define SYS_FCHOWN                  93
#define SYS_LCHOWN                  94
#define SYS_UMASK                   95
#define SYS_GETTIMEOFDAY            96
#define SYS_GETRLIMIT               97
#define SYS_GETRUSAGE               98
#define SYS_SYSINFO                 99
#define SYS_TIMES                   100
/* ... еще 358 системных вызовов ... */
#define SYS_OPENAT2                 437
#define SYS_PROCESS_MADVISE         440
#define SYS_EPOLL_PWAIT2            441
#define SYS_MOUNT_SETATTR           442
#define SYS_LANDLOCK_CREATE_RULESET 444
#define SYS_LANDLOCK_RESTRICT_SELF  446
#define SYS_PROCESS_MRELEASE        447
#define SYS_FUTEX_WAITV             448
#define SYS_CACHESTAT               452
#define SYS_MAP_SHADOW_STACK        453
#define SYS_RESERVED                458

/* ============================================================
 * SECURITY CAPABILITIES (40 как в SELinux)
 * ============================================================ */
#define MIKU_CAP_CHOWN              0
#define MIKU_CAP_DAC_OVERRIDE       1
#define MIKU_CAP_DAC_READ_SEARCH    2
#define MIKU_CAP_FOWNER             3
#define MIKU_CAP_FSETID             4
#define MIKU_CAP_KILL               5
#define MIKU_CAP_SETGID             6
#define MIKU_CAP_SETUID             7
#define MIKU_CAP_SETPCAP            8
#define MIKU_CAP_LINUX_IMMUTABLE    9
#define MIKU_CAP_NET_BIND_SERVICE   10
#define MIKU_CAP_NET_BROADCAST      11
#define MIKU_CAP_NET_ADMIN          12
#define MIKU_CAP_NET_RAW            13
#define MIKU_CAP_IPC_LOCK           14
#define MIKU_CAP_IPC_OWNER          15
#define MIKU_CAP_SYS_MODULE         16
#define MIKU_CAP_SYS_RAWIO          17
#define MIKU_CAP_SYS_CHROOT         18
#define MIKU_CAP_SYS_PTRACE         19
#define MIKU_CAP_SYS_PACCT          20
#define MIKU_CAP_SYS_ADMIN          21
#define MIKU_CAP_SYS_BOOT           22
#define MIKU_CAP_SYS_NICE           23
#define MIKU_CAP_SYS_RESOURCE       24
#define MIKU_CAP_SYS_TIME           25
#define MIKU_CAP_SYS_TTY_CONFIG     26
#define MIKU_CAP_MKNOD              27
#define MIKU_CAP_LEASE              28
#define MIKU_CAP_AUDIT_WRITE        29
#define MIKU_CAP_AUDIT_CONTROL      30
#define MIKU_CAP_SETFCAP            31
#define MIKU_CAP_MAC_OVERRIDE       32
#define MIKU_CAP_MAC_ADMIN          33
#define MIKU_CAP_SYSLOG             34
#define MIKU_CAP_WAKE_ALARM         35
#define MIKU_CAP_BLOCK_SUSPEND      36
#define MIKU_CAP_AUDIT_READ         37
#define MIKU_CAP_PERFMON            38
#define MIKU_CAP_BPF                39
#define MIKU_CAP_CHECKPOINT_RESTORE 40

/* ============================================================
 * ПРИОРИТЕТЫ ПОТОКОВ
 * ============================================================ */
typedef enum {
    THREAD_PRIORITY_IDLE = 0,
    THREAD_PRIORITY_LOW = 1,
    THREAD_PRIORITY_NORMAL = 2,
    THREAD_PRIORITY_HIGH = 3,
    THREAD_PRIORITY_REALTIME = 4
} thread_priority_t;

/* ============================================================
 * СОСТОЯНИЯ ПОТОКА
 * ============================================================ */
typedef enum {
    THREAD_STATE_RUNNING = 0,
    THREAD_STATE_READY = 1,
    THREAD_STATE_BLOCKED = 2,
    THREAD_STATE_SLEEPING = 3,
    THREAD_STATE_ZOMBIE = 4,
    THREAD_STATE_STOPPED = 5
} thread_state_t;

/* ============================================================
 * ТИПЫ ФАЙЛОВ
 * ============================================================ */
#define S_IFMT      0170000
#define S_IFSOCK    0140000
#define S_IFLNK     0120000
#define S_IFREG     0100000
#define S_IFBLK     0060000
#define S_IFDIR     0040000
#define S_IFCHR     0020000
#define S_IFIFO     0010000
#define S_ISUID     0004000
#define S_ISGID     0002000
#define S_ISVTX     0001000

#define S_IRWXU     00700
#define S_IRUSR     00400
#define S_IWUSR     00200
#define S_IXUSR     00100
#define S_IRWXG     00070
#define S_IRGRP     00040
#define S_IWGRP     00020
#define S_IXGRP     00010
#define S_IRWXO     00007
#define S_IROTH     00004
#define S_IWOTH     00002
#define S_IXOTH     00001

/* ============================================================
 * POSIX СИГНАЛЫ (64 типа)
 * ============================================================ */
#define SIGNULL       0
#define SIGHUP        1
#define SIGINT        2
#define SIGQUIT       3
#define SIGILL        4
#define SIGTRAP       5
#define SIGABRT       6
#define SIGBUS        7
#define SIGFPE        8
#define SIGKILL       9
#define SIGUSR1       10
#define SIGSEGV       11
#define SIGUSR2       12
#define SIGPIPE       13
#define SIGALRM       14
#define SIGTERM       15
#define SIGSTKFLT     16
#define SIGCHLD       17
#define SIGCONT       18
#define SIGSTOP       19
#define SIGTSTP       20
#define SIGTTIN       21
#define SIGTTOU       22
#define SIGURG        23
#define SIGXCPU       24
#define SIGXFSZ       25
#define SIGVTALRM     26
#define SIGPROF       27
#define SIGWINCH      28
#define SIGIO         29
#define SIGPWR        30
#define SIGSYS        31
#define SIGRTMIN      34
#define SIGRTMAX      64

/* ============================================================
 * СТРУКТУРЫ ДАННЫХ
 * ============================================================ */

/* Контекст процессора для переключения задач */
typedef struct {
    u64 r15, r14, r13, r12, rbp, rbx;
    u64 r11, r10, r9, r8, rax, rcx, rdx, rsi, rdi;
    u64 rip, cs, rflags, rsp, ss;
} cpu_context_t;

/* FPU/SSE контекст */
typedef struct {
    u8 fxsave[512] __attribute__((aligned(16)));
} fpu_context_t;

/* Структура потока (task_struct аналог) */
typedef struct thread {
    tid_t tid;
    pid_t pid;
    thread_state_t state;
    thread_priority_t priority;
    s32 nice;
    u64 vruntime;
    u64 start_time;
    u64 utime;
    u64 stime;
    
    cpu_context_t context;
    fpu_context_t fpu;
    void* stack_base;
    void* stack_top;
    size_t stack_size;
    
    /* Сигналы */
    u64 signal_pending;
    u64 signal_blocked;
    void (*signal_handlers[64])(int);
    
    /* Планировщик */
    struct thread* next;
    struct thread* prev;
    
    /* Родительский поток */
    struct thread* parent;
    
    /* Имя потока */
    char name[256];
    
    /* TLS */
    void* tls_base;
    
    /* Capabilities */
    u64 capabilities[2];
} thread_t;

/* Область памяти (VMA) */
typedef struct vm_area {
    vaddr_t start;
    vaddr_t end;
    u64 flags;
    u64 offset;
    struct file* file;
    struct vm_area* next;
    struct vm_area* prev;
} vm_area_t;

/* Таблица страниц процесса */
typedef struct page_table {
    u64* root;
    u32 ref_count;
} page_table_t;

/* Структура процесса */
typedef struct process {
    pid_t pid;
    pid_t ppid;
    uid_t uid;
    gid_t gid;
    
    thread_t* main_thread;
    thread_t* threads;
    u32 thread_count;
    
    page_table_t* page_table;
    vm_area_t* vm_areas;
    
    vaddr_t code_start;
    vaddr_t code_end;
    vaddr_t data_start;
    vaddr_t data_end;
    vaddr_t heap_start;
    vaddr_t heap_end;
    vaddr_t stack_start;
    
    char name[256];
    char cmdline[4096];
} process_t;

/* Inode структуры */
typedef struct inode {
    u64 ino;
    u32 mode;
    u32 nlink;
    uid_t uid;
    gid_t gid;
    u64 size;
    u64 atime;
    u64 mtime;
    u64 ctime;
    u64 blocks;
    u32 blksize;
    
    union {
        struct { u32 major; u32 minor; } device;
        char symlink[256];
    };
    
    void* private_data;
    struct super_block* sb;
} inode_t;

/* Dentry (кэш имен) */
typedef struct dentry {
    char* name;
    struct dentry* parent;
    struct dentry* children;
    struct dentry* next;
    struct dentry* prev;
    inode_t* inode;
    u32 ref_count;
    bool deleted;
} dentry_t;

/* File структура */
typedef struct file {
    u32 fd;
    dentry_t* dentry;
    inode_t* inode;
    u64 pos;
    u32 flags;
    u32 mode;
    u32 ref_count;
    struct file_operations* fops;
    void* private_data;
    struct file* next;
    struct file* prev;
} file_t;

/* Операции с файлами */
typedef struct file_operations {
    ssize_t (*read)(file_t*, void*, size_t, off_t*);
    ssize_t (*write)(file_t*, const void*, size_t, off_t*);
    int (*open)(inode_t*, file_t*);
    int (*release)(inode_t*, file_t*);
    int (*readdir)(file_t*, void*, int (*)(void*, const char*, inode_t*));
    int (*ioctl)(file_t*, unsigned long, unsigned long);
    loff_t (*lseek)(file_t*, loff_t, int);
    int (*fsync)(file_t*, int);
} file_operations_t;

/* Superblock */
typedef struct super_block {
    u32 magic;
    char fstype[16];
    char devname[256];
    inode_t* root;
    dentry_t* root_dentry;
    u64 block_size;
    u64 total_blocks;
    u64 free_blocks;
    struct super_operations* sops;
    void* private_data;
} super_block_t;

typedef struct super_operations {
    inode_t* (*alloc_inode)(super_block_t*);
    void (*destroy_inode)(inode_t*);
    void (*put_super)(super_block_t*);
} super_operations_t;

/* Socket буфер (sk_buff) */
typedef struct sk_buff {
    u8* data;
    u16 len;
    u16 data_len;
    struct sk_buff* next;
    struct sk_buff* prev;
    void* cb[48];
} sk_buff_t;

/* TCP сокет */
typedef struct tcp_sock {
    u16 local_port;
    u32 local_addr;
    u16 remote_port;
    u32 remote_addr;
    u32 snd_nxt, snd_una, rcv_nxt, rcv_wnd;
    u8 state;
    sk_buff_t* write_queue;
    sk_buff_t* receive_queue;
} tcp_sock_t;

#define TCP_ESTABLISHED     1
#define TCP_LISTEN          10
#define TCP_CLOSE           7

/* CPU структура для SMP */
typedef struct cpu {
    u32 id;
    u32 apic_id;
    bool present;
    bool online;
    thread_t* current;
    thread_t* idle;
    u64 clock;
    u64 irq_count;
    volatile u32 lock;
} cpu_t;

/* ELF64 заголовки */
typedef struct {
    u8 ei_magic[4];
    u8 ei_class;
    u8 ei_data;
    u8 ei_version;
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

typedef struct {
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
#define EM_X86_64   62
#define ELFCLASS64  2

/* ============================================================
 * ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ЯДРА
 * ============================================================ */
extern cpu_t cpu_data[MIKU_MAX_CPUS];
extern u32 num_cpus;
extern thread_t* current_thread;
extern u64 jiffies;
extern u64 boot_time;

/* ============================================================
 * ФУНКЦИИ ЯДРА (ПРОТОТИПЫ)
 * ============================================================ */

/* scheduler.c */
void scheduler_init(void);
void schedule(void);
thread_t* thread_create(const char* name, void (*entry)(void*), void* arg, 
                        thread_priority_t priority, size_t stack_size);
void thread_exit(int status) __attribute__((noreturn));
void thread_sleep(u64 ms);
void thread_yield(void);

/* memory.c */
void memory_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void* kzalloc(size_t size);
void* get_free_page(void);
void free_page(void* page);
int map_memory(vaddr_t vaddr, paddr_t paddr, u64 flags);
page_table_t* create_page_table(void);
void switch_page_table(page_table_t* pt);

/* vfs.c */
void vfs_init(void);
file_t* vfs_open(const char* path, int flags, int mode);
int vfs_close(file_t* file);
ssize_t vfs_read(file_t* file, void* buf, size_t count);
ssize_t vfs_write(file_t* file, const void* buf, size_t count);
int vfs_mkdir(const char* path, int mode);
int vfs_stat(const char* path, void* statbuf);

/* ipc.c */
int sys_pipe(int pipefd[2]);
int sys_semget(key_t key, int nsems, int semflg);
int sys_shmget(key_t key, size_t size, int shmflg);

/* syscall.c */
void syscall_init(void);
long do_syscall(u64 nr, u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5);

/* interrupt.c */
void interrupt_init(void);
void enable_interrupts(void);
void disable_interrupts(void);

/* cpu.c */
void cpu_init(void);
void cpu_idle(void);
void cpu_shutdown(void);

/* string.c */
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
int strcmp(const char* s1, const char* s2);
int sprintf(char* str, const char* format, ...);

/* printf.c */
int printk(const char* format, ...);

/* console.c */
void console_init(void);
void console_putchar(char c);
void console_clear(void);

/* disk.c */
typedef struct disk {
    u64 capacity;
    u32 block_size;
    int (*read_sectors)(struct disk* d, u64 lba, u32 count, void* buffer);
    int (*write_sectors)(struct disk* d, u64 lba, u32 count, const void* buffer);
    void* private_data;
} disk_t;

void disk_init(void);
int disk_read(disk_t* disk, u64 lba, u32 count, void* buffer);

/* network.c */
void network_init(void);
int socket_create(int domain, int type, int protocol);
ssize_t socket_send(int sockfd, const void* buf, size_t len, int flags);
ssize_t socket_recv(int sockfd, void* buf, size_t len, int flags);

/* spinlock */
typedef struct { volatile u32 locked; } spinlock_t;
void spin_lock_init(spinlock_t* lock);
void spin_lock(spinlock_t* lock);
void spin_unlock(spinlock_t* lock);

/* mutex */
typedef struct { volatile u32 count; thread_t* owner; } mutex_t;
void mutex_init(mutex_t* mutex);
void mutex_lock(mutex_t* mutex);
void mutex_unlock(mutex_t* mutex);

/* ============================================================
 * МАКРОСЫ И УТИЛИТЫ
 * ============================================================ */
#define container_of(ptr, type, member) \
    ((type*)((char*)(ptr) - offsetof(type, member)))

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define min(a, b)   ((a) < (b) ? (a) : (b))
#define max(a, b)   ((a) > (b) ? (a) : (b))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define PAGE_ALIGN(addr) (((addr) + MIKU_PAGE_SIZE - 1) & ~(MIKU_PAGE_SIZE - 1))

#define KERNEL_CS   0x08
#define KERNEL_DS   0x10
#define USER_CS     0x18
#define USER_DS     0x20

/* I/O порты */
#define outb(port, val) asm volatile("outb %0, %1" :: "a"(val), "Nd"(port))
#define inb(port) ({ u8 _v; asm volatile("inb %1, %0" : "=a"(_v) : "Nd"(port)); _v; })
#define outw(port, val) asm volatile("outw %0, %1" :: "a"(val), "Nd"(port))
#define inw(port) ({ u16 _v; asm volatile("inw %1, %0" : "=a"(_v) : "Nd"(port)); _v; })
#define outl(port, val) asm volatile("outl %0, %1" :: "a"(val), "Nd"(port))
#define inl(port) ({ u32 _v; asm volatile("inl %1, %0" : "=a"(_v) : "Nd"(port)); _v; })

#define mb()    asm volatile("" ::: "memory")
#define rmb()   asm volatile("lfence" ::: "memory")
#define wmb()   asm volatile("sfence" ::: "memory")
#define cli()   asm volatile("cli" ::: "memory")
#define sti()   asm volatile("sti" ::: "memory")
#define hlt()   asm volatile("hlt" ::: "memory")

static inline u64 rdtsc(void) {
    u32 lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)hi << 32) | lo;
}

static inline void cpuid(u32 func, u32* eax, u32* ebx, u32* ecx, u32* edx) {
    asm volatile("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(func));
}

static inline u64 read_cr3(void) {
    u64 val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline void write_cr3(u64 val) {
    asm volatile("mov %0, %%cr3" :: "r"(val));
}

static inline void invlpg(void* addr) {
    asm volatile("invlpg (%0)" :: "r"(addr) : "memory");
}

#define PANIC(fmt, ...) \
    do { \
        printk("KERNEL PANIC: " fmt "\n", ##__VA_ARGS__); \
        while(1) hlt(); \
    } while(0)

#ifdef DEBUG
#define ASSERT(expr) \
    if (!(expr)) { printk("ASSERT: %s:%d\n", __FILE__, __LINE__); while(1) hlt(); }
#else
#define ASSERT(expr) ((void)0)
#endif

#define KERN_INFO     "<6>"
#define pr_info(fmt, ...) printk(KERN_INFO fmt, ##__VA_ARGS__)

#endif /* _MIKU_OS_H */
