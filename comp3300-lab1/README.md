# Lab 01 -- Introduction to xv6 and Unix utilities

Welcome to the first lab of COMP3300/COMP6330! 

In this lab, you will be introduced to MIT's xv6 operating system. The xv6 OS is a simple OS designed for education purposes, and was implemented based on John Lion's "Commentary on UNIX 6th Edition". 

There are a number of tasks to go through in this lab, some tasks are marked as extensions, and will not be covered during the lab, but you are encouraged to attempt them outside the lab hours. 

## Task 0: lab environment setup

The toolchains needed to compile and run the xv6 operating systems have already been installed in the linux lab computers, so there is nothing to set up during the lab session if you are using the linux lab computers. 

If you wish to setup the toolchains in your own computer, there are a few options depending on your hardware/OS: 

- If you are already running Ubuntu (or any Debian derivatives), the easiest way is to use a recent linux distribution (the equivalent of Ubuntu 24.04 LTS), and install the following packages: 
    * build-essential 
    * gdb 
    * git 
    * gcc-multilib 
    * qemu-system 

- Or you can install VirtualBox and install [the lab VM image](./vm_setup.md) -- which is a virtual machine running Ubuntu 24.04 Server. See [these instructions](./vm_setup.md) for details.

- Or install [Docker Desktop](https://docs.docker.com/desktop/) and create a [docker container for this lab.](./docker.md). Note that a docker container is not a complete operating system -- this is meant to be used to compile and test the xv6 OS only; it may not be suitable to learn the internals of Linux kernel.


## Task 1: compiling and running xv6

We will run the xv6 OS in an emulated environment using the emulator [QEMU](https://www.qemu.org). 
Please note that the version of xv6 we are using in this course is the one that is based on the Intel x86 (32-bit) architecture. The most recent version of xv6 is based on RISC-V, which will not be used in this course. 

The xv6 source code that is included in this repository has been slightly re-organised to make it less cluttered and hopefully easier to navigate through, read and modify. 

Before you start this lab, it is encouraged that you read Chapter 0 of the [xv6 book.](./xv6-book-rev10.pdf)

1. First, clone this repository to your computer (or your virtual machine) where the compiler toolchains and QEMU are installed. 

2. Go to the `xv6` directory and type make: 

    ```
    cd xv6
    make 
    ```

    This should compile the xv6 kernel and some selected user-level programs, packed into two image files (`xv6.img` and `fs.img`). 

3. Launch xv6: 

    ```
    make qemu-nox
    ```

    This will call the qemu emulator to run the xv6 kernel in the terminal. 

4. There's not much yet to do in this very simple OS. But we will implement some basic utilities in the next tasks to see how to make use of system calls in this OS. 

5. To quit the xv6, type `Ctrl-a x` (this means, press the Control key and the `a` key simultaneously, then release them, and press `x`). Note that this is a crude way to terminate the OS. It basically shuts down the emulator (so it's the equivalent of powering a physical computer). In the lab next week, we will implement a proper way to shutdown xv6.   

## Task 2: "hello world!" 

For this task, we will write a simple user-level program to print the "hello world!" message and exit. This is to give you some idea of how to write a user-level program and compile it, and insert it to the xv6 file system, so you can run it inside xv6. 

1. Create a file `user/hello.c` containing the following simple program:  

    ```C
    #include "types.h"
    #include "stat.h"
    #include "user.h"

    int
    main(int argc, char *argv[])
    {
        printf(1, "Hello, world!\n");
        exit();
    }
    ```

2. Modify the `xv6/Makefile`, by adding the line `$U/_hello\`  to the definition of `UPROG` variable in the Makefile. So the UPROG definition should look like the following: 

    ```
    UPROGS=\
        $U/_cat\
        $U/_echo\
        $U/_forktest\
        $U/_grep\
        $U/_init\
        $U/_kill\
        $U/_ln\
        $U/_ls\
        $U/_mkdir\
        $U/_rm\
        $U/_sh\
        $U/_stressfs\
        $U/_usertests\
        $U/_wc\
        $U/_zombie\
        $U/_hello\
    ```

3. Run `make qemu-nox`. This will compile the hello.c program and launches xv6. You should now see the program `hello` when you run `ls` in the shell of xv6. Run the `hello` program to confirm that it works as expected.



## Task 3: sleep

For this task, you will implement a `sleep` program, similar to the one in linux. You will implement this as user-level program, making use of the `sleep` syscall. Name the program `sleep.c`, and put it in `xv6/user/`, and follow the instruction in the previous task to compile and run the program. 

```
$ make qemu-nox
      ...
      init: starting sh
$ sleep 50
(the shell will pause for a short while)
$
```

When implemented correctly, the program should compile and when run you should observe a pause in the xv6 shell. 

User-level programs are all located in the directory `xv6/user/`. Have a look at some programs there, e.g., the `cat.c` or `ls.c` to see how these programs use xv6 syscalls. The file `usertests.c` contain examples of all the syscalls implemented in xv6, so you can use that as a reference if you are unsure of how to use a particular syscall. All the header files are located in `xv6/include/`. 

When writing a program to run in xv6, it is important to note that you cannot use the C standard library provided by linux, e.g., glibc, as this is not implemented in xv6. Currently only a limited set of functions are supported -- these may look very similar to the standard C library functions, such as `printf`, `exit`, or `atoi`, with some subtle differences. For example, you may find a `printf` statement in an xv6 program to take the form `printf(1, "Hello world")` -- here the first argument represents the _file descriptor_ where the string "Hello world" will be written to; in this case, it is the standard output. 

The list of available syscalls and some re-implementation of standard C library functions can be found in the header file `xv6/include/user.h`. 


## Task 4 (extension): pingpong

For this task, we will look at how we can use a "pipe" as a communication channel between processes. (You may want to refresh your knowledge of fork(), wait() and pipe() syscalls from COMP2310). 

A pipe is implemented as an integer array of two-elements, e.g., `int fd[2]`. 
The first element `fd[0]` is the file descriptor used for reading, and `fd[1]` is used for writing. So when process A wants to send a message to process B, A will write to `fd[1]`, and B will read from `fd[0]`. Note that it is a good idea of closing either end or both ends of a pipe when it is no longer used, to avoid a process waiting for it to close.

Here you will implement a two-way communication between a parent process and a child process using two pipes: one pipe to send a character from the parent to the child, and another to send a character from the child to the parent. The parent should start by sending a byte to the child, and upon receiving it, the child will print a message "<pid>: received ping". The child then send a character back to the parent, and when the parent receives it, it should print "<pid>: received pong". Here `<pid>` refers to the process id of the current process (which you can obtain using the `getpid()` function). For example, a correct implementation may print something like (the process ids may vary across different runs): 

```
$ pingpong
6: received ping
5: received pong
$ 
```

Here you may find the sample code in `xv6/user/usertests.c` useful to understand how pipes are used. 

Note: beware of the zombies! Make sure you use the `wait()` syscall in the parent process to check that the child process has terminated properly before exiting the program. 

## Task 5 (extension): find

For this task, you will write a simple version of the UNIX `find` program for xv6, to find all the files in a directory tree with a specific name. Name your program `find.c` and put it `xv6/user/` and compile and run it as in previous tasks. Here is an example of a run of a correct implementation (where we use `mkdir`, `echo` and output redirection to create files and directories): 

```
$ echo > b
$ mkdir a
$ echo > a/b
$ mkdir a/aa
$ echo > a/aa/b
$ find . b
./b
./a/b
./a/aa/b
$
```

For this task, you may find that studying how the `ls` command is implemented useful (see `xv6/user/ls.c`).

## Acknowledgment

The material for this lab was adapted from the following sources:

- MIT xv6 OS: [https://github.com/mit-pdos/xv6-public](https://github.com/mit-pdos/xv6-public)

- MIT's 6.1810 Operating System Engineering (selected lab exercises): [https://pdos.csail.mit.edu/6.828/2023/](https://pdos.csail.mit.edu/6.828/2023/)

- OSTEP projects: [https://github.com/remzi-arpacidusseau/ostep-projects](https://github.com/remzi-arpacidusseau/ostep-projects)

