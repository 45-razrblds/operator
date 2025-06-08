# Operator OS

A simple 32-bit operating system (i386).

## Building and Running

This is a 32-bit x86 operating system that can be built and run on any platform using QEMU for emulation.

### Prerequisites

- gcc (with multilib support for 32-bit compilation)
- nasm (for assembling x86 boot code)
- qemu-system-i386 (for emulation)
- grub-mkrescue (for creating bootable ISO)

### Commands

```fish
# Clean build artifacts
make clean

# Build the OS
make os-image

# Run in QEMU
make run

# Build and run in one command
make all
```

## System Details

The operating system is built for x86 (i386) architecture and uses GRUB as the bootloader. QEMU emulates the x86 processor, allowing you to develop and test the OS on any platform, including Apple Silicon Macs.