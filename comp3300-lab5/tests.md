
# COMP3300/COMP6330 Coding Assignment 4 -- Tests

This assignment will be tested against the following categories. The provided `a4tests.c` contains sample tests for each category -- the actual assessment will use a variant of these tests, but the categories to be tested are the same. 

- Test 1: testing basic working of clone() -- updating a shared counter between the main thread and the clone.  A correct output should show the updated shared counter is seen in the main process. The stack passed to the clone() must be page aligned. A correct output should show something like the following: 

    ```
    $ a4tests 1
    Calling clone() with an unaligned stack address: 3001
    Calling clone() with a page-aligned stack address: 4000
    waiting for threads to finish
    My first thread in xv6!
    Counter value: 123
    zombie!
    ```

    Note that you may notice a `zombie` warning as the main thread does not wait for the clone, so it's passed on to the init process and reaped. You will see these appearing in cases where the parent process exits before its clones do. 

- Test 2: testing basic working of clone() & join() -- updating a shared counter between two clones. A correct output should show traces of counter updates in both clones, and the final counter value is updated to greater than or equal to 100. Note that race condition is expected as there's no lock used yet, so the final counter value in the main process may not be exactly 200. A correct output would look something like: 

    ```
    $ a4tests 2
    Thread 1 stack: 3000
    Thread 2 stack: 4000
    Counter value: 0
    waiting for threads to finish
    [0] thread 1, counter = 0
    [0] thread 2, counter = 0
    [1] thread 2, counter = 1
    [2] thread 2, counter = 2
    [3] thread 2, counter = 3
    [4] thread 2, counter = 4
    ....
    [95] thread 1, counter = 107
    [96] thread 1, counter = 108
    [97] thread 1, counter = 109
    [98] thread 1, counter = 110
    [99] thread 1, counter = 111
    Counter value: 112
    ```

    Your tests may show a different order of thread execution. The final counter value here could range from 100 - 200, due to race conditions. 


- Test 3: testing wait() & exit() -- this is similar to Test 2, but instead of join(), wait() is used to wait for the threads to finish. A correct implementation of wait() should ignore clones, and as the main process exits, all its clones should be killed. The clones should turn into zombies shortly after the main process exits, and reaped by the init process. Using Ctrl-P key on QEMU should show a list of processes, and there should be no zombies left. A correct output should show something like: 

    ```
    $ a4tests 3
    Thread 1 stack: 3000
    Thread 2 stack: 4000
    Counter value: 0
    zombie!
    zombie!
    $ 
    ```

    You may also occasionally observe the threads running, they should never complete their runs, so the counter value in the main process should be 0 or close to 0.


- Test 4: testing a nested call clone() within a fork(). The program will create a a clone and a child process; the latter will create another clone. Both clones are updating the global counter, but their address space should be separate, so their updates should be independent of each other. At termination, the counter value should be the same for both the parent and the child processes. A correct run would show something like: 

    ```
    $ a4tests 4
    Thread 1 stack: 3000
    Thread 2 stack: 4000
    Counter value: 0
    [0] thread 2, counter = 0
    [1] thread 2, counter = 1
    [0] thread 1, counter = 0
    [2] thread 2, counter = 2
    [3] thread 2, counter = 3
    ....
    [95] thread 1, counter = 95
    [96] thread 1, counter = 96
    [97] thread 1, counter = 97
    [98] thread 1, counter = 98
    [99] thread 1, counter = 99
    Counter value in child process: 100
    Counter value in main process: 100
    ```
    The two counter values shown at the last two lines should be identical. 

- Test 5: testing a nested call of fork() within a clone(). The main process creates two clones, and each clone creates a child process to update the shared counter. The counter value updates inside the child processes will not be visible, so in a correct output, the counter value in a thread should differ from the counter value shown in its child process, e.g.,

    ```
    $ a4tests 5
    child process in thread 1: counter = 101
    thread 1: counter = 1
    child process in thread 2: counter = 102
    thread 2: counter = 2
    main thread: counter = 2
    ```

- Test 6: testing locks and thread synchronization. This example creates 5 clones to update a counter concurrently. The updates are synched through a lock. Each clone increments the counter by 10, so when they finish, the counter value should be equal to 50, e.g., here's a sample run:

    ```
    $ a4tests 6
    thread 0: counter = 1
    thread 0: counter = 2
    ...
    thread 1: counter = 49
    thread 3: counter = 49
    thread 3: counter = 50
    Counter value in main process: 50
    ```

- Test 7: testing malloc() & free() are correctly synchronised across threads. For this test, the program break is extended in advance to ensure there's enough mapped address space for malloc to use, avoiding the need for the kernel to call growproc() when malloc() is repeatedly called. A correct run should show the final counter value of 100, and it should not crash the kernel or generate a trap 14 error in the user space. This test is good for you to check that your synchronisation for malloc/free is working correctly, without having to worry about the kernel level synchronisation yet. 

- Test 8: testing basic thread_create/thread_join functionalities -- one thread. As in Test 7, program break is extended prior to calls to thread_create to accommodate malloc() calls without changing process size. This is basically a repeat of Test 7 to make sure that your solution is robust enough (still under the scenario where the address space of the clones does not change throughout their execution).

- Test 9: similar to test 8, but with many threads. Again the program break is sufficiently extended prior to calls to thread_create. 

- Test 10: similar to test 7, but with thread_create/thread_join replacing clone/join. 

- Test 11: testing calls to sbrk() in many threads. This will test that the process size and the page directory updates are synchronised across threads. If these updates are not synchronised, this could generate either a trap 14 error in the user space or a kernel panic. This program involves 5 iterations; each iteration creates a cluster of 10 threads to update a shared counter. The difference is that each thread, the program break is extended using `sbrk()`, so without a kernel level synchronisation for the page table update, this will likely crash the kernel. A correct output will show the counter value updated correctly (=500), and your system does not crash. 

- Test 12: testing calls to large mallocs within multiple threads -- this will test that memory allocation synchronisation across threads, both at the user level and the kernel level. A correct output will show the final counter value of 200, and the program does not crash (and neither does the kernel).

- Test 13: testing a large number of calls to pairs of thread_create/thread_join, and malloc/free within each thread. This will potentially identify issues such as memory leak (free not implemented correctly) or other edge cases. In terms of thread creation, it is very simple as it only every creates one thread in every iteration, and waits for the thread to complete before proceeding to the next. A correct output should show the final counter value of 1000. 

- Test 14: testing passing complex arguments back and forth to a thread. This program creates a list of random numbers. The list structure is a simple linked list. It is to demonstrate that we can pass complex data structures to a thread and get its value updated. The threads in this case effectively perform an insertion sort. A correct output should show the random list and its correctly sorted version, e.g.,

    ```
    $ a4tests 14
    Random list: 23 46 97 56 47 10 89 24 31 70 45 68 7 98 65 36 31 78 53 40 
    Sorted list: 7 10 23 24 31 31 36 40 45 46 47 53 56 65 68 70 78 89 97 98 
    ```

- Test 15: testing a large number of calls to clusters of thread_create/thread_join. This is similar to test 13 but with added checks for concurrency issues. A correct output should show the final counter value of 10000 (and no kernel panic or crashes). 

- Test 16: this is just Test 14 repreated 500 times. 

