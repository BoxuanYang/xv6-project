# COMP3300/COMP6330 Lab 2 (Coding Assignment 1) -- Implementing System Calls

In this lab we will look at how syscalls are implemented in xv6, and write our own custom syscalls. 
This lab will be covered in Week 3 and Week 4. 

The xv6 source code for this lab is slightly modified from the previous lab -- please make sure you use [the version of xv6 included in this lab.](./xv6/) 

For Week 3, we will focus on understanding the control flow of syscalls (Task 0, Taks 1 and Task 2), starting from the user space down to the kernel space and back. We will start with debugging a syscall. This is then followed by implementing a simple syscall to shutdown the OS and tracing syscalls. 

For Week 4, we will be implementing slightly more complicated system calls and userspace programs (Task 3 & Task 4). The objectives of these tasks is to understand how to pass information from the userspace to the kernel and vice versa. 

This lab also serves as Coding Assignment 1 -- you must submit your solution to the tasks below (except for Task 0) via CECS Teaching Gitlab, following the instructions below. 

# Deadline and submission instructions

**Deadline: Wednesday, 20 August 2024, 5pm Canberra time**

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


**IMPORTANT NOTES:** Your implementation for this assignment must only modify a subset of the following files: 

- include/defs.h
- include/syscall.h
- include/user.h
- kernel/proc.c
- kernel/syscall.c
- kernel/sysfile.c
- kernel/sysproc.c
- kernel/time.c
- user/usys.S
- user/ps.c

If you modify any files outside this set in your final submission, you risk your submission be considered invalid. See also the "Marking guidelines" below.

# Problem description

There are 5 (five) tasks in this assignment: 

* [Task 0 (not graded) -- debugging system calls.](./gdb.md) This is an exercise to familiarise yourself with kernel debugging and learning how syscalls are implemented. It is not graded. 

* [Task 1 (mark 10/100) -- poweroff syscall.](./poweroff.md) In this task you will implement a syscall to issue the poweroff command to QEMU to shutdown xv6.

* [Task 2 (mark: 30/100) -- tracing system calls.](./trace.md). You are asked to implement a system call `trace()` that works similarly the `strace` command of linux.

* [Task 3 (mark: 30/100) --  implementing time() and stime() syscalls.](./time.md) For this task, you will learn how time is represented in Unix, and implement two syscalls that are commonly found in UNIX-like operating systems: `time()` (to read the current time) and `stime()` (to set the current time).

* [Task 4 (mark: 30/100 ) -- querying process information.](./procinfo.md) For this task you will implement a syscall `procinfo` (with **syscall number 24**) to obtain some information about processes from the kernel, and write a user level program (`ps`) to use this syscall to print information about the processes. 




# Marking guidelines

- In the marking process, we will copy only the following files to our internal xv6 repository for testing:
  - include/defs.h
  - include/syscall.h
  - include/user.h
  - kernel/proc.c
  - kernel/syscall.c
  - kernel/sysfile.c
  - kernel/sysproc.c
  - kernel/time.c
  - user/usys.S
  - user/ps.c

  If the resulting repository compiles, we will proceed to the next stage. If not, you will get 0 mark, so **make sure that your implementation changes only a subset of the above files**, otherwise you risk your assignment being considered invalid.

- Assuming your code compiles (subject to the restriction in the previous point),  marking for this assignment is entirely based on a set of (semi-)automated tests.  


# Statement of originality

You must complete the [statement of originality form](./statement-of-originality.md) -- a missing or incomplete statement may render your submission invalid. 

The use of Generative AI Tools (e.g., ChatGPT) is permitted in this course, given that proper citation and prompts are provided, along with a description of how the tool contributed to this assignment, in the statement of originality.  

Guidelines regarding appropriate citation and use can be found on the ANU library website [https://libguides.anu.edu.au/generative-ai](https://libguides.anu.edu.au/generative-ai).
Marks will reflect the contribution of the student rather than the contribution of the tools. Further guidance on appropriate use should be directed to the convener for this course.


## Acknowledgment

- The trick to shutdown QEMU from a guest OS was sourced from the QEMU sample boot code for i386:
[https://github.com/qemu/qemu/blob/master/tests/tcg/i386/system/boot.S](https://github.com/qemu/qemu/blob/master/tests/tcg/i386/system/boot.S)

- The `trace` syscall exercises were adapted from [https://pdos.csail.mit.edu/6.828/2023/labs/syscall.html](https://pdos.csail.mit.edu/6.828/2023/labs/syscall.html)
