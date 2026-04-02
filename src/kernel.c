/*
 * ================================================================
 *  MyOS v7.0 MAX - Advanced Filesystem Edition
 *  Features: RAM FS + Disk Read/Write + Full Shell
 * ================================================================
 */

#include <stdint.h>
#include <stddef.h>

/* ==================== MULTIBOOT ==================== */
#define MULTIBOOT_MAGIC     0x1BADB002
#define MULTIBOOT_FLAGS     0x00000003
#define MULTIBOOT_CHECKSUM  -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

__attribute__((aligned(4), section(".multiboot")))
static uint32_t multiboot_header[3] = {
    MULTIBOOT_MAGIC,
    MULTIBOOT_FLAGS,
    MULTIBOOT_CHECKSUM
};

/* ==================== VGA ==================== */
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEM     0xB8000

static size_t cx = 0, cy = 0;
static volatile uint8_t color = 0x0F;
static uint16_t* vmem = (uint16_t*) VGA_MEM;
static volatile uint32_t uptime_ticks = 0;

/* ==================== DISK ==================== */
#define ATA_PRIMARY_DATA      0x1F0
#define ATA_PRIMARY_STATUS    0x1F7
#define ATA_PRIMARY_CMD       0x1F7
#define ATA_PRIMARY_DRIVE     0x1F6
#define ATA_PRIMARY_SECCOUNT  0x1F2
#define ATA_PRIMARY_LBA_LOW   0x1F3
#define ATA_PRIMARY_LBA_MID   0x1F4
#define ATA_PRIMARY_LBA_HIGH  0x1F5

#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_ERR  0x01
#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY      0xEC

#define SECTOR_SIZE 512
#define DISK_BUFFER_SECTORS 8

static uint8_t disk_buffer[SECTOR_SIZE * DISK_BUFFER_SECTORS];
static int disk_detected = 0;
static uint32_t disk_sectors = 0;

/* ==================== FILESYSTEM ==================== */
#define MAX_FILES 128
#define MAX_FILENAME 64
#define MAX_CONTENT 512
#define MAX_PATH 128

typedef enum { FS_FILE, FS_DIRECTORY } fs_type_t;

typedef struct {
    char name[MAX_FILENAME];
    char path[MAX_PATH];
    fs_type_t type;
    char content[MAX_CONTENT];
    uint32_t size;
    int parent_index;
    int is_used;
    int is_on_disk;
    uint32_t disk_sector;
} fs_entry_t;

static fs_entry_t fs_entries[MAX_FILES];
static int current_dir_index = 0;
static char current_path[MAX_PATH] = "/";

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

static inline uint16_t vga_entry(unsigned char c, uint8_t col) {
    return (uint16_t) c | ((uint16_t) col << 8);
}

static void putc(char c) {
    if (c == '\n') { cx = 0; cy++; return; }
    if (c == '\r') { cx = 0; return; }
    if (c == '\b') {
        if (cx > 0) { cx--; vmem[cy * VGA_WIDTH + cx] = vga_entry(' ', color); }
        return;
    }
    if (cx >= VGA_WIDTH) { cx = 0; cy++; }
    if (cy >= VGA_HEIGHT) {
        for (size_t i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
            vmem[i] = vmem[i + VGA_WIDTH];
        for (size_t i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++)
            vmem[i] = vga_entry(' ', color);
        cy = VGA_HEIGHT - 1;
    }
    vmem[cy * VGA_WIDTH + cx] = vga_entry(c, color);
    cx++;
}

static void puts(const char* s) { while (*s) putc(*s++); }

static void clear() {
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vmem[i] = vga_entry(' ', color);
    cx = cy = 0;
}

/* ==================== STRING FUNCTIONS ==================== */

static int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}

static size_t strlen(const char* s) {
    size_t n = 0; while (*s++) n++; return n;
}

static int strncmp(const char* a, const char* b, size_t n) {
    while (n && *a && *a == *b) { a++; b++; n--; }
    if (n == 0) return 0;
    return *(unsigned char*)a - *(unsigned char*)b;
}

static void strcpy(char* dst, const char* src) { while ((*dst++ = *src++)); }

static void strcat(char* dst, const char* src) {
    while (*dst) dst++;
    while ((*dst++ = *src++));
}

static char* strchr(const char* s, char c) {
    while (*s && *s != c) s++;
    return (*s == c) ? (char*)s : 0;
}

/* ==================== ATA DISK FUNCTIONS ==================== */

static void ata_wait_ready() {
    while (inb(ATA_PRIMARY_STATUS) & ATA_STATUS_BSY);
}

static int ata_detect() {
    outb(ATA_PRIMARY_DRIVE, 0xA0);
    io_wait();
    outb(ATA_PRIMARY_SECCOUNT, 0);
    outb(ATA_PRIMARY_LBA_LOW, 0);
    outb(ATA_PRIMARY_LBA_MID, 0);
    outb(ATA_PRIMARY_LBA_HIGH, 0);
    outb(ATA_PRIMARY_CMD, ATA_CMD_IDENTIFY);
    io_wait();
    
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status == 0) return 0;
    
    ata_wait_ready();
    if (inb(ATA_PRIMARY_STATUS) & ATA_STATUS_ERR) return 0;
    
    /* Читаем размер диска */
    uint16_t* buf = (uint16_t*)disk_buffer;
    for (int i = 0; i < 256; i++) buf[i] = inw(ATA_PRIMARY_DATA);
    
    /* Размер в секторах (слова 60-61) */
    disk_sectors = buf[60] | (buf[61] << 16);
    
    disk_detected = 1;
    return 1;
}

static int ata_read_sector(uint32_t lba, uint8_t* buffer) {
    if (!disk_detected) return -1;
    if (lba >= disk_sectors) return -1;
    
    ata_wait_ready();
    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, 1);
    outb(ATA_PRIMARY_LBA_LOW, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(ATA_PRIMARY_CMD, ATA_CMD_READ_SECTORS);
    ata_wait_ready();
    
    uint16_t* buf = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) buf[i] = inw(ATA_PRIMARY_DATA);
    
    return 0;
}

static int ata_write_sector(uint32_t lba, const uint8_t* buffer) {
    if (!disk_detected) return -1;
    if (lba >= disk_sectors) return -1;
    
    ata_wait_ready();
    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, 1);
    outb(ATA_PRIMARY_LBA_LOW, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(ATA_PRIMARY_CMD, ATA_CMD_WRITE_SECTORS);
    ata_wait_ready();
    
    const uint16_t* buf = (const uint16_t*)buffer;
    for (int i = 0; i < 256; i++) outw(ATA_PRIMARY_DATA, buf[i]);
    
    ata_wait_ready();
    return 0;
}

/* ==================== FILESYSTEM FUNCTIONS ==================== */

static void fs_init() {
    for (int i = 0; i < MAX_FILES; i++) {
        fs_entries[i].is_used = 0;
        fs_entries[i].name[0] = '\0';
        fs_entries[i].path[0] = '\0';
        fs_entries[i].content[0] = '\0';
        fs_entries[i].size = 0;
        fs_entries[i].parent_index = -1;
        fs_entries[i].is_on_disk = 0;
        fs_entries[i].disk_sector = 0;
    }
    
    /* Корневая директория */
    fs_entries[0].is_used = 1;
    fs_entries[0].type = FS_DIRECTORY;
    strcpy(fs_entries[0].name, "/");
    strcpy(fs_entries[0].path, "/");
    fs_entries[0].parent_index = -1;
}

static int fs_find_free() {
    for (int i = 1; i < MAX_FILES; i++)
        if (!fs_entries[i].is_used) return i;
    return -1;
}

static int fs_find_entry(const char* name, int parent) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs_entries[i].is_used && 
            fs_entries[i].parent_index == parent &&
            strcmp(fs_entries[i].name, name) == 0)
            return i;
    }
    return -1;
}

static int fs_save_to_disk(int idx) {
    if (!disk_detected) return -1;
    
    /* Выделяем сектор на диске (начиная с сектора 100) */
    uint32_t sector = 100 + idx;
    
    /* Копируем данные в буфер */
    uint8_t buffer[SECTOR_SIZE] = {0};
    fs_entry_t* entry = &fs_entries[idx];
    
    /* Заголовок: magic + индекс */
    buffer[0] = 'M';
    buffer[1] = 'Y';
    buffer[2] = 'F';
    buffer[3] = 'S';
    buffer[4] = idx & 0xFF;
    buffer[5] = (idx >> 8) & 0xFF;
    
    /* Данные */
    for (int i = 0; i < MAX_FILENAME && i < 32; i++) buffer[8 + i] = entry->name[i];
    for (int i = 0; i < MAX_CONTENT && i < 400; i++) buffer[64 + i] = entry->content[i];
    
    /* Пишем на диск */
    if (ata_write_sector(sector, buffer) == 0) {
        entry->is_on_disk = 1;
        entry->disk_sector = sector;
        return 0;
    }
    return -1;
}

static int fs_load_from_disk(int idx) {
    if (!disk_detected) return -1;
    
    uint32_t sector = 100 + idx;
    uint8_t buffer[SECTOR_SIZE];
    
    if (ata_read_sector(sector, buffer) != 0) return -1;
    
    /* Проверка magic */
    if (buffer[0] != 'M' || buffer[1] != 'Y' || buffer[2] != 'F' || buffer[3] != 'S')
        return -1;
    
    fs_entry_t* entry = &fs_entries[idx];
    entry->is_used = 1;
    entry->is_on_disk = 1;
    entry->disk_sector = sector;
    
    /* Читаем имя */
    for (int i = 0; i < 32 && i < MAX_FILENAME - 1; i++) {
        entry->name[i] = buffer[8 + i];
        if (entry->name[i] == '\0') break;
    }
    entry->name[MAX_FILENAME - 1] = '\0';
    
    /* Читаем контент */
    for (int i = 0; i < 400 && i < MAX_CONTENT - 1; i++) {
        entry->content[i] = buffer[64 + i];
        if (entry->content[i] == '\0') break;
    }
    entry->content[MAX_CONTENT - 1] = '\0';
    entry->size = strlen(entry->content);
    
    return 0;
}

static void fs_sync_all() {
    puts("\nSyncing filesystem to disk...\n");
    int saved = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs_entries[i].is_used && !fs_entries[i].is_on_disk && 
            fs_entries[i].type == FS_FILE) {
            if (fs_save_to_disk(i) == 0) saved++;
        }
    }
    puts("Saved ");
    putc(saved / 10 + '0');
    putc(saved % 10 + '0');
    puts(" files\n\n");
}

static void fs_load_all() {
    puts("\nLoading filesystem from disk...\n");
    int loaded = 0;
    for (int i = 1; i < MAX_FILES; i++) {
        if (fs_load_from_disk(i) == 0) {
            fs_entries[i].parent_index = 0;  /* Все в корень */
            loaded++;
        }
    }
    puts("Loaded ");
    putc(loaded / 10 + '0');
    putc(loaded % 10 + '0');
    puts(" files\n\n");
}

/* ==================== INPUT BUFFER ==================== */

#define INPUT_BUFFER_SIZE 256
static char input_buffer[INPUT_BUFFER_SIZE];
static size_t input_index = 0;

static void prompt() { puts("\n[myos]$ "); }

/* ==================== COMMAND DECLARATIONS ==================== */

static void cmd_help();
static void cmd_ls();
static void cmd_mkdir(const char* name);
static void cmd_cd(const char* name);
static void cmd_pwd();
static void cmd_cat(const char* name);
static void cmd_echo(const char* arg);
static void cmd_rm(const char* name);
static void cmd_clear();
static void cmd_version();
static void cmd_info();
static void cmd_uptime();
static void cmd_hello();
static void cmd_test();
static void cmd_reboot();
static void cmd_color(const char* arg);
static void cmd_colors();
static void cmd_calc(const char* expr);
static void cmd_disk();
static void cmd_sync();
static void cmd_load();
static void cmd_tree();

/* ==================== COMMANDS ==================== */

static void cmd_help() {
    puts("\n╔════════════════════════════════════════╗\n");
    puts("║          MyOS v7.0 MAX Help            ║\n");
    puts("╚════════════════════════════════════════╝\n\n");
    puts("System:\n");
    puts("  help      - Show this help\n");
    puts("  clear     - Clear screen\n");
    puts("  version   - OS version\n");
    puts("  info      - System information\n");
    puts("  uptime    - System uptime\n");
    puts("  reboot    - Reboot system\n\n");
    puts("Disk:\n");
    puts("  disk      - Disk status\n");
    puts("  sync      - Save FS to disk\n");
    puts("  load      - Load FS from disk\n\n");
    puts("Filesystem:\n");
    puts("  ls        - List directory\n");
    puts("  tree      - Directory tree\n");
    puts("  mkdir N   - Create directory\n");
    puts("  cd N      - Change directory\n");
    puts("  pwd       - Show path\n");
    puts("  cat N     - View file\n");
    puts("  echo T to F - Write to file\n");
    puts("  rm N      - Remove file\n\n");
    puts("Utilities:\n");
    puts("  calc EXP  - Calculator\n");
    puts("  color N   - Set color (0-15)\n");
    puts("  hello     - Greeting\n\n");
    prompt();
}

static void cmd_ls() {
    puts("\n");
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs_entries[i].is_used && fs_entries[i].parent_index == current_dir_index) {
            if (fs_entries[i].type == FS_DIRECTORY) {
                uint8_t old = color; color = 0x09;
                puts("  📁 "); puts(fs_entries[i].name); color = old;
            } else {
                puts("  📄 "); puts(fs_entries[i].name);
                if (fs_entries[i].is_on_disk) puts(" [disk]");
            }
            puts("\n");
            found = 1;
        }
    }
    if (!found) puts("  (empty directory)\n");
    puts("\n");
    prompt();
}

static void cmd_tree() {
    puts("\n=== Directory Tree ===\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs_entries[i].is_used) {
            if (fs_entries[i].parent_index == 0) {
                puts("├── ");
                if (fs_entries[i].type == FS_DIRECTORY) {
                    uint8_t old = color; color = 0x09;
                    puts(fs_entries[i].name); color = old;
                } else {
                    puts(fs_entries[i].name);
                }
                puts("\n");
            }
        }
    }
    puts("\n");
    prompt();
}

static void cmd_mkdir(const char* name) {
    if (!name || !*name) { puts("\nUsage: mkdir <name>\n\n"); prompt(); return; }
    if (fs_find_entry(name, current_dir_index) >= 0) {
        puts("\nError: Already exists\n\n"); prompt(); return;
    }
    int idx = fs_find_free();
    if (idx < 0) { puts("\nError: Filesystem full\n\n"); prompt(); return; }
    
    fs_entries[idx].is_used = 1;
    fs_entries[idx].type = FS_DIRECTORY;
    strcpy(fs_entries[idx].name, name);
    strcpy(fs_entries[idx].path, current_path);
    strcat(fs_entries[idx].path, "/");
    strcat(fs_entries[idx].path, name);
    fs_entries[idx].parent_index = current_dir_index;
    
    puts("\n✓ Directory created: "); puts(name); puts("\n\n");
    prompt();
}

static void cmd_cd(const char* name) {
    if (!name || !*name || strcmp(name, "/") == 0) {
        current_dir_index = 0;
        strcpy(current_path, "/");
        puts("\n✓ Changed to root\n\n");
        prompt();
        return;
    }
    if (strcmp(name, "..") == 0) {
        int p = fs_entries[current_dir_index].parent_index;
        if (p >= 0) {
            current_dir_index = p;
            strcpy(current_path, "/");
            puts("\n✓ Changed to parent\n\n");
        } else {
            puts("\nAlready at root\n\n");
        }
        prompt();
        return;
    }
    int idx = fs_find_entry(name, current_dir_index);
    if (idx < 0) { puts("\nError: Not found\n\n"); prompt(); return; }
    if (fs_entries[idx].type != FS_DIRECTORY) {
        puts("\nError: Not a directory\n\n"); prompt(); return;
    }
    current_dir_index = idx;
    strcpy(current_path, fs_entries[idx].path);
    puts("\n✓ Changed to: "); puts(current_path); puts("\n\n");
    prompt();
}

static void cmd_pwd() {
    puts("\nCurrent path: ");
    puts(current_path);
    puts("\n\n");
    prompt();
}

static void cmd_cat(const char* name) {
    if (!name || !*name) { puts("\nUsage: cat <file>\n\n"); prompt(); return; }
    int idx = fs_find_entry(name, current_dir_index);
    if (idx < 0) { puts("\nError: File not found\n\n"); prompt(); return; }
    if (fs_entries[idx].type == FS_DIRECTORY) {
        puts("\nError: Is a directory\n\n"); prompt(); return;
    }
    puts("\n╔════════════════════════════════════════╗\n");
    puts("║ File: "); puts(fs_entries[idx].name);
    puts("\n╠════════════════════════════════════════╣\n");
    puts(fs_entries[idx].content);
    puts("\n╚════════════════════════════════════════╝\n\n");
    prompt();
}

static void cmd_echo(const char* arg) {
    if (!arg || !*arg || *arg == '\n') {
        puts("\nUsage: echo \"text\" to filename\n\n");
        return;
    }
    
    const char* to_pos = 0;
    for (int i = 0; arg[i] && arg[i] != '\n'; i++) {
        if (arg[i] == ' ' && arg[i+1] == 't' && arg[i+2] == 'o' && arg[i+3] == ' ') {
            to_pos = &arg[i+1];
            break;
        }
    }
    
    const char* q1 = 0, *q2 = 0;
    char qchar = 0;
    for (int i = 0; arg[i] && arg[i] != '\n'; i++) {
        if ((arg[i] == '"' || arg[i] == '\'') && !q1) {
            q1 = &arg[i];
            qchar = arg[i];
        } else if (arg[i] == qchar && q1 && &arg[i] != q1) {
            q2 = &arg[i];
            break;
        }
    }
    
    if (!to_pos) {
        puts("\n");
        if (q1 && q2) {
            const char* p = q1 + 1;
            while (p < q2) putc(*p++);
        } else {
            for (int i = 0; arg[i] && arg[i] != '\n'; i++) putc(arg[i]);
        }
        puts("\n\n");
        return;
    }
    
    char text[MAX_CONTENT];
    int ti = 0;
    if (q1 && q2 && q1 < to_pos) {
        const char* p = q1 + 1;
        while (p < q2 && ti < MAX_CONTENT - 1) text[ti++] = *p++;
    }
    text[ti] = '\0';
    
    char fname[MAX_FILENAME];
    int fi = 0;
    const char* p = to_pos + 3;
    while (*p && *p != ' ' && *p != '\n' && fi < MAX_FILENAME - 1) fname[fi++] = *p++;
    fname[fi] = '\0';
    
    if (fi == 0) { puts("\nError: No filename\n\n"); return; }
    
    int idx = fs_find_entry(fname, current_dir_index);
    if (idx >= 0) {
        puts("\nError: File already exists\n\n");
        return;
    }
    
    idx = fs_find_free();
    if (idx < 0) { puts("\nError: Filesystem full\n\n"); return; }
    
    fs_entries[idx].is_used = 1;
    fs_entries[idx].type = FS_FILE;
    fs_entries[idx].parent_index = current_dir_index;
    strcpy(fs_entries[idx].name, fname);
    strcpy(fs_entries[idx].path, current_path);
    if (strcmp(current_path, "/") != 0) {
        strcat(fs_entries[idx].path, "/");
        strcat(fs_entries[idx].path, fname);
    }
    
    for (int i = 0; i < MAX_CONTENT - 1 && i < ti; i++)
        fs_entries[idx].content[i] = text[i];
    fs_entries[idx].content[MAX_CONTENT - 1] = '\0';
    fs_entries[idx].size = ti;
    
    puts("\n✓ Created: "); puts(fname);
    puts(" ("); putc(ti / 10 + '0'); putc(ti % 10 + '0'); puts(" bytes)\n\n");
}

static void cmd_rm(const char* name) {
    if (!name || !*name) { puts("\nUsage: rm <file>\n\n"); prompt(); return; }
    int idx = fs_find_entry(name, current_dir_index);
    if (idx < 0) { puts("\nError: Not found\n\n"); prompt(); return; }
    if (fs_entries[idx].type == FS_DIRECTORY) {
        puts("\nError: Cannot remove directory\n\n"); prompt(); return;
    }
    fs_entries[idx].is_used = 0;
    puts("\n✓ Removed: "); puts(name); puts("\n\n");
    prompt();
}

static void cmd_clear() { clear(); prompt(); }

static void cmd_version() {
    puts("\n╔════════════════════════════════════════╗\n");
    puts("║       MyOS Version 7.0 MAX             ║\n");
    puts("║       Advanced Filesystem Edition      ║\n");
    puts("╚════════════════════════════════════════╝\n\n");
    puts("Kernel: x86 32-bit Protected Mode\n");
    puts("Build: GCC + NASM\n");
    puts("License: Public Domain\n\n");
    prompt();
}

static void cmd_info() {
    puts("\n╔════════════════════════════════════════╗\n");
    puts("║         System Information             ║\n");
    puts("╚════════════════════════════════════════╝\n\n");
    puts("OS:         MyOS v7.0 MAX\n");
    puts("Kernel:     x86 32-bit\n");
    puts("Mode:       Protected Mode\n");
    puts("VGA:        80x25 Text (16 colors)\n");
    puts("Memory:     128 MB (estimated)\n");
    puts("Disk:       "); puts(disk_detected ? "ATA/IDE" : "None"); puts("\n");
    if (disk_detected) {
        puts("Sectors:    ");
        putc(disk_sectors / 1000000 + '0');
        puts("M\n");
    }
    puts("FS:         RAM + Disk Hybrid\n");
    puts("Max Files:  "); putc(MAX_FILES / 10 + '0'); putc(MAX_FILES % 10 + '0'); puts("\n");
    puts("Uptime:     ");
    uint32_t sec = uptime_ticks / 18;
    putc(sec / 60 + '0'); puts(":"); putc((sec % 60) / 10 + '0'); putc(sec % 10 + '0');
    puts("\n\n");
    prompt();
}

static void cmd_uptime() {
    puts("\nSystem Uptime: ");
    uint32_t sec = uptime_ticks / 18;
    uint32_t min = sec / 60;
    uint32_t hr = min / 60;
    sec = sec % 60;
    min = min % 60;
    putc(hr / 10 + '0'); putc(hr % 10 + '0');
    puts(":");
    putc(min / 10 + '0'); putc(min % 10 + '0');
    puts(":");
    putc(sec / 10 + '0'); putc(sec % 10 + '0');
    puts("\n\n");
    prompt();
}

static void cmd_hello() {
    uint8_t old = color;
    color = 0x0A;
    puts("\n╔════════════════════════════════════════╗\n");
    puts("║   *** Hello from MyOS! ***             ║\n");
    puts("║   Your OS is working perfectly!        ║\n");
    puts("╚════════════════════════════════════════╝\n\n");
    color = old;
    prompt();
}

static void cmd_test() {
    puts("\n╔════════════════════════════════════════╗\n");
    puts("║      Keyboard Test Mode                ║\n");
    puts("║      Press any key (ESC to exit)       ║\n");
    puts("╚════════════════════════════════════════╝\n\n");
    prompt();
}

static void cmd_disk() {
    puts("\n╔════════════════════════════════════════╗\n");
    puts("║         Disk Status                    ║\n");
    puts("╚════════════════════════════════════════╝\n\n");
    
    if (!disk_detected) {
        puts("Status:     NOT DETECTED\n");
        puts("Tip:        Use qemu -hda disk.img\n\n");
        prompt();
        return;
    }
    
    puts("Status:     DETECTED\n");
    puts("Interface:  ATA/IDE (Primary Master)\n");
    puts("Sectors:    ");
    putc(disk_sectors / 1000000 + '0');
    puts(" MB\n");
    puts("Buffer:     "); putc(DISK_BUFFER_SECTORS + '0'); puts(" sectors\n\n");
    
    /* Читаем и показываем первый сектор */
    if (ata_read_sector(0, disk_buffer) == 0) {
        puts("Sector 0 (hex):\n");
        for (int i = 0; i < 64; i++) {
            uint8_t b = disk_buffer[i];
            putc("0123456789ABCDEF"[b >> 4]);
            putc("0123456789ABCDEF"[b & 0xF]);
            putc(' ');
            if ((i + 1) % 16 == 0) puts("\n");
        }
        puts("\n");
    }
    prompt();
}

static void cmd_sync() {
    fs_sync_all();
    prompt();
}

static void cmd_load() {
    fs_load_all();
    prompt();
}

static void cmd_reboot() {
    puts("\nRebooting system...\n");
    while (1) { __asm__ volatile("cli; hlt"); }
}

static void cmd_color(const char* arg) {
    if (!arg || !*arg) {
        puts("\nUsage: color N (0-15)\n");
        puts("Current: "); putc(color + '0'); puts("\n\n");
        prompt();
        return;
    }
    uint8_t c = 0;
    while (*arg >= '0' && *arg <= '9') { c = c * 10 + (*arg - '0'); arg++; }
    if (c > 15) { puts("\nError: 0-15 only\n\n"); prompt(); return; }
    color = c;
    puts("\n✓ Color changed!\n\n");
    prompt();
}

static void cmd_colors() {
    puts("\nVGA Color Palette:\n");
    uint8_t old = color;
    for (uint8_t i = 0; i < 16; i++) {
        color = i;
        putc('#'); putc(' ');
    }
    color = old;
    puts("\n\n");
    prompt();
}

static void cmd_calc(const char* expr) {
    if (!expr || !*expr) { puts("\nUsage: calc A+B\n\n"); prompt(); return; }
    int32_t a = 0, b = 0; char op = 0;
    const char* p = expr;
    while (*p >= '0' && *p <= '9') { a = a * 10 + (*p - '0'); p++; }
    while (*p == ' ') p++;
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/') op = *p++;
    while (*p == ' ') p++;
    while (*p >= '0' && *p <= '9') { b = b * 10 + (*p - '0'); p++; }
    
    int32_t res = 0;
    switch (op) {
        case '+': res = a + b; break;
        case '-': res = a - b; break;
        case '*': res = a * b; break;
        case '/': if (b == 0) { puts("\nError: Division by zero\n\n"); prompt(); return; } res = a / b; break;
        default: puts("\nError: Invalid operator\n\n"); prompt(); return;
    }
    
    puts("\n"); puts(expr); puts(" = ");
    if (res < 0) { putc('-'); res = -res; }
    char buf[12] = {0}; int i = 10;
    do { buf[i--] = (res % 10) + '0'; res /= 10; } while (res > 0);
    puts(&buf[i + 1]); puts("\n\n");
    prompt();
}

/* ==================== COMMAND ROUTER ==================== */

static void run_cmd(const char* cmd) {
    if (!cmd || !*cmd) { prompt(); return; }
    
    if (strcmp(cmd, "help") == 0) cmd_help();
    else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0) cmd_clear();
    else if (strcmp(cmd, "version") == 0) cmd_version();
    else if (strcmp(cmd, "info") == 0) cmd_info();
    else if (strcmp(cmd, "uptime") == 0) cmd_uptime();
    else if (strcmp(cmd, "hello") == 0) cmd_hello();
    else if (strcmp(cmd, "test") == 0) cmd_test();
    else if (strcmp(cmd, "colors") == 0) cmd_colors();
    else if (strcmp(cmd, "ls") == 0) cmd_ls();
    else if (strcmp(cmd, "tree") == 0) cmd_tree();
    else if (strcmp(cmd, "disk") == 0) cmd_disk();
    else if (strcmp(cmd, "sync") == 0) cmd_sync();
    else if (strcmp(cmd, "load") == 0) cmd_load();
    else if (strcmp(cmd, "pwd") == 0) cmd_pwd();
    else if (strcmp(cmd, "reboot") == 0) cmd_reboot();
    else if (strncmp(cmd, "color", 5) == 0) {
        const char* arg = cmd + 5; while (*arg == ' ') arg++; cmd_color(arg); return;
    }
    else if (strncmp(cmd, "mkdir", 5) == 0) {
        const char* arg = cmd + 5; while (*arg == ' ') arg++; cmd_mkdir(arg); return;
    }
    else if (strncmp(cmd, "cd", 2) == 0) {
        const char* arg = cmd + 2; while (*arg == ' ') arg++; cmd_cd(arg); return;
    }
    else if (strncmp(cmd, "cat", 3) == 0) {
        const char* arg = cmd + 3; while (*arg == ' ') arg++; cmd_cat(arg); return;
    }
    else if (strncmp(cmd, "echo", 4) == 0) {
        const char* arg = cmd + 4; while (*arg == ' ') arg++; cmd_echo(arg); return;
    }
    else if (strncmp(cmd, "rm", 2) == 0) {
        const char* arg = cmd + 2; while (*arg == ' ') arg++; cmd_rm(arg); return;
    }
    else if (strncmp(cmd, "calc", 4) == 0) {
        const char* arg = cmd + 4; while (*arg == ' ') arg++; cmd_calc(arg); return;
    }
    else { puts("\nUnknown command: "); puts(cmd); puts("\nType 'help'\n\n"); }
    prompt();
}

/* ==================== KEYBOARD ==================== */

static const char scancode_to_ascii[] = {
    0,0,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,
    '*',0,' '
};

static void init_keyboard() {
    outb(0x64, 0xAE); io_wait();
    outb(0x64, 0x60); io_wait();
    outb(0x60, 0x45); io_wait();
}

static void check_keyboard() {
    uint8_t status = inb(0x64);
    if (status & 0x01) {
        uint8_t sc = inb(0x60);
        if (sc & 0x80) return;
        if (sc == 0x01) { putc('\n'); prompt(); return; }
        if (sc == 0x1C) {
            putc('\n');
            input_buffer[input_index] = '\0';
            run_cmd(input_buffer);
            input_index = 0;
        }
        else if (sc == 0x0E) {
            if (input_index > 0) { input_index--; putc('\b'); }
        }
        else if (sc < sizeof(scancode_to_ascii)) {
            char c = scancode_to_ascii[sc];
            if (c && input_index < INPUT_BUFFER_SIZE - 1) {
                input_buffer[input_index++] = c;
                putc(c);
            }
        }
    }
}

/* ==================== PIT TIMER ==================== */

static void init_pit() {
    outb(0x43, 0x34);
    outb(0x40, 0x9C);
    outb(0x40, 0x0E);
}

static void timer_tick() { uptime_ticks++; }

/* ==================== MAIN ==================== */

void main() {
    __asm__ volatile("cli");
    
    init_keyboard();
    init_pit();
    fs_init();
    ata_detect();
    
    clear();
    
    puts("╔════════════════════════════════════════╗\n");
    puts("║         Welcome to MyOS                ║\n");
    puts("║         Version 7.0 MAX                ║\n");
    puts("║    Advanced Filesystem Edition         ║\n");
    puts("╚════════════════════════════════════════╝\n\n");
    
    if (disk_detected) {
        puts("✓ Disk: DETECTED (");
        putc(disk_sectors / 1000000 + '0');
        puts(" MB)\n");
    } else {
        puts("✗ Disk: NOT DETECTED\n");
    }
    
    puts("✓ Filesystem: READY\n");
    puts("✓ Keyboard: ENABLED\n\n");
    
    puts("Type 'help' for available commands\n");
    puts("Type 'info' for system information\n\n");
    prompt();
    
    while (1) {
        check_keyboard();
        timer_tick();
    }
}