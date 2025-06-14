#include "opfs.h"
#include "terminal.h"
#include "memory.h"
#include "minilibc.h"
#include "error.h"
#include "keyboard.h"

#define OPFS_MAX_FILES 32
#define OPFS_MAX_DIRS 16
#define OPFS_MAX_FILENAME 32
#define OPFS_MAX_FILESIZE 1024
#define OPFS_MAX_PATH 128
#define OPFS_MAX_CHILDREN 16

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

static opfs_node_t opfs_root;
static opfs_node_t* opfs_cwd = &opfs_root;

int opfs_init(void) {
    memset(&opfs_root, 0, sizeof(opfs_root));
    opfs_root.type = OPFS_DIR;
    strcpy(opfs_root.name, "/");
    opfs_root.parent = NULL;
    opfs_root.data.dir.child_count = 0;
    opfs_cwd = &opfs_root;
    opfs_node_t* f1 = (opfs_node_t*)kmalloc(sizeof(opfs_node_t));
    if (!f1) {
        return -1;
    }
    memset(f1, 0, sizeof(opfs_node_t));
    f1->type = OPFS_FILE;
    strcpy(f1->name, "readme.txt");
    strcpy(f1->data.file.content, "Welcome to OPERATOR OS!\nType 'help' for commands.");
    f1->data.file.size = strlen(f1->data.file.content);
    f1->parent = &opfs_root;
    opfs_root.data.dir.children[opfs_root.data.dir.child_count++] = f1;
    opfs_node_t* f2 = (opfs_node_t*)kmalloc(sizeof(opfs_node_t));
    if (!f2) {
        return -1;
    }
    memset(f2, 0, sizeof(opfs_node_t));
    f2->type = OPFS_FILE;
    strcpy(f2->name, "notes.txt");
    strcpy(f2->data.file.content, "This is a simple text file.\nYou can edit me!");
    f2->data.file.size = strlen(f2->data.file.content);
    f2->parent = &opfs_root;
    opfs_root.data.dir.children[opfs_root.data.dir.child_count++] = f2;
    return 0;
}

static opfs_node_t* opfs_find_in_dir(opfs_node_t* dir, const char* name) {
    if (!dir || dir->type != OPFS_DIR) return NULL;
    for (int i = 0; i < dir->data.dir.child_count; i++) {
        if (strcmp(dir->data.dir.children[i]->name, name) == 0) {
            return dir->data.dir.children[i];
        }
    }
    return NULL;
}

static opfs_node_t* opfs_resolve(const char* path) {
    if (!path || !*path) return opfs_cwd;
    opfs_node_t* node = (path[0] == '/') ? &opfs_root : opfs_cwd;
    char buf[OPFS_MAX_PATH];
    strncpy(buf, path, OPFS_MAX_PATH-1); buf[OPFS_MAX_PATH-1] = 0;
    char* token = strtok(buf, "/");
    while (token) {
        if (strcmp(token, "..") == 0) {
            if (node->parent) node = node->parent;
        } else if (strcmp(token, ".") == 0) {
        } else {
            node = opfs_find_in_dir(node, token);
            if (!node) return NULL;
        }
        token = strtok(NULL, "/");
    }
    return node;
}

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
    opfs_node_t* f = (opfs_node_t*)kmalloc(sizeof(opfs_node_t));
    memset(f, 0, sizeof(opfs_node_t));
    f->type = OPFS_FILE;
    strcpy(f->name, fname);
    f->parent = dir;
    dir->data.dir.children[dir->data.dir.child_count++] = f;
    terminal_writestring("File created.\n");
    opfs_save();
}

void opfs_edit(const char* path) {
    opfs_node_t* f = opfs_resolve(path);
    if (!f || f->type != OPFS_FILE) {
        terminal_set_color(0x0C);
        terminal_writestring("File not found.\n");
        terminal_set_color(0x07);
        return;
    }

    terminal_set_color(0x0A);
    terminal_writestring("Editing file. Press ESC to save and exit.\n");
    terminal_writestring(f->data.file.content);
    terminal_writestring("\n");
    terminal_set_color(0x07);

    int pos = strlen(f->data.file.content);
    while (1) {
        uint16_t key = keyboard_getchar();
        if (key == 0x1B) break; // ESC
        else if (key < 0x100 && pos < OPFS_MAX_FILESIZE - 1) {
            f->data.file.content[pos++] = key;
            f->data.file.content[pos] = 0;
            terminal_putchar(key);
        }
    }
    f->data.file.size = pos;
    opfs_save();
}

void opfs_write(const char* path, const char* content, size_t len) {
    opfs_node_t* f = opfs_resolve(path);
    if (!f || f->type != OPFS_FILE) {
        terminal_set_color(0x0C);
        terminal_writestring("File not found.\n");
        terminal_set_color(0x07);
        return;
    }

    if (len >= OPFS_MAX_FILESIZE) {
        terminal_set_color(0x0C);
        terminal_writestring("Content too large.\n");
        terminal_set_color(0x07);
        return;
    }

    memcpy(f->data.file.content, content, len);
    f->data.file.content[len] = '\0';
    f->data.file.size = len;

    opfs_save();
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
    opfs_save();
}

void opfs_mkdir(const char* path) {
    char buf[OPFS_MAX_PATH]; strncpy(buf, path, OPFS_MAX_PATH-1); buf[OPFS_MAX_PATH-1]=0;
    char* last = strrchr(buf, '/');
    opfs_node_t* dir = opfs_cwd;
    char* dname = buf;
    if (last) { *last = 0; dir = opfs_resolve(buf); dname = last+1; }
    if (!dir || dir->type != OPFS_DIR) { terminal_set_color(0x0C); terminal_writestring("Invalid path.\n"); terminal_set_color(0x07); return; }
    if (opfs_find_in_dir(dir, dname)) { terminal_writestring("Exists.\n"); return; }
    opfs_node_t* d = (opfs_node_t*)kmalloc(sizeof(opfs_node_t));
    memset(d, 0, sizeof(opfs_node_t));
    d->type = OPFS_DIR;
    strcpy(d->name, dname);
    d->parent = dir;
    dir->data.dir.children[dir->data.dir.child_count++] = d;
    terminal_writestring("Directory created.\n");
    opfs_save();
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

void opfs_load(void) {
    // For now, just call opfs_init as fallback
    if (opfs_init() != 0) {
        terminal_writestring("Fehler: OPFS-Initialisierung fehlgeschlagen\n");
        return;
    }
}

void opfs_save(void) {
    // TODO: Save OPFS to disk (e.g., write to a file or block device)
    // Not implemented yet
}
