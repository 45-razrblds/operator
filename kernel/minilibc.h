#ifndef MINILIBC_H
#define MINILIBC_H
#include <stddef.h>
void* memset(void* s, int c, size_t n);
void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
size_t strlen(const char* s);
char* strcpy(char* d, const char* s);
char* strncpy(char* d, const char* s, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strchr(const char* s, int c);
char* strrchr(const char* s, int c);
char* strtok(char* str, const char* delim);
int snprintf(char* buf, size_t n, const char* fmt, ...);
#endif // MINILIBC_H
