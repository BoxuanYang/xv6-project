#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "procinfo.h"

// needed for sys_procinfo 
extern int getprocinfo(int, struct procinfo_t *); 

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

/*
If a thread calls sbrk() to shrink the process size, the call 
should fail (return -1), but not panic.

However, if the current thread is the only thread running, 
then such a call should be handled normally.
*/
int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  acquire(&memlock);
  // cprintf("sbrk n: %d \n", n);
  int has_other_threads = has_shared_threads(myproc());
  

  // 如果是负增长并且有其他线程，拒绝
  if(n < 0 && has_other_threads == 1){
    release(&memlock);
    return -1;
  }


  addr = myproc()->sz;
  
  if(growproc(n) < 0){
    release(&memlock);
    return -1;
  }

  release(&memlock);
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// new syscalls

// shutdown the system
int 
sys_halt(void)
{
  outw(0x604, 0x2000);

  return 0; // never reached if shutdown successful
}

// Ignore this
int sys_trace(void)
{
   return -1; 
}

// Ignore this
int sys_procinfo(void)
{
  return -1; 
}

int sys_clone(void)
{
  int n;
  void*fcn; 
  void *arg1;
  void *arg2;
  void *stack;
  if(argint(0, &n) < 0) return -1;
  fcn = (void *)n; 
  if(argint(1, &n) < 0) return -1;
  arg1 = (void *)n; 
  if(argint(2, &n) < 0) return -1;
  arg2 = (void *)n; 
  if(argint(3, &n) < 0) return -1;
  stack = (void *)n; 
   
  return clone(fcn, arg1, arg2, stack);
}

int sys_join(void)
{
  int n; 
  if(argint(0, &n) < 0) return -1;
  
  return join((void **)n); 
}