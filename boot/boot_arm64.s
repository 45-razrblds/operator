.text
.global _start

_start:
    /* Only proceed on CPU 0 */
    mrs     x0, mpidr_el1
    and     x0, x0, #3
    cbz     x0, 2f
1:  wfe
    b       1b
2:
    // Set up the stack
    ldr     x0, =_start
    mov     sp, x0

    // Clear BSS
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
    sub     x1, x1, x0
3:  str     xzr, [x0], #8
    subs    x1, x1, #8
    b.gt    3b

    // Jump to kernel_main
    bl      kernel_main

    // If kernel_main returns, halt the CPU
4:  wfe
    b       4b
