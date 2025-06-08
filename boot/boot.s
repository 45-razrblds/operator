; Multiboot header for GRUB
MAGIC    equ 0x1BADB002
FLAGS    equ 0x0
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text
extern kernel_main
global _start
_start:
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang
