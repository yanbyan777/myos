/*
 * Miku OS v3.0 "Hatsune Ultimate" - Bootloader через GRUB (пункт 1)
 * Multiboot совместимый загрузчик
 */

#include <stdint.h>
#include <multiboot2.h>
#include "../include/miku_os.h"

#define MULTIBOOT_MAGIC 0x36d76289
#define MULTIBOOT_FLAGS (MULTIBOOT2_TAG_TYPE_ADDRESS | MULTIBOOT2_ARCHITECTURE_I386)

__attribute__((section(".multiboot"), used))
struct multiboot_header {
    uint32_t magic;
    uint32_t architecture;
    uint32_t header_length;
    uint32_t checksum;
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t header_addr;
    uint32_t load_addr;
    uint32_t load_end_addr;
    uint32_t bss_end_addr;
    uint32_t entry_addr;
};

static struct multiboot_header mb_header __attribute__((aligned(8))) = {
    .magic = MULTIBOOT_MAGIC,
    .architecture = MULTIBOOT2_ARCHITECTURE_I386,
    .header_length = sizeof(mb_header),
    .checksum = -(MULTIBOOT_MAGIC + MULTIBOOT2_ARCHITECTURE_I386 + sizeof(mb_header)),
    .type = MULTIBOOT2_TAG_TYPE_ADDRESS,
    .flags = 0,
    .size = sizeof(mb_header),
    .header_addr = (uint32_t)&mb_header,
    .load_addr = (uint32_t)&mb_header,
    .load_end_addr = 0,
    .bss_end_addr = 0,
    .entry_addr = 0
};

extern void kernel_main(uint32_t magic, struct multiboot_info* mboot);

__attribute__((noreturn))
void boot_entry(uint32_t magic, struct multiboot_info* mboot) {
    /* Проверка Multiboot магии */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        while (1) {
            __asm__ volatile ("hlt");
        }
    }
    
    /* Переход к основному коду ядра */
    kernel_main(magic, mboot);
    
    /* Никогда не достигаем сюда */
    while (1) {
        __asm__ volatile ("hlt");
    }
}
