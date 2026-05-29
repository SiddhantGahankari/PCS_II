#include "types.h"
#include "stat.h"
#include "user.h"

void do_work(int id, int pid, int priority) {
  int j;
  for (j = 0; j < 3; j++) {
    printf(1, "Process %d (PID %d, Prio %d) is running (iteration %d)\n", id, pid, priority, j);
    // Busy wait
    int k, l;
    for (k = 0; k < 1000; k++) {
      for (l = 0; l < 10000; l++) {
        asm volatile("nop");
      }
    }
  }
  printf(1, "Process %d (PID %d, Prio %d) finished\n", id, pid, priority);
  exit();
}

int main(int argc, char *argv[]) {
  int n = 3;
  int i;
  int pid;
  int priorities[] = {5, 10, 20}; // Increasing priorities

  printf(1, "Starting prioritytest with %d processes\n", n);

  for (i = 0; i < n; i++) {
    pid = fork();
    if (pid < 0) {
      printf(1, "fork failed\n");
      exit();
    }
    if (pid == 0) {
      // Child process
      setpriority(getpid(), priorities[i]);
      do_work(i, getpid(), priorities[i]);
    }
  }

  // Parent waits for all children
  for (i = 0; i < n; i++) {
    wait();
  }

  printf(1, "prioritytest finished\n");
  exit();
}
