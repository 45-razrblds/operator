.PHONY: all clean

all: os.bin

clean:
	rm -rf *.o *.bin *.iso isodir

multiboot_header.o: boot/multiboot_header.s
	nasm -f elf32 $< -o $@

kernel.o: kernel/kernel.c kernel/version.h
	x86_64-el	f-gcc -m32 -ffreestanding -c $< -o $@

shutdown.o: kernel/shutdown.c kernel/shutdown.h
	x86_64-elf-gcc -m32 -ffreestanding -c $< -o $@

reboot.o: kernel/reboot.c kernel/reboot.h
	x86_64-elf-gcc -m32 -ffreestanding -c $< -o $@

os.bin: multiboot_header.o kernel.o shutdown.o reboot.o
	x86_64-elf-ld -m elf_i386 -T linker.ld -o $@ $^

# To build a bootable ISO, use:
#   ./build_iso.sh
# To run:
#   qemu-system-i386 -cdrom os.iso
