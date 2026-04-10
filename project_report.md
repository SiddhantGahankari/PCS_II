# [Project Report: Process Memory Information in xv6](https://github.com/SiddhantGahankari/PCS_II.git)
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
|testalloc.c| NEW FILE - User test program for part a|             
|meminfo.c  | NEW FILE - User test program for part b,c|  


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
7. testalloc.c - User Test Program for part a
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

8. meminfo.c - User Test Program for part a and b

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
