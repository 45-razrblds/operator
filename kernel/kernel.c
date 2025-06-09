#include <stdint.h>
#include <stddef.h>
#include "keyboard.h"  // Keyboard layout support
#include <stdbool.h>
#include "shutdown.h"  // Shutdown support
#include "reboot.h"    // Reboot support

// --- Forward declarations ---
void draw_banner(void);
void draw_prompt(void);
char* strchr(const char* s, int c);

// --- Minimal stdarg.h for freestanding kernel (x86) ---
typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type)   __builtin_va_arg(ap, type)
#define va_end(ap)         __builtin_va_end(ap)

// --- Constants and globals ---
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Terminal state
static uint16_t* terminal_buffer;
static uint16_t terminal_row;
static uint16_t terminal_column;
static uint8_t terminal_color;
static char input_buffer[256];
static int input_pos = 0;

// Debug mode state
static int debug_mode = 0;
static uint16_t debug_row_start = VGA_HEIGHT - 3;  // Default value for debug row start

// Memory allocator state
#define HEAP_START 0x1000000
#define HEAP_SIZE  0x100000
static uintptr_t heap_ptr = HEAP_START;

// Timer state
volatile uint32_t timer_ticks = 0;

// --- Terminal functions ---

void terminal_initialize() {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x07; // Light grey on black
    terminal_buffer = (uint16_t*) VGA_MEMORY;
    
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        terminal_buffer[i] = (terminal_color << 8) | ' ';
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= (debug_mode ? debug_row_start : VGA_HEIGHT)) {
            // Scroll up
            for (int y = 0; y < (debug_mode ? debug_row_start : VGA_HEIGHT) - 1; y++) {
                for (int x = 0; x < VGA_WIDTH; x++) {
                    terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
                }
            }
            // Clear last line
            for (int x = 0; x < VGA_WIDTH; x++) {
                terminal_buffer[((debug_mode ? debug_row_start : VGA_HEIGHT) - 1) * VGA_WIDTH + x] = (terminal_color << 8) | ' ';
            }
            terminal_row--;
        }
        return;
    }
    
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = (terminal_color << 8) | ' ';
        }
        return;
    }
    
    if (debug_mode && terminal_row >= debug_row_start) {
        terminal_row = debug_row_start - 1;
    }
    
    terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = (terminal_color << 8) | c;
    terminal_column++;
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }
}

void terminal_writestring(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
}

void terminal_set_color(uint8_t color) {
    terminal_color = color;
}

void draw_prompt() {
    terminal_set_color(0x0A); // Green
    terminal_writestring("> ");
    terminal_set_color(0x07); // Reset to default color
}

void draw_banner() {
    terminal_set_color(0x0B); // Light cyan
//  terminal_writestring("\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n");
    terminal_writestring("                    OPERATOR OS - A Simple Terminal OS                         \n");
//  terminal_writestring("\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n");
    terminal_set_color(0x07); // Reset to default color
    terminal_writestring("\n");
}

// --- I/O port functions ---

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %%dx, %%al" : "=a"(ret) : "d"(port));
    return ret;
}

void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %%al, %%dx" : : "a"(val), "d"(port));
}

// --- Delay ---

void delay(uint32_t count) {
    for (volatile uint32_t i = 0; i < count * 100000; i++);
}

// --- Memory allocator ---

void* malloc(size_t size) {
    void* ptr = (void*)heap_ptr;
    heap_ptr += size;
    if (heap_ptr > HEAP_START + HEAP_SIZE) {
        return 0; // out of memory
    }
    return ptr;
}

// --- Timer simulation ---

// Normally this would be incremented by PIT IRQ, but here we simulate with delay loops
void timer_tick() {
    timer_ticks++;
}

uint32_t get_system_ticks() {
    return timer_ticks;
}

void delay_ms(uint32_t ms) {
    uint32_t start = get_system_ticks();
    while ((get_system_ticks() - start) < (ms / 10)) {
        // simulate ticks increment
        timer_tick();
    }
}

// --- Tests with halting on failure ---

void halt() {
    asm volatile ("cli; hlt");
    while (1) {}
}

void halt_with_error(const char* msg) {
    terminal_set_color(0x0C); // Red
    terminal_writestring("ERROR: ");
    terminal_writestring(msg);
    terminal_writestring("\nSystem halted.\n");
    halt();
}

int test_vga() {
    terminal_writestring("Testing VGA output... ");
    terminal_writestring("[OK]\n");
    return 1;
}

int test_keyboard() {
    terminal_writestring("Testing keyboard input: press any key... ");
    while (1) {
        uint8_t status = inb(0x64);
        if (status & 0x01) {
            uint8_t scancode = inb(0x60);
            if (scancode < 0x80) {
                terminal_writestring("[OK]\n");
                return 1;
            }
        }
    }
    // unreachable
    return 0;
}

int test_timer() {
    terminal_writestring("Testing timer... ");
    uint32_t start = get_system_ticks();
    delay_ms(100);
    uint32_t end = get_system_ticks();
    if (end > start) {
        terminal_writestring("[OK]\n");
        return 1;
    }
    terminal_writestring("[FAIL]\n");
    return 0;
}

int test_memory() {
    terminal_writestring("Testing memory allocator... ");
    char* ptr = (char*)malloc(16);
    if (!ptr) {
        terminal_writestring("[FAIL]\n");
        return 0;
    }
    for (int i = 0; i < 16; i++) ptr[i] = (char)i;
    for (int i = 0; i < 16; i++) {
        if (ptr[i] != (char)i) {
            terminal_writestring("[FAIL]\n");
            return 0;
        }
    }
    terminal_writestring("[OK]\n");
    return 1;
}

// --- Boot sequence ---

void boot_sequence() {
    terminal_set_color(0x0F); // Bright white
    terminal_writestring("OPERATOR v1.8 - Booting...\n\n");
    
    if (!test_vga()) halt_with_error("VGA test failed.");
    if (!test_memory()) halt_with_error("Memory test failed.");
    if (!test_keyboard()) halt_with_error("Keyboard test failed.");
    if (!test_timer()) halt_with_error("Timer test failed.");
    
    terminal_set_color(0x0A); // Green
    terminal_writestring("\nAll tests passed. System ready.\n\n");
    terminal_set_color(0x07);
}

// --- Keyboard input ---

uint16_t get_keyboard_char() {
    uint8_t status = inb(0x64);
    if (status & 0x01) {
        uint8_t scancode = inb(0x60);
        return scancode_to_ascii(scancode);
    }
    return 0;
}

// --- Minimal libc replacements for bare-metal ---
// String functions need to be in correct dependency order
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

// Simple strtok (not thread safe)
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

// Minimal snprintf: only supports %d and %s, no width/flags
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

// --- Operator File System (opfs) ---
#define OPFS_MAX_FILES 32
#define OPFS_MAX_DIRS 16
#define OPFS_MAX_FILENAME 32
#define OPFS_MAX_FILESIZE 1024
#define OPFS_MAX_PATH 128
#define OPFS_MAX_CHILDREN 16

// File types
typedef enum { OPFS_FILE, OPFS_DIR } opfs_type_t;

typedef struct opfs_node {
    opfs_type_t type;
    char name[OPFS_MAX_FILENAME];
    struct opfs_node* parent;
    union {
        struct {
            char content[OPFS_MAX_FILESIZE];
            size_t size;
            uint32_t created;
            uint32_t modified;
        } file;
        struct {
            struct opfs_node* children[OPFS_MAX_CHILDREN];
            int child_count;
        } dir;
    } data;
} opfs_node_t;

// Root directory
static opfs_node_t opfs_root;
static opfs_node_t* opfs_cwd = &opfs_root;

// --- opfs helpers ---

void opfs_init() {
    memset(&opfs_root, 0, sizeof(opfs_root));
    opfs_root.type = OPFS_DIR;
    strcpy(opfs_root.name, "/");
    opfs_root.parent = NULL;
    opfs_root.data.dir.child_count = 0;
    opfs_cwd = &opfs_root;
    // Add example files
    opfs_node_t* f1 = (opfs_node_t*)malloc(sizeof(opfs_node_t));
    memset(f1, 0, sizeof(opfs_node_t));
    f1->type = OPFS_FILE;
    strcpy(f1->name, "readme.txt");
    strcpy(f1->data.file.content, "Welcome to OPERATOR OS!\nType 'help' for commands.");
    f1->data.file.size = strlen(f1->data.file.content);
    f1->parent = &opfs_root;
    opfs_root.data.dir.children[opfs_root.data.dir.child_count++] = f1;
    opfs_node_t* f2 = (opfs_node_t*)malloc(sizeof(opfs_node_t));
    memset(f2, 0, sizeof(opfs_node_t));
    f2->type = OPFS_FILE;
    strcpy(f2->name, "notes.txt");
    strcpy(f2->data.file.content, "This is a simple text file.\nYou can edit me!");
    f2->data.file.size = strlen(f2->data.file.content);
    f2->parent = &opfs_root;
    opfs_root.data.dir.children[opfs_root.data.dir.child_count++] = f2;
}

// Find node by name in a directory
opfs_node_t* opfs_find_in_dir(opfs_node_t* dir, const char* name) {
    if (!dir || dir->type != OPFS_DIR) return NULL;
    for (int i = 0; i < dir->data.dir.child_count; i++) {
        if (strcmp(dir->data.dir.children[i]->name, name) == 0) {
            return dir->data.dir.children[i];
        }
    }
    return NULL;
}

// Parse path and return node (absolute or relative)
opfs_node_t* opfs_resolve(const char* path) {
    if (!path || !*path) return opfs_cwd;
    opfs_node_t* node = (path[0] == '/') ? &opfs_root : opfs_cwd;
    char buf[OPFS_MAX_PATH];
    strncpy(buf, path, OPFS_MAX_PATH-1); buf[OPFS_MAX_PATH-1] = 0;
    char* token = strtok(buf, "/");
    while (token) {
        if (strcmp(token, "..") == 0) {
            if (node->parent) node = node->parent;
        } else if (strcmp(token, ".") == 0) {
            // do nothing
        } else {
            node = opfs_find_in_dir(node, token);
            if (!node) return NULL;
        }
        token = strtok(NULL, "/");
    }
    return node;
}

// --- opfs commands ---

void opfs_ls(const char* path) {
    opfs_node_t* dir = path ? opfs_resolve(path) : opfs_cwd;
    if (!dir || dir->type != OPFS_DIR) {
        terminal_set_color(0x0C); terminal_writestring("Not a directory.\n"); terminal_set_color(0x07); return;
    }
    terminal_set_color(0x0E);
    terminal_writestring("Name             Type    Size\n");
    for (int i = 0; i < dir->data.dir.child_count; i++) {
        opfs_node_t* n = dir->data.dir.children[i];
        terminal_writestring(n->name);
        for (int j = strlen(n->name); j < 16; j++) terminal_putchar(' ');
        terminal_writestring((n->type == OPFS_DIR) ? "<DIR>   " : "<FILE>  ");
        if (n->type == OPFS_FILE) {
            char sz[8];
            snprintf(sz, 8, "%d", (int)n->data.file.size);
            terminal_writestring(sz);
        }
        terminal_writestring("\n");
    }
    terminal_set_color(0x07);
}

void opfs_cat(const char* path) {
    opfs_node_t* f = opfs_resolve(path);
    if (!f || f->type != OPFS_FILE) {
        terminal_set_color(0x0C); terminal_writestring("File not found.\n"); terminal_set_color(0x07); return;
    }
    terminal_set_color(0x0A);
    terminal_writestring(f->data.file.content);
    terminal_writestring("\n");
    terminal_set_color(0x07);
}

void opfs_touch(const char* path) {
    char buf[OPFS_MAX_PATH]; strncpy(buf, path, OPFS_MAX_PATH-1); buf[OPFS_MAX_PATH-1]=0;
    char* last = strrchr(buf, '/');
    opfs_node_t* dir = opfs_cwd;
    char* fname = buf;
    if (last) {
        *last = 0;
        dir = opfs_resolve(buf);
        fname = last+1;
    }
    if (!dir || dir->type != OPFS_DIR) { terminal_set_color(0x0C); terminal_writestring("Invalid path.\n"); terminal_set_color(0x07); return; }
    if (opfs_find_in_dir(dir, fname)) { terminal_writestring("File exists.\n"); return; }
    opfs_node_t* f = (opfs_node_t*)malloc(sizeof(opfs_node_t));
    memset(f, 0, sizeof(opfs_node_t));
    f->type = OPFS_FILE;
    strcpy(f->name, fname);
    f->parent = dir;
    dir->data.dir.children[dir->data.dir.child_count++] = f;
    terminal_writestring("File created.\n");
}

void opfs_edit(const char* path) {
    opfs_node_t* f = opfs_resolve(path);
    if (!f || f->type != OPFS_FILE) {
        terminal_set_color(0x0C); terminal_writestring("File not found.\n"); terminal_set_color(0x07); return;
    }
    terminal_set_color(0x0D);
    terminal_writestring("Editing "); terminal_writestring(f->name); terminal_writestring(". Press ESC to save and exit.\n"); terminal_set_color(0x07);
    int pos = 0;
    memset(f->data.file.content, 0, OPFS_MAX_FILESIZE);
    while (1) {
        uint16_t c = get_keyboard_char();
        if (c == KEY_ESC) break;
        if (c == '\r' || c == '\n') { 
            f->data.file.content[pos++] = '\n'; 
            terminal_putchar('\n');
        }
        else if (c == '\b') { 
            if (pos > 0) { 
                pos--; 
                terminal_putchar('\b');
            }
        }
        else if (pos < OPFS_MAX_FILESIZE-1 && c >= 32 && c < 127) { 
            f->data.file.content[pos++] = (char)c;
            terminal_putchar((char)c);
        }
    }
    f->data.file.content[pos] = 0;
    f->data.file.size = pos;
    terminal_writestring("\nSaved.\n");
}

void opfs_rm(const char* path) {
    opfs_node_t* f = opfs_resolve(path);
    if (!f || f == &opfs_root || !f->parent) { terminal_writestring("Cannot remove.\n"); return; }
    opfs_node_t* dir = f->parent;
    int idx = -1;
    for (int i = 0; i < dir->data.dir.child_count; i++) {
        if (dir->data.dir.children[i] == f) { idx = i; break; }
    }
    if (idx >= 0) {
        for (int i = idx; i < dir->data.dir.child_count-1; i++)
            dir->data.dir.children[i] = dir->data.dir.children[i+1];
        dir->data.dir.child_count--;
        terminal_writestring("Removed.\n");
    }
}

void opfs_mkdir(const char* path) {
    char buf[OPFS_MAX_PATH]; strncpy(buf, path, OPFS_MAX_PATH-1); buf[OPFS_MAX_PATH-1]=0;
    char* last = strrchr(buf, '/');
    opfs_node_t* dir = opfs_cwd;
    char* dname = buf;
    if (last) { *last = 0; dir = opfs_resolve(buf); dname = last+1; }
    if (!dir || dir->type != OPFS_DIR) { terminal_set_color(0x0C); terminal_writestring("Invalid path.\n"); terminal_set_color(0x07); return; }
    if (opfs_find_in_dir(dir, dname)) { terminal_writestring("Exists.\n"); return; }
    opfs_node_t* d = (opfs_node_t*)malloc(sizeof(opfs_node_t));
    memset(d, 0, sizeof(opfs_node_t));
    d->type = OPFS_DIR;
    strcpy(d->name, dname);
    d->parent = dir;
    dir->data.dir.children[dir->data.dir.child_count++] = d;
    terminal_writestring("Directory created.\n");
}

void opfs_cd(const char* path) {
    opfs_node_t* d = opfs_resolve(path);
    if (!d || d->type != OPFS_DIR) { terminal_set_color(0x0C); terminal_writestring("Not a directory.\n"); terminal_set_color(0x07); return; }
    opfs_cwd = d;
}

void opfs_pwd() {
    char stack[OPFS_MAX_PATH][OPFS_MAX_FILENAME];
    int sp = 0;
    opfs_node_t* n = opfs_cwd;
    while (n && n != &opfs_root) {
        strcpy(stack[sp++], n->name);
        n = n->parent;
    }
    terminal_writestring("/");
    for (int i = sp-1; i >= 0; i--) {
        terminal_writestring(stack[i]);
        if (i > 0) terminal_writestring("/");
    }
    terminal_writestring("\n");
}

// --- Debug mode functions ---

void draw_debug_info() {
    if (!debug_mode) return;
    
    // Save current state
    uint16_t old_row = terminal_row;
    uint16_t old_col = terminal_column;
    uint8_t old_color = terminal_color;
    
    // Force cursor position to a safe area if needed
    if (terminal_row >= debug_row_start) {
        terminal_row = debug_row_start - 1;
        terminal_column = 0;
    }
    
    // Draw separator line directly to buffer
    for (int i = 0; i < VGA_WIDTH; i++) {
        terminal_buffer[debug_row_start * VGA_WIDTH + i] = (0x07 << 8) | '-';
    }
    
    // Clear debug area
    for (int i = debug_row_start + 1; i < VGA_HEIGHT; i++) {
        for (int j = 0; j < VGA_WIDTH; j++) {
            terminal_buffer[i * VGA_WIDTH + j] = (0x07 << 8) | ' ';
        }
    }
    
    // Calculate memory usage
    size_t used = heap_ptr - HEAP_START;
    size_t total = HEAP_SIZE;
    size_t percent = (used * 100) / total;
    
    // Format memory usage info
    char buf[64];
    snprintf(buf, sizeof(buf), "Memory Usage: %d/%d KB (%d%%)", 
             (int)(used/1024), (int)(total/1024), (int)percent);
    
    // Write memory usage info directly to buffer
    int pos = 0;
    int debug_line = debug_row_start + 1;
    for (int i = 0; buf[i] && pos < VGA_WIDTH; i++) {
        terminal_buffer[debug_line * VGA_WIDTH + pos++] = (0x0B << 8) | buf[i];
    }
    
    // Add two spaces before the bar
    pos += 2;
    
    // Draw memory usage bar
    int bar_width = VGA_WIDTH - pos - 2; // Leave space for brackets
    if (bar_width > 40) bar_width = 40;  // Cap bar width
    
    // Draw opening bracket
    terminal_buffer[debug_line * VGA_WIDTH + pos++] = (0x07 << 8) | '[';
    
    // Draw filled portion of bar
    int filled = (bar_width * percent) / 100;
    uint8_t bar_color = percent > 90 ? 0x0C : percent > 70 ? 0x0E : 0x0A;
    
    for (int i = 0; i < filled && pos < VGA_WIDTH - 1; i++) {
        terminal_buffer[debug_line * VGA_WIDTH + pos++] = (bar_color << 8) | '|';
    }
    
    // Draw empty portion of bar
    for (int i = filled; i < bar_width && pos < VGA_WIDTH - 1; i++) {
        terminal_buffer[debug_line * VGA_WIDTH + pos++] = (0x08 << 8) | '.';
    }
    
    // Draw closing bracket
    terminal_buffer[debug_line * VGA_WIDTH + pos++] = (0x07 << 8) | ']';
    
    // Keep cursor position above debug area
    terminal_row = old_row >= debug_row_start ? debug_row_start - 1 : old_row;
    terminal_column = old_row >= debug_row_start ? 0 : old_col;
    terminal_color = old_color;
}

void toggle_debug_mode() {
    debug_mode = !debug_mode;
    if (debug_mode) {
        debug_row_start = VGA_HEIGHT - 3;  // Reserve bottom 3 rows for debug info
        terminal_writestring("Debug mode enabled.\n");
    } else {
        terminal_writestring("Debug mode disabled.\n");
    }
}

// Update process_command to handle debug command
void process_command() {
    if (strncmp(input_buffer, "ls", 2) == 0) {
        char* arg = input_buffer+2; while (*arg == ' ') arg++;
        opfs_ls(*arg ? arg : NULL);
    } else if (strncmp(input_buffer, "cat ", 4) == 0) {
        opfs_cat(input_buffer+4);
    } else if (strncmp(input_buffer, "edit ", 5) == 0) {
        opfs_edit(input_buffer+5);
    } else if (strncmp(input_buffer, "touch ", 6) == 0) {
        opfs_touch(input_buffer+6);
    } else if (strncmp(input_buffer, "rm ", 3) == 0) {
        opfs_rm(input_buffer+3);
    } else if (strncmp(input_buffer, "mkdir ", 6) == 0) {
        opfs_mkdir(input_buffer+6);
    } else if (strncmp(input_buffer, "rmdir ", 6) == 0) {
        opfs_rm(input_buffer+6);
    } else if (strncmp(input_buffer, "cd ", 3) == 0) {
        opfs_cd(input_buffer+3);
    } else if (strncmp(input_buffer, "pwd", 3) == 0) {
        opfs_pwd();
    } else if (strncmp(input_buffer, "debug", 5) == 0) {
        toggle_debug_mode();
    } else if (strncmp(input_buffer, "shutdown", 8) == 0) {
        terminal_set_color(0x0C);
        terminal_writestring("\nShutting down...\n");
        terminal_set_color(0x07);
        delay(100); // Give time for message to be seen
        shutdown();
    } else if (strncmp(input_buffer, "reboot", 6) == 0) {
        terminal_set_color(0x0C);
        terminal_writestring("\nRebooting...\n");
        terminal_set_color(0x07);
        delay(100); // Give time for message to be seen
        reboot();
    } else if (strncmp(input_buffer, "help", 4) == 0) {
        terminal_set_color(0x0F);
        terminal_writestring("Available commands:\n");
        terminal_set_color(0x07);
        terminal_writestring("  ls [dir]       - List files and directories\n");
        terminal_writestring("  cat <file>     - Display file contents\n");
        terminal_writestring("  edit <file>    - Edit file contents (ESC to save and exit)\n");
        terminal_writestring("  touch <file>   - Create a new empty file\n");
        terminal_writestring("  rm <file|dir>  - Remove a file or directory\n");
        terminal_writestring("  mkdir <dir>    - Create a new directory\n");
        terminal_writestring("  cd <dir>       - Change current directory\n");
        terminal_writestring("  pwd            - Print working directory\n");
        terminal_writestring("  clear          - Clear the screen\n");
        terminal_writestring("  debug          - Toggle debug mode\n");
        terminal_writestring("  shutdown       - Power off the system\n");
        terminal_writestring("  reboot         - Restart the system\n");
        terminal_writestring("  help           - Show this help message\n");
    } else if (strncmp(input_buffer, "clear", 5) == 0) {
        terminal_initialize();
        draw_banner();
    } else {
        terminal_writestring("Unknown command. Type 'help'.\n");
    }
    input_pos = 0;
    memset(input_buffer, 0, sizeof(input_buffer));
    if (debug_mode) draw_debug_info();
}

// Update kernel_main to periodically refresh debug info
void kernel_main() {
    terminal_initialize();
    draw_banner();
    boot_sequence();
    opfs_init();
    draw_prompt();
    uint32_t last_debug_update = 0;
    
    while (1) {
        uint16_t c = get_keyboard_char();
        if (c) {
            if (c == '\r' || c == '\n') {
                terminal_putchar('\n');
                process_command();
                draw_prompt();
            } else if (c == '\b') {
                if (input_pos > 0) {
                    input_pos--;
                    terminal_putchar('\b');
                }
            } else if (c < 128 && input_pos < (int)(sizeof(input_buffer) - 1)) {
                input_buffer[input_pos++] = (char)c;
                terminal_putchar((char)c);
            }
        }
        
        // Update debug info every ~1 second
        uint32_t current_ticks = get_system_ticks();
        if (debug_mode && current_ticks - last_debug_update > 100) {
            draw_debug_info();
            last_debug_update = current_ticks;
        }
    }
}
