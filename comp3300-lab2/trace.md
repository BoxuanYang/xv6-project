# Task 2 (Marks: 30/100) --- tracing system calls

_This task was adapted from [https://pdos.csail.mit.edu/6.828/2023/labs/syscall.html](https://pdos.csail.mit.edu/6.828/2023/labs/syscall.html)_

For this task, you will implement a `trace` syscall (with **syscall number 23**) to print what syscalls a process are making. 

The function `trace()` should take one argument, an integer "mask", whose bits specify which system calls to trace. For example, to trace the fork system call, a program calls `trace(1 << SYS_fork)`, where `SYS_fork` is a syscall number from kernel/syscall.h. You will need to modify the xv6 kernel to print a line when each system call is about to return, if the system call's number is set in the mask. The line should contain the process id, the name of the system call, one or more arguments of the system call and the return value. The `trace` system call should enable tracing for the process that calls it and any children that it subsequently forks, but should not affect other processes. 

For this task, you only need to trace the following syscalls: open, close, read, write, fork and exec. Use the provided user level program [trace.c](./xv6/user/trace.c) to test your syscall implementation. The program takes an integer mask and a command to trace.  

For the exec() syscall, you only need to print the argument (i.e., the program being executed).
For example, here is a sample output of tracing exec(): 

```
$ trace 128 echo hello world
13: exec("echo", ...) = 0
```

Notice that the mask value is 128 in this example, since `SYS_exec=7` and therefore the mask is `(1 << 7) = 128`. 

With the exception of exec(), if an argument of a syscall is expected to be a string, the `trace` call should print the content of the string, enclosed in quotes. You don't need to print the entire string -- it is sufficient to print a prefix of it (up to 64 bytes) and append "..."  if it is longer than 64 bytes. For example: tracing the read syscall on the command `cat README` should give us: 

```
$ trace 32 cat README
9: read(3, "NOTE: we have stopped maintaining the x86 version of xv6, and sw"..., 512) = 512
9: read(3, ",\n2000)). See also https://pdos.csail.mit.edu/6.828/, which\npr"..., 512) = 512
$ 
```

Note that non-printable ASCII characters should be replaced by their "escape" form (e.g., newline becomes "\n", etc). 

To see what modifications are needed to the user and the kernel code, have a look at how other syscalls are implemented, e.g., the `sleep` syscall. The the `proc` data structure (in `proc.h`) has been modified to add a trace mask:
```C
struct proc {
  ... // the standard fields of the proc struct; omitted here for brevity
  // add a trace mask at the end of the struct
  int tmask; 
}; 
```
but you will need to modify other parts of the kernel to update the tmask, e.g.,  you need make sure that child processes inherit the trace mask of the parent process, so you also would need to modify the `fork()` (in `kernel/proc.c`) function to copy the trace mask from the parent to the child process. 

You would also need to modify a number of other files, e.g.,
- Makefile: to add the user program (add `$U/_trace` to the variable UPROGS, like what we did in Lab 01).
- include/user.h: add a signature for the user-level wrapper for the syscall, i.e., `int trace(int);`.
- include/syscall.h: define the sycall number as `SYS_trace`. As convention, we will use the syscall number 25 to represent sys_trace. 
- user/usys.S: add a macro to expand SYS_trace.


Use the provided [tracetest program](./xv6/user/tracetest.c) to test your implementation. 
See this [sample output](./tests.md) to see the expected output of a correct implementation.
