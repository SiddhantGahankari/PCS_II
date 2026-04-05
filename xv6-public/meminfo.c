#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "Process Memory Size : %d\n", getmemsize());
  printf(1, "Virtual Pages Used  : %d\n", getvpages());
  printf(1, "Page Table Entries  : %d\n", getptentries());
  exit();
}