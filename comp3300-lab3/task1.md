# Task 1 (mark: 10/100) -- changing scheduler latency.

The xv6 scheduler relies on a timer interrupt to pre-empt a running process: everytime the timer interrupt occurs, the kernel would perform a context switch from the current process to the scheduler. The global variable `ticks` (in `kernel/trap.c`) keeps track of the number of times the timer interrupt has fired since the kernel started -- it is incremented by one everytime the timer interrupt handler is executed. So in a way, we can think of the scheduler latency of the xv6 (round robin) scheduler as lasting for one "tick" (which is approximately 10ms). 

In this task, we are going to modify the xv6 kernel so that the scheduler latency can be set to any value between 1 and 10 (inclusive). A scheduler latency of, say 2, means that context switching from the current process to the scheduler happens only after the process has been running for 2 ticks. 

To implement this change, we need a couple of data structure. The first one is to store the scheduler latency -- this is implemented as the global variable `sched_latency` in `kernel/proc.c`. Its value is initialised to 1. 
The second data we need to store is the (remaining) `timeslice` of a process. This keeps track of how many ticks the process is allowed to run for. This `timeslice` data is implemented as part of a larger structure, called `sched_struct` (in `include/proc.h`) representing various scheduler metrics:  

```C
struct sched_struct {
  int tickets;    // number of tickets, for lottery scheduler
  int timeslice;  // remaining timeslice
  uint rt;        // process's runtime in ticks 
  uint ct;        // creation time of a process in ticks
  int delay;      // the number of ticks this process has been waiting, before run for the first time. 
                  // if the process has not been run at all, this should be -1. 
}; 
```

This is stored in a modified `proc` structure (`include/proc.h`):

```C
// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  // A1 --
  int tmask;        // trace mask
  long start_time;  // the time the process was created (in Epoch time) 
  // -- A1

  // A2 --
  struct sched_struct sched_data;  // scheduler related data
  // -- A2
};
```

We will come back to the other fields of `sched_data`; for now, we will only be making use of the `sched_data.timeslice`. The `timeslice` of each process is initialised to `sched_latency` (this is already done in the code).  You are tasked to modify the code for the pre-emption (in `kernel/trap.c`) and the round robin schduler `rr_scheduler()` (in `kernel/proc.c`) so that:

- Every time the timer interrupt occurs, decrement `sched_data.timeslice` of the running process by one, and print some information about the running process to the debug console (using `dprintf` -- see the sample code below). 

- If `sched_data.timeslice` of the current process is 0, perform a context switch to the (round robin) scheduler, and print this information to the debug console.

- In the round robin scheduler, when a process is scheduled to run next, update its timeslice to `sched_latency`.


## Testing your code

To test that you have implemented the modified timeslice correctly, you can use the `dprintf` function in the kernel to output information about the current process and whether it is about to yield execution to the scheduler. The file `kernel/trap.c` contains an example usage of `dprintf`, e.g.,

```
  if(myproc() && myproc()->state == RUNNING) {
    #ifdef DEBUG
      dprintf("[%d]\t process %d (%s) running\n", ticks, myproc()->pid, myproc()->name); 
    #endif
  }
```

Here the `dprintf` statement is guarded by a macro `DEBUG` -- this is optional, but it is a common way to turn on or off debugging information during the kernel development. The `DEBUG` macro is currently not defined in the xv6 source code, but we can insert its definition through a compiler flag. For example, to make sure that the above code prints to the debug console, you can compile xv6 with the following command:

```
make clean
KERNELCONFIG="-DDEBUG" make qemu-nox-dc
```

_Note: You should run a `make clean` before re-compiling the kernel, if you had previously compiled the kernel without the DEBUG option._

This sets the environment variable KERNELCONFIG with the compiler flag `-DDEBUG`, which will be inserted to the source code prior to compilation. The above command also creates a debug console server listening on port 45454 at localhost. To see the output of `dprintf` in this case, run the following command in a separate terminal to connect to the debug console:

```
nc localhost 45454
```

Once you have implemented the timeslice update, you can test setting the scheduler latency to, e.g., 2, and run any command (e.g., `ls`) and observe the output in the debug console. For example: if you run 

```
$ schedlatency 2
scheduler latency set to 2 ticks
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2286
cat            2 3 20216
....
$  
```

you should observe something like the following in the debug console:

```
...
[3524]   process 5 (ls) running
[3525]   process 5 (ls) running
[3525]   process 5 (ls) yields
[3526]   process 5 (ls) running
[3527]   process 5 (ls) running
[3527]   process 5 (ls) yields
[3528]   process 5 (ls) running
[3529]   process 5 (ls) running
[3529]   process 5 (ls) yields
...
```

We see that the process `ls` (pid=5) runs for 2 ticks at a time before yielding execution to the scheduler. 

Note that due to concurrency in the kernel (if you have multiple CPUs assigned to xv6), you may sometimes see interleaved outputs from different processes, so occasionally you may observe more than 2 "running" messages before a "yields" message. But for a long running process the majority of the output sequence would show a process running for 2 ticks before yielding. For example, you can use the provided program `schedtest` to create such long running programs.
`schedtest` takes two arguments: the first argument specifies how many child processes to create, and the second argument specifies the "size" of computation the child process will perform -- the largest the size, the longer the child process will run. For example, setting the scheduler latency to 3 and using schedtest to create two long running processes, you will observe the expected sequence of running/yielding per process:

```
$ schedlatency 3
$ schedtest 2 100000
PIDs created:  7  8 
```

In the debug console, you should observe messages such as these:

```
...
[6018]   process 7 (schedtest) running
[6019]   process 7 (schedtest) running
[6020]   process 7 (schedtest) running
[6020]   process 7 (schedtest) yields
[6021]   process 8 (schedtest) running
[6022]   process 8 (schedtest) running
[6023]   process 8 (schedtest) running
[6023]   process 8 (schedtest) yields
[6024]   process 7 (schedtest) running
[6025]   process 7 (schedtest) running
[6026]   process 7 (schedtest) running
[6026]   process 7 (schedtest) yields
[6027]   process 8 (schedtest) running
[6028]   process 8 (schedtest) running
[6029]   process 8 (schedtest) running
[6029]   process 8 (schedtest) yields
[6030]   process 7 (schedtest) running
[6031]   process 7 (schedtest) running
[6032]   process 7 (schedtest) running
[6032]   process 7 (schedtest) yields
...
```

# Marking

This task will be tested by running a number of workers under various scheduler latency. For each scheduler latency, the test will be performed after a fresh reboot of xv6. For each test, we will inspect the debug console to check whether it shows the expected output (eg interleaved of running and yielding messages as exemplified above).
