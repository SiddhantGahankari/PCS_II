# Comprehensive Project Report: OS Modifications in xv6

## Topic 1: Process Memory Information in xv6(https://github.com/SiddhantGahankari/PCS_II.git)
##  Memory In XV6 
The kernel stores the state of each process using the struct proc defined in proc.h. It contains all the information about the process such as total size(uint sz;), the table (which is another struct) along with information like inodes and process details.

vm.c has the functions such as allocuvm(), deallocuvm(), mappages(), walkpgdir() which handle allocation/deallocation, mapping virtual to physical memory and accessing frames respectively.

When a process executes operations like sbrk() to dynamically grow its heap memory, xv6 handles the memory allocation using the allocuvm() which increments the process size, allocates physical frames using kalloc(), initializes the frames to zero, and calls mappages() to update the page tables.

## The required system calls

The calls that we implemented are as follows:

- **getmemsize()** : This returns the total size of the virtual address space of the current process (in bytes). This directly returns the `sz` field from the process structure, which tracks the current process memory size.

- **getvpages()** : This returns the number of virtual pages used by the process. Page size in xv6 is defined by the constant `PGSIZE`. This function calculates the total number of virtual pages by dividing the memory size by the page size.

- **getptentries()** : Returns the number of valid entries in the page table. This function iterates through the page table and counts only the mapped entries.

## Implementation Details

### System Call Workflow

A user program (`meminfo.c`) calls `getmemsize()`. This call goes through a small assembly stub (`usys.S`), which places the system call number in a register and triggers a software interrupt. The OS catches this interrupt in `trap.c`, then `syscall.c` uses the number to find the correct kernel function. That function (like `sys_getmemsize()` in `sysproc.c`) runs in kernel mode, gets the memory info, and returns it back to the user program.

### Files Modified

|File       | Changes |
|---|---|
|syscall.h  |Added 3 syscall numbers (22-24)|               
|syscall.c  |Added extern declarations and mapped syscall array|
|sysproc.c  |Implemented 3 syscall functions|
|vm.c       |allocuvm() and deallocuvm() updated with tracking functionality|
|user.h     |Added 3 function prototypes|                    
|usys.S     |Added 3 SYSCALL macros|                         
|Makefile   |Added _meminfo to UPROGS|   
|testalloc.c| NEW FILE - User test program for memory allocation|             
|meminfo.c  | NEW FILE - User test program for memory information|  


### Changes Made

1. syscall.h - Added System Call Numbers
   
    ```c
    #define SYS_getmemsize 22
    #define SYS_getvpages  23
    #define SYS_getptentries 24
    ```
    ---
    
2. syscall.c - Registered System Calls
    Added the `extern` definitions for our functions and mapped them in the system call array so the kernel can route the traps correctly:
    ```c
    extern int sys_getmemsize(void);
    extern int sys_getvpages(void);
    extern int sys_getptentries(void);
    // Inside the syscalls array:
    [SYS_getmemsize]   sys_getmemsize,
    [SYS_getvpages]    sys_getvpages,
    [SYS_getptentries] sys_getptentries,
    ```
    ---

3. sysproc.c - Implemented System Call Functions
   ```c
   int sys_getmemsize(void)
   {
     return myproc()->sz;
   }

   int sys_getvpages(void)
   {
     return (myproc()->sz + PGSIZE - 1) / PGSIZE;
   }

   int sys_getptentries(void)
   {
     struct proc *p = myproc();
     pde_t *pgdir = p->pgdir;
     pde_t *pde;
     pte_t *pgtab;
     uint a;
     int count = 0;

     for(a = 0; a < p->sz; a += PGSIZE){
       pde = &pgdir[PDX(a)];
       if((*pde & PTE_P) == 0)
         continue;
       pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
       if(pgtab[PTX(a)] & PTE_P)
         count++;
     }
     return count;
   }
   ```
   
   **Explanation of Functions:**
   - **sys_getmemsize():** Directly returns myproc()->sz, basically the process memory size, in bytes.
   - **sys_getvpages():** Returns virtual pages by calculating `(sz + PGSIZE - 1) / PGSIZE`
   - **sys_getptentries():** This function implements the page table entry counting mechanism. The function:
     - Retrieves the current process and its page directory (`pgdir`)
     - Iterates through virtual memory from 0 to process size (`p->sz`) in page-sized increments (as specified by `PGSIZE`)
     - For each page, uses `PDX()` and `PTX()` macros (defined in `mmu.h`) to extract page directory and page table indices
     - Checks the `PTE_P` (Page Table Entry Present) bit to determine if the page is mapped in physical memory
     - Counts only valid, mapped page table entries (All the virtual pages are iterated, from 0 to sz, and checks are made to know whether each page table entry is present).
   
   This approach follows the use of `vm.c` and `mmu.h` for understanding page table management and virtual address translation.
   ---
4. vm.c - updated allocuvm() and deallocuvm() with tracking functionality
   ```c
    cprintf("allocuvm: tracking memory allocation from %d to %d bytes\n", oldsz, newsz);

    cprintf("deallocuvm: tracking memory deallocation from %d to %d bytes\n", oldsz, newsz);
   ```
   These prototypes allow user programs to call the new system calls.

   ---
5. user.h - Added User-Level Function Prototypes
   ```c
    int getmemsize(void);
    int getvpages(void);
    int getptentries(void);
   ```
   These prototypes allow user programs to call the new system calls.

   ---
6. usys.S - Added Assembly Stubs
   ```assembly
    SYSCALL(getmemsize)
    SYSCALL(getvpages)
    SYSCALL(getptentries)
   ```
   These macros expand to assembly code that:
   - Loads the system call number into %eax  
   - Triggers interrupt T_SYSCALL  
   - Returns to caller with result in %eax  
    ---
7. testalloc.c - User Test Program for memory allocation
    A user programe that uses sbrk() to allocate memory and deallocate memory.
   ```c
    char *mem = sbrk(4096);
    if(mem == (char*)-1){
      printf(1, "testalloc: sbrk failed!\n");
      exit();
    }
    printf(1, "testalloc: allocation successful, returned address: 0x%x\n", mem);
  
    printf(1, "testalloc: dynamically deallocating 1 page using sbrk...\n");
    char *demem = sbrk(-4096);
   ```
    During exectution we see what allocuvm and deallocuvm does due to tracking code added allowing us to see memory behaviour.
   ---

8. meminfo.c - User Test Program for memory information

    A user program that calls the three system calls implemented (`getmemsize`, `getvpages`, `getptentries`) and displays memory information in a user-friendly format to observe process memory statistics. 
    
    Here is a snippet demonstrating how the program tests memory allocation using `sbrk()`:
    ```c
    // TEST 2: After sbrk allocations
    printf(1, "\n[TEST 2] After allocating 3 extra pages (sbrk x3):\n");
    sbrk(4096);
    sbrk(4096);
    sbrk(4096);
    printinfo();
    ```
    This program conducts testing scenarios, such as:
    - Running it immediately after system startup to observe base memory usage
    - Modifying it to allocate additional memory to observe how values change
    - Calling it from forked child processes to compare parent and child memory information
    
    ---
## Key Learning Outcomes

### Files Studied and Referenced:
- **proc.h** - Used to understand the process structure and locate the `sz` field that stores process memory size
- **proc.c** - Studied to understand how `myproc()` retrieves the current process
- **vm.c** - Referenced for virtual memory management functions and page table operations
- **mmu.h** - Used to understand page table macros like `PDX()`, `PTX()`, `PTE_ADDR()`, and the `PTE_P` flag
- **syscall.c and sysproc.c** - Modified to extend xv6 with new system calls following existing patterns

## Output of testalloc.c
```output
$ testalloc
allocuvm: tracking memory allocation from 16384 to 49152 bytes
allocuvm: tracking memory allocation from 0 to 3000 bytes
allocuvm: tracking memory allocation from 4096 to 12288 bytes
deallocuvm: tracking memory deallocation from -2147483648 to 0 bytes
testalloc: dynamically allocating 1 page (4096 bytes) using sbrk...
allocuvm: tracking memory allocation from 12288 to 16384 bytes
testalloc: allocation successful, returned address: 0x3000
testalloc: dynamically deallocating 1 page using sbrk...
deallocuvm: tracking memory deallocation from 16384 to 12288 bytes
testalloc: deallocation successful
testalloc: finishing test program.
deallocuvm: tracking memory deallocation from -2147483648 to 0 bytes
$ 
```

## Output of meminfo.c 

```output 
$ meminfo

xv6 Memory Info Tester

[TEST 1] Baseline memory:
  Process Memory Size : 12288 bytes
  Virtual Pages Used  : 3 pages
  VPages (ceil)       : 3 pages
  Page Table Entries  : 3 entries

[TEST 2] After allocating 3 extra pages (sbrk x3):
  Process Memory Size : 24576 bytes
  Virtual Pages Used  : 6 pages
  VPages (ceil)       : 6 pages
  Page Table Entries  : 6 entries

[TEST 3] Consistency check:
[PASS] pages matches sz/PGSIZE = 6
[PASS] page table entries = 6 matches pages

[TEST 4] Fork test:
[CHILD]  Memory Size : 24576
[CHILD]  VPages       : 6
[CHILD]  PTE         : 6
[PARENT] Memory Size : 24576
[PARENT] VPages       : 6
[PARENT] PTE         : 6
[CHECK]  Parent and child should match above

All tests done!
$
```


---

## Topic 2: CPU Scheduling in xv6

## Understanding the default scheduler

The default xv6 scheduler is implemented in `proc.c` within the `scheduler()` function. It operates as a simple round-robin scheduler. The scheduler continuously loops through the process table (`ptable.proc`). For each process that is in the `RUNNABLE` state, it acquires the `ptable.lock`, switches to the process using `swtch`, and allows it to run for a time slice (until it yields the CPU, typically due to a timer interrupt). Once the process yields, control returns to the scheduler, which then proceeds to the next process in the table, thus guaranteeing fairness across all runnable processes.

We wrote a test program (`schedtest.c`) to observe this behavior. It created multiple processes that each run a CPU-intensive loop, which demonstrated that their execution interleaved evenly as expected with round-robin.

## Priority-based Scheduler and System Call

We implemented a priority-based scheduler where each process has an assigned priority level. The scheduler always selects the `RUNNABLE` process with the highest priority. If multiple processes share the highest priority, the scheduler uses round-robin among them. The default priority assigned to a new process is `10`. Higher numerical values represent a higher priority.

To allow users to modify a process's priority, we introduced a new system call `setpriority(int pid, int priority)`.

### Implementation Details

| File       | Changes |
|------------|---------|
| `proc.h`   | Added `int priority;` to `struct proc` |
| `proc.c`   | Initialized priority in `allocproc()`. Modified `scheduler()` to always pick the highest priority RUNNABLE process. Implemented `setpriority()` function to update a process's priority. |
| `defs.h`   | Added `setpriority(int, int);` function declaration |
| `syscall.h`| Added `#define SYS_setpriority 25` |
| `syscall.c`| Registered the new system call in the syscall array |
| `sysproc.c`| Added `sys_setpriority(void)` wrapper to fetch arguments and call `setpriority` |
| `user.h`   | Added user-space prototype `int setpriority(int, int);` |
| `usys.S`   | Added `SYSCALL(setpriority)` assembly stub |
| `Makefile` | Added `_schedtest` and `_prioritytest` to `UPROGS` |

#### 1. Modifying the Scheduler
In `proc.c`, the `allocproc()` function was updated to assign `p->priority = 10;` to newly created processes.
The `scheduler()` function was rewritten to iterate over the `ptable` and, for each `RUNNABLE` process `p`, check if there is any other `RUNNABLE` process `p1` with a priority strictly greater than `p->priority`. If not, `p` is run. This guarantees we always run processes with the maximum priority, and naturally falls back to round-robin for processes with the same highest priority.

#### 2. Adding `setpriority`
We added `int setpriority(int pid, int priority)` in `proc.c` which iterates through the process table to find the given `pid` and updates its `priority`. The `sys_setpriority()` function in `sysproc.c` fetches the `pid` and `priority` arguments from the user stack and passes them to the kernel `setpriority()` function.

## Testing the Scheduler

We added two user programs to test the scheduler behavior:

1. **`schedtest.c`:** Forks 3 children with the default priority (10) and performs busy-waiting loops. It demonstrates round-robin scheduling because all processes have the same priority.
2. **`prioritytest.c`:** Forks 3 children and uses `setpriority()` to assign them different priorities (`5`, `10`, and `20`). Each child performs the same busy-wait loop.

### Output of `schedtest`
```output
$ schedtest
Starting schedtest with 3 processes
Process 0 (PID 4) is running (iteration 0)
Process 0 (PID 4) is running (iteration 1)
Process 1 (PID 5) is running (iteration 0)
Process 1 (PID 5) is running (iteration 1)
Process 0 (PID 4) is running (iteration 2)
Process 1 (PID 5) is running (iteration 2)
Process 2 (PID 6) is running (iteration 0)
Process 2 (PID 6) is running (iteration 1)
Process 0 (PID 4) finished
Process 1 (PID 5) finished
Process 2 (PID 6) is running (iteration 2)
Process 2 (PID 6) finished
schedtest finished
```
*(The execution neatly interleaves because the processes share the same priority.)*

### Output of `prioritytest`
```output
$ prioritytest
Starting prioritytest with 3 processes
Process 0 (PID 8, Prio 5) is running (iteration 0)
Process 1 (PID 9, Prio 10) is running (iteration 0)
Process 2 (PID 10, Prio 20) is running (iteration 0)
Process 2 (PID 10, Prio 20) is running (iteration 1)
Process 2 (PID 10, Prio 20) is running (iteration 2)
Process 2 (PID 10, Prio 20) finished
Process 1 (PID 9, Prio 10) is running (iteration 1)
Process 1 (PID 9, Prio 10) is running (iteration 2)
Process 1 (PID 9, Prio 10) finished
Process 0 (PID 8, Prio 5) is running (iteration 1)
Process 0 (PID 8, Prio 5) is running (iteration 2)
Process 0 (PID 8, Prio 5) finished
prioritytest finished
```
*(As soon as Process 2 with Priority 20 is scheduled, it monopolizes the CPU until completion. Then Process 1 with Priority 10 runs to completion, followed lastly by Process 0 with Priority 5. This clearly verifies the priority-based scheduling logic.)*


---

## Topic 3: File System Exploration in xv6

## Understanding the xv6 File System

The xv6 file system architecture provides persistence using an on-disk layout defined by a superblock, a log area, an array of inodes, a free block bitmap, and data blocks. 
- **Inodes**: The `struct dinode` is the on-disk format and `struct inode` is the in-memory cache representation. Each inode stores file metadata including size, type (File, Directory, or Device), and the block addresses that map to the file's data. 
- **Directories**: Directories are essentially special files whose data blocks contain an array of `struct dirent` (Directory Entries), mapping file names (strings up to 14 characters) to their respective inode numbers.
- **Block Allocation**: Disk blocks are allocated by iterating through the block bitmap and finding a `0` bit (free block) using the `balloc` function.

To observe these operations, we created the `fs_demo` user program which invokes standard system calls like `mkdir()` and `open(O_CREATE)` to generate new directories and files on the disk, directly translating to the allocation of new inodes and disk blocks.

## File System Statistics System Call

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


## Testing the File System Call

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
