; ================================================================
;  MyOS Boot Loader with GDT
; ================================================================

[bits 32]

[global gdt_start]
[global gdt_end]
[global gdt_descriptor]

gdt_start:
    dq 0x0

    ; Code segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

    ; Data segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ 0x08
DATA_SEG equ 0x10
