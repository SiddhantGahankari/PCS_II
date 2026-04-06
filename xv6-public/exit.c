#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void){
  printf(1, "Shutting down xv6...\n");
  shutdown();
  exit();
}
