/*
 * Miku OS v3.0 "Hatsune Ultimate" - VGA Driver (пункт 3)
 * Вывод текста и графики через VGA буфер
 */

#include "../include/miku_os.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

/* Цвета VGA */
enum vga_color {
    VGA_BLACK = 0,
    VGA_BLUE = 1,
    VGA_GREEN = 2,
    VGA_CYAN = 3,
    VGA_RED = 4,
    VGA_MAGENTA = 5,
    VGA_BROWN = 6,
    VGA_LIGHT_GREY = 7,
    VGA_DARK_GREY = 8,
    VGA_LIGHT_BLUE = 9,
    VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11,
    VGA_LIGHT_RED = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_LIGHT_BROWN = 14,
    VGA_WHITE = 15,
};

static u16* g_vga_buffer = (u16*)VGA_MEMORY;
static u32 g_cursor_x = 0;
static u32 g_cursor_y = 0;
static u8 g_fg_color = VGA_WHITE;
static u8 g_bg_color = VGA_BLACK;
static spinlock_t g_vga_lock = {0};

/* Получить атрибут цвета */
static inline u8 vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

/* Создать символ VGA */
static inline u16 vga_entry(unsigned char uc, u8 color) {
    return (u16)uc | ((u16)color << 8);
}

/* Инициализация VGA */
void vga_init(void) {
    spin_lock(&g_vga_lock);
    
    g_cursor_x = 0;
    g_cursor_y = 0;
    g_fg_color = VGA_WHITE;
    g_bg_color = VGA_BLACK;
    
    /* Очистить экран */
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            g_vga_buffer[index] = vga_entry(' ', vga_entry_color(g_fg_color, g_bg_color));
        }
    }
    
    spin_unlock(&g_vga_lock);
}

/* Установить цвет */
void vga_set_color(u8 fg, u8 bg) {
    spin_lock(&g_vga_lock);
    g_fg_color = fg;
    g_bg_color = bg;
    spin_unlock(&g_vga_lock);
}

/* Передвинуть курсор */
static void vga_move_cursor(u32 x, u32 y) {
    u16 pos = y * VGA_WIDTH + x;
    
    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

/* Прокрутка экрана */
static void vga_scroll(void) {
    if (g_cursor_y >= VGA_HEIGHT) {
        /* Сдвинуть все строки вверх */
        for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                g_vga_buffer[y * VGA_WIDTH + x] = 
                    g_vga_buffer[(y + 1) * VGA_WIDTH + x];
            }
        }
        
        /* Очистить последнюю строку */
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            g_vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = 
                vga_entry(' ', vga_entry_color(g_fg_color, g_bg_color));
        }
        
        g_cursor_y = VGA_HEIGHT - 1;
    }
}

/* Вывод символа */
void vga_putchar(char c) {
    spin_lock(&g_vga_lock);
    
    switch (c) {
        case '\n':
            g_cursor_x = 0;
            g_cursor_y++;
            break;
            
        case '\r':
            g_cursor_x = 0;
            break;
            
        case '\t':
            g_cursor_x = (g_cursor_x + 8) & ~7;
            if (g_cursor_x >= VGA_WIDTH) {
                g_cursor_x = 0;
                g_cursor_y++;
            }
            break;
            
        case '\b':
            if (g_cursor_x > 0) {
                g_cursor_x--;
                g_vga_buffer[g_cursor_y * VGA_WIDTH + g_cursor_x] = 
                    vga_entry(' ', vga_entry_color(g_fg_color, g_bg_color));
            }
            break;
            
        default:
            if (g_cursor_x >= VGA_WIDTH) {
                g_cursor_x = 0;
                g_cursor_y++;
            }
            
            vga_scroll();
            
            g_vga_buffer[g_cursor_y * VGA_WIDTH + g_cursor_x] = 
                vga_entry(c, vga_entry_color(g_fg_color, g_bg_color));
            g_cursor_x++;
            break;
    }
    
    vga_move_cursor(g_cursor_x, g_cursor_y);
    spin_unlock(&g_vga_lock);
}

/* Вывод строки */
void vga_putstr(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

/* Вывод числа в шестнадцатеричном формате */
void vga_puthex(u64 value) {
    static const char hex_chars[] = "0123456789ABCDEF";
    char buffer[18];
    int i = 0;
    
    buffer[i++] = '0';
    buffer[i++] = 'x';
    
    for (int j = 14; j >= 0; j -= 2) {
        buffer[i++] = hex_chars[(value >> j) & 0xF];
        buffer[i++] = hex_chars[(value >> (j - 4)) & 0xF];
    }
    
    buffer[i] = '\0';
    vga_putstr(buffer);
}

/* Вывод десятичного числа */
void vga_putdec(s64 value) {
    char buffer[22];
    int i = 0;
    bool negative = false;
    
    if (value < 0) {
        negative = true;
        value = -value;
    }
    
    do {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);
    
    if (negative) {
        buffer[i++] = '-';
    }
    
    /* Развернуть строку */
    for (int j = 0; j < i / 2; j++) {
        char tmp = buffer[j];
        buffer[j] = buffer[i - 1 - j];
        buffer[i - 1 - j] = tmp;
    }
    
    buffer[i] = '\0';
    vga_putstr(buffer);
}

/* Очистка экрана */
void vga_clear(void) {
    spin_lock(&g_vga_lock);
    
    g_cursor_x = 0;
    g_cursor_y = 0;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            g_vga_buffer[index] = vga_entry(' ', vga_entry_color(g_fg_color, g_bg_color));
        }
    }
    
    vga_move_cursor(0, 0);
    spin_unlock(&g_vga_lock);
}

/* Получить размеры экрана */
u32 vga_get_width(void) {
    return VGA_WIDTH;
}

u32 vga_get_height(void) {
    return VGA_HEIGHT;
}

/* Структура драйвера для регистрации */
vga_driver_t g_vga_driver = {
    .buffer = (u16*)VGA_MEMORY,
    .width = VGA_WIDTH,
    .height = VGA_HEIGHT,
    .cursor_x = 0,
    .cursor_y = 0,
    .fg_color = VGA_WHITE,
    .bg_color = VGA_BLACK,
    .lock = {0}
};
