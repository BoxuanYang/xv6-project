# COMP3300/COMP6330 Lab 05/Coding Assignment 4 -- Kernel Threads


In this lab, you will implement real kernel threads in xv6. We borrow the concept of lightweight processes from the linux kernel in this implementation (albeit in a very much simplified form) in implementing threads as schedulable entities in the kernel -- so the kernel will be in charge of managing thread creation. 
You will also build a thread library that interfaces with the kernel threads, and use a userspace synchronisation primitive to manage thread level synchronisation.

This lab consists of two parts, Part 1 and Part 2. Part 1 is not graded, and is meant for you to learn the basic mechanisms needed to build a basic working kernel thread. Part 2 is an extension of Part 1; it is graded and it corresponds to Coding Assignment 4 for this course. 


## Deadline and submission instructions

**Deadline: Monday, 20 October 2025, 5pm Canberra time**

**Late submission penalty: 100% of total mark** 


- Fork this repository to your own namespace. 
    * **Make sure** that the 'visibility' of your fork is set to **private**.
    * **Make sure** that you select _your_ namespace. (this is only applicable to students who have greater than normal Gitlab access - others will only be able to select their own namespace.)
    * **DO NOT RENAME THE PROJECT - LEAVE THE NAME AND URL SLUG EXACTLY AS IS**
    * You may notice an additional member in your project - `comp3300-2025-s2-marker`. **Do not remove this member from your project**.

- Clone the repository to your machine. You most likely will have to use HTTPS for this as SSH is unavailable to most connections.

- Make sure to commit and push to the Gitlab regularly to save your work.
 
- Make sure your work is **committed and pushed** to this repository **before** the deadline (accounting for extensions/EAPs). 
  - As per the usual, a 100% late submission deduction applies.

- Make sure you complete the [Statement of Originality](./statement-of-originality.md) for this assignment (i.e., Part 2 in the problem description below). An incomplete Statement of Originality may result in your submission to be considered invalid. 

- Once your work has been pushed to your Gitlab, you do not need to do anything further! We are able to fork your submissions for marking.
  - You can double check your submission status by going to your fork of the assignment and checking for your most recent commit.
  - Don't be afraid to ask questions about the process in the labs or on EdStem!


# Problem description

To implement a functioning thread library, we first implement a more basic system call, `clone()` (see linux `man clone` to see a real-world version of this system call) for creating a kernel thread, and another syscall `join()` to wait for a thread. Part 1 implements a basic version of these syscalls, and Part 2 extends these versions to build a fully functioning thread library. 

Part 1 is not graded so you are free to discuss the solutions with your classmates and the tutors. Part 2 corresponds to Coding Assignment 4, which you must complete on your own. 

* [Part 1 (not graded): Building `clone()` from `fork()`](./part1.md)

* [Part 2: Building a thread library](./part2.md)


# Submission requirements

For this assignment, you are allowed to modify only the following files:

- include/defs.h
- include/proc.h
- kernel/exec.c
- kernel/proc.c
- kernel/syscall.c
- kernel/sysproc.c
- kernel/vm.c
- user/thread.c
- user/umalloc.c

Please note that during assessment, we will replace the `user/a4tests.c` file with a variant which will test the same scenarios but with minor differences in some details. So make sure you do not change the APIs required by this test file, or you will risk your assignment failing all tests. 

The necessary constants and definitions to patch the syscall from the user space to the kernel have already been done for you. Similarly, the necessary header files for the new thread library and the lock library have been provided (in `user/ulock.c`). All you have to do is to implement the actual functions that perform the mapping/unmapping; for the thread library, this is located in `user/thread.c`.



# Marking guidelines

- In the marking process, we will copy only the files listed in the "Submisison Requirements" section to our internal xv6 repository for testing. If the resulting repository compiles, we will proceed to the next stage. If not, you will get 0 mark, so **make sure that your implementation changes only a subset of the above files**, otherwise you risk your assignment to be considered invalid.

- Your implementation will be tested against the 16 scenarios as described in [tests.md](./tests.md). Each scenario is worth 6.25% of the total mark for this assignment. The tests will be done semi-automatically, using a variant of the provided test program (`user/a4tests.c`).  

- Your mark is based on the number of tests you pass. However, we will also perform manual inspection of source code, for compliance checks and for plagiarim checks. 


# Reference

Parts of this lab were adapted from: 

- [OSTEP kernel threads project description.](https://github.com/remzi-arpacidusseau/ostep-projects/blob/master/concurrency-xv6-threads/README.md)
