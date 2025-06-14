#include "minilibc.h"
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    if (d == s) return dest;
    
    if (d < s) {
        // Copy forwards if destination is before source
        while (n--) *d++ = *s++;
    } else {
        // Copy backwards if destination is after source
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

size_t strlen(const char* s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

char* strcpy(char* d, const char* s) {
    char* r = d;
    while ((*d++ = *s++));
    return r;
}

char* strncpy(char* d, const char* s, size_t n) {
    size_t i = 0;
    for (; i < n && s[i]; i++) d[i] = s[i];
    for (; i < n; i++) d[i] = 0;
    return d;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) { s1++; s2++; n--; }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* strchr(const char* s, int c) {
    while (*s) { if (*s == (char)c) return (char*)s; s++; }
    return 0;
}

char* strrchr(const char* s, int c) {
    char* last = 0;
    while (*s) { if (*s == (char)c) last = (char*)s; s++; }
    return last;
}

char* strtok(char* str, const char* delim) {
    static char* last;
    if (str) last = str;
    if (!last) return 0;
    char* start = last;
    while (*start && strchr(delim, *start)) start++;
    if (!*start) { last = 0; return 0; }
    char* end = start;
    while (*end && !strchr(delim, *end)) end++;
    if (*end) { *end = 0; last = end + 1; } else { last = 0; }
    return start;
}

// Einfache vsnprintf-Implementierung
int vsnprintf(char* str, size_t size, const char* format, va_list args) {
    int i = 0;
    char c;
    
    while ((c = *format++) != '\0' && i < size - 1) {
        if (c == '%') {
            c = *format++;
            switch (c) {
                case 'd': {
                    int num = va_arg(args, int);
                    // Einfache Dezimal-Konvertierung
                    if (num < 0) {
                        str[i++] = '-';
                        num = -num;
                    }
                    int temp = num;
                    int digits = 0;
                    do {
                        temp /= 10;
                        digits++;
                    } while (temp > 0);
                    
                    temp = num;
                    for (int j = digits - 1; j >= 0; j--) {
                        str[i + j] = '0' + (temp % 10);
                        temp /= 10;
                    }
                    i += digits;
                    break;
                }
                case 's': {
                    const char* s = va_arg(args, const char*);
                    while (*s && i < size - 1) {
                        str[i++] = *s++;
                    }
                    break;
                }
                case 'c': {
                    char ch = (char)va_arg(args, int);
                    str[i++] = ch;
                    break;
                }
                case '%': {
                    str[i++] = '%';
                    break;
                }
                default:
                    str[i++] = '%';
                    str[i++] = c;
                    break;
            }
        } else {
            str[i++] = c;
        }
    }
    
    str[i] = '\0';
    return i;
}

// Einfache snprintf-Implementierung
int snprintf(char* str, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(str, size, format, args);
    va_end(args);
    return result;
}
