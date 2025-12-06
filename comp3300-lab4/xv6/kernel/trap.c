#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

// A1 --
extern long xtime; 
// -- A1


void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      // A1 --
      if(ticks % 100 == 0) ++xtime;  // 1 tick is 10ms, so need 100 ticks to make 1 sec
      // -- A1
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;
  case T_PGFLT:{
    // Get the faulting virtual address
    uint fault_addr = rcr2();

    // According to Intel System Programming Guide, a page fault err code contains
    // the following information:
    int present = tf->err & 0x1;    // 0 => not-present, 1 => protection violation
    int write = tf->err & 0x2;      // 1 => write access caused fault

    

    
    // Decide read/write/execute string
    const char *op;
    // 0 -- The access causing the fault was a read.
    if (write == 0){
      op = "read from";
    }
    // 1 -- The access causing the fault was a write
    else{
      op = "write to";
    }


    // 0 -- The fault was caused by a non-present page.
    if ((tf->cs & 3) == DPL_USER){
      if(present == 0){
        cprintf("Segmentation fault: process %d attempted to %s an unmapped address (0x%x).\n",
              myproc()->pid, op, fault_addr);
      }
      // 1 -- The fault was caused by a page-level protection violation
      else{
        cprintf("Segmentation fault: process %d attempted to %s a protected address (0x%x).\n",
              myproc()->pid, op, fault_addr);
      }
      // mark process as killed
      myproc()->killed = 1;
    }
    else{
      tf->eax = -1;
      return;
    }

    
    break;
  }
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // A2 --

  if(myproc() && myproc()->state == RUNNING) {
    #ifdef DEBUG
      dprintf("[%d]\t process %d (%s) running\n", ticks, myproc()->pid, myproc()->name); 
    #endif
  }

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
  {
    #ifdef DEBUG
    dprintf("[%d]\t process %d (%s) yields\n", ticks, myproc()->pid, myproc()->name); 
    #endif
    yield();
  }
  // --- A2
  
  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
