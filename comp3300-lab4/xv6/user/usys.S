#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(halt)
SYSCALL(time)
SYSCALL(stime)
SYSCALL(trace)
SYSCALL(procinfo)
// A2 --
SYSCALL(setpriority)
SYSCALL(procstat)
SYSCALL(schedlatency)
// -- A2

// A3 --
SYSCALL(mprotect)
SYSCALL(munprotect)
SYSCALL(mmap)
SYSCALL(munmap)
SYSCALL(pageinfo)
// -- A3 