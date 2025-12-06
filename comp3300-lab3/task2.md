# Task 2 (mark 20/100): scheduler metrics.

Recall that the `proc` structure has been modified to include a `sched_data` field, of the following type:


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

In Task 1, you have been asked to ensure that `timeslice` is updated correctly. For this task, you will additionally update two metrics: the `rt` (runtime of a process) and `delay` (the number of ticks the process has been waiting before it is run for the first time, i.e.,  `sched_data.delay = (ticks - sched_data.ct)`). The latter metric is useful for measuring the responsiveness of the scheduler. These two fields need to be updated as follows:

- The runtime `rt` needs to be incremented everytime the timer interrupt occurs -- provided the current process is in a "running" state.

- The `delay` metric needs to be updated only when a process's state is changed to "running" for the first time. So this needs to be updated inside the scheduler. For this task, we assume the default scheduler (i.e., the round robin scheduler `rr_scheduler()`) is being used.


## Testing your code

To test your implementation, compile and run xv6 with 

```
make qemu-nox
```

To test whether you have implemented the update correctly, you can use the program `schedtest` to create a number of workers, and use `pstat` to probe the `sched_data` to see if they have been updated. For example, the following is an output of the test with 10 workers, 

```
Booting from Hard Disk..xv6...
cpu0: starting 0
This kernel is running the round robin scheduler
sb: size 8000 nblocks 7940 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ schedtest 10 100000
$ PIDs created:  5  6  7  8  9  10  11  12  13  14 
$ pstat
PID     State   Tickets Runtime Delay
1       S       10      32      2
2       S       10      12      0
7       RE      10      62      3
4       S       10      15      1
5       RE      10      67      1
6       RE      10      65      2
8       RE      10      62      3
9       RE      10      61      4
10      RE      10      60      5
11      RE      10      59      6
12      RE      10      58      7
13      RE      10      57      8
14      RE      10      55      9
15      RN      10      9       10
$ pstat
PID     State   Tickets Runtime Delay
1       S       10      32      2
2       S       10      14      0
7       RE      10      163     3
4       S       10      15      1
5       RE      10      168     1
6       RE      10      166     2
8       RE      10      163     3
9       RE      10      162     4
10      RE      10      161     5
11      RE      10      160     6
12      RE      10      159     7
13      RE      10      158     8
14      RE      10      156     9
16      RN      10      10      10
$ pstat
PID     State   Tickets Runtime Delay
1       S       10      32      2
2       S       10      16      0
7       RE      10      276     3
4       S       10      15      1
5       RE      10      281     1
6       RE      10      279     2
8       RE      10      276     3
9       RE      10      275     4
10      RE      10      274     5
11      RE      10      273     6
12      RE      10      272     7
13      RE      10      271     8
14      RE      10      269     9
17      RN      10      11      10
$
```

The `delay` metric for a process should not change across different runs of `pstat`, as it is measured only once. For the round robin scheduler, the `delay` metric should also show a linear increase in the number of processes, e.g., if we launch 10 processes, we should expect to see the delay to be increasing across different processes as the scheduler goes through the process queue sequentially. 

The `rt` metrics for the workers should change at each run of `pstat`, and each worker should receive approximately the same amount of runtime for the round robin scheduler.

The `tickets` metric is not used in the round robin scheduler so its value should remain constant. 

Increasing the scheduler latency should also have an effect on the `delay` -- the increase in the delay should also be proportional to the increase in the scheduler latency. 
For example, running the schedtest with scheduler latency of 2 should show something like the following:

```
Booting from Hard Disk...
xv6...
cpu0: starting 0
This kernel is running the round robin scheduler
sb: size 8000 nblocks 7940 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ schedlatency 2
scheduler latency set to 2 ticks
$ schedtest 10 100000
$ PIDs created:  6  7  8  9  10  11  12  13  14  15 

$ pstat
PID     State   Tickets Runtime Delay
1       S       10      26      2
2       S       10      21      0
8       RE      10      78      8
5       S       10      12      0
6       RE      10      84      1
7       RE      10      82      3
9       RE      10      80      5
10      RE      10      78      8
11      RE      10      78      9
12      RE      10      76      12
13      RE      10      76      13
14      RE      10      74      16
15      RE      10      74      17
17      RN      10      9       20
$ 
```
and running it (after restarting the OS) with scheduler latency of 3 shows a proportional increase in delay:

```
Booting from Hard Disk...
xv6...
cpu0: starting 0
This kernel is running the round robin scheduler
sb: size 8000 nblocks 7940 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ schedlatency 3
scheduler latency set to 3 ticks
$ schedtest 10 100000
$ PIDs created:  6  7  8  9  10  11  12  13  14  15 

$ pstat
PID     State   Tickets Runtime Delay
1       S       10      32      2
2       S       10      20      0
9       RE      10      87      10
5       S       10      12      0
6       RE      10      93      2
7       RE      10      93      4
8       RE      10      90      8
10      RE      10      87      12
11      RE      10      87      14
12      RE      10      87      16
13      RE      10      84      21
14      RE      10      84      24
15      RE      10      84      26
17      RN      10      9       30
$ 
```

# Marking

This task will be tested against the following scenarios; for each scenario, we will create a number of long-running workers and using pstat to probe its stats.

1. Delay metric: The delay metrics for the workers should be updated only once when the workers start executing. The values of the delay should be increasing proportionally to the number of workers.

2. Delay metric after scheduler latency updates: Increasing the scheduler latency should have a proportional effect on the delay metrics of the workers.

3. Runtime metric: The runtime metrics of the workers should be updated at each tick and they should converge to roughly similar numbers. 

