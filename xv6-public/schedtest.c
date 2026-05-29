#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
  int n = 3;
  int i, j;
  int pid;

  printf(1, "Starting schedtest with %d processes\n", n);

  for (i = 0; i < n; i++) {
    pid = fork();
    if (pid < 0) {
      printf(1, "fork failed\n");
      exit();
    }
    if (pid == 0) {
      // Child process
      for (j = 0; j < 3; j++) {
        printf(1, "Process %d (PID %d) is running (iteration %d)\n", i, getpid(), j);
        // Busy wait
        int k, l;
        for (k = 0; k < 1000; k++) {
          for (l = 0; l < 10000; l++) {
            asm volatile("nop");
          }
        }
      }
      printf(1, "Process %d (PID %d) finished\n", i, getpid());
      exit();
    }
  }

  // Parent waits for all children
  for (i = 0; i < n; i++) {
    wait();
  }

  printf(1, "schedtest finished\n");
  exit();
}
