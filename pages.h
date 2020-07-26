#ifndef PAGES_H
#define PAGES_H

#include <stdio.h>
#include <time.h>

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes for file
    int num_blocks; //number of blocks
    int blocks[12]; //references to block numbers
    time_t atim; //time of last access
    time_t mtim; //last time of modification
    time_t ctim; //last time of status change
} pnode;

typedef struct bitmap {
	int vals[246];
} bitmap;

void   pages_init(const char* path);
void   pages_free();
void*  pages_get_page(int pnum);
pnode* pages_get_node(int node_id);
int    pages_find_empty();
void   print_node(pnode* node);

#endif
