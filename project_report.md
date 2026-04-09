# Project Report: Process Memory Information in xv6

>#### Submitted by:
>
>Siddarth Modugu (B24CM1057)  
>Siddhant Gahankari (B24CM1058)  
>Snehil Gautam (B24CM1059)  

## Overview

This assignment required us to implement three system calls to process memory information from xv6-public along with a user program to demonstrate them.

## The required system calls

The calls that we implemented are as follows:

- getmemsize() : This returns the total size of memory used by the current process
- getvpages() : This returns the number of virtual pages used by the process.
- getptentries() : Returns the number of valid entries in the page table

## Implementation Details

### Files Modified

|File       | Changes                                       | Lines affected|
|---|---|---|
|syscall.h  |Added 3 syscall numbers (22-24)                |3  lines       |
|sysproc.c  |Implemented 3 syscall functions                |31 lines(46-76)|
|user.h     |Added 3 function prototypes                    |3  lines       |
|usys.S     |Added 3 SYSCALL macros                         |3  lines       |
|Makefile   |Added _meminfo to UPROGS and rmeoved _Werror   |3  lines       |
|meminfo.c  |**NEW FILE** - User test program               |12 lines       |

#### Some Additional Changes

- Added clear function (clears the screen and moves cursor to the home position using escape sequence)
- An exit sequence (made another syscall function shutdown() to implement this)

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
   ---
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

    ​int main(int argc, char *argv[]){
      printf(1, "Process Memory Size : %d\n", getmemsize());
      printf(1, "Virtual Pages Used  : %d\n", getvpages());
      printf(1, "Page Table Entries  : %d\n", getptentries());  exit();
    }
    ```
    ---
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