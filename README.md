# Operator OS

![Language](https://img.shields.io/badge/language-C-blue.svg?style=flat)
![Assembly](https://img.shields.io/badge/assembly-NASM-orange.svg?style=flat)
![Architecture](https://img.shields.io/badge/arch-i386-green.svg?style=flat)
![Build](https://img.shields.io/badge/build-Make-red.svg?style=flat)
![Emulator](https://img.shields.io/badge/emulator-QEMU-purple.svg?style=flat)
![Bootloader](https://img.shields.io/badge/bootloader-GRUB-yellow.svg?style=flat)

A 32-bit operating system targeting the i386 architecture. Built with C and x86 assembly, using GRUB as bootloader and QEMU for emulation and testing.

## Technical Overview

### Architecture
- **Target**: Intel 32-bit x86 (i386)
- **Bootloader**: GRUB multiboot specification
- **Kernel**: Monolithic kernel written in C with inline assembly
- **Memory Model**: 32-bit protected mode
- **Build System**: GNU Make

### Implementation Details
- Boot process handled via GRUB multiboot header
- Kernel entry point in assembly transitioning to C
- Hardware abstraction through driver interfaces
- Memory management with paging support
- Interrupt handling and system call interface

## Build Requirements

### Toolchain
- **GCC**: Cross-compilation support for i386 target
- **NASM**: Netwide Assembler for x86 assembly code
- **GNU Make**: Build automation
- **GRUB**: `grub-mkrescue` for ISO generation
- **QEMU**: `qemu-system-i386` for emulation

### Dependencies Installation

**Ubuntu/Debian:**
```bash
apt install gcc-multilib nasm qemu-system-x86 grub-pc-bin xorriso
```

**Arch Linux:**
```bash
pacman -S gcc-multilib nasm qemu-system-x86 grub xorriso
```

**macOS:**
```bash
brew install nasm qemu grub xorriso
# Additional setup may be required for GCC multilib
```

## Build Process

```bash
# Clean build artifacts
make clean

# Compile kernel and create bootable image
make os-image

# Launch in QEMU emulator
make run

# Build and run
make all
```

## System Components

### Boot Sequence
1. GRUB loads multiboot-compliant kernel
2. Assembly bootstrap sets up protected mode
3. C kernel initialization
4. Hardware detection and driver loading
5. System ready state

### Memory Layout
- **0x00000000-0x000FFFFF**: Real mode memory and BIOS
- **0x00100000+**: Kernel loaded by GRUB
- **Higher half**: Kernel virtual memory space

### Development Environment
The system uses QEMU for hardware emulation, allowing development on any host platform. The emulated environment provides standard PC hardware interfaces including VGA display, keyboard, and timer interrupts.

## Technical Specifications

- **Instruction Set**: x86-32 (IA-32)
- **Memory Management**: Paging with 4KB pages
- **Interrupt Handling**: IDT-based interrupt management
- **System Calls**: Software interrupt interface
- **Device Drivers**: Modular driver architecture

## Repository Structure

Standard OS development layout with boot code, kernel sources, and hardware drivers separated into logical modules.


![GitHub stats](https://github-readme-stats.vercel.app/api?username=45-razrblds&repo=operator&show_icons=true)