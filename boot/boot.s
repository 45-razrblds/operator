; Multiboot header for GRUB
MAGIC    equ 0x1BADB002
FLAGS    equ 0x0
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
    align 4
    dd MAGIC          ; magic
    dd FLAGS                ; flags
    dd CHECKSUM       ; checksum

section .text
extern kernel_main
global _start
_start:
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang
