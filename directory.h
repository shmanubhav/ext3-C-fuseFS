#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_SIZE 64
#define DIR_NAME 48

#include "slist.h"
#include "pages.h"
#include "storage.h"

typedef struct directory {
    int num_entries;
    file_data entries[DIR_SIZE];
} directory;

void directory_init();
directory directory_from_pnum(int pnum);
int directory_lookup_pnum(directory dd, const char* name);
int tree_lookup_pnum(const char* path);
directory directory_from_path(const char* path);
int directory_put_ent(directory dd, const char* name, int pnum);
int directory_delete(directory dd, const char* name);
slist* directory_list(const char* path);
void print_directory(directory dd);

#endif

