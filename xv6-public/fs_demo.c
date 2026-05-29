#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char *argv[]) {
  printf(1, "fs_demo: Creating files and directories...\n");
  
  if(mkdir("testdir1") < 0) {
    printf(1, "mkdir testdir1 failed\n");
  }
  if(mkdir("testdir2") < 0) {
    printf(1, "mkdir testdir2 failed\n");
  }
  
  int fd1 = open("testfile1.txt", O_CREATE | O_WRONLY);
  if (fd1 >= 0) {
    write(fd1, "hello world", 11);
    close(fd1);
  } else {
    printf(1, "open testfile1.txt failed\n");
  }
  
  int fd2 = open("testdir1/testfile2.txt", O_CREATE | O_WRONLY);
  if (fd2 >= 0) {
    write(fd2, "foo bar", 7);
    close(fd2);
  } else {
    printf(1, "open testdir1/testfile2.txt failed\n");
  }
  
  printf(1, "fs_demo: Done. You can use 'ls' to see the changes.\n");
  exit();
}
