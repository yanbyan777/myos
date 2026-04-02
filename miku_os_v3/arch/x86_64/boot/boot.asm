; ============================================================================
; Miku OS v3.0 "Hatsune Ultimate" - Ассемблерный загрузчик (пункты 1, 2)
; Переход в 64-битный режим, установка GDT, IDT
; ============================================================================

[BITS 16]
[ORG 0x7c00]

KERNEL_OFFSET equ 0x100000

section .multiboot
align 8
MultibootHeader:
    dd 0xe85250d6               ; Magic number
    dd 0                        ; Protected mode
    dd HeaderEnd - MultibootHeader ; Header length
    
    ; Checksum
    dd -(0xe85250d6 + 0 + (HeaderEnd - MultibootHeader))
    
    ; End tag
    dw 0                        ; Type
    dw 0                        ; Flags
    dd 8                        ; Size

HeaderEnd:

section .text
global _start
extern kernel_main
extern gdt64_pointer
extern idt_load

_start:
    ; ========================================
    ; ПУНКТ 1: Bootloader через GRUB
    ; ========================================
    cli                         ; Отключаем прерывания
    
    ; Проверка Multiboot
    cmp eax, 0x36d76289
    jne hang
    
    ; Сохраняем информацию о загрузке
    mov [multiboot_info], ebx
    
    ; ========================================
    ; ПУНКТ 2: Переход в Protected Mode
    ; ========================================
    
    ; Отключаем все прерывания
    cli
    
    ; Отключаем PIC
    mov al, 0x11
    out 0x20, al
    out 0xA0, al
    
    mov al, 0x20
    out 0x21, al
    mov al, 0x28
    out 0xA1, al
    
    mov al, 0x04
    out 0x21, al
    mov al, 0x02
    out 0xA1, al
    
    mov al, 0x01
    out 0x21, al
    out 0xA1, al
    
    ; Загружаем GDT
    lgdt [gdt32_descriptor]
    
    ; Включаем Protected Mode
    mov eax, cr0
    or al, 1
    mov cr0, eax
    
    ; Дальний прыжок в 32-битный режим
    jmp 0x08:.pmode

[BITS 32]
.pmode:
    ; Устанавливаем сегментные регистры
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Устанавливаем стек
    mov esp, 0x800000
    
    ; ========================================
    ; Переход в Long Mode (64-bit)
    ; ========================================
    
    ; Включаем PAE (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    
    ; Создаем PML4 таблицу
    mov edi, 0x1000              ; Адрес PML4
    xor eax, eax
    mov ecx, 512
.clear_pml4:
    mov [edi + ecx * 8], eax
    loop .clear_pml4
    
    ; Создаем PDPT таблицу
    mov edi, 0x2000              ; Адрес PDPT
    xor eax, eax
    mov ecx, 512
.clear_pdpt:
    mov [edi + ecx * 8], eax
    loop .clear_pdpt
    
    ; Настраиваем первую запись PDPT для映射 первых 1GB
    mov eax, 0x3000 | 0x3        ; Адрес PDT + Present + Writable
    mov [0x2000], eax
    
    ; Создаем PDT таблицу для 1GB
    mov edi, 0x3000
    mov eax, 0x80000003          ; 2MB страницы, Present, Writable
    mov ecx, 512
.map_2mb:
    mov [edi + (ecx-1) * 8], eax
    add eax, 0x200000
    loop .map_2mb
    
    ; Загружаем CR3 (адрес PML4)
    mov eax, 0x1000
    mov cr3, eax
    
    ; Включаем EFER (Extended Feature Enable Register)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8               ; LME (Long Mode Enable)
    wrmsr
    
    ; Включаем paging
    mov eax, cr0
    or eax, 1 << 31              ; PG (Paging)
    mov cr0, eax
    
    ; Переход в 64-битный режим
    jmp 0x28:.longmode           ; Селектор кода x64

[BITS 64]
.longmode:
    ; Устанавливаем сегментные регистры для 64-бит
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Устанавливаем 64-битное стек
    mov rsp, 0xFFFF800000100000
    mov rbp, rsp
    
    ; ========================================
    ; Вызов ядра
    ; ========================================
    extern multiboot_info
    mov rdi, [multiboot_info]    ; Передаем multiboot info
    call kernel_main
    
    ; Если вернулись - зависаем
.hang:
    cli
    hlt
    jmp .hang

; Данные
multiboot_info: dq 0

; GDT для 32-бит
align 16
gdt32_start:
    dq 0x0000000000000000        ; Null descriptor
    dq 0x00CF9A000000FFFF        ; Code segment, base=0, limit=4GB
    dq 0x00CF92000000FFFF        ; Data segment, base=0, limit=4GB
gdt32_end:

gdt32_descriptor:
    dw gdt32_end - gdt32_start - 1
    dq gdt32_start

; GDT для 64-бит
align 16
gdt64_start:
    dq 0x0000000000000000        ; Null descriptor
    dq 0x00209A0000000000        ; Code segment, 64-bit
    dq 0x0000920000000000        ; Data segment
gdt64_end:

gdt64_descriptor:
    dw gdt64_end - gdt64_start - 1
    dq gdt64_start

section .bss
align 16
stack_bottom:
    resb 65536                   ; 64KB стек
stack_top:
