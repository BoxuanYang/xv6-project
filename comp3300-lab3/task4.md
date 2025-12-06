# Task 4 (mark: 40/100) -- hybrid scheduler

For this task, you will implement a hybrid scheduler that combines the round robin scheduler and the lottery scheduler. The two key objectives of this hybrid algorithm are:

- To improve the overall response time for newly created processes (through `fork()`). These processes should be given the top priority when the scheduler selects the next process to run.

- To ensure that long running processes do not monopolise the CPUs when there are shorter running jobs in the scheduling queue. 

These are essentially the objectives of Linux's Completely Fair Scheduler (CFS). But instead of using variable time slices, we will adjust the `tickets` that a process holds to change its priority. 


## Data structures

To implement this hybrid scheduler, we will track all metrics in `sched_data` as in the previous task. 
Additionally, there are two important parameters in `proc.h`: 

```C
#define MAX_TS  100   
#define PENALTY   5
```

The `MAX_TS` parameter is the maximum number of ticks a process can run before its priority is reduced, and `PENALTY` specifies the reduction that is applied to the number of tickets the process is currently holding. This is essentially to ensure that the tickets a process holds are reduced the longer it runs, until it hits the `MIN_TICKETS` (in `param.h`). 

## The hybrid scheduling algorithm 

The main scheduler loop is still similar to the lottery scheduler algorithm, but instead of running a lottery straightaway to pick a winner, it first goes through the list of processes in the process table (`ptable`) and selects the first runnable process that has a `sched_data.delay` value of `-1` -- as this indicates that that process has just been created and not yet scheduled. 

If such a process exists, its tickets should be set to `MAX_TICKETS` and it should be scheduled to be run next. This is a way for the scheduler to say that newly created processes should be given the highest priority.

If there are no such processes, then proceed with running a lottery to pick the winning process. 

Once a process is selected to run, you need to update a number of fields in sched_data:

- Its `sched_data.delay` field must be changed to the number of ticks that has elapsed since this process was first set to runnable -- so `sched_data.delay = (ticks-sched_data.ct)`. 

- If the process has run for `MAX_TS` since its tickets (`sched_data.tickets`) was last changed, then decrease `sched_data.tickets` by `PENALTY` amount. But make sure that `sched_data.tickets` does not go below `MIN_TICKETS` -- so `MIN_TICKETS` is the absolute minimum number of tickets a process should hold. 


## Compiling and running the hybrid scheduler

To compile the kernel to run the hybrid scheduler, you need to set the paramter SCHEDULER to 2, through the make command: 

```
make clean
KERNELCONFIG="-DSCHEDULER=2" make qemu-nox
```

If you want to use `dprintf` and the debug console, replace `qemu-nox` with `qemu-nox-dc`. 

## Testing your implementation

You can use the test program `user/schedtest.c` to generate workloads to test the scheduler as in previous tasks. 

A typical testing sequence would be to create a number of long running workers (with large payload), and then add a worker at a time and observes the response time (the `Delay` column). If you implemented the scheduler correctly, you should see only minor variations in the response time across different processes (typically within the range of 10 ticks or so -- but this again may depend on how fast/slow your computer is). 

See [hybrid_tests.md](./hybrid_tests.md) for some sample tests along with brief explanations. 


# Marking

This task will be tested against 4 scenarios as described below and exemplified in [tests.md](./tests.md). Each scenario is worth 25% of the total mark for this task. Each of these test scenarios will be performed manually. We will take care of adjusting various parameters (`MAX_TS`, `PENALTY`, arguments of `schedtest`, etc) to ensure the updates are manually observable (e.g., the worker processes can still be observed in `pstat` within a second or two after running `schedtest`). 

  1. Tickets are initialised and updated correctly. Updates to tickets should only apply to runnable processes. 

  2. Fairness of the scheduler. With a fixed number of workers (with similar start time and workload), we should see the runtime converging to roughly equal proportions. 

  3. The response times (i.e., the `delay` column) are within a fixed bound (e.g., +/- 5 ticks). This can be tested by adding batches of long running processes, one batch at a time and then observing the response time. A correct implementation should show only small variations of response times. 

  4. Short running jobs are not starved and can finish quickly. This will be tested by first adding a large number long running processes, then add shorter jobs, and observer whether the shorter jobs get more runtime in comparison. 

You are provided with four test programs (a2test1.c, .., a2test4.c) to test the four scenarios above. You are recommended to use these test programs to test your implementation. Each of this program accepts an argument which is the workload for each worker. Choose a large workload, e.g., 100000, to ensure the workers do not complete their tasks too quickly (so the asymptotic behaviour of the scheduler is more observable).