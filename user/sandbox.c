#include "kernel/types.h"
#include "user/user.h"

// Parse a nonnegative decimal int; never silently turn a typo into mask 0.
static int
parsemask(const char *s)
{
  uint mask = 0;
  uint max = ~0U >> 1;

  if (*s == '\0')
    return -1;
  for (; *s; s++) {
    if (*s < '0' || *s > '9' || mask > (max - (*s - '0')) / 10)
      return -1;
    mask = mask * 10 + (*s - '0');
  }
  return mask;
}

int
main(int argc, char *argv[])
{
  int pid;
  int status;
  int mask;

  if (argc < 4) {
    fprintf(2, "sandbox: usage: sandbox mask path command [args ...]\n");
    exit(1);
  }

  mask = parsemask(argv[1]);
  if (mask < 0) {
    fprintf(2, "sandbox: invalid mask\n");
    exit(1);
  }

  pid = fork();
  if (pid < 0) {
    fprintf(2, "sandbox: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    if (interpose(mask, argv[2]) < 0) {
      fprintf(2, "sandbox: interpose failed\n");
      exit(1);
    }
    exec(argv[3], &argv[3]);
    fprintf(2, "sandbox: cannot exec %s\n", argv[3]);
    exit(1);
  }

  if (wait(&status) < 0) {
    fprintf(2, "sandbox: wait failed\n");
    exit(1);
  }
  exit(status);
}
