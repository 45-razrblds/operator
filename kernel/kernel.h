#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include "module.h"

// uwu~ Kernel version
#define KERNEL_VERSION "0.1.0"

// kawaii Kernel modules >w<
extern module_status_t kernel_modules[7];

// uwu~ Kernel functions :3
void kernel_init(void);
void kernel_main(void);
void kernel_panic(const char* message);

#endif // KERNEL_H 