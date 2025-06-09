#include "minilibc.h"
#include <stddef.h>
#include <stdarg.h>
void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
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
int snprintf(char* buf, size_t n, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t i = 0;
    for (; *fmt && i < n-1; fmt++) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'd') {
                int v = va_arg(ap, int);
                char tmp[16];
                int j = 0, neg = 0;
                if (v < 0) { neg = 1; v = -v; }
                do { tmp[j++] = '0' + (v % 10); v /= 10; } while (v && j < 15);
                if (neg) tmp[j++] = '-';
                while (j-- && i < n-1) buf[i++] = tmp[j];
            } else if (*fmt == 's') {
                char* s = va_arg(ap, char*);
                while (*s && i < n-1) buf[i++] = *s++;
            }
        } else {
            buf[i++] = *fmt;
        }
    }
    buf[i] = 0;
    va_end(ap);
    return i;
}
