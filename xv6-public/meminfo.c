#include "types.h"
#include "user.h"

void printinfo(void) {
  printf(1, "  Process Memory Size : %d bytes\n", getmemsize());
  printf(1, "  Virtual Pages Used  : %d pages\n", getpages());
  printf(1, "  VPages (ceil)       : %d pages\n", getvpages());
  printf(1, "  Page Table Entries  : %d entries\n", getptentries());
}

int main(void) {

  printf(1, "\n=============================\n");
  printf(1, "  xv6 Memory Info Tester\n");
  printf(1, "=============================\n");

  // TEST 1: Baseline
  printf(1, "\n[TEST 1] Baseline memory:\n");
  printinfo();

  // TEST 2: After sbrk allocations
  printf(1, "\n[TEST 2] After allocating 3 extra pages (sbrk x3):\n");
  sbrk(4096);
  sbrk(4096);
  sbrk(4096);
  printinfo();

  // TEST 3: Consistency check
  printf(1, "\n[TEST 3] Consistency check:\n");
  int sz    = getmemsize();
  int pages = getpages();
  int pte   = getptentries();
  int expected_pages = (sz + 4096 - 1) / 4096;

  if(pages == expected_pages)
    printf(1, "  [PASS] pages matches sz/PGSIZE = %d\n", expected_pages);
  else
    printf(1, "  [FAIL] pages=%d but sz/PGSIZE=%d\n", pages, expected_pages);

  if(pte == pages)
    printf(1, "  [PASS] page table entries = %d matches pages\n", pte);
  else
    printf(1, "  [WARN] pte=%d != pages=%d (may be ok)\n", pte, pages);

  // TEST 4: Fork test
  printf(1, "\n[TEST 4] Fork test:\n");
  int pid = fork();
  if(pid == 0) {
    printf(1, "  [CHILD]  Memory Size : %d\n", getmemsize());
    printf(1, "  [CHILD]  Pages       : %d\n", getpages());
    printf(1, "  [CHILD]  PTE         : %d\n", getptentries());
    exit();
  } else {
    wait();
    printf(1, "  [PARENT] Memory Size : %d\n", getmemsize());
    printf(1, "  [PARENT] Pages       : %d\n", getpages());
    printf(1, "  [PARENT] PTE         : %d\n", getptentries());
    printf(1, "  [CHECK]  Parent and child should match above\n");
  }

  printf(1, "\n=============================\n");
  printf(1, "  All tests done!\n");
  printf(1, "=============================\n\n");

  exit();
}
