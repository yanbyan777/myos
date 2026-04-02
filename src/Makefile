CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-builtin -fno-pic
LDFLAGS = -m elf_i386 -nostdlib -T linker.ld

all: kernel.bin

boot.o: boot.asm
	nasm -f elf32 boot.asm -o boot.o

kernel.o: kernel.c
	gcc $(CFLAGS) -c kernel.c -o kernel.o

kernel.bin: boot.o kernel.o
	ld $(LDFLAGS) boot.o kernel.o -o kernel.bin

iso: kernel.bin
	mkdir -p isodir/boot/grub
	cp kernel.bin isodir/boot/
	echo 'set timeout=0' > isodir/boot/grub/grub.cfg
	echo 'set default=0' >> isodir/boot/grub/grub.cfg
	echo 'menuentry "MyOS" {' >> isodir/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin' >> isodir/boot/grub/grub.cfg
	echo '  boot' >> isodir/boot/grub/grub.cfg
	echo '}' >> isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

run: kernel.bin
	qemu-system-i386 -kernel kernel.bin -drive format=raw,file=disk.img -vnc :0

clean:
	rm -rf isodir myos.iso *.o *.bin
