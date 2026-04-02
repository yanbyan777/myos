/*
 * Miku OS v3.0 - Kernel Entry Point (C)
 * Основная точка входа ядра после перехода в 64-bit режим
 */

#include "miku_os.h"

/* ============================================================
 * ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
 * ============================================================ */
cpu_t cpu_data[MIKU_MAX_CPUS];
u32 num_cpus = 1;
thread_t* current_thread = NULL;
u64 jiffies = 0;
u64 boot_time = 0;

/* Multiboot2 информация */
static u32 mb_magic = 0;
static void* mb_mboot_ptr = NULL;

/* ============================================================
 * VGA КОНСОЛЬ (простой драйвер)
 * ============================================================ */
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

static size_t vga_x = 0;
static size_t vga_y = 0;
static u8 vga_color = 0x0F; /* Белый на черном */

static inline u8 vga_entry_color(u8 fg, u8 bg) {
    return fg | (bg << 4);
}

static inline u16 vga_entry(unsigned char uc, u8 color) {
    return (u16)uc | ((u16)color << 8);
}

void vga_putchar(char c) {
    u16* vga = (u16*)VGA_MEMORY;
    
    if (c == '\n') {
        vga_x = 0;
        vga_y++;
        return;
    }
    
    if (c == '\r') {
        vga_x = 0;
        return;
    }
    
    if (c == '\t') {
        vga_x = (vga_x + 8) & ~7;
        if (vga_x >= VGA_WIDTH) {
            vga_x = 0;
            vga_y++;
        }
        return;
    }
    
    if (vga_x >= VGA_WIDTH) {
        vga_x = 0;
        vga_y++;
    }
    
    if (vga_y >= VGA_HEIGHT) {
        /* Scroll */
        for (size_t i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
            vga[i] = vga[i + VGA_WIDTH];
        }
        for (size_t i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
            vga[i] = vga_entry(' ', vga_color);
        }
        vga_y = VGA_HEIGHT - 1;
    }
    
    vga[vga_y * VGA_WIDTH + vga_x] = vga_entry(c, vga_color);
    vga_x++;
}

void vga_print(const char* str) {
    while (*str) {
        vga_putchar(*str);
        str++;
    }
}

void vga_printf(const char* format, ...) {
    /* Простая реализация printf */
    vga_print(format);
}

void vga_clear(void) {
    u16* vga = (u16*)VGA_MEMORY;
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = vga_entry(' ', vga_color);
    }
    vga_x = 0;
    vga_y = 0;
}

/* ============================================================
 * КОНСОЛЬ ЯДРА
 * ============================================================ */
void console_init(void) {
    vga_clear();
    vga_print("\n");
    vga_print("  __  __ _   _ _   _ ___ ____  _     ___ _   _  ____ \n");
    vga_print(" |  \\/  | \\ | | \\ | |_ _/ ___|| |   |_ _| \\ | |/ ___|\n");
    vga_print(" | |\\/| |  \\| |  \\| || |\\___ \\| |    | ||  \\| | |  _ \n");
    vga_print(" | |  | | |\\  | |\\  || | ___) | |___ | || |\\  | |_| |\n");
    vga_print(" |_|  |_|_| \\_|_| \\_|___|____/|_____|___|_| \\_|\\____|\n");
    vga_print("\n");
}

int printk(const char* format, ...) {
    vga_print(format);
    return 0;
}

/* ============================================================
 * ЗАГЛУШКИ ФУНКЦИЙ (будут реализованы отдельно)
 * ============================================================ */

void* memcpy(void* dest, const void* src, size_t n) {
    u8* d = (u8*)dest;
    const u8* s = (const u8*)src;
    while (n--) *d++ = *s++;
    return dest;
}

void* memset(void* s, int c, size_t n) {
    u8* p = (u8*)s;
    while (n--) *p++ = (u8)c;
    return s;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

/* Заглушки для остальных функций */
void memory_init(void) { pr_info("[MM] Memory manager initialized\n"); }
void* kmalloc(size_t size) { static u8 heap[1048576]; static size_t offset = 0; void* ptr = heap + offset; offset += size; return ptr; }
void kfree(void* ptr) { (void)ptr; }
void* kzalloc(size_t size) { void* p = kmalloc(size); memset(p, 0, size); return p; }
void* get_free_page(void) { return kmalloc(MIKU_PAGE_SIZE); }
void free_page(void* page) { (void)page; }
int map_memory(vaddr_t vaddr, paddr_t paddr, u64 flags) { return 0; }
page_table_t* create_page_table(void) { static page_table_t pt; return &pt; }
void switch_page_table(page_table_t* pt) { (void)pt; }

void scheduler_init(void) { pr_info("[SCHED] Scheduler initialized\n"); }
void schedule(void) { }
thread_t* thread_create(const char* name, void (*entry)(void*), void* arg, thread_priority_t priority, size_t stack_size) { 
    static thread_t threads[64]; static int next_tid = 0;
    thread_t* t = &threads[next_tid++];
    t->tid = next_tid;
    t->state = THREAD_STATE_READY;
    strcpy(t->name, name);
    return t;
}
void thread_exit(int status) { (void)status; while(1) hlt(); }
void thread_sleep(u64 ms) { (void)ms; }
void thread_yield(void) { }

void vfs_init(void) { pr_info("[VFS] Virtual filesystem initialized\n"); }
file_t* vfs_open(const char* path, int flags, int mode) { (void)path; (void)flags; (void)mode; return NULL; }
int vfs_close(file_t* file) { (void)file; return 0; }
ssize_t vfs_read(file_t* file, void* buf, size_t count) { (void)file; (void)buf; (void)count; return 0; }
ssize_t vfs_write(file_t* file, const void* buf, size_t count) { (void)file; (void)buf; (void)count; return 0; }
int vfs_mkdir(const char* path, int mode) { (void)path; (void)mode; return 0; }
int vfs_stat(const char* path, void* statbuf) { (void)path; (void)statbuf; return 0; }

void interrupt_init(void) { pr_info("[IRQ] Interrupt system initialized\n"); }
void enable_interrupts(void) { sti(); }
void disable_interrupts(void) { cli(); }

void syscall_init(void) { pr_info("[SYSCALL] System call interface initialized\n"); }
long do_syscall(u64 nr, u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) { 
    (void)nr; (void)arg0; (void)arg1; (void)arg2; (void)arg3; (void)arg4; (void)arg5; 
    return -1; 
}

void cpu_init(void) { 
    cpu_data[0].id = 0;
    cpu_data[0].present = true;
    cpu_data[0].online = true;
    num_cpus = 1;
    pr_info("[CPU] BSP initialized (CPU 0)\n");
}
void cpu_idle(void) { while(1) hlt(); }
void cpu_shutdown(void) { cli(); while(1) hlt(); }

void disk_init(void) { pr_info("[DISK] Disk subsystem initialized\n"); }
int disk_read(disk_t* disk, u64 lba, u32 count, void* buffer) { (void)disk; (void)lba; (void)count; (void)buffer; return 0; }

void network_init(void) { pr_info("[NET] Network stack initialized\n"); }
int socket_create(int domain, int type, int protocol) { (void)domain; (void)type; (void)protocol; return -1; }
ssize_t socket_send(int sockfd, const void* buf, size_t len, int flags) { (void)sockfd; (void)buf; (void)len; (void)flags; return -1; }
ssize_t socket_recv(int sockfd, void* buf, size_t len, int flags) { (void)sockfd; (void)buf; (void)len; (void)flags; return -1; }

void spin_lock_init(spinlock_t* lock) { lock->locked = 0; }
void spin_lock(spinlock_t* lock) { while(__sync_lock_test_and_set(&lock->locked, 1)) { while(lock->locked) { __asm__("pause"); } } }
void spin_unlock(spinlock_t* lock) { __sync_lock_release(&lock->locked); }

void mutex_init(mutex_t* mutex) { mutex->count = 1; mutex->owner = NULL; }
void mutex_lock(mutex_t* mutex) { while(!__sync_bool_compare_and_swap(&mutex->count, 1, 0)) { } }
void mutex_unlock(mutex_t* mutex) { __sync_lock_release(&mutex->count); }

/* ============================================================
 * ТОЧКА ВХОДА ЯДРА (C)
 * ============================================================ */
void kernel_main_c(u32 magic, void* mboot_ptr) {
    mb_magic = magic;
    mb_mboot_ptr = mboot_ptr;
    
    /* Инициализация консоли */
    console_init();
    
    pr_info("[BOOT] Miku OS v" MIKU_VERSION_STRING "\n");
    pr_info("[BOOT] Kernel loaded at 0x100000\n");
    pr_info("[BOOT] Multiboot2 magic: 0x");
    /* Здесь можно вывести magic число */
    pr_info("\n");
    
    /* Проверка Multiboot2 */
    if (magic != 0x36d76289) {
        pr_info("[WARN] Invalid Multiboot2 magic (expected 0x36d76289)\n");
    } else {
        pr_info("[OK] Multiboot2 validated\n");
    }
    
    /* Инициализация подсистем */
    pr_info("[INIT] Initializing subsystems...\n");
    
    cpu_init();
    memory_init();
    interrupt_init();
    scheduler_init();
    vfs_init();
    disk_init();
    network_init();
    syscall_init();
    
    pr_info("[OK] All subsystems initialized!\n");
    pr_info("\n");
    pr_info("========================================\n");
    pr_info("  Miku OS ready! Type commands:\n");
    pr_info("  help     - Show help\n");
    pr_info("  version  - Show version\n");
    pr_info("  info     - System info\n");
    pr_info("  threads  - List threads\n");
    pr_info("========================================\n");
    pr_info("\n");
    
    /* Создание тестовых потоков */
    pr_info("[TEST] Creating test threads...\n");
    thread_create("idle", NULL, NULL, THREAD_PRIORITY_IDLE, MIKU_STACK_SIZE);
    thread_create("init", NULL, NULL, THREAD_PRIORITY_NORMAL, MIKU_STACK_SIZE);
    thread_create("kworker/0", NULL, NULL, THREAD_PRIORITY_LOW, MIKU_STACK_SIZE);
    
    pr_info("[OK] System ready! ♪\n");
    
    /* Переход в цикл планировщика */
    pr_info("[SCHED] Entering scheduler loop...\n");
    
    /* Бесконечный цикл (в реальности здесь будет планировщик) */
    while (1) {
        hlt();
    }
}
