#ifndef BOOT_H
#define BOOT_H
void boot_sequence(void);
void halt_with_error(const char* msg);
void halt(void);
#endif // BOOT_H
