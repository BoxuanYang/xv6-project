## Task 3 (mark: 30/100) -- implementing time() and stime() syscalls.

For this task, you will learn how time is represented in Unix, and implement two syscalls that are commonly found in UNIX-like operating systems: `time()` (to read the current time) and `stime()` (to set the current time). Unix represents time as a 32-bit integer, denoting the number of seconds that has elapsed since 1970-01-01 00:00:00. This is also known as the [Epoch time.](https://en.wikipedia.org/wiki/Unix_time) 

These syscalls have the following interfaces in the userspace:

- `int time(long *t)`: This sycall takes a pointer to `long` and returns the current time. If `t` is not null, then the current time will also be stored in `t.` In the event of an error, it returns -1.

- `int stime(long *t)`: This syscall sets the current time to `*t` and returns 0 if the operation is successful, otherwise it returns -1.

To keep track of the current time, following linux implementation, we keep a global variable (called `xtime`, initialised to 0) in `kernel/proc.c`:

```
volatile long xtime = 0; 
```

This global variable is initilised when the kernel starts, by calling `time_init()` (in `kernel/time.c`), and updated every time the timer interrupt handler is called. Currently the `time_init()` function is not yet implemented yet. You will need to implement it as follows:

- First query the hardware clock using the provided `cmostime()` (in `kernel/lapic.c`). This will give you an `rtcdate` structure (see `include/date.h`) containing the `year`, `month`, `day`, `hour`, `minute` and `second` fields. 

- After you obtain the `rtcdate` data, convert it into the Epoch time representation, i.e., calculate how many seconds have lapsed since the reference date (Jan 1st, 1970, 12am). Note: make sure that you take into account the leap years in your calculation. 

The syscalls `time()` and `stime()` can then be implemented by simply reading from/writing to `xtime`.  Use the provided program `user/date.c` to test the `time()` syscall. Running the `date` command should return the current time, in the Unix time format. As a sanity check, you can compare it to linux's `date` command (by running `date +%s` in any standard linux distribution). 

If you implemented the above correctly, running `date` repeatedly (after some delay between runs) should show the time increasing, and running `settime` should affect the subsequent runs of `date`, e.g.,

```
$ date
1754385915
$ date
1754385916
$ date
1754385918
$ settime 0
Current time set to 0
$ date
1
$ settime 1754385918
Current time set to 1754385918
$ date
1754385920
$ 
```

The provided program `user/testdatetime` contains additional tests for edge cases. 

