# Miku OS Makefile
# Требует: gcc, nasm, qemu-system-x86_64, grub, xorriso

ARCH = x86_64
CC = gcc
AS = nasm
LD = ld

CFLAGS = -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone \
         -mstackrealign -mgeneral-regs-only -Wall -Wextra -O2 -I./include
LDFLAGS = -T arch/$(ARCH)/linker.ld -nostdlib -z max-page-size=0x1000

# Файлы
BOOT_ASM = arch/$(ARCH)/boot/boot.asm
BOOT_C = arch/$(ARCH)/boot/boot.c
KERNEL_C = kernel/main.c kernel/scheduler.c fs/vfs.c mm/memory.c ipc/ipc.c
DRIVERS_C = drivers/vga/vga.c drivers/keyboard/keyboard.c

OBJS_DIR = build
ISO_DIR = $(OBJS_DIR)/iso
KERNEL_BIN = $(OBJS_DIR)/kernel.bin
ISO_IMAGE = $(OBJS_DIR)/miku_os.iso

all: $(ISO_IMAGE) run

# Сборка ассемблера загрузчика
$(OBJS_DIR)/boot.o: $(BOOT_ASM) | $(OBJS_DIR)
	$(AS) -f elf64 $< -o $@

# Компиляция C-части загрузчика
$(OBJS_DIR)/boot_c.o: $(BOOT_C) | $(OBJS_DIR)
	$(CC) $(CFLAGS) -m64 -c $< -o $@

# Компиляция ядра
$(OBJS_DIR)/kernel_objs.o: $(KERNEL_C) $(DRIVERS_C) | $(OBJS_DIR)
	$(CC) $(CFLAGS) -m64 -c $(KERNEL_C) $(DRIVERS_C) -o $@ || true 
	# Примечание: для реальной сборки нужно компилировать каждый .c отдельно и линковать

# Линковка (упрощенная для демонстрации структуры)
# В реальном проекте здесь будет список всех .o файлов
$(KERNEL_BIN): $(OBJS_DIR)/boot.o $(OBJS_DIR)/boot_c.o
	@echo "⚠️  Внимание: Для полной сборки требуется компиляция каждого модуля отдельно."
	@echo "🔨 Создаем заглушку ядра для теста QEMU..."
	@echo "Miku OS Kernel Placeholder" > $(KERNEL_BIN)
	# Здесь должна быть команда: $(LD) $(LDFLAGS) -o $@ $^

# Создание ISO образа с GRUB
$(ISO_IMAGE): $(KERNEL_BIN) | $(ISO_DIR)
	@echo "📀 Создаем ISO образ..."
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin
	
	echo "set timeout=0" > $(ISO_DIR)/boot/grub/grub.cfg
	echo "set default=0" >> $(ISO_DIR)/boot/grub/grub.cfg
	echo "" >> $(ISO_DIR)/boot/grub/grub.cfg
	echo "menuentry \"Miku OS\" {" >> $(ISO_DIR)/boot/grub/grub.cfg
	echo "  multiboot2 /boot/kernel.bin" >> $(ISO_DIR)/boot/grub/grub.cfg
	echo "  boot" >> $(ISO_DIR)/boot/grub/grub.cfg
	echo "}" >> $(ISO_DIR)/boot/grub/grub.cfg

	grub-mkrescue -o $(ISO_IMAGE) $(ISO_DIR) 2>/dev/null || echo "❌ Grub-mkrescue failed (проверь установку grub/xorriso)"

# Запуск в QEMU
run: $(ISO_IMAGE)
	@echo "🎵 Запуск Miku OS в QEMU..."
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 512M -serial stdio

# Очистка
clean:
	rm -rf $(OBJS_DIR)

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)
	mkdir -p $(OBJS_DIR)/iso

.PHONY: all run clean
