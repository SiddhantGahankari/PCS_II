#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_getmemsize(void)
{
  return myproc()->sz;
}

int
sys_getpages(void)
{
  return (myproc()->sz + PGSIZE - 1) / PGSIZE;
}

int
sys_getvpages(void)
{
  return (myproc()->sz + PGSIZE - 1) / PGSIZE;
}

int
sys_getptentries(void)
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

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// shutdown sequence
int
sys_shutdown(void)
{
  outw(0xB004, 0x2000);
  
  outw(0x604, 0x2000);
  
  return 0;
}
