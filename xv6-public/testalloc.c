#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "testalloc: dynamically allocating 1 page (4096 bytes) using sbrk...\n");
  char *mem = sbrk(4096);
  if(mem == (char*)-1){
    printf(1, "testalloc: sbrk failed!\n");
    exit();
  }
  printf(1, "testalloc: allocation successful, returned address: 0x%x\n", mem);
  
  printf(1, "testalloc: dynamically deallocating 1 page using sbrk...\n");
  char *demem = sbrk(-4096);
  if(demem == (char*)-1){
    printf(1, "testalloc: sbrk deallocation failed!\n");
  } else {
    printf(1, "testalloc: deallocation successful\n");
  }

  printf(1, "testalloc: finishing test program.\n");
  exit();
}
