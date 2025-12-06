#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "procinfo.h"
#include "procstat.h"


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

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
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

// A1 --
// shutdown the system
int 
sys_halt(void)
{
  outw(0x604, 0x2000);

  return 0; // never reached if shutdown successful
}

// stub
int sys_trace(void)
{
  return -1;
}

// stub
int sys_procinfo(void)
{
  return -1; 
}

// -- A1


// A2 --

int sys_setpriority(void)
{
  int pid, n; 

  // get the first argument, which is the process id we want to change the priority of
  if(argint(0, &pid) < 0)
    return -1;

  // get the second argument, which is the new priority
  if(argint(1, &n) < 0)
    return -1;

  return set_priority(pid,n); 
}


int sys_procstat(void)
{
  int n; 
  char *arg; 
  n = argptr(0, &arg, sizeof(struct procstat_t)); 
  if(n < 0)
    return -1; 
  return getprocstat((struct procstat_t *) arg); 
}

int sys_schedlatency(void)
{
  int n; 

  // get the first argument, which is the trace mask
  if(argint(0, &n) < 0)
    return -1;

  return set_schedlatency(n); 
}

// -- A2 