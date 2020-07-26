
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "storage.h"
#include "pages.h"
#include "directory.h"

const int BLOCKS_START = 10;

int
streq(const char* aa, const char* bb)
{
    return strcmp(aa, bb) == 0;
}

file_data*
get_file_data(const char* path) {
    directory* dir_ptr = (directory*)pages_get_page(BLOCKS_START);
    for (int i = 0; i < dir_ptr->num_entries; i++) {
        if (streq(dir_ptr->entries[i].path, path)) {
            return &(dir_ptr->entries[i]);
        }
    }

    return 0;
}

void
storage_init(const char* path)
{
    pages_init(path);

    bitmap* inode_bitmap = (bitmap*)pages_get_page(1);
    inode_bitmap->vals[0] = 1;

    bitmap* data_bitmap = (bitmap*)pages_get_page(2);
    data_bitmap->vals[0] = 1;

    directory* dir_ptr = (directory*)pages_get_page(BLOCKS_START);
    dir_ptr->num_entries = 1;
    const char root_dir_val[48] = "/";
    memcpy(dir_ptr->entries[0].path,root_dir_val,48);
    dir_ptr->entries[0].inode_num = 0;

    file_data* root = get_file_data("/");

    pnode* root_node = pages_get_node(root->inode_num);

    root_node->mode = 040775;
    root_node->blocks[0] = 0;
    root_node->atim = time(NULL);
    root_node->mtim = time(NULL);
    root_node->ctim = time(NULL); 
}

int
get_stat(const char* path, struct stat* st)
{
    file_data* dat = get_file_data(path);
    if (!dat) {
        return -1;
    }

    memset(st, 0, sizeof(struct stat));
    st->st_uid  = getuid();
    pnode* inode_ptr = pages_get_node(dat->inode_num);
    st->st_mode = inode_ptr->mode;
    st->st_size = inode_ptr->size;
    st->st_atime = inode_ptr->atim;
    st->st_mtime = inode_ptr->mtim;
    st->st_ctime = inode_ptr->ctim;
    return 0;
}

char*
get_data(const char* path)
{
    file_data* dat = get_file_data(path);
    if (!dat) {
        return 0;
    }

    pnode* inode_ptr = pages_get_node(dat->inode_num);
    void* ptr = pages_get_page(inode_ptr->blocks[0]);

    return ptr;
}

int 
storage_set_time(const char* path, const struct timespec ts[2])
{
    file_data* dat = get_file_data(path);
    if (!dat) {
        return -1;
    }

    pnode* inode_ptr = pages_get_node(dat->inode_num);
    inode_ptr->atim = ts[0].tv_sec;
    inode_ptr->mtim = ts[1].tv_sec;

    return 0;
}

