#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct file_data {
    char path[48];
    int inode_num;
} file_data;

void storage_init(const char* path);
int get_stat(const char* path, struct stat* st);
char* get_data(const char* path);
file_data* get_file_data(const char* path);
int streq(const char* aa, const char* bb);
int storage_set_time(const char* path, const struct timespec ts[2]);

#endif
