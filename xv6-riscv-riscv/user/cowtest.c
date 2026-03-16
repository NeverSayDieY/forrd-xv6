#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/riscv.h"
#include "user/user.h"

static int
test_isolation(void)
{
  char *p = sbrk(PGSIZE);
  if(p == SBRK_ERROR)
    return -1;

  p[0] = 'A';
  int pid = fork();
  if(pid < 0)
    return -1;

  if(pid == 0){
    p[0] = 'B';
    if(p[0] != 'B')
      exit(1);
    exit(0);
  }

  int st = -1;
  if(wait(&st) < 0)
    return -1;
  if(st != 0)
    return -1;
  if(p[0] != 'A')
    return -1;

  return 0;
}

static int
test_fork_loop(void)
{
  for(int i = 0; i < 32; i++){
    int pid = fork();
    if(pid < 0)
      return -1;
    if(pid == 0)
      exit(0);
    if(wait(0) < 0)
      return -1;
  }
  return 0;
}

static int
test_copyout_smoke(void)
{
  char *p = sbrk(PGSIZE);
  if(p == SBRK_ERROR)
    return -1;

  p[0] = 'X';

  int pid = fork();
  if(pid < 0)
    return -1;

  if(pid == 0){
    int fds[2];
    if(pipe(fds) < 0)
      exit(1);
    if(write(fds[1], "Z", 1) != 1)
      exit(1);
    if(read(fds[0], p, 1) != 1)
      exit(1);
    if(p[0] != 'Z')
      exit(1);
    exit(0);
  }

  int st = -1;
  if(wait(&st) < 0)
    return -1;
  return st == 0 ? 0 : -1;
}

int
main(void)
{
  if(test_isolation() < 0){
    printf("cowtest: isolation failed\n");
    exit(1);
  }

  if(test_fork_loop() < 0){
    printf("cowtest: fork loop failed\n");
    exit(1);
  }

  if(test_copyout_smoke() < 0){
    printf("cowtest: copyout smoke failed\n");
    exit(1);
  }

  printf("cowtest: ok\n");
  exit(0);
}
