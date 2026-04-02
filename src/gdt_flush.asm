bits 32

global gdt_flush

gdt_flush:
    mov eax, [esp + 4]      ; Получаем адрес GDT из аргумента
    lgdt [eax]              ; Загружаем GDT
    
    ; Обновляем сегментные регистры
    mov ax, 0x10            ; Селектор данных (2-й дескриптор, индекс 2 * 8 = 0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Стек для возврата
    jmp 0x08:.flush         ; Селектор кода (1-й дескриптор, индекс 1 * 8 = 0x08)
.flush:
    ret
