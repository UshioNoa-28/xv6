#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/memlayout.h"
#include "user/user.h"

static void
check(int ok, const char *msg)
{
  if (!ok) {
    fprintf(2, "ugetpidtest: %s\n", msg);
    exit(1);
  }
}

int
main(int argc, char *argv[])
{
  int pid, status, fd[2];

  check(ugetpid() == getpid(), "PID mismatch");
  if (argc == 2) {
    check(ugetpid() == atoi(argv[1]), "PID changed across exec");
    exit(0);
  }

  pid = fork();
  check(pid >= 0, "fork failed");
  if (pid == 0) {
    int childpid = getpid();
    char buf[16];
    char *s = buf + sizeof(buf);

    check(ugetpid() == childpid, "child PID mismatch");
    *--s = '\0';
    do {
      *--s = '0' + childpid % 10;
      childpid /= 10;
    } while (childpid > 0);
    char *args[] = { "ugetpidtest", s, 0 };
    exec(args[0], args);
    check(0, "exec failed");
  }
  check(wait(&status) == pid && status == 0, "child failed");

  check(pipe(fd) == 0, "pipe failed");
  check(write(fd[1], "x", 1) == 1, "write to pipe failed");
  check(read(fd[0], (void *)USYSCALL, 1) == -1, "mapping was writable");
  close(fd[0]);
  close(fd[1]);

  printf("ugetpidtest: passed\n");
  exit(0);
}
