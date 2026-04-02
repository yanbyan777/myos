/*
 * Miku OS v3.0 "Hatsune Ultimate" - Keyboard Driver (пункт 4)
 * Обработка ввода с PS/2 клавиатуры через прерывания
 */

#include "../include/miku_os.h"

#define KB_DATA_PORT    0x60
#define KB_STATUS_PORT  0x64
#define KB_CMD_PORT     0x64

/* Статус клавиатуры */
#define KB_STATUS_OUTPUT_FULL   0x01
#define KB_STATUS_INPUT_FULL    0x02
#define KB_STATUS_SYSTEM_FLAG   0x04
#define KB_STATUS_COMMAND_DATA  0x08
#define KB_STATUS_TIMEOUT_ERROR 0x40
#define KB_STATUS_PARITY_ERROR  0x80

/* Коды модификаторов */
#define KB_LSHIFT_RELEASED  0xAA
#define KB_LSHIFT_PRESSED   0x2A
#define KB_RSHIFT_PRESSED   0x36
#define KB_RSHIFT_RELEASED  0xB6
#define KB_CTRL_PRESSED     0x1D
#define KB_CTRL_RELEASED    0x9D
#define KB_ALT_PRESSED      0x38
#define KB_ALT_RELEASED     0xB8
#define KB_CAPSLOCK         0x3A
#define KB_NUMLOCK          0x45
#define KB_SCROLLLOCK       0x46

/* Раскладка QWERTY */
static const char g_kb_us_map[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' '
};

/* Shift-раскладка */
static const char g_kb_us_shift_map[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*',
    0, ' '
};

/* Буфер клавиатуры */
static keyboard_driver_t g_keyboard = {
    .shift_pressed = 0,
    .ctrl_pressed = 0,
    .alt_pressed = 0,
    .caps_lock = 0,
    .num_lock = 0,
    .scroll_lock = 0,
    .buf_start = 0,
    .buf_end = 0,
    .lock = {0},
    .wait = {0}
};

/* Проверка наличия данных в буфере клавиатуры */
static bool kb_data_available(void) {
    return (inb(KB_STATUS_PORT) & KB_STATUS_OUTPUT_FULL) != 0;
}

/* Чтение скан-кода */
static u8 kb_read_scancode(void) {
    while (!kb_data_available()) {
        __asm__ volatile ("hlt");
    }
    return inb(KB_DATA_PORT);
}

/* Обработка нажатия клавиши */
static void kb_handle_keypress(u8 scancode) {
    spin_lock(&g_keyboard.lock);
    
    char c = 0;
    
    /* Обработка модификаторов */
    switch (scancode) {
        case KB_LSHIFT_PRESSED:
        case KB_RSHIFT_PRESSED:
            g_keyboard.shift_pressed = 1;
            goto unlock;
            
        case KB_LSHIFT_RELEASED:
        case KB_RSHIFT_RELEASED:
            g_keyboard.shift_pressed = 0;
            goto unlock;
            
        case KB_CTRL_PRESSED:
            g_keyboard.ctrl_pressed = 1;
            goto unlock;
            
        case KB_CTRL_RELEASED:
            g_keyboard.ctrl_pressed = 0;
            goto unlock;
            
        case KB_ALT_PRESSED:
            g_keyboard.alt_pressed = 1;
            goto unlock;
            
        case KB_ALT_RELEASED:
            g_keyboard.alt_pressed = 0;
            goto unlock;
            
        case KB_CAPSLOCK:
            g_keyboard.caps_lock = !g_keyboard.caps_lock;
            goto unlock;
            
        case KB_NUMLOCK:
            g_keyboard.num_lock = !g_keyboard.num_lock;
            goto unlock;
            
        case KB_SCROLLLOCK:
            g_keyboard.scroll_lock = !g_keyboard.scroll_lock;
            goto unlock;
    }
    
    /* Получение символа */
    if (scancode < sizeof(g_kb_us_map)) {
        if (g_keyboard.shift_pressed) {
            c = g_kb_us_shift_map[scancode];
        } else {
            c = g_kb_us_map[scancode];
        }
        
        /* Обработка CapsLock для букв */
        if (g_keyboard.caps_lock && c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        } else if (g_keyboard.caps_lock && c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }
        
        /* Обработка Ctrl */
        if (g_keyboard.ctrl_pressed && c >= '@' && c <= '_') {
            c = c - '@';
        }
        
        /* Добавление в буфер */
        if (c != 0) {
            u32 next_end = (g_keyboard.buf_end + 1) % sizeof(g_keyboard.buffer);
            
            if (next_end != g_keyboard.buf_start) {
                g_keyboard.buffer[g_keyboard.buf_end] = c;
                g_keyboard.buf_end = next_end;
                
                /* Разбудить ожидающие процессы */
                wake_up(&g_keyboard.wait);
            }
        }
    }
    
unlock:
    spin_unlock(&g_keyboard.lock);
}

/* Обработчик прерывания клавиатуры (IRQ1) */
void keyboard_irq_handler(struct pt_regs* regs) {
    u8 scancode = kb_read_scancode();
    
    /* Игнорируем release codes (бит 7 установлен) */
    if (!(scancode & 0x80)) {
        kb_handle_keypress(scancode);
    }
}

/* Инициализация клавиатуры */
void keyboard_init(void) {
    spin_lock(&g_keyboard.lock);
    
    /* Очистка буфера */
    g_keyboard.buf_start = 0;
    g_keyboard.buf_end = 0;
    g_keyboard.shift_pressed = 0;
    g_keyboard.ctrl_pressed = 0;
    g_keyboard.alt_pressed = 0;
    g_keyboard.caps_lock = 0;
    g_keyboard.num_lock = 0;
    g_keyboard.scroll_lock = 0;
    
    /* Настройка контроллера клавиатуры */
    outb(KB_CMD_PORT, 0xAE);  /* Enable keyboard interface */
    outb(KB_CMD_PORT, 0x20);  /* Get current configuration */
    
    u8 config = inb(KB_DATA_PORT);
    config |= 0x01;           /* Enable keyboard interrupt */
    config &= ~0x10;          /* Disable translation */
    
    outb(KB_CMD_PORT, 0x60);  /* Set configuration */
    outb(KB_DATA_PORT, config);
    
    /* Включить сканирование */
    outb(KB_DATA_PORT, 0xF4);
    
    spin_unlock(&g_keyboard.lock);
    
    /* Установить обработчик прерываний */
    irq_install_handler(1, keyboard_irq_handler);
}

/* Чтение символа из буфера (блокирующее) */
char keyboard_read_char(void) {
    char c;
    
    spin_lock(&g_keyboard.lock);
    
    while (g_keyboard.buf_start == g_keyboard.buf_end) {
        spin_unlock(&g_keyboard.lock);
        wait_event(&g_keyboard.wait);
        spin_lock(&g_keyboard.lock);
    }
    
    c = g_keyboard.buffer[g_keyboard.buf_start];
    g_keyboard.buf_start = (g_keyboard.buf_start + 1) % sizeof(g_keyboard.buffer);
    
    spin_unlock(&g_keyboard.lock);
    
    return c;
}

/* Чтение строки (до newline или max_len) */
int keyboard_read_line(char* buffer, int max_len) {
    int i = 0;
    
    while (i < max_len - 1) {
        char c = keyboard_read_char();
        
        if (c == '\n' || c == '\r') {
            buffer[i] = '\0';
            return i;
        }
        
        if (c == '\b') {
            if (i > 0) {
                i--;
            }
        } else {
            buffer[i++] = c;
        }
    }
    
    buffer[i] = '\0';
    return i;
}

/* Проверка наличия символов в буфере */
bool keyboard_has_input(void) {
    bool has;
    
    spin_lock(&g_keyboard.lock);
    has = (g_keyboard.buf_start != g_keyboard.buf_end);
    spin_unlock(&g_keyboard.lock);
    
    return has;
}

/* Получить статус модификаторов */
u8 keyboard_get_modifiers(void) {
    u8 mods = 0;
    
    spin_lock(&g_keyboard.lock);
    if (g_keyboard.shift_pressed) mods |= 0x01;
    if (g_keyboard.ctrl_pressed)  mods |= 0x02;
    if (g_keyboard.alt_pressed)   mods |= 0x04;
    if (g_keyboard.caps_lock)     mods |= 0x08;
    if (g_keyboard.num_lock)      mods |= 0x10;
    if (g_keyboard.scroll_lock)   mods |= 0x20;
    spin_unlock(&g_keyboard.lock);
    
    return mods;
}
