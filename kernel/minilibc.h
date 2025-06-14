#ifndef MINILIBC_H
#define MINILIBC_H

#include <stdarg.h>
#include <stddef.h>

// String-Funktionen
void* memset(void* s, int c, size_t n);
void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strchr(const char* s, int c);
char* strrchr(const char* s, int c);
char* strtok(char* str, const char* delim);

// Formatierte Ausgabe
int vsnprintf(char* str, size_t size, const char* format, va_list args);
int snprintf(char* str, size_t size, const char* format, ...);

#endif // MINILIBC_H
