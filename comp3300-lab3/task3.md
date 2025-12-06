# Task 3 (mark: 30/100) -- lottery scheduler

The lottery scheduling algorithm in the OSTEP book assumes a linked list structure. For this task, we will implement something simpler. The xv6 OS has only a small number of processes in the system, and all their data is stored in the process table (`ptable` structure in `proc.c`), so we can simply iterate through the entries of this ptable to select the winner at every lottery run, rather than maintaining a separate linked-list queue. 

Before we get to the main scheduling algorithm, there are a few things to note:

- Parameters: Two parameters related to the lottery scheduler are `MIN_TICKETS` and `MAX_TICKETS`, defined in `include/param.h`. These determine the minimum and the maximum number of tickets a process can be assigned to. 

- Data structure: The scheduler related data is stored in the same `sched_data` field of the `proc` structure as in Task 2 -- the difference is now we need to consider additionally the `tickets` field in `sched_struct`. This field stores the number of tickets assigned to a process. The `tickets` field is initialised to `MIN_TICKETS` (which is defined as 10 in `include/param.h`), and the the parent's tickets are copied to its child processes (see `fork()` in `kernel/proc.c`). 

- Random number generators: To run a lottery, we need to somehow generate a stream of (pseudo-)random numbers; the xv6 kernel does not have one so you'd have to implement it. You are free to use your favorite generators (see, e.g.,  `user/userstest.c` for a very simple random number generator). Note that any pseudo-random number generators (PRNG) you implement will necessarily be deterministic, as xv6 does not have any access to a source of entropy -- for the purpose of implementing a scheduler, this is not likely to be an issue, but such a PRNG would not be suitable for security-sensitive applications. 


# The scheduling algorithm

We will use the algorithm presented in OSTEP Chapter 9 (see Figure 9.1). However, instead of using a linked list, we work directly with the process table `ptable`. The basic algorithm to select the winner is pretty straightforward: 

- Find out the total number of tickets currently held by **runnable processes** (so processes in other states must not be included in the lottery) in the process table. 

- Generate a random number, modulo the total number of tickets held by all processes, and that will be our winning ticket.  

- Iterate through the entries of ptable: for each runnable process encountered, add that process's tickets to the counter. If the counter exceeds the winning number, that process (in the current iteration) is the winner, and should be scheduled next. 

- Have a look at the `rr_scheduler()` of xv6, to figure out where to plug in the above lottery scheduling. Make sure that you acquire and release the ptable.lock as appropriate. Again, examining the `rr_scheduler()` code should give you a good idea of how to modify it to fit in your lottery scheduler. 

- Implement your lottery scheduler as a separate function (in the provided stub `lottery_scheduler()`). 

Additionally, make sure you update the `rt` and `delay` fields in `sched_data` as needed to record the runtime and the delay metrics for each process. 

# Testing your lottery scheduler

The `scheduler()` function in the kernel has been refactored to allow one to use different schedulers when compiling the kernel. This is controlled through the `SCHEDULER` parameter that must be supplied through the `make` command. To switch the scheduler to the lottery scheduler, compile and run xv6 using the following command:

```
make clean
KERNELCONFIG="-DSCHEDULER=1" make qemu-nox
```

_Note: You should do a `make clean` if you had previously compiled the kernel to use a different scheduler. This ensures that the kernel is recompiled to reflect the changes._

If you want to use `dprintf` and the debug console, replace `qemu-nox` with `qemu-nox-dc`. 

Test your lottery scheduler using the provided user program `user/schedtest.c`. This program takes two arguments: the number of worker processes to spawn, and the number of interations each worker does. Each worker does some simple (arbitrarily chosen) arithmetic operations, in three level nested loops. This is meant to be computation intensive enough to produce some meaningful statistics. 

Run a few experiments using `schedtest.c` and compare the runtime between worker processes. You should see very similar runtime, for large enough iterations. The runtime stats printed by `pstat` are in the unit of `ticks` (which is basically xv6 unit of timeslices). Compare the results with the round robin scheduler -- you should observe very similar distributions of runtime. For example, here is a sample run with 10 workers (pid=5,..,14):

```
Booting from Hard Disk...
xv6...
cpu0: starting 0
This kernel is running the lottery scheduler
sb: size 8000 nblocks 7940 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ schedtest 10 100000
$ PIDs created:  5  6  7  8  9  10  11  12  13  14 

$ pstat
PID     State   Tickets Runtime Delay
1       S       10      29      1
2       S       10      15      0
6       RE      10      75      8
4       S       10      12      1
5       RE      10      76      1
7       RE      10      53      1
8       RE      10      54      3
9       RE      10      73      2
10      RE      10      77      1
11      RE      10      64      5
12      RE      10      52      6
13      RE      10      57      4
14      RE      10      57      8
16      RN      10      9       12
$ pstat
PID     State   Tickets Runtime Delay
1       S       10      29      1
2       S       10      16      0
6       RE      10      244     8
4       S       10      12      1
5       RE      10      189     1
7       RE      10      201     1
8       RE      10      201     3
9       RE      10      227     2
10      RE      10      240     1
11      RE      10      187     5
12      RE      10      190     6
13      RE      10      199     4
14      RE      10      207     8
17      RN      10      10      3
$ pstat
PID     State   Tickets Runtime Delay
1       S       10      29      1
2       S       10      17      0
6       RE      10      470     8
4       S       10      12      1
5       RE      10      417     1
7       RE      10      478     1
8       RE      10      459     3
9       RE      10      460     2
10      RE      10      505     1
11      RE      10      468     5
12      RE      10      434     6
13      RE      10      437     4
14      RE      10      476     8
18      RN      10      10      2
```

The stats are very similar to the round robin scheduler, though the variations in the runtime are not as uniform in the initial run. The longer the workers are running eventually their runtime should converge to a more uniform distribution just like in round robin. 

# Changing process priority 

Now that you have a functioning lottery scheduler, you can start experimenting setting priorities for processes. By default, each process gets assigned the minimum number of tickets (`MIN_TICKETS`). You will implement a system call to change this ticket number. We will name this system call `setpriority`. Most of the boilerplate code for implementing this syscall has been done -- you only need to complete the function `set_priority()` in `kernel/proc.c` that does the actual work of setting the priority. The function `set_priority(int pid, int n)` is supposed to set the number of tickets for the process with pid given in its first argument to the number given in the second argument. A user program `priority.c` has been provided for you to test this syscall. 

A few things to watch out for in your implementation of `set_priority(int pid, int n)`:

- Make sure that you acquire the `ptable.lock` prior to modifying the process table, and release it when you are done.

- You need to make sure that `pid` is indeed a valid process id and the state of the process is either sleeping, running or runnable. 

- The value of the argument `n` must be between `MIN_TICKETS` and `MAX_TICKETS` (inclusive). 

Test this new syscall by running `schedtest` to create a number of workers, and change the priority of one of the workers using `priority`, and use `pstat` to check their stats. Run some experiments to see if the differences in runtimes between the worker processes are as expected, e.g., if you run two worker processes, with one worker having twice the number of tickets compared to the other, you should expect to see the elapsed time of the first worker to be significantly lower than the elapsed time for the second worker. Note that the ratio between the runtimes may not match exacly the ratio between priorities among the worker processes, due to the overhead of context switching. Below is an example of a run of the `schedtest` with 10 workers, and one of the workers (pid=5) has its tickets increased to 30. Notice that after a prolonged run, the runtime of worker 5 accumulates roughly three times faster than other workers. 


```
Booting from Hard Disk..xv6...
cpu0: starting 0
This kernel is running the lottery scheduler
sb: size 8000 nblocks 7940 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ schedtest 10 100000
$ PIDs created:  5  6  7  8  9  10  11  12  13  14 

$ priority 5 30
Process 5's tickets set to 30
$ pstat
PID     State   Tickets Runtime Delay
1       S       10      32      2
2       S       10      17      1
5       RE      30      231     2
4       S       10      13      1
6       RE      10      134     4
7       RE      10      103     1
8       RE      10      119     1
9       RE      10      149     2
10      RE      10      119     1
11      RE      10      113     5
12      RE      10      94      6
13      RE      10      131     4
14      RE      10      105     8
17      RN      10      9       8
$ pstat
PID     State   Tickets Runtime Delay
1       S       10      32      2
2       S       10      19      1
5       RE      30      913     2
4       S       10      13      1
6       RE      10      372     4
7       RE      10      340     1
8       RE      10      356     1
9       RE      10      361     2
10      RE      10      340     1
11      RE      10      332     5
12      RE      10      333     6
13      RE      10      360     4
14      RE      10      314     8
18      RN      10      10      7
$ pstat
PID     State   Tickets Runtime Delay
1       S       10      32      2
2       S       10      20      1
5       RE      30      1979    2
4       S       10      13      1
6       RE      10      713     4
7       RE      10      665     1
8       RE      10      692     1
9       RE      10      687     2
10      RE      10      664     1
11      RE      10      669     5
12      RE      10      649     6
13      RE      10      657     4
14      RE      10      622     8
19      RN      10      11      3
$ pstat
PID     State   Tickets Runtime Delay
1       S       10      32      2
2       S       10      22      1
5       RE      30      2413    2
4       S       10      13      1
6       RE      10      852     4
7       RE      10      807     1
8       RE      10      836     1
9       RE      10      845     2
10      RE      10      791     1
11      RE      10      802     5
12      RE      10      801     6
13      RE      10      794     4
14      RE      10      746     8
20      RN      10      11      1
$ 
```

# Marking

This task will be tested against the following scenarios; for each scenario, we will create a number of long-running workers and using pstat to probe its stats.

1. Delay metric: The delay metrics for the workers should be updated only once when the workers start executing. The values of the delay should be increasing proportionally to the number of workers.

2. Runtime metric: The runtime metrics of the workers should be updated at each tick and they should converge to roughly similar numbers. 

3. Priority updates: The syscall set_priority correctly updates the priority of the designated process (as reported by `pstat`). 

4. Runtime metrics after priority updates: The runtime metric of a worker that has its priority increased or decreased will change proportionally to the change in its priority. 