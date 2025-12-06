
# Part 2: Building a thread library

In this part, you will implement two user library `thread_create()` and `thread_join()` that provide a more user-friendly interface to `clone()` and `join()`. Additionally you will need to modify some various parts of the kernel to accommodate multi-threading. 

The modification mainly has to do with managing the stack that is passed to `clone()` and returned from `join()`. The two functions you must implement are: 

```
int thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2);
int thread_join();
```

The requirements for `thread_create()` are as follows:

- The first argument `start_routine` is the function that the thread will run (so this is what is passed to the `fcn` parameter of the `clone()` syscall), and the remaining arguments are the two arguments that `start_routine` expects (so these two arguments correspond directly to `arg1` and `arg2` of the `clone()` syscall). 

- `thread_create()` returns the pid of the thread, if the thread is successfully created. Otherwise it returns -1. 

- `thread_create()` is responsible for creating the user space stack to pass to `clone()`. This must be done through `malloc()` -- so you are not allowed to pass a statically declared array for example, and if you do so, your code will likely fail the majority of the tests. Note that you must ensure that the stack address you pass to `clone()` is page-aligned -- see further hints below on how to do this (`malloc()` is not guaranteed to return a page-aligned address). 

The `thread_join()` function will call `join()` and ensure:

- The stack associated with the exiting thread is freed (using `free()`) -- since this was allocated using `malloc()`. 

- On success, `thread_join()` should return the PID of waited-for child, and on failure it should return -1. 


## The wait() syscall

You will need to adjust the wait() syscall so that it waits only for a child process that does not share the address space with the current process executing wait(). An important thing to note when modifying wait(): 
The wait() syscall is the place where the page directory and the mapped pages of the process are freed (using `freevm()`). Make sure that when you call `freevm()`, there are no other processes that share the same address space with the process calling `freevm()` -- otherwise you risk kernel panic.


## The exit() syscall

As per the POSIX specification, when a process exits, all its threads should be killed -- but its child processes (that do not share its address space) should be left alone. Some things you need to pay attention to when implementing this feature:

- Make sure that when you kill a child thread, set its parent to `initproc` and wake up the `initproc` (using `wakeup1()` function) -- this is so that the `initproc` is aware that there is a thread to be reapedit next time it is scheduled to run. 

## The sbrk() syscall

The `sbrk()` syscall can be used to grow or shrink the process size. Calling `sbrk` inside a thread is generally a bad practice, but if a thread does call sbrk() to **shrink** the process's size (i.e., the argument to sbrk() in this case is a negative number), then the call should fail (return -1), but it should not cause a kernel panic or a trap (page fault). However, if the current thread is the only thread running, then such a call should be handled as normal.


# Some hints

The main difference between this part and Part 1 is the additional dynamic memory management you need to do, both in the user space and in the kernel space. 

- In the user space, if there are multiple threads running and two or more call `malloc()` or `free()`, there can be a potential race condition in reading/writing to the free pointers maintained by the malloc library (in `user/umalloc.c`). This could easily corrupt the heap data structure and crash your program. You will need to implement a user level lock and use this lock to ensure mutual exclusion when updating heap meta data. A simple spinlock library has been provided for you (in `user/ulock.c`) -- so you can use this or create your own. 

- `malloc()` in `xv6` uses `sbrk()` system call to grow the process heap size when it runs out of free memory. In turn, `sbrk()` calls `growproc()` (in `kernel/proc.c`) to allocate additional memory pages and update the page table (`pgdir` field in `struct proc`), and to adjust the "size" of the process (the `sz` field in `struct proc`) accordingly. Multiple processes sharing the same address space means that both `pgdir` and `sz` can potentially be updated concurrently by different processes, which could create a race condition and potentially causes kernel panic. You would need to ensure that read/write to these structures are mutually exclusive, so you would need another lock to guard these structures. 

- The stack address passed to `clone()` must be of one page in length and is  page-aligned (i.e., the address must be a multiple of page size). Note that `malloc()` may not return a page-aligned address, so to ensure that you get a page-aligned address, you could malloc two pages, and find the closest aligned address from the address returned from malloc. Since we allocated two pages, this will ensure that there is still at least 1 page memory available after this adjustment. So let's say that the address returned by malloc is 16, so we have the following situation:

    ```
    [16 bytes][-------- 4096 bytes -------]|[--- 4080 bytes --][16 bytes]|
    |        |                             |                   |         |
    A        page boundary                 page boundary       B         page boundary
    ```

    Let's say malloc() returns two pages at this range [A, B]. Then A + 16 should be the address use for the stack passed to `clone()` -- so `clone` will actually see the stack in the range [A+16, A+4112], which is still contained within the allocated space [A,B]. 

    That is the easy part. The tricky part is to make sure that the stack returned by `join()` can be freed. Using the above example, `join` would return the address A+16 that we passed to `clone`, so it's not the address that we should free (it will likely cause heap corruption if we did). So we have to somehow store that piece of information about the 16 byte adjustment to the stack address. This part is left as an exercise for you. 

# Testing your implementation

We have provided a test program `user/a4tests.c` to test your implementation. 
There are a total of 16 test cases -- see the file [tests.md](./tests.md) for the description of each test, and the source code of `a4tests.c` for further details.  
