.PHONY: all clean

# uwu~ Colors and Icons >w<
BLUE=\033[0;34m
GREEN=\033[0;32m
RED=\033[0;31m
YELLOW=\033[0;33m
NC=\033[0m # No Color

# kawaii Nerd Font Icons :3
ICON_BUILD=🔨
ICON_CLEAN=🧹
ICON_COMPILE=⚙️
ICON_LINK=🔗
ICON_ERROR=❌
ICON_SUCCESS=✅

CC=x86_64-elf-gcc
CFLAGS=-m32 -ffreestanding -I./kernel

all: os.bin
	@echo "${GREEN}${ICON_SUCCESS} Build completed uwu~${NC}"

clean:
	@echo "${YELLOW}${ICON_CLEAN} Cleaning up...${NC}"
	@rm -rf *.o *.bin *.iso isodir kernel/*.o
	@echo "${GREEN}${ICON_SUCCESS} Cleanup completed >w<${NC}"

multiboot_header.o: boot/multiboot_header.s
	@echo "${BLUE}${ICON_COMPILE} Compiling Multiboot Header...${NC}"
	@nasm -f elf32 $< -o $@

kernel.o: kernel/kernel.c kernel/version.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Kernel...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

error.o: kernel/error.c kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Error Handler...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

minilibc.o: kernel/minilibc.c kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Mini LibC...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

shutdown.o: kernel/shutdown.c kernel/shutdown.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Shutdown Module...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

reboot.o: kernel/reboot.c kernel/reboot.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Reboot Module...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

memory.o: kernel/memory.c kernel/memory.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Memory Manager...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

timer.o: kernel/timer.c kernel/timer.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Timer...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

terminal.o: kernel/terminal.c kernel/terminal.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Terminal...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

debug.o: kernel/debug.c kernel/debug.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Debug Module...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

command.o: kernel/command.c kernel/command.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Command Handler...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

opfs.o: kernel/opfs.c kernel/opfs.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling OPFS...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

boot.o: kernel/boot.c kernel/boot.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Boot Module...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

kernel/keyboard.o: kernel/keyboard.c kernel/keyboard.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling Keyboard Driver...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

kernel/io.o: kernel/io.c kernel/io.h kernel/error.h kernel/minilibc.h
	@echo "${BLUE}${ICON_COMPILE} Compiling IO Module...${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

os.bin: multiboot_header.o kernel.o error.o minilibc.o shutdown.o reboot.o memory.o timer.o terminal.o debug.o command.o opfs.o boot.o kernel/keyboard.o kernel/io.o
	@echo "${YELLOW}${ICON_LINK} Linking objects to Kernel...${NC}"
	@x86_64-elf-ld -m elf_i386 -T linker.ld -o $@ $^
	@echo "${GREEN}${ICON_SUCCESS} Kernel linked successfully uwu~${NC}"

# To build a bootable ISO, use:
#   ./build_iso.sh
# To run:
#   qemu-system-i386 -cdrom os.iso
