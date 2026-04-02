# Miku OS v3.0 Makefile
CC = gcc
AS = nasm
LD = ld
CFLAGS = -ffreestanding -O2 -Wall -Wextra -Iinclude/ -m64 -mno-red-zone -mcmodel=large -fno-pic -fno-pie -nostdlib -nodefaultlibs
ASFLAGS = -f elf64
LDFLAGS = -n -Ttext 0x100000 --oformat binary

OBJS = boot.o main.o

all: miku_os.bin miku_os.iso

boot.o: arch/x86_64/boot/boot.asm
	$(AS) $(ASFLAGS) $< -o $@

main.o: kernel/main.c include/miku_os.h
	$(CC) $(CFLAGS) -c $< -o $@

miku_os.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

miku_os.iso: miku_os.bin
	mkdir -p iso/boot/grub
	cp miku_os.bin iso/boot/
	echo 'set timeout=5' > iso/boot/grub/grub.cfg
	echo 'menuentry "Miku OS v3.0" {' >> iso/boot/grub/grub.cfg
	echo '    multiboot2 /boot/miku_os.bin' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ iso 2>/dev/null || echo "Note: Install grub for ISO creation"

clean:
	rm -f *.o miku_os.bin miku_os.iso
	rm -rf iso/

run: miku_os.iso
	qemu-system-x86_64 -cdrom $< -smp 2 -m 1G -name "Miku OS v3.0"

debug: miku_os.iso
	qemu-system-x86_64 -cdrom $< -s -S -smp 2 -m 1G

gdb:
	gdb -ex "target remote localhost:1234" -ex "symbol-file miku_os.bin"

.PHONY: all clean run debug gdb
