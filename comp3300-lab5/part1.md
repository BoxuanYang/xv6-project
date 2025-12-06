# Part 1 (not graded): Building `clone()` from `fork()`

_Notes: This part (except for the Tests section) is sourced from the kernel threads project of OSTEP (see Reference below), reproduced here for convenience. All copyrights remain with the authors of OSTEP._

Your new clone system call should look like this: 

```C
int clone(void(*fcn)(void*, void *), void *arg1, void *arg2, void *stack)
```

This call creates a new kernel thread which shares the calling process's address space. File
descriptors are copied as in `fork()`. The new process uses `stack` as its
user stack, which is passed two arguments (`arg1` and `arg2`) and uses a fake
return PC (`0xffffffff`); a proper thread will simply call `exit()` when it is
done (and not `return`). The stack should be one page in size and
**page-aligned**. The new thread starts executing at the address specified by
`fcn`. As with `fork()`, the PID of the new thread is returned to the parent
(for simplicity, threads each have their own process ID).

The other new system call is 
```C
int join(void **stack). 
``` 
This call waits for a child thread that shares the address space with the calling process to
exit. It returns the PID of waited-for child or -1 if none. The location of
the child's user stack is copied into the argument `stack` (which can then be
freed).

You also need to think about the semantics of a couple of existing system
calls. For example, `int wait()` should wait for a child process that does not
share the address space with this process. It should also free the address
space if this is last reference to it. Also, `exit()` should work as before
but for both processes and threads; little change is required here.

Note in particular:
- `join()` should not wait for a child process that it does not share the address space with -- that is the job of `wait()`. 

- The argument `stack` is a pointer to a pointer of a stack. Normally it should dereference to the stack address passed to the `clone()` syscall when creating a thread. This also means that the `clone()` syscall must somehow record this stack address in the process structure when it is called, so that the `join()` call can later return it. 
You are free to modify the `struct proc` (in `kernel/proc.h`) to accommodate this addtional information. 

## Tests

The following test files are provided to test your implementation. Here are a brief summary. You should also look at the source code to understand what each file does. 

- `clonetest0`: this is a simple test to see if your `clone()` works. It does not use `join()`, so this could be used to test `clone()` separately before you implement `join()`. It also shows you how to set up (statically) a page aligned memory to pass to `clone()`. 

- `clonetest1`: this is a simple test combining `clone()` and `join()`. It also illustrates a possible race condition between two threads. 

- `clonetest2`: this is similar to `clonetest1`, but uses `wait()` to wait for the threads. This is to test that `wait()` waits only for processes with separate virtual address space, so a correct implementation of `wait()` should ignore threads (and vice versa for `join()`). Additionally, this example tests your implementation of `exit()` -- if a parent process exists, all child threads must be terminated (but child processes with separate virtual address space are handled as usual, i.e., adopted by the init process). 

- `clonetest3`: this tests creation of threads in a child process. A correct implementation should show that the threads created under different address space do not interact. 

- `clonetest4`: this tests user level synchronisation between two threads updating a shared counter. A correct implementation should show that the counter is updated correctly, with the final value equal to 50.


