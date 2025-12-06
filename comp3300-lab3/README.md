# COMP3300/COMP6330 Lab 3 (Coding Assignment 2) -- Scheduling

In this lab we will implement variants of the lottery scheduler for xv6. Recall from the lectures on scheduling and Chapter 9 of OSTEP, in a lottery scheduler, each process is assigned a number of "tickets", and every time context switching happens, the scheduler decides which process to run next by a random selection. This lab will be covered in Week 5 and Week 6. 

This lab also serves as Coding Assignment 2 -- you must submit your solution to the tasks below via CECS Teaching Gitlab, following the instructions below. 


## Deadline and submission instructions

**Deadline: Wednesday, 3 September 2025, 5pm Canberra time**

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

- Make sure you complete the [Statement of Originaility](./statement-of-originality.md) for this assignment. An incomplete Statement of Originality may result in your submission to be considered invalid. 

- Once your work has been pushed to your Gitlab, you do not need to do anything further! We are able to fork your submissions for marking.
  - You can double check your submission status by going to your fork of the assignment and checking for your most recent commit.
  - Don't be afraid to ask questions about the process in the labs or on EdStem!

# Lab setup

For this lab, we use a slightly modified version of `xv6`, which you can find in the [xv6 subdirectory in this folder](./xv6/). Make sure you use this version of xv6 when attempting this lab. 

This is mostly the same as the one we have seen in Lab 01 and Lab 02, but with a few minor differences:

- You now have the option of printing messages in the kernel to a "debug console". The debug console is a separate console from the standard I/O console that you normally use to interact with the xv6 OS. To print this debug console, you will use the function `dprintf` (in `kernel/console.c`), and run xv6 using the command 

    ```
    make qemu-nox-dc
    ```

    This compilation option creates a network server listening at port 45454. To see the output of the debug console, you then connect to it (in another terminal) using:

    ```
    nc localhost 45454
    ```

- An environment variable `KERNELCONFIG` is added to the `Makefile` to allow one to pass additional compiler options while running `make`. This will be used to enable a debug mode in the kernel, and to instruct the compiler to use a particular scheduler (see the next point).

- The implementation of the scheduler() function in `kernel/proc.c` has been changed to allow one to use different schedulers. The original xv6 scheduler is now renamed to `rr_scheduler()` (i.e., the round robin scheduler). This is the default scheduler. In `kernel/proc.c`, you are also provided with two stubs (`lottery_scheduler()` and `hybrid_scheduler()`) that you must implement. By passing the compiler option `SCHEDULER`, you can select which scheduler to use when compiling the kernel. For selecting the lottery scheduler, use `-DSCHEDULER=1` and for selecting the hybrid scheduler, use `-DSCHEDULER=2`. For example, to compile the kernel to use the lottery scheduler, run:

    ```
    KERNELCONFIG="-DSCHEDULER=1" make qemu-nox
    ```


# Problem description


There are 4 tasks in this lab:

- [Task 1 (mark: 10/100) -- changing scheduler latency.](./task1.md)

- [Task 2 (mark: 20/100) -- scheduler metrics.](./task2.md)

- [Task 3 (mark: 30/100) -- lottery scheduler](./task3.md)

- [Task 4 (mark: 40/100) -- hybrid scheduler](./task4.md)




## Marking guidelines (TODO)

- In the marking process, we will copy only the following files to our internal xv6 repository for testing:
    - kernel/proc.c
    - kernel/trap.c

    If the resulting repository compiles, we will proceed to the next stage. If not, you will get 0 mark, so **make sure that your implementation changes only the above files**, otherwise you risk your assignment being considered invalid.


- For each task, your implementation will be tested against the scenarios exemplified in the task description. See the task description for details. 

- Your code will be checked manually for compliance with the requirements; a non-compliant solution (e.g., a lottery scheduler that does not use a random number generator, or manipulation of scheduler metrics, etc) will receive no marks. 

