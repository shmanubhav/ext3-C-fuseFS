#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <bsd/string.h>
#include <assert.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "storage.h"
#include "pages.h"
#include "directory.h"

// MODIFIED
// implementation for: man 2 access
// Checks if a file exists. 
// (1) is true and (0) is false
int
nufs_access(const char *path, int mask)
{
    printf("access(%s, %04o)\n", path, mask);

    file_data* fd = get_file_data(path);

    if (fd != NULL) {
    	return 0;
    }
    else {
    	return -ENOENT;
    }
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
{
    printf("getattr(%s)\n", path);
    int rv = get_stat(path, st);
    if (rv == -1) {
        return -ENOENT;
    }
    else {
        return 0;
    }
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    struct stat st;

    printf("readdir(%s)\n", path);

    directory* dir_ptr = (directory*) pages_get_page(10);

    for (int i = 0; i < dir_ptr->num_entries; i++) {
        file_data fd = dir_ptr->entries[i];
        get_stat(fd.path, &st);
        if (streq(fd.path, "/")) {
            filler(buf, ".", &st, 0);
        } else {
            filler(buf, &(fd.path[1]), &st, 0);
        }
    }
    return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    printf("mknod(%s, %04o)\n", path, mode);

    int free_page = pages_find_empty();

    if (free_page == -1) {
        return -ENOENT;
    }
    


    bitmap* inode_bitmap = (bitmap*)pages_get_page(1);
    int i = -1;
    for (i = 0; i < 246; i++) {
        if (inode_bitmap->vals[i] == 0) {
            inode_bitmap->vals[i] = 1;
            break;
        }
    }

    bitmap* data_bitmap = (bitmap*)pages_get_page(2);

    data_bitmap->vals[free_page] = 1;

    pnode* ptr = (pnode*)pages_get_page(3);
    ptr[i].refs = 0;
    ptr[i].mode = mode;
    ptr[i].size = 0;
    ptr[i].num_blocks = 1;
    ptr[i].blocks[0] = free_page;
    ptr[i].atim = time(NULL);
    ptr[i].mtim = time(NULL);
    ptr[i].ctim = time(NULL); 

    directory* dir_ptr = (directory*)pages_get_page(10);
    dir_ptr->num_entries = dir_ptr->num_entries + 1;
    memcpy(dir_ptr->entries[dir_ptr->num_entries - 1].path,path,48);
    dir_ptr->entries[dir_ptr->num_entries - 1].inode_num = i;

    return 0;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
    printf("mkdir(%s)\n", path);
    return -1;
}

int
nufs_unlink(const char *path)
{
    printf("unlink(%s)\n", path);
    directory* dir_ptr = (directory*) pages_get_page(10);
   
    file_data* fd = get_file_data(path); 
   
    bitmap* inode_bitmap = (bitmap*) pages_get_page(1);
    inode_bitmap->vals[fd->inode_num] = 0;
    pnode* n = pages_get_node(fd->inode_num);
    int block_num = n->blocks[0];

    bitmap* data_block_bitmap = (bitmap*) pages_get_page(2);
    data_block_bitmap->vals[block_num] = 0;
    
    dir_ptr->num_entries = dir_ptr->num_entries - 1;
    return 0;
}

int
nufs_rmdir(const char *path)
{
    printf("rmdir(%s)\n", path);
    return -1;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
    printf("rename(%s => %s)\n", from, to);
    directory* dir_ptr = (directory*)pages_get_page(10);
    int val_no = -1;

    for (int i = 0; i < dir_ptr->num_entries; i++) {
        if (streq((char*)&(dir_ptr->entries[i].path),from)) {
            memcpy(&(dir_ptr->entries[i].path),to,48);
            val_no = 0;
        }
    }

    if (val_no == -1) {
        return -ENOENT;
    }

    return 0;
}

int
nufs_chmod(const char *path, mode_t mode)
{
    printf("chmod(%s, %04o)\n", path, mode);
    file_data* data_ptr = (file_data*) get_file_data(path);

    if (data_ptr == 0) {
        return -ENOENT;
    } else {
        pnode* inode_ptr = (pnode*)pages_get_node(data_ptr->inode_num);
        inode_ptr->mode = mode;
    }

    return 0;
}

int
nufs_truncate(const char *path, off_t size)
{
    printf("truncate(%s, %ld bytes)\n", path, size);
    file_data* fd = get_file_data(path);
    pnode* n = pages_get_node(fd->inode_num);
    n->size = size;

    return 0;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
    printf("open(%s)\n", path);
    return 0;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    printf("read(%s, %ld bytes, @%ld)\n", path, size, offset);
    char* data = (char*)get_data(path);
    int len = strlen(data) + 1;
    if (size < len) {
        len = size;
    }
    strlcpy(buf, data, len);
    return len;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    printf("write(%s, %ld bytes, @%ld)\n", path, size, offset);

    file_data* fd = get_file_data(path);
    pnode* n = pages_get_node(fd->inode_num);
    n->size = size;

    char* ptr = (char*)get_data(path);

    memcpy(&ptr[offset], ( (char*) buf), size);
    return size;
}

// Update the timestamps on a file or directory.
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    int rv = storage_set_time(path, ts);
    printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
           path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
	return rv;
}

void
nufs_init_ops(struct fuse_operations* ops)
{
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access   = nufs_access;
    ops->getattr  = nufs_getattr;
    ops->readdir  = nufs_readdir;
    ops->mknod    = nufs_mknod;
    ops->mkdir    = nufs_mkdir;
    ops->unlink   = nufs_unlink;
    ops->rmdir    = nufs_rmdir;
    ops->rename   = nufs_rename;
    ops->chmod    = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open	  = nufs_open;
    ops->read     = nufs_read;
    ops->write    = nufs_write;
    ops->utimens  = nufs_utimens;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
    assert(argc > 2 && argc < 6);
    storage_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}

