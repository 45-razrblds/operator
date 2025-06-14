#ifndef OPFS_H
#define OPFS_H

#include <stdint.h>

int opfs_init(void);
void opfs_ls(const char* path);
void opfs_cat(const char* path);
void opfs_touch(const char* path);
void opfs_edit(const char* path);
void opfs_rm(const char* path);
void opfs_mkdir(const char* path);
void opfs_cd(const char* path);
void opfs_pwd(void);
void opfs_load(void);
void opfs_save(void);

#endif // OPFS_H
