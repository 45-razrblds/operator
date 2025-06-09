.PHONY: all clean

CC=x86_64-elf-gcc
CFLAGS=-m32 -ffreestanding

all: os.bin

clean:
	rm -rf *.o *.bin *.iso isodir kernel/*.o

multiboot_header.o: boot/multiboot_header.s
	nasm -f elf32 $< -o $@

kernel.o: kernel/kernel.c kernel/version.h
	$(CC) $(CFLAGS) -c $< -o $@

shutdown.o: kernel/shutdown.c kernel/shutdown.h
	$(CC) $(CFLAGS) -c $< -o $@

reboot.o: kernel/reboot.c kernel/reboot.h
	$(CC) $(CFLAGS) -c $< -o $@

memory.o: kernel/memory.c kernel/memory.h
	$(CC) $(CFLAGS) -c $< -o $@

timer.o: kernel/timer.c kernel/timer.h
	$(CC) $(CFLAGS) -c $< -o $@

terminal.o: kernel/terminal.c kernel/terminal.h
	$(CC) $(CFLAGS) -c $< -o $@

debug.o: kernel/debug.c kernel/debug.h
	$(CC) $(CFLAGS) -c $< -o $@

command.o: kernel/command.c kernel/command.h
	$(CC) $(CFLAGS) -c $< -o $@

opfs.o: kernel/opfs.c kernel/opfs.h
	$(CC) $(CFLAGS) -c $< -o $@

minilibc.o: kernel/minilibc.c kernel/minilibc.h
	$(CC) $(CFLAGS) -c $< -o $@

boot.o: kernel/boot.c kernel/boot.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel/keyboard.o: kernel/keyboard.c kernel/keyboard.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel/io.o: kernel/io.c kernel/io.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel/serial.o: kernel/serial.c kernel/serial.h
	$(CC) $(CFLAGS) -c $< -o $@

os.bin: multiboot_header.o kernel.o shutdown.o reboot.o memory.o timer.o terminal.o debug.o command.o opfs.o minilibc.o boot.o kernel/keyboard.o kernel/io.o kernel/serial.o
	x86_64-elf-ld -m elf_i386 -T linker.ld -o $@ $^

# To build a bootable ISO, use:
#   ./build_iso.sh
# To run:
#   qemu-system-i386 -cdrom os.iso
