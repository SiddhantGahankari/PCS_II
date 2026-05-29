# Project Report: File System Exploration in xv6 (Topic 3)

## Part A: Understanding the xv6 File System

The xv6 file system architecture provides persistence using an on-disk layout defined by a superblock, a log area, an array of inodes, a free block bitmap, and data blocks. 
- **Inodes**: The `struct dinode` is the on-disk format and `struct inode` is the in-memory cache representation. Each inode stores file metadata including size, type (File, Directory, or Device), and the block addresses that map to the file's data. 
- **Directories**: Directories are essentially special files whose data blocks contain an array of `struct dirent` (Directory Entries), mapping file names (strings up to 14 characters) to their respective inode numbers.
- **Block Allocation**: Disk blocks are allocated by iterating through the block bitmap and finding a `0` bit (free block) using the `balloc` function.

To observe these operations, we created the `fs_demo` user program which invokes standard system calls like `mkdir()` and `open(O_CREATE)` to generate new directories and files on the disk, directly translating to the allocation of new inodes and disk blocks.

## Part B & C: File System Statistics System Call

We implemented a new system call, `getfsinfo()`, which parses the file system's on-disk structures and returns essential statistics to the user in a specialized `struct fsinfo`.

### `struct fsinfo`
Defined in `stat.h`, the struct holds the file system metrics:
```c
struct fsinfo {
  int num_files;
  int num_dirs;
  int allocated_inodes;
  int free_disk_blocks;
};
```

### Implementation Details

| File         | Changes |
|--------------|---------|
| `stat.h`     | Added `struct fsinfo` definition. |
| `fs.c`       | Implemented `get_fs_stats(int dev, struct fsinfo *fsi)`. This function scans the disk directly by mapping buffer blocks (`bread`). It loops through `sb.ninodes` to count allocated inodes (type != 0), files (`T_FILE`), and directories (`T_DIR`). It also parses the block bitmap (`sb.bmapstart`) to count zeros, indicating free data blocks. |
| `defs.h`     | Declared `get_fs_stats()` globally. |
| `syscall.h`  | Defined the new system call number `SYS_getfsinfo` as `27`. |
| `syscall.c`  | Registered `sys_getfsinfo` in the syscall array. |
| `sysfile.c`  | Wrote the kernel boundary function `sys_getfsinfo()` which fetches the pointer arguments using `argptr` and calls `get_fs_stats(ROOTDEV, fsi)`. |
| `user.h`     | Exported the `getfsinfo` prototype for user-level programs. |
| `usys.S`     | Added the `SYSCALL(getfsinfo)` stub for hardware interrupt invocation. |
| `Makefile`   | Appended `_fs_demo` and `_fsinfo_test` to the `UPROGS` variable for compilation. |


## Part D: Testing the File System Call

We tested the correctness of our new implementation using QEMU in multiple steps.

### Step 1: Baseline System Boot
We executed the `fsinfo_test` user program on a fresh boot to capture the initial conditions of the xv6 file system.

```output
$ fsinfo_test
File System Information:
--------------------------
Total files:       25
Total directories: 1
Allocated inodes:  27
Free disk blocks:  78
--------------------------
```
**Observation**: There are 25 default user program files (like `cat`, `ls`, etc.) and 1 directory (root `/`). Along with 1 device inode (`console`), this totals exactly 27 allocated inodes. Out of 1000 blocks in the file system, xv6's `sb.nblocks` states 941 data blocks. The data blocks occupied minus 941 equals exactly 78 free blocks.

### Step 2: Creating Files (`fs_demo`)
We executed `fs_demo` which creates `testdir1`, `testdir2`, `testfile1.txt`, and `testdir1/testfile2.txt`.
```output
$ fs_demo
fs_demo: Creating files and directories...
fs_demo: Done. You can use 'ls' to see the changes.
```

### Step 3: Verification
We re-ran `fsinfo_test` to verify the deltas reflected the allocations performed in step 2.

```output
$ fsinfo_test
File System Information:
--------------------------
Total files:       27
Total directories: 3
Allocated inodes:  31
Free disk blocks:  74
--------------------------
```
**Observation**: 
- `Total files` increased by 2 (`testfile1.txt`, `testfile2.txt`).
- `Total directories` increased by 2 (`testdir1`, `testdir2`).
- `Allocated inodes` correctly increased by exactly 4.
- `Free disk blocks` correctly dropped by exactly 4. 

The implementation works flawlessly and precisely traverses the disk-level bitmaps and inode blocks to fetch the true state of the filesystem.
