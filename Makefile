all: clean

clean:
	rm -rf *.o *.bin *.iso isodir

multiboot_header.o: boot/multiboot_header.s
	nasm -f elf32 $< -o $@

kernel.o: kernel/kernel.c
	x86_64-elf-gcc -m32 -ffreestanding -c $< -o $@

os.bin: multiboot_header.o kernel.o
	x86_64-elf-ld -m elf_i386 -T linker.ld -o $@ multiboot_header.o kernel.o

# To build a bootable ISO, use:
#   ./build_iso.sh
# To run:
#   qemu-system-i386 -cdrom os.iso
