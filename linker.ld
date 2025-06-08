ENTRY(_start)

SECTIONS {
    /* Place Multiboot header at the very start */
    . = 0x0;
    .multiboot : {
        *(.multiboot)
    }
    /* Now set the load address for the rest of the kernel */
    . = 1M;
    .text : {
        *(.text*)
    }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    .bss : {
        *(.bss*)
        *(COMMON)
    }
}
