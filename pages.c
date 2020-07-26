
#define _GNU_SOURCE
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "pages.h"
#include "slist.h"
#include "util.h"

const int NUFS_SIZE  = 1024 * 1024; // 1MB
const int PAGE_COUNT = 256;
const int DATA_BLOCK_COUNT = 246;

static int   pages_fd   = -1;
static void* pages_base =  0;

void
pages_init(const char* path)
{
    pages_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(pages_fd != -1);

    int rv = ftruncate(pages_fd, NUFS_SIZE);
    assert(rv == 0);

    pages_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pages_fd, 0);
    assert(pages_base != MAP_FAILED);

    bitmap* inode_bitmap = (bitmap*) pages_get_page(1);
    for (int i = 0; i < DATA_BLOCK_COUNT; i++) {
        inode_bitmap->vals[i] = 0;
    }

    bitmap* data_block_bitmap = (bitmap*) pages_get_page(2);
    for (int i = 0; i < DATA_BLOCK_COUNT; i++) {
        data_block_bitmap->vals[i] = 0;
    }

    pnode* inodes = (pnode*) pages_get_page(3);
    for (int i = 0; i < DATA_BLOCK_COUNT; i++) {
        inodes->refs = 0;
        inodes->mode = 0;
        inodes->size = 0;
        inodes->num_blocks = 0;
        for (int j = 0; j < 12; j++) {
            inodes->blocks[i] = 0;
        }
        inodes++;
    }
}

void
pages_free()
{
    int rv = munmap(pages_base, NUFS_SIZE);
    assert(rv == 0);
}

void*
pages_get_page(int pnum)
{
    return pages_base + 4096 * pnum;
}

pnode*
pages_get_node(int node_id)
{
    pnode* idx = (pnode*) pages_get_page(3);
    return &(idx[node_id]);
}

int
pages_find_empty()
{
    int pnum = -1;
    bitmap* data_bitmap = (bitmap*)pages_get_page(2);
    for (int i = 0; i < DATA_BLOCK_COUNT; i++) {
        if (data_bitmap->vals[i] == 0) {
            pnum = i;
            break;
        }
    }
    return pnum;
}

void
print_node(pnode* node)
{
    if (node) {
        printf("node{refs: %d, mode: %04o, size: %d, num_blocks: %d\n",
               node->refs, node->mode, node->size, node->num_blocks);
        for (int i=0; i < node->num_blocks; i++) {
            if (i > 9) {
                break;
            }
            printf("data_block %d : %d\n", i, node->blocks[i]);
        }
    }
    else {
        printf("node{null}\n");
    }
}


