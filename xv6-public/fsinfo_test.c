#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
  struct fsinfo fsi;
  
  if (getfsinfo(&fsi) < 0) {
    printf(1, "getfsinfo failed\n");
    exit();
  }
  
  printf(1, "File System Information:\n");
  printf(1, "--------------------------\n");
  printf(1, "Total files:       %d\n", fsi.num_files);
  printf(1, "Total directories: %d\n", fsi.num_dirs);
  printf(1, "Allocated inodes:  %d\n", fsi.allocated_inodes);
  printf(1, "Free disk blocks:  %d\n", fsi.free_disk_blocks);
  printf(1, "--------------------------\n");
  
  exit();
}
