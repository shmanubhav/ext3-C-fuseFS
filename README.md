## ext3 FS implemented using FUSE 

Created a 1MB ext3 disk image filesystem using the FUSE filesystem driver and C.

----

### Advantages: 
- No need to follow a linked list structure for writing/reading
  large files. We use inodes which stores the data block numbers
  as an array.
- We can easily see how many inodes and data blocks are free by
  looking at the bitmap data structure. Constant time lookup.
- Each inode should essentially be able to store data block 
  numbers for 40KB on the first level.
- With smaller files we have very fast data access.

### Disadvantages:
- As of right now we cannot store large files (>4KB).
- No softlinks/hardlinks.
- If a file is < 4KB it is allocated an entire 4KB page which
  causes fragmentation.
- No directories have been implemented except the root directory.

### Improvements to be made:
- Add functionality for directories other than the root directory.
- Implement soft links and hard links.
- Allow to write files > 4KB.

### Features Completed:
- Able to create a file.
- Able to write to a file.
- Able to read from a file.
- Able to delete files.
- Able to rename a file.
- Able to change permissions for a file.
- Able to list all the files in a particular directory.
- Able to change the date of access/modification of a file.
- Super Block, Inode Bitmap, And Data Bitmap allow constant access
  to free/used blocks.
- Support for metadata.

### Still Missing:
- Directories.
- Hardlinks and softlinks.
- Files > 4KB.
