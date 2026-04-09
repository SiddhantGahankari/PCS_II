# Project Report: Process Memory Information in xv6

>#### Submitted by:
>
>Siddarth Modugu (B24CM1057)  
>Siddhant Gahankari (B24CM1058)  
>Snehil Gautam (B24CM1059)  

## Overview

This assignment, as specified in the project description, required us to implement three system calls to process memory information from xv6-public along with a user program to demonstrate them. The project is divided into three parts:
- **Part A**: Understanding Process Memory in xv6
- **Part B**: Implementing Memory Information System Calls (getmemsize, getvpages, getptentries)
- **Part C**: Writing a User Program to display memory information

## The required system calls

The calls that we implemented are as follows:

- **getmemsize()** : This returns the total size of memory used by the current process (in bytes). As per the project description Part B, this directly returns the `sz` field from the process structure, which tracks the current process memory size.

- **getvpages()** : This returns the number of virtual pages used by the process. The project description mentions that page size in xv6 is defined by the constant `PGSIZE`. This function calculates the total number of virtual pages by dividing the memory size by the page size.

- **getptentries()** : Returns the number of valid entries in the page table. The project description emphasizes studying `vm.c` for virtual memory management and `mmu.h` for page table constants. This function iterates through the page table as described in Part B's hints and counts only the mapped entries.

## Implementation Details

### Files Modified

|File       | Changes |
|---|---|
|syscall.h  |Added 3 syscall numbers (22-24)|               
|sysproc.c  |Implemented 3 syscall functions|                
|user.h     |Added 3 function prototypes|                    
|usys.S     |Added 3 SYSCALL macros|                         
|Makefile   |Added _meminfo to UPROGS and rmeoved _Werror|   
|meminfo.c  | NEW FILE - User test program|               


### Changes Made

1. syscall.h - Added System Call Numbers
   
    ```
    #define SYS_getmemsize 22
    #define SYS_getvpages  23
    #define SYS_getptentries 24
    ```
    ---

2. sysproc.c - Implemented System Call Functions
   ```
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
   
   **Explanation of sys_getptentries():**
   This function implements the page table entry counting mechanism as required by the project description Part B. It:
   - Retrieves the current process and its page directory (`pgdir`)
   - Iterates through virtual memory from 0 to process size (`p->sz`) in page-sized increments (as specified by `PGSIZE`)
   - For each page, uses `PDX()` and `PTX()` macros (defined in `mmu.h` as mentioned in project description) to extract page directory and page table indices
   - Checks the `PTE_P` (Page Table Entry Present) bit to determine if the page is mapped in physical memory
   - Counts only valid, mapped page table entries
   
   This approach directly addresses the project description's guidance to study `vm.c` and `mmu.h` for understanding page table management and virtual address translation.
   - sys_getmemsize(): Directly returns myproc()->sz, basically the process memory size, in bytes.
   - sus_getvpages(): Returns virtual pages by calculating `(sz + PGSIZE - 1) / PGSIZE`
   - sys_getptentries(): All the virtual pages are iterated, from 0 to sz, and checks are made to know whether each page table entry is present. Only counts the mapped entries.
   ---
3. user.h - Added User-Level Function Prototypes
   ```
    int getmemsize(void);
    int getvpages(void);
    int getptentries(void);
   ```
   ---
   These prototypes allow user programs to call the new system calls.

   ---
4. usys.S - Added Assembly Stubs
   ```
    SYSCALL(getmemsize)
    SYSCALL(getvpages)
    SYSCALL(getptentries)
   ```
   ---
   These macros expand to assembly code that:

   - Loads the system call number into %eax  
   - Triggers interrupt T_SYSCALL  
   - Returns to caller with result in %eax  
    ---
5. meminfo.c - User Program Implementation
    ```
    #include "types.h"
    #include "stat.h"
    #include "user.h"

    int main(int argc, char *argv[]){
      printf(1, "Process Memory Size : %d\n", getmemsize());
      printf(1, "Virtual Pages Used  : %d\n", getvpages());
      printf(1, "Page Table Entries  : %d\n", getptentries());
      exit();
    }
    ```
    ---
    
    **Alignment with Project Description Part C:**
    This user program directly fulfills the requirements stated in Part C of the project description. It:
    - Calls the three system calls implemented in Part B (getmemsize, getvpages, getptentries)
    - Displays memory information in a user-friendly format as specified in the project requirements
    - Demonstrates the use of the system calls to observe process memory statistics
    - Can be compiled into xv6 by adding it to the Makefile, allowing it to run inside the emulator
    
    This program can be used to conduct the testing scenarios mentioned in the project description, such as:
    - Running it immediately after system startup to observe base memory usage
    - Modifying it to allocate additional memory to observe how values change
    - Calling it from forked child processes to compare parent and child memory information
6. Makefile - Registered User Program
   ```
    UPROGS=\
        _cat\  
        _echo\  
        _forktest\  
        _grep\  
        _init\  
        _kill\  
        _ln\  
        _ls\  
        _mkdir\  
        _meminfo\    <--Added  
        _rm\  
        _sh\  
        _stressfs\  
        _usertests\  
        _wc\  
        _zombie\  
        _mytest\  
        _clear\ 
        _exit\ 
   ```

## Key Learning Outcomes - Alignment with Project Requirements

As per the project description, this implementation involved deep study of critical xv6 components:

### Files Studied and Referenced:
- **proc.h** - Used to understand the process structure and locate the `sz` field that stores process memory size
- **proc.c** - Studied to understand how `myproc()` retrieves the current process
- **vm.c** - Referenced for virtual memory management functions and page table operations
- **mmu.h** - Used to understand page table macros like `PDX()`, `PTX()`, `PTE_ADDR()`, and the `PTE_P` flag
- **syscall.c and sysproc.c** - Modified to extend xv6 with new system calls following existing patterns

### Part A Understanding (Process Memory):
The implementation demonstrates a thorough understanding of xv6's memory organization:
- Virtual address spaces and their organization (code, data, heap, stack segments)
- Process memory size tracking via the `sz` field in the `proc` structure
- Virtual-to-physical address mapping through page tables

### Part B Implementation:
Successfully extended xv6 with three system calls that provide essential process memory information, following the syscall implementation pattern demonstrated in existing calls like `getpid()`.

### Part C Testing:
The meminfo program serves as a testing tool to validate the system calls and can be extended for the experimental scenarios suggested in the project description (memory allocation tests, fork() behavior analysis, etc.).