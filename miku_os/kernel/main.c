/*
 * Miku OS - Main Kernel Entry Point
 * 
 * This is the main entry point for the Miku OS kernel.
 * It initializes all subsystems and starts the init process.
 */

#include "miku_os.h"

/* Global kernel variables */
scheduler_t g_scheduler;
task_struct_t *g_current_task = NULL;
task_struct_t *g_init_task = NULL;
mm_struct_t g_kernel_mm;
struct superblock *g_rootfs = NULL;

/* Kernel version banner */
static const char *kernel_banner = 
    "\n"
    "  __  __ _     _     ___  ____   ____  _   _ _____ \n"
    " |  \\/  | |   | |   / _ \\|  _ \\ / __ \\| | | | ____|\n"
    " | |\\/| | |   | |  | | | | |_) | |  | | | | |  _|  \n"
    " | |  | | |___| |__| |_| |  _ <| |__| | |_| | |___ \n"
    " |_|  |_|_____|_____\\___/|_| \\_\\\\____/ \\___/|_____|\n"
    "                                                   \n"
    "           Miku OS v" MIKU_VERSION_STRING " - \"" MIKU_CODENAME "\"\n"
    "        A modern, Linux-like 64-bit operating system\n"
    "           Built with ♪ by Hatsune Miku fans\n"
    "\n";

/* Forward declarations */
static void kernel_main_early(void);
static void kernel_main_late(void);
static void init_idle_thread(void *arg);
static void init_process(void *arg);

/* ============================================================================
 * KERNEL ENTRY POINT (called from boot.asm)
 * ============================================================================ */
void kernel_main(u32 magic, u64 mbinfo) {
    (void)magic;
    (void)mbinfo;
    
    /* Early initialization (console, basic memory, CPU) */
    kernel_main_early();
    
    console_printf("%s", kernel_banner);
    
    log_info("Miku OS Kernel starting...");
    log_info("Version: %s (%s)", MIKU_VERSION_STRING, MIKU_CODENAME);
    log_info("Compiled for x86_64 architecture");
    
    /* Late initialization (VFS, drivers, scheduler, etc.) */
    kernel_main_late();
    
    log_info("Kernel initialization complete!");
    log_info("Starting init process...");
    
    /* Create the init process (PID 1) */
    g_init_task = task_create("init", init_process, NULL, 0);
    if (!g_init_task) {
        log_panic("Failed to create init process!");
    }
    
    log_info("Init process created (PID=%d, TID=%d)", 
             g_init_task->pid, g_init_task->tid);
    
    /* Start scheduling */
    log_info("Starting scheduler...");
    scheduler_schedule();
    
    /* Should never reach here */
    log_panic("Scheduler returned unexpectedly!");
}

/* ============================================================================
 * EARLY INITIALIZATION
 * ============================================================================ */
static void kernel_main_early(void) {
    /* Initialize console (must be first!) */
    console_init();
    
    /* Initialize logging */
    log_init();
    
    /* Initialize CPU features */
    cpu_init();
    
    /* Initialize memory management */
    mm_init();
    
    /* Initialize interrupts */
    irq_init();
    
    log_info("Early initialization complete");
}

/* ============================================================================
 * LATE INITIALIZATION
 * ============================================================================ */
static void kernel_main_late(void) {
    /* Initialize timekeeping */
    time_init();
    
    /* Initialize scheduler */
    scheduler_init();
    
    /* Initialize VFS */
    vfs_init();
    
    /* Initialize IPC mechanisms */
    ipc_init();
    
    /* Initialize device model */
    // device_model_init();
    
    /* Initialize built-in filesystems */
    // fs_init();
    
    /* Mount root filesystem */
    // vfs_mount("rootfs", "/", "ramfs", 0, NULL);
    
    log_info("Late initialization complete");
}

/* ============================================================================
 * IDLE THREAD
 * ============================================================================ */
static void init_idle_thread(void *arg) {
    (void)arg;
    
    while (true) {
        /* Halt CPU until next interrupt */
        cpu_halt();
    }
}

/* ============================================================================
 * INIT PROCESS (PID 1)
 * ============================================================================ */
static void init_process(void *arg) {
    (void)arg;
    
    console_printf("\n");
    console_printf("  ╔════════════════════════════════════════╗\n");
    console_printf("  ║  Welcome to Miku OS v%s!          ║\n", MIKU_VERSION_STRING);
    console_printf("  ║  Type 'help' for available commands  ║\n");
    console_printf("  ╚════════════════════════════════════════╝\n");
    console_printf("\n");
    
    log_info("Init process running (PID=%d)", g_current_task->pid);
    
    /* Main init loop */
    while (true) {
        /* 
         * In a real implementation, this would:
         * 1. Run /etc/init.d scripts
         * 2. Start system daemons
         * 3. Launch user session manager
         * 4. Handle zombie children
         */
        
        /* For now, just sleep and let other threads run */
        scheduler_sleep(100); /* 1 second at 100 Hz */
        
        /* Check for zombie children */
        // reap_zombies();
    }
}

/* ============================================================================
 * KERNEL PANIC
 * ============================================================================ */
void kernel_panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    irq_disable();
    
    console_set_color(0x0F, 0x04); /* White on red */
    console_clear();
    
    console_printf("\n\n");
    console_printf("  ╔═══════════════════════════════════════════╗\n");
    console_printf("  ║           MIKU OS KERNEL PANIC            ║\n");
    console_printf("  ╚═══════════════════════════════════════════╝\n");
    console_printf("\n");
    console_printf("  The system has encountered a fatal error.\n");
    console_printf("  Please report this to the Miku OS developers.\n");
    console_printf("\n");
    console_printf("  Error: ");
    
    /* Simple printf without formatting for safety */
    console_putstr(fmt);
    
    console_printf("\n\n");
    console_printf("  System halted. Please restart manually.\n");
    console_printf("\n");
    console_printf("  ♪ Thank you for using Miku OS! ♪\n");
    console_printf("\n");
    
    va_end(args);
    
    /* Halt forever */
    while (true) {
        cpu_halt();
    }
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/* Get current task */
task_struct_t *scheduler_get_current(void) {
    return g_current_task;
}

/* Simple string copy */
char *strcpy(char *dest, const char *src) {
    char *orig = dest;
    while ((*dest++ = *src++) != '\0');
    return orig;
}

/* Simple memory set */
void *memset(void *s, int c, size_t n) {
    u8 *p = (u8 *)s;
    while (n--) {
        *p++ = (u8)c;
    }
    return s;
}

/* Simple memory copy */
void *memcpy(void *dest, const void *src, size_t n) {
    u8 *d = (u8 *)dest;
    const u8 *s = (const u8 *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

/* String length */
size_t strlen(const char *s) {
    size_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

/* String compare */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}
