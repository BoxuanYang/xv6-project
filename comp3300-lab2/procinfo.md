## Task 4 (Marks: 30/100)

For this task you will implement a syscall `procinfo` (with **syscall number 26**) to obtain some information about processes from the kernel. This will involve passing a pointer, containing a structure to hold information about a process. In this case, you only need to print the process id, its state, its name, the `start time` (i.e., the time when the process was created), and the `elapsed time` (i.e., current time minus the start time). 

The process structure `proc` in `include/proc.h` has been extended to record the start time of a process:

```C
// Per-process state
struct proc {
    ...
    long start_time;  // the time the process was created (in Epoch time) 
};
```

Before you can implement the procinfo syscall, you will need to update one or more functions in the kernel to record the process start time -- this is part of the task, so you would need to figure out where in the kernel to insert this update. 

Once the kernel is patched to record process start time, you can start implementing the procinfo syscall. The syscall must copy relevant data from the kernel to the user-supplied buffer in the following data structure (included in the file `include/procinfo.h`):

```C
struct procinfo_t {
    int pid;        // process id
    char name[16];  // process name
    char st[3];     // process state, as a string.
                    // "UN" (UNUSED), "EM" (EMBRYO), "S" (SLEEPING)
                    // "RE" (RUNNABLE), "RN" (RUNNING), "Z" (ZOMBIE)
    long start_time;     // The process start time
    long elapsed_time; // How long the process has been running
}; 
```

Specifically, the procinfo syscall should take two arguments: 

```C
int procinfo(int, struct procinfo_t *)
```
the first is an integer and the second is an array of `struct procinfo_t`. The first argument specifies the number of entries in the second argument. Calling `procinfo(n, p)` should return the number of process entries read from the process table, or -1 if the syscall encountered an error. On a successful return, the array `p` should be populated with information from the processes that are **not** in the UNUSED state. 

You will use this `procinfo` syscall to implement a simple `ps` program (use the provided template `user/ps.c`) to list the processes in the system. 

If you implemented this syscall correctly, running `ps` should show you something like this:

```
$ ps
PID     State   Name                    Started                 Elapsed
1       S       init                    2025-08-05 13:21:59     00:00:08
2       S       sh                      2025-08-05 13:21:59     00:00:08
3       RN      ps                      2025-08-05 13:22:06     00:00:01
```

The `Started` column displays the process start time in a standard date/time format "yyyy-mm-dd hh:mm:ss". The `Elapsed` column displays the elapsed time since the process started. If the elaposed time is less than 24 hours, it is displayed in a "hh:mm:ss" format; if it is greater than 24 hours, it displays it in the format "d-hh:mm:ss" where "d" is the number of days. For example, if we set the current time to some days ahead of the actual current time, the ps command would show something like the following: 

```
$ settime 1760000000
Current time set to 1760000000
$ ps
PID     State   Name                    Started                 Elapsed
1       S       init                    2025-08-05 13:21:59     64-19:31:24
2       S       sh                      2025-08-05 13:21:59     64-19:31:24
5       RN      ps                      2025-10-09 08:53:23     00:00:00
$ 
```



Some hints that may be helpful:

* Have a look at how the `fstat` syscall is implemented -- this syscall also involves copying information from the kernel space to the user space.

* The function `procdump()` in `kernel/proc.c` should provide you with an example of how to access the process table. 


A test program ([proctest.c](./xv6/user/proctest.c)) is provided for you to test your implementation of `sys_procinfo()`. When run, the program `proctest` creates a number of processes, organised in a hierarchy. You can then use the `ps` program to display the process table information.  
See this [sample output](./tests.md) to see the expected output of a correct implementation of `sys_procinfo()`.


