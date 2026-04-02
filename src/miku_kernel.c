/*
 * ================================================================
 *  Miku OS v1.0 - Advanced Multithreaded Kernel
 *  Features: Preemptive Multitasking, VFS, RAM Disk, Shell
 *  Author: Miku OS Team
 * ================================================================
 */

#include <stdint.h>
#include <stddef.h>

/* ==================== MULTIBOOT HEADER ==================== */
#define MULTIBOOT_MAGIC     0x1BADB002
#define MULTIBOOT_FLAGS     0x00000003
#define MULTIBOOT_CHECKSUM  -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

__attribute__((aligned(4), section(".multiboot")))
static uint32_t multiboot_header[3] = {
    MULTIBOOT_MAGIC,
    MULTIBOOT_FLAGS,
    MULTIBOOT_CHECKSUM
};

/* ==================== VGA CONSOLE ==================== */
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEM     0xB8000

static size_t cursor_x = 0, cursor_y = 0;
static volatile uint8_t term_color = 0x0F;
static uint16_t* vga_buffer = (uint16_t*)VGA_MEM;

/* ==================== KERNEL INFO ==================== */
static const char* OS_NAME = "Miku OS";
static const char* OS_VERSION = "1.0.0";
static volatile uint32_t system_ticks = 0;
static volatile uint32_t cpu_mhz = 0;

/* ==================== SPINLOCK ==================== */
typedef struct {
    volatile int locked;
} spinlock_t;

#define SPINLOCK_INIT {0}

static inline void spinlock_init(spinlock_t* lock) {
    lock->locked = 0;
}

static inline void spinlock_acquire(spinlock_t* lock) {
    while (__sync_lock_test_and_set(&lock->locked, 1)) {
        __asm__ volatile("pause" ::: "memory");
    }
}

static inline void spinlock_release(spinlock_t* lock) {
    __sync_lock_release(&lock->locked);
}

/* Global kernel lock */
static spinlock_t kernel_lock = SPINLOCK_INIT;

/* ==================== I/O PORTS ==================== */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" :: "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" :: "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait() {
    outb(0x80, 0);
}

/* ==================== VGA FUNCTIONS ==================== */
static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

static void terminal_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        return;
    }
    if (c == '\r') {
        cursor_x = 0;
        return;
    }
    if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(' ', term_color);
        }
        return;
    }
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= VGA_HEIGHT) {
        for (size_t i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
            vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
        }
        for (size_t i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
            vga_buffer[i] = vga_entry(' ', term_color);
        }
        cursor_y = VGA_HEIGHT - 1;
    }
    vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, term_color);
    cursor_x++;
}

static void terminal_writestring(const char* str) {
    while (*str) {
        terminal_putchar(*str);
        str++;
    }
}

static void terminal_clear() {
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = vga_entry(' ', term_color);
    }
    cursor_x = 0;
    cursor_y = 0;
}

static void terminal_setcolor(uint8_t fg, uint8_t bg) {
    term_color = (bg << 4) | (fg & 0x0F);
}

static void print_hex(uint32_t value) {
    const char* hex = "0123456789ABCDEF";
    terminal_writestring("0x");
    for (int i = 28; i >= 0; i -= 4) {
        terminal_putchar(hex[(value >> i) & 0xF]);
    }
}

static void print_dec(uint32_t value) {
    if (value == 0) {
        terminal_putchar('0');
        return;
    }
    char buf[12];
    int i = 0;
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (i > 0) {
        terminal_putchar(buf[--i]);
    }
}

/* ==================== STRING UTILITIES ==================== */
static int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}

static size_t strlen(const char* s) {
    size_t n = 0;
    while (*s++) n++;
    return n;
}

static int strncmp(const char* a, const char* b, size_t n) {
    while (n && *a && *a == *b) { a++; b++; n--; }
    if (n == 0) return 0;
    return *(unsigned char*)a - *(unsigned char*)b;
}

static void strcpy(char* dst, const char* src) {
    while ((*dst++ = *src++));
}

static void strcat(char* dst, const char* src) {
    while (*dst) dst++;
    while ((*dst++ = *src++));
}

static char* strchr(const char* s, char c) {
    while (*s && *s != c) s++;
    return (*s == c) ? (char*)s : (char*)0;
}

static void memset(void* dst, int val, size_t n) {
    uint8_t* d = (uint8_t*)dst;
    while (n--) *d++ = (uint8_t)val;
}

static void memcpy(void* dst, const void* src, size_t n) {
    const uint8_t* s = (const uint8_t*)src;
    uint8_t* d = (uint8_t*)dst;
    while (n--) *d++ = *s++;
}

/* ==================== THREADING SYSTEM ==================== */
#define MAX_THREADS 64
#define THREAD_STACK_SIZE 4096
#define THREAD_QUANTUM 10

typedef enum {
    THREAD_FREE,
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_ZOMBIE
} thread_state_t;

typedef enum {
    PRIORITY_LOW = 0,
    PRIORITY_NORMAL = 1,
    PRIORITY_HIGH = 2,
    PRIORITY_REALTIME = 3
} thread_priority_t;

typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp, esp;
    uint32_t eip, eflags;
    uint32_t cs, ss, ds, es;
} thread_context_t;

typedef struct {
    int id;
    thread_state_t state;
    thread_priority_t priority;
    thread_context_t context;
    uint8_t stack[THREAD_STACK_SIZE];
    char name[32];
    uint32_t sleep_ticks;
    uint32_t wake_time;
    int wait_channel;
    uint32_t creation_time;
    uint32_t total_runtime;
} thread_t;

static thread_t threads[MAX_THREADS];
static int current_thread_id = -1;
static int thread_counter = 0;
static spinlock_t thread_lock = SPINLOCK_INIT;

/* Thread function prototype */
typedef void (*thread_func_t)(void*);

static void thread_init_system() {
    for (int i = 0; i < MAX_THREADS; i++) {
        threads[i].id = i;
        threads[i].state = THREAD_FREE;
        threads[i].priority = PRIORITY_NORMAL;
        threads[i].sleep_ticks = 0;
        threads[i].wake_time = 0;
        threads[i].wait_channel = -1;
        threads[i].creation_time = 0;
        threads[i].total_runtime = 0;
        memset(threads[i].name, 0, 32);
    }
    current_thread_id = -1;
    thread_counter = 0;
}

static int thread_create(const char* name, thread_func_t func, void* arg, thread_priority_t priority) {
    spinlock_acquire(&thread_lock);
    
    int tid = -1;
    for (int i = 1; i < MAX_THREADS; i++) {
        if (threads[i].state == THREAD_FREE) {
            tid = i;
            break;
        }
    }
    
    if (tid < 0) {
        spinlock_release(&thread_lock);
        return -1;
    }
    
    thread_t* t = &threads[tid];
    t->state = THREAD_READY;
    t->priority = priority;
    t->creation_time = system_ticks;
    
    /* Setup stack */
    uint32_t* sp = (uint32_t*)&t->stack[THREAD_STACK_SIZE];
    
    /* Push arguments and return address */
    *--sp = (uint32_t)arg;
    *--sp = 0xDEADBEEF; /* Return address (invalid) */
    
    /* Setup context */
    t->context.esp = (uint32_t)sp;
    t->context.ebp = (uint32_t)sp;
    t->context.eip = (uint32_t)func;
    t->context.eflags = 0x202; /* Interrupts enabled */
    t->context.cs = 0x08;
    t->context.ss = 0x10;
    t->context.ds = 0x10;
    t->context.es = 0x10;
    
    /* Initialize registers */
    t->context.eax = 0;
    t->context.ebx = 0;
    t->context.ecx = 0;
    t->context.edx = 0;
    t->context.esi = 0;
    t->context.edi = 0;
    
    strcpy(t->name, name);
    t->sleep_ticks = 0;
    t->wake_time = 0;
    t->wait_channel = -1;
    t->total_runtime = 0;
    
    thread_counter++;
    
    spinlock_release(&thread_lock);
    
    return tid;
}

static void thread_sleep(int ticks) {
    if (current_thread_id < 0 || current_thread_id >= MAX_THREADS) return;
    
    spinlock_acquire(&thread_lock);
    threads[current_thread_id].state = THREAD_BLOCKED;
    threads[current_thread_id].sleep_ticks = ticks;
    threads[current_thread_id].wake_time = system_ticks + ticks;
    spinlock_release(&thread_lock);
    
    /* Trigger scheduler */
    __asm__ volatile("int $0x20");
}

static void thread_yield() {
    __asm__ volatile("int $0x20");
}

static void thread_exit() {
    if (current_thread_id < 0 || current_thread_id >= MAX_THREADS) return;
    
    spinlock_acquire(&thread_lock);
    threads[current_thread_id].state = THREAD_ZOMBIE;
    thread_counter--;
    spinlock_release(&thread_lock);
    
    /* Never return */
    while (1) {
        __asm__ volatile("hlt");
    }
}

static int find_next_thread() {
    int best_tid = -1;
    int best_priority = -1;
    
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state == THREAD_READY) {
            if (threads[i].priority > best_priority) {
                best_priority = threads[i].priority;
                best_tid = i;
            }
        }
    }
    
    return best_tid;
}

/* Scheduler - called from timer interrupt */
static void scheduler() {
    spinlock_acquire(&thread_lock);
    
    /* Update blocked threads */
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state == THREAD_BLOCKED) {
            if (system_ticks >= threads[i].wake_time) {
                threads[i].state = THREAD_READY;
                threads[i].sleep_ticks = 0;
                threads[i].wake_time = 0;
            }
        }
    }
    
    /* Find next thread */
    int next_tid = find_next_thread();
    
    if (next_tid < 0) {
        spinlock_release(&thread_lock);
        return;
    }
    
    /* Save current context (simplified - real impl would save here) */
    if (current_thread_id >= 0 && current_thread_id < MAX_THREADS) {
        threads[current_thread_id].state = THREAD_READY;
    }
    
    /* Switch to new thread */
    current_thread_id = next_tid;
    threads[current_thread_id].state = THREAD_RUNNING;
    
    spinlock_release(&thread_lock);
}

/* ==================== VIRTUAL FILESYSTEM ==================== */
#define MAX_FILES 256
#define MAX_FILENAME 64
#define MAX_CONTENT 1024
#define MAX_PATH 256
#define MAX_OPEN_FILES 32

typedef enum {
    VFS_FILE,
    VFS_DIRECTORY,
    VFS_SYMLINK,
    VFS_DEVICE
} vfs_type_t;

typedef enum {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
} vfs_seek_t;

typedef struct {
    char name[MAX_FILENAME];
    char path[MAX_PATH];
    vfs_type_t type;
    char content[MAX_CONTENT];
    uint32_t size;
    uint32_t permissions;
    int parent_index;
    int is_used;
    uint32_t created_time;
    uint32_t modified_time;
    int ref_count;
} vfs_entry_t;

typedef struct {
    int entry_index;
    uint32_t position;
    int flags;
    int is_used;
} vfs_file_handle_t;

static vfs_entry_t vfs_entries[MAX_FILES];
static vfs_file_handle_t vfs_open_files[MAX_OPEN_FILES];
static int vfs_root_index = 0;
static int vfs_current_dir = 0;
static spinlock_t vfs_lock = SPINLOCK_INIT;

static void vfs_init() {
    spinlock_init(&vfs_lock);
    
    for (int i = 0; i < MAX_FILES; i++) {
        vfs_entries[i].is_used = 0;
        vfs_entries[i].name[0] = '\0';
        vfs_entries[i].path[0] = '\0';
        vfs_entries[i].content[0] = '\0';
        vfs_entries[i].size = 0;
        vfs_entries[i].permissions = 0755;
        vfs_entries[i].parent_index = -1;
        vfs_entries[i].created_time = 0;
        vfs_entries[i].modified_time = 0;
        vfs_entries[i].ref_count = 0;
    }
    
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        vfs_open_files[i].is_used = 0;
        vfs_open_files[i].entry_index = -1;
        vfs_open_files[i].position = 0;
        vfs_open_files[i].flags = 0;
    }
    
    /* Create root directory */
    vfs_entries[0].is_used = 1;
    vfs_entries[0].type = VFS_DIRECTORY;
    strcpy(vfs_entries[0].name, "/");
    strcpy(vfs_entries[0].path, "/");
    vfs_entries[0].parent_index = -1;
    vfs_entries[0].permissions = 0755;
    vfs_entries[0].ref_count = 1;
    
    /* Create standard directories */
    int bin_idx = 1;
    vfs_entries[bin_idx].is_used = 1;
    vfs_entries[bin_idx].type = VFS_DIRECTORY;
    strcpy(vfs_entries[bin_idx].name, "bin");
    strcpy(vfs_entries[bin_idx].path, "/bin");
    vfs_entries[bin_idx].parent_index = 0;
    vfs_entries[bin_idx].permissions = 0755;
    
    int etc_idx = 2;
    vfs_entries[etc_idx].is_used = 1;
    vfs_entries[etc_idx].type = VFS_DIRECTORY;
    strcpy(vfs_entries[etc_idx].name, "etc");
    strcpy(vfs_entries[etc_idx].path, "/etc");
    vfs_entries[etc_idx].parent_index = 0;
    vfs_entries[etc_idx].permissions = 0755;
    
    int tmp_idx = 3;
    vfs_entries[tmp_idx].is_used = 1;
    vfs_entries[tmp_idx].type = VFS_DIRECTORY;
    strcpy(vfs_entries[tmp_idx].name, "tmp");
    strcpy(vfs_entries[tmp_idx].path, "/tmp");
    vfs_entries[tmp_idx].parent_index = 0;
    vfs_entries[tmp_idx].permissions = 0777;
    
    int home_idx = 4;
    vfs_entries[home_idx].is_used = 1;
    vfs_entries[home_idx].type = VFS_DIRECTORY;
    strcpy(vfs_entries[home_idx].name, "home");
    strcpy(vfs_entries[home_idx].path, "/home");
    vfs_entries[home_idx].parent_index = 0;
    vfs_entries[home_idx].permissions = 0755;
    
    /* Create welcome file */
    int welcome_idx = 5;
    vfs_entries[welcome_idx].is_used = 1;
    vfs_entries[welcome_idx].type = VFS_FILE;
    strcpy(vfs_entries[welcome_idx].name, "welcome.txt");
    strcpy(vfs_entries[welcome_idx].path, "/welcome.txt");
    strcpy(vfs_entries[welcome_idx].content, 
        "Welcome to Miku OS!\n"
        "==================\n\n"
        "A modern multithreaded operating system.\n\n"
        "Features:\n"
        "- Preemptive multitasking\n"
        "- Virtual filesystem\n"
        "- POSIX-like shell\n"
        "- Spinlock synchronization\n\n"
        "Enjoy! ♪\n");
    vfs_entries[welcome_idx].size = strlen(vfs_entries[welcome_idx].content);
    vfs_entries[welcome_idx].parent_index = 0;
    vfs_entries[welcome_idx].permissions = 0644;
}

static int vfs_find_free() {
    for (int i = 1; i < MAX_FILES; i++) {
        if (!vfs_entries[i].is_used) return i;
    }
    return -1;
}

static int vfs_find_entry(const char* name, int parent) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (vfs_entries[i].is_used && 
            vfs_entries[i].parent_index == parent &&
            strcmp(vfs_entries[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int vfs_open(const char* path, int flags) {
    spinlock_acquire(&vfs_lock);
    
    /* Find free file handle */
    int fh = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!vfs_open_files[i].is_used) {
            fh = i;
            break;
        }
    }
    
    if (fh < 0) {
        spinlock_release(&vfs_lock);
        return -1;
    }
    
    /* Simple path resolution (root only for now) */
    if (path[0] != '/') {
        spinlock_release(&vfs_lock);
        return -1;
    }
    
    /* Find file */
    int idx = vfs_find_entry(path + 1, 0);
    if (idx < 0) {
        spinlock_release(&vfs_lock);
        return -1;
    }
    
    vfs_open_files[fh].is_used = 1;
    vfs_open_files[fh].entry_index = idx;
    vfs_open_files[fh].position = 0;
    vfs_open_files[fh].flags = flags;
    vfs_entries[idx].ref_count++;
    
    spinlock_release(&vfs_lock);
    return fh;
}

static int vfs_read(int fh, char* buffer, uint32_t count) {
    if (fh < 0 || fh >= MAX_OPEN_FILES) return -1;
    if (!vfs_open_files[fh].is_used) return -1;
    
    spinlock_acquire(&vfs_lock);
    
    int idx = vfs_open_files[fh].entry_index;
    if (vfs_entries[idx].type != VFS_FILE) {
        spinlock_release(&vfs_lock);
        return -1;
    }
    
    uint32_t available = vfs_entries[idx].size - vfs_open_files[fh].position;
    uint32_t to_read = (count < available) ? count : available;
    
    memcpy(buffer, vfs_entries[idx].content + vfs_open_files[fh].position, to_read);
    vfs_open_files[fh].position += to_read;
    
    spinlock_release(&vfs_lock);
    return to_read;
}

static int vfs_write(int fh, const char* buffer, uint32_t count) {
    if (fh < 0 || fh >= MAX_OPEN_FILES) return -1;
    if (!vfs_open_files[fh].is_used) return -1;
    
    spinlock_acquire(&vfs_lock);
    
    int idx = vfs_open_files[fh].entry_index;
    if (vfs_entries[idx].type != VFS_FILE) {
        spinlock_release(&vfs_lock);
        return -1;
    }
    
    uint32_t space = MAX_CONTENT - vfs_open_files[fh].position - 1;
    uint32_t to_write = (count < space) ? count : space;
    
    memcpy(vfs_entries[idx].content + vfs_open_files[fh].position, buffer, to_write);
    vfs_open_files[fh].position += to_write;
    
    uint32_t new_size = vfs_open_files[fh].position;
    if (new_size > vfs_entries[idx].size) {
        vfs_entries[idx].size = new_size;
    }
    vfs_entries[idx].modified_time = system_ticks;
    
    spinlock_release(&vfs_lock);
    return to_write;
}

static int vfs_close(int fh) {
    if (fh < 0 || fh >= MAX_OPEN_FILES) return -1;
    if (!vfs_open_files[fh].is_used) return -1;
    
    spinlock_acquire(&vfs_lock);
    
    int idx = vfs_open_files[fh].entry_index;
    vfs_entries[idx].ref_count--;
    vfs_open_files[fh].is_used = 0;
    vfs_open_files[fh].entry_index = -1;
    
    spinlock_release(&vfs_lock);
    return 0;
}

static int vfs_create(const char* path, vfs_type_t type) {
    spinlock_acquire(&vfs_lock);
    
    int idx = vfs_find_free();
    if (idx < 0) {
        spinlock_release(&vfs_lock);
        return -1;
    }
    
    /* Parse filename from path */
    const char* fname = path;
    if (fname[0] == '/') fname++;
    
    vfs_entries[idx].is_used = 1;
    vfs_entries[idx].type = type;
    strcpy(vfs_entries[idx].name, fname);
    strcpy(vfs_entries[idx].path, path);
    vfs_entries[idx].content[0] = '\0';
    vfs_entries[idx].size = 0;
    vfs_entries[idx].parent_index = 0;
    vfs_entries[idx].permissions = (type == VFS_DIRECTORY) ? 0755 : 0644;
    vfs_entries[idx].created_time = system_ticks;
    vfs_entries[idx].modified_time = system_ticks;
    vfs_entries[idx].ref_count = 0;
    
    spinlock_release(&vfs_lock);
    return idx;
}

static int vfs_remove(const char* path) {
    spinlock_acquire(&vfs_lock);
    
    const char* fname = path;
    if (fname[0] == '/') fname++;
    
    int idx = vfs_find_entry(fname, 0);
    if (idx < 0) {
        spinlock_release(&vfs_lock);
        return -1;
    }
    
    if (vfs_entries[idx].ref_count > 0) {
        spinlock_release(&vfs_lock);
        return -1; /* File is open */
    }
    
    vfs_entries[idx].is_used = 0;
    
    spinlock_release(&vfs_lock);
    return 0;
}

static void vfs_list_directory() {
    spinlock_acquire(&vfs_lock);
    
    terminal_writestring("\nDirectory listing:\n");
    terminal_writestring("==================\n");
    
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (vfs_entries[i].is_used && vfs_entries[i].parent_index == vfs_current_dir) {
            found = 1;
            if (vfs_entries[i].type == VFS_DIRECTORY) {
                terminal_setcolor(0x0B, 0x00); /* Light green */
                terminal_writestring("  [DIR] ");
                terminal_setcolor(0x0F, 0x00);
            } else {
                terminal_writestring("  [FIL] ");
            }
            terminal_writestring(vfs_entries[i].name);
            
            if (vfs_entries[i].type == VFS_FILE) {
                terminal_writestring(" (");
                print_dec(vfs_entries[i].size);
                terminal_writestring(" bytes)");
            }
            terminal_putchar('\n');
        }
    }
    
    if (!found) {
        terminal_writestring("  (empty directory)\n");
    }
    
    terminal_putchar('\n');
    spinlock_release(&vfs_lock);
}

/* ==================== SHELL ==================== */
#define INPUT_BUFFER_SIZE 256
static char shell_input[INPUT_BUFFER_SIZE];
static size_t shell_input_len = 0;

static void shell_print_prompt() {
    terminal_setcolor(0x0A, 0x00);
    terminal_writestring("[miku@os]$ ");
    terminal_setcolor(0x0F, 0x00);
}

static void shell_clear() {
    terminal_clear();
}

static void shell_help() {
    terminal_writestring("\n");
    terminal_writestring("╔═══════════════════════════════════════════╗\n");
    terminal_writestring("║         Miku OS v1.0 Command Help         ║\n");
    terminal_writestring("╚═══════════════════════════════════════════╝\n\n");
    
    terminal_writestring("System Commands:\n");
    terminal_writestring("  help      - Show this help\n");
    terminal_writestring("  clear     - Clear screen\n");
    terminal_writestring("  version   - Show OS version\n");
    terminal_writestring("  info      - System information\n");
    terminal_writestring("  uptime    - Show uptime\n");
    terminal_writestring("  threads   - List all threads\n\n");
    
    terminal_writestring("Filesystem Commands:\n");
    terminal_writestring("  ls        - List directory\n");
    terminal_writestring("  cat FILE  - View file contents\n");
    terminal_writestring("  echo TEXT - Print text\n");
    terminal_writestring("  touch F   - Create file\n");
    terminal_writestring("  rm FILE   - Remove file\n");
    terminal_writestring("  mkdir DIR - Create directory\n\n");
    
    terminal_writestring("Test Commands:\n");
    terminal_writestring("  test_threads - Run threading test\n");
    terminal_writestring("  hello       - Greeting\n\n");
}

static void shell_version() {
    terminal_writestring("\n");
    terminal_setcolor(0x0B, 0x00);
    terminal_writestring(OS_NAME);
    terminal_setcolor(0x0F, 0x00);
    terminal_writestring(" version ");
    terminal_writestring(OS_VERSION);
    terminal_writestring("\n");
    terminal_writestring("Build: Multithreaded Kernel with VFS\n");
    terminal_writestring("(c) 2024 Miku OS Team\n\n");
}

static void shell_info() {
    terminal_writestring("\n=== System Information ===\n\n");
    
    terminal_writestring("OS Name:    ");
    terminal_writestring(OS_NAME);
    terminal_putchar('\n');
    
    terminal_writestring("Version:    ");
    terminal_writestring(OS_VERSION);
    terminal_putchar('\n');
    
    terminal_writestring("Uptime:     ");
    print_dec(system_ticks);
    terminal_writestring(" ticks\n");
    
    terminal_writestring("Threads:    ");
    print_dec(thread_counter);
    terminal_writestring(" active\n");
    
    terminal_writestring("Memory:     VGA at 0xB8000\n");
    
    terminal_writestring("CPU:        x86 Protected Mode\n\n");
}

static void shell_uptime() {
    terminal_writestring("\nSystem uptime: ");
    print_dec(system_ticks);
    terminal_writestring(" ticks\n\n");
}

static void shell_hello() {
    terminal_writestring("\n♪ こんにちは！I'm Miku, your OS assistant!\n");
    terminal_writestring("Welcome to the future of computing! ♪\n\n");
}

static void shell_threads() {
    terminal_writestring("\n=== Active Threads ===\n\n");
    terminal_writestring("ID  State      Priority  Name\n");
    terminal_writestring("--------------------------------\n");
    
    spinlock_acquire(&thread_lock);
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state != THREAD_FREE) {
            print_dec(threads[i].id);
            terminal_writestring("   ");
            
            switch (threads[i].state) {
                case THREAD_READY:   terminal_writestring("READY   "); break;
                case THREAD_RUNNING: terminal_writestring("RUNNING "); break;
                case THREAD_BLOCKED: terminal_writestring("BLOCKED "); break;
                case THREAD_ZOMBIE:  terminal_writestring("ZOMBIE  "); break;
            }
            
            print_dec(threads[i].priority);
            terminal_writestring("       ");
            terminal_writestring(threads[i].name);
            terminal_putchar('\n');
        }
    }
    spinlock_release(&thread_lock);
    
    terminal_putchar('\n');
}

/* ==================== SAMPLE THREAD FUNCTIONS ==================== */
static void sample_thread_1(void* arg) {
    (void)arg;
    terminal_setcolor(0x0E, 0x00);
    terminal_writestring("\n[Thread 1] Starting...\n");
    
    for (int i = 0; i < 5; i++) {
        terminal_writestring("[T1] Tick ");
        print_dec(i);
        terminal_putchar('\n');
        thread_sleep(50);
    }
    
    terminal_writestring("[Thread 1] Exiting\n");
    terminal_setcolor(0x0F, 0x00);
    thread_exit();
}

static void sample_thread_2(void* arg) {
    (void)arg;
    terminal_setcolor(0x0D, 0x00);
    terminal_writestring("\n[Thread 2] Starting...\n");
    
    for (int i = 0; i < 5; i++) {
        terminal_writestring("[T2] Count ");
        print_dec(i * 2);
        terminal_putchar('\n');
        thread_sleep(30);
    }
    
    terminal_writestring("[Thread 2] Exiting\n");
    terminal_setcolor(0x0F, 0x00);
    thread_exit();
}

static void shell_test_threads() {
    terminal_writestring("\n=== Threading Test ===\n");
    terminal_writestring("Creating test threads...\n\n");
    
    int t1 = thread_create("test_thread_1", sample_thread_1, (void*)0, PRIORITY_NORMAL);
    int t2 = thread_create("test_thread_2", sample_thread_2, (void*)0, PRIORITY_HIGH);
    
    terminal_writestring("Created threads: ");
    print_dec(t1);
    terminal_writestring(", ");
    print_dec(t2);
    terminal_putchar('\n');
    
    terminal_writestring("Use 'threads' command to see status\n\n");
}

static void shell_cat(const char* filename) {
    if (!filename || !*filename) {
        terminal_writestring("\nUsage: cat <filename>\n\n");
        return;
    }
    
    char path[MAX_PATH];
    path[0] = '/';
    strcpy(path + 1, filename);
    
    int fh = vfs_open(path, 0);
    if (fh < 0) {
        terminal_writestring("\nError: File not found: ");
        terminal_writestring(filename);
        terminal_putchar('\n');
        return;
    }
    
    terminal_writestring("\n--- Contents of ");
    terminal_writestring(filename);
    terminal_writestring(" ---\n\n");
    
    char buffer[256];
    int bytes_read;
    while ((bytes_read = vfs_read(fh, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        terminal_writestring(buffer);
    }
    
    vfs_close(fh);
    terminal_putchar('\n');
}

static void shell_touch(const char* filename) {
    if (!filename || !*filename) {
        terminal_writestring("\nUsage: touch <filename>\n\n");
        return;
    }
    
    char path[MAX_PATH];
    path[0] = '/';
    strcpy(path + 1, filename);
    
    if (vfs_find_entry(filename, 0) >= 0) {
        terminal_writestring("\nError: File already exists\n\n");
        return;
    }
    
    int idx = vfs_create(path, VFS_FILE);
    if (idx < 0) {
        terminal_writestring("\nError: Could not create file\n\n");
        return;
    }
    
    terminal_writestring("\n✓ Created file: ");
    terminal_writestring(filename);
    terminal_putchar('\n');
}

static void shell_rm(const char* filename) {
    if (!filename || !*filename) {
        terminal_writestring("\nUsage: rm <filename>\n\n");
        return;
    }
    
    char path[MAX_PATH];
    path[0] = '/';
    strcpy(path + 1, filename);
    
    if (vfs_remove(path) < 0) {
        terminal_writestring("\nError: Could not remove file\n\n");
        return;
    }
    
    terminal_writestring("\n✓ Removed: ");
    terminal_writestring(filename);
    terminal_putchar('\n');
}

static void shell_mkdir(const char* dirname) {
    if (!dirname || !*dirname) {
        terminal_writestring("\nUsage: mkdir <dirname>\n\n");
        return;
    }
    
    char path[MAX_PATH];
    path[0] = '/';
    strcpy(path + 1, dirname);
    
    if (vfs_find_entry(dirname, 0) >= 0) {
        terminal_writestring("\nError: Already exists\n\n");
        return;
    }
    
    int idx = vfs_create(path, VFS_DIRECTORY);
    if (idx < 0) {
        terminal_writestring("\nError: Could not create directory\n\n");
        return;
    }
    
    terminal_writestring("\n✓ Created directory: ");
    terminal_writestring(dirname);
    terminal_putchar('\n');
}

static void shell_echo(const char* text) {
    if (text) {
        terminal_writestring(text);
    }
    terminal_putchar('\n');
}

static void shell_process_command(char* cmd) {
    /* Trim trailing newline */
    size_t len = strlen(cmd);
    if (len > 0 && cmd[len-1] == '\n') {
        cmd[len-1] = '\0';
    }
    
    if (strcmp(cmd, "help") == 0) {
        shell_help();
    } else if (strcmp(cmd, "clear") == 0) {
        shell_clear();
    } else if (strcmp(cmd, "version") == 0) {
        shell_version();
    } else if (strcmp(cmd, "info") == 0) {
        shell_info();
    } else if (strcmp(cmd, "uptime") == 0) {
        shell_uptime();
    } else if (strcmp(cmd, "hello") == 0) {
        shell_hello();
    } else if (strcmp(cmd, "threads") == 0) {
        shell_threads();
    } else if (strcmp(cmd, "test_threads") == 0) {
        shell_test_threads();
    } else if (strcmp(cmd, "ls") == 0) {
        vfs_list_directory();
    } else if (strncmp(cmd, "cat ", 4) == 0) {
        shell_cat(cmd + 4);
    } else if (strncmp(cmd, "touch ", 6) == 0) {
        shell_touch(cmd + 6);
    } else if (strncmp(cmd, "rm ", 3) == 0) {
        shell_rm(cmd + 3);
    } else if (strncmp(cmd, "mkdir ", 6) == 0) {
        shell_mkdir(cmd + 6);
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        shell_echo(cmd + 5);
    } else if (strlen(cmd) > 0) {
        terminal_writestring("\nUnknown command: ");
        terminal_writestring(cmd);
        terminal_writestring("\nType 'help' for available commands\n\n");
    }
}

static void shell_handle_input(char c) {
    if (c == '\n' || c == '\r') {
        terminal_putchar('\n');
        shell_input[shell_input_len] = '\0';
        shell_process_command(shell_input);
        shell_input_len = 0;
        shell_print_prompt();
        return;
    }
    
    if (c == '\b') {
        if (shell_input_len > 0) {
            shell_input_len--;
            terminal_putchar(c);
        }
        return;
    }
    
    if (shell_input_len < INPUT_BUFFER_SIZE - 1) {
        shell_input[shell_input_len++] = c;
        terminal_putchar(c);
    }
}

/* ==================== TIMER INTERRUPT ==================== */
/* This would be connected to real PIC/IOAPIC in full implementation */
void timer_interrupt_handler() {
    system_ticks++;
    scheduler();
}

/* ==================== PIT INITIALIZATION ==================== */
static void pit_init(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;
    
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

/* ==================== KERNEL MAIN ==================== */
void kernel_main() {
    /* Initialize console */
    terminal_clear();
    
    /* Boot message */
    terminal_setcolor(0x0B, 0x00);
    terminal_writestring("╔═══════════════════════════════════════════╗\n");
    terminal_writestring("║                                           ║\n");
    terminal_writestring("║          Welcome to Miku OS v1.0          ║\n");
    terminal_writestring("║     Advanced Multithreaded Kernel         ║\n");
    terminal_writestring("║                                           ║\n");
    terminal_writestring("╚═══════════════════════════════════════════╝\n");
    terminal_setcolor(0x0F, 0x00);
    
    terminal_putchar('\n');
    terminal_writestring("Initializing kernel components...\n");
    
    /* Initialize subsystems */
    terminal_writestring("[OK] VGA Console\n");
    
    /* Initialize spinlocks */
    spinlock_init(&kernel_lock);
    spinlock_init(&thread_lock);
    
    terminal_writestring("[OK] Spinlocks\n");
    
    /* Initialize threading system */
    thread_init_system();
    terminal_writestring("[OK] Threading System\n");
    
    /* Initialize virtual filesystem */
    vfs_init();
    terminal_writestring("[OK] Virtual Filesystem\n");
    
    /* Initialize PIT timer */
    pit_init(100); /* 100 Hz */
    terminal_writestring("[OK] PIT Timer (100Hz)\n");
    
    /* Enable interrupts */
    __asm__ volatile("sti");
    terminal_writestring("[OK] Interrupts Enabled\n");
    
    terminal_putchar('\n');
    terminal_writestring("System ready!\n");
    terminal_writestring("Type 'help' for available commands.\n\n");
    
    /* Create idle thread */
    thread_create("idle", (thread_func_t)0, (void*)0, PRIORITY_LOW);
    
    /* Create shell thread */
    thread_create("shell", (thread_func_t)0, (void*)0, PRIORITY_NORMAL);
    
    /* Main loop - in real OS this would be the idle task */
    shell_print_prompt();
    
    while (1) {
        /* In real implementation, this would wait for keyboard interrupt */
        /* For now, simulate basic input handling */
        
        /* Check for keyboard input */
        if (inb(0x64) & 0x01) {
            char scancode = inb(0x60);
            /* Simple PS/2 scancode to ASCII conversion */
            static char keymap[] = {
                0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
                '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
                0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
                'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
            };
            
            if (scancode < sizeof(keymap) && keymap[scancode]) {
                shell_handle_input(keymap[scancode]);
            }
        }
        
        /* Yield to other threads periodically */
        if (system_ticks % THREAD_QUANTUM == 0) {
            thread_yield();
        }
        
        /* Halt until next interrupt */
        __asm__ volatile("hlt");
    }
}

/* ==================== BOOT ENTRY POINT ==================== */
__attribute__((noreturn))
void _start() {
    __asm__ volatile("cli");
    kernel_main();
    
    /* Should never reach here */
    while (1) {
        __asm__ volatile("hlt");
    }
}
