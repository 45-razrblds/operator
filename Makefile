all: clean os-image run

clean:
	rm -rf *.o *.bin *.iso isodir

kernel.o: kernel/kernel.c
	gcc -m32 -ffreestanding -c $< -o $@

boot.o: boot/boot.s
	nasm -f elf32 $< -o $@

os.bin: linker.ld boot.o kernel.o
	ld -m elf_i386 -T linker.ld -o $@ boot.o kernel.o

os-image: os.bin
	mkdir -p isodir/boot/grub
	cp $< isodir/boot/os.bin
	echo 'set timeout=0\nmenuentry "My OS" {\n multiboot /boot/os.bin\n}' > isodir/boot/grub/grub.cfg
	grub-mkrescue -o os.iso isodir

run:
	qemu-system-i386 -cdrom os.iso
