; IRQ Stub для клавиатуры (IRQ1 = interrupt 33)
bits 32
global irq1_handler
extern keyboard_handler

irq1_handler:
    pushad
    push ds
    push es
    
    call keyboard_handler
    
    pop es
    pop ds
    popad
    iret
