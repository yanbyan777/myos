; ============================================================
; Miku OS v3.0 - Bootloader (Multiboot2 + Long Mode)
; 16-bit Real Mode → 32-bit Protected Mode → 64-bit Long Mode
; ============================================================

MB2_MAGIC equ 0xe85250d6
MB2_ARCH equ 0
MB2_HEADER_LEN equ header_end - miku_header

section .multiboot
align 8

miku_header:
    dd MB2_MAGIC              ; magic number
    dw 0                      ; architecture (0 = i386)
    dd header_end - miku_header ; header length
    dd -(MB2_MAGIC + 0 + (header_end - miku_header)) ; checksum

; Tag: Request memory map
dw 5                          ; type (memory info)
dw 1                          ; flags
dd 8                          ; size
header_end:

; Stack for early boot
section .bss
align 16
stack_low:
    resb 8192
stack_low_end:

section .text
bits 32

global _start
extern kernel_main
extern gdt64_desc
extern idt_load

_start:
    ; ========================================
    ; 1. Multiboot2 entry point (32-bit)
    ; ========================================
    cli
    
    ; Save Multiboot2 magic and pointer
    mov [mb_magic], eax
    mov [mb_mboot_ptr], ebx
    
    ; Set up stack
    mov esp, stack_low_end
    
    ; Disable interrupts
    cli
    
    ; ========================================
    ; 2. Enable PAE (Physical Address Extension)
    ; ========================================
    mov eax, cr4
    or eax, (1 << 5)        ; PAE bit
    mov cr4, eax
    
    ; ========================================
    ; 3. Create Page Map Level 4 Table (PML4T)
    ; ========================================
    lea edi, [pml4_table]
    xor eax, eax
    mov ecx, 512
.clear_pml4:
    mov [edi], eax
    add edi, 8
    loop .clear_pml4
    
    ; ========================================
    ; 4. Create Page Directory Pointer Table (PDPT)
    ; ========================================
    lea edi, [pdpt_table]
    xor eax, eax
    mov ecx, 512
.clear_pdpt:
    mov [edi], eax
    add edi, 8
    loop .clear_pdpt
    
    ; Map first 2MB (identity mapping for kernel)
    lea eax, [pd_table]
    or eax, (1 << 0)        ; Present
    or eax, (1 << 1)        ; Read/Write
    mov [pdpt_table], eax
    
    ; ========================================
    ; 5. Create Page Directory (PD)
    ; ========================================
    lea edi, [pd_table]
    xor ecx, ecx
.map_pd:
    push ecx
    shl ecx, 21             ; Calculate physical address
    mov eax, ecx
    and eax, ~0x1FFFFF      ; Align to 2MB
    or eax, (1 << 7)        ; 2MB pages
    or eax, (1 << 1)        ; Read/Write
    or eax, (1 << 0)        ; Present
    mov [edi], eax
    pop ecx
    add edi, 8
    inc ecx
    cmp ecx, 512
    jl .map_pd
    
    ; ========================================
    ; 6. Load CR3 with PML4T address
    ; ========================================
    lea eax, [pml4_table]
    mov cr3, eax
    
    ; ========================================
    ; 7. Enable Long Mode (EFER.LME)
    ; ========================================
    mov ecx, 0xC0000080     ; EFER MSR
    rdmsr
    or eax, (1 << 8)        ; LME bit
    wrmsr
    
    ; ========================================
    ; 8. Enable Paging (CR0.PG)
    ; ========================================
    mov eax, cr0
    or eax, (1 << 31)       ; PG bit
    mov cr0, eax
    
    ; ========================================
    ; 9. Jump to 64-bit mode
    ; ========================================
    lgdt [gdt64_desc]
    
    jmp 0x08:kernel_main_64
    
    ; Should never reach here
.hang:
    hlt
    jmp .hang

; ============================================================
; 64-bit Kernel Entry Point
; ============================================================
section .text
bits 64

global kernel_main_64
extern kernel_main_c

kernel_main_64:
    ; Set up segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    
    ; Set up stack
    lea rsp, [stack_high_end]
    
    ; Call C kernel main (pass Multiboot2 info)
    push qword [mb_mboot_ptr]
    push qword [mb_magic]
    call kernel_main_c
    
    ; Halt if kernel returns
.halt:
    cli
    hlt
    jmp .halt

; ============================================================
; GDT for 64-bit mode
; ============================================================
section .rodata
align 16

gdt64_start:
    ; Null descriptor
    dq 0x0000000000000000
    
    ; Code segment: base=0, limit=4GB, exec/read, DPL=0
    dq 0x00CF9A000000FFFF
    
    ; Data segment: base=0, limit=4GB, read/write, DPL=0
    dq 0x00CF92000000FFFF
    
    ; User code segment: DPL=3
    dq 0x00AFFA000000FFFF
    
    ; User data segment: DPL=3
    dq 0x00AF92000000FFFF
gdt64_end:

gdt64_desc:
    dw gdt64_end - gdt64_start - 1
    dq gdt64_start

; ============================================================
; Page Tables (aligned to 4KB)
; ============================================================
section .bss
align 4096

pml4_table:
    resq 512

pdpt_table:
    resq 512

pd_table:
    resq 512

; High stack for kernel
align 16
stack_high:
    resb 65536
stack_high_end:

; Multiboot2 info storage
section .data
align 8
mb_magic:       dq 0
mb_mboot_ptr:   dq 0
