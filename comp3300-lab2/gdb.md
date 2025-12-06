## Task 0 (not graded) -- Debugging syscalls 

### Compiling x6 

Before you start this task, you need to edit `user/usys.S` to add an interface to all the syscalls you will implement in this assingment. 
Add the following lines to `user/usys.S`:

```
SYSCALL(halt)
SYSCALL(time)
SYSCALL(stime)
SYSCALL(trace)
SYSCALL(procinfo)
```

This will register these new syscalls at the userspace. 


### Installing GEF extension

- We will be using the [GEF extension](https://github.com/hugsy/gef) to gdb for this lab. You can use the following command to install GEF. 

    ```
    bash -c "$(curl -fsSL https://gef.blah.cat/sh)"
    ```

- Read [this overview of xv6 syscalls](https://github.com/remzi-arpacidusseau/ostep-projects/blob/master/initial-xv6/background.md) to familiarise yourself with the way system calls are implemented in xv6. Review Chapter 1 of the xv6 book for a more in-depth description of xv6 syscalls. 

- Review Week 2 lectures (if you missed the lectures). 


### Example: debugging the read syscall

Let's try to trace the sequence of calls from a user-level program to the kernel via a syscall. For this example, we will trace the `read` syscall made by `user/cat` program. 

- Launch xv6 with gdb server:

    ```
    make qemu-nox-gdb
    ```

  This will launch a gdb server, listening on a tcp port (e.g., port 26000, but you may see a different port number on your machine). Here is an example of what the output should look like: 

  ```
    sed "s/localhost:1234/localhost:26000/" < .gdbinit.tmpl > .gdbinit
    *** Now run 'gdb'.
    qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 2 -m 512  -S -gdb tcp::26000
  ```

  In this example, the server was listening at tcp port 26000. When first launched, the xv6 instance will wait at the entry point of the bootloader (address 0x7c00). It will not proceed unless we issue a command from gdb for it to continue. 

- Open another terminal, go to the xv6 directory, launch gdb.  Note: the `$` sign here is not part of the command; it is there to indicate that these commands are run on a shell, e.g., bash:

    ```
    $ gdb 
    gef>
    ```

  You should see the gef prompt. 

- Next we load the kernel debugging symbols from `kernel/kernel`: 

  ```
  gef>  file kernel/kernel
  Reading symbols from kernel/kernel...
  ```

- Connect to the gdb server at the given tcp port (26000 in this example): 

    ```
    gef> target remote localhost:26000
    ```

- Now type `continue` at the gef prompt to allow the xv6 boot to complete. In the gdb prompt, we see that it is waiting for further breakpoints. But because we have not set any, it is just stuck there waiting for further events.

- Let's now interrupt the execution of the OS, by typing Ctrl-C (that is, press the control key and `C` at the same time) in the gdb window. This will pause the execution. This will allow us to set a break point. Since we want to trace the syscalls in the user program `cat`, we will now load the debugging symbols from `user/_cat`, and set a breakpoint at the read() function, and then continue the execution:  

  ```
  (remote) gef➤  file user/_cat
  Reading symbols from user/_cat...
  (remote) gef➤  break read
  Breakpoint 1 at 0x37b: file user/usys.S, line 15.
  (remote) gef➤  continue
  ```

- Now switch to the other terminal (where the xv6 is running), and type `cat README`, to trigger the breakpoint. 

- On the gdb window, you will see something like this: 

```
────────────────────────────────────────────────────────────────────────────────────────────────────────────── code:x86:32 ────
        0x373 <pipe+0000>      mov    eax, 0x4
        0x378 <pipe+0005>      int    0x40
        0x37a <pipe+0007>      ret    
 →      0x37b <read+0000>      mov    eax, 0x5
        0x380 <read+0005>      int    0x40
        0x382 <read+0007>      ret    
        0x383 <write+0000>     mov    eax, 0x10
        0x388 <write+0005>     int    0x40
        0x38a <write+0007>     ret    
──────────────────────────────────────────────────────────────────────────────────────────────────── source:user/usys.S+15 ────
     10  
     11  SYSCALL(fork)
     12  SYSCALL(exit)
     13  SYSCALL(wait)
     14  SYSCALL(pipe)
 →   15  SYSCALL(read)
     16  SYSCALL(write)
     17  SYSCALL(close)
     18  SYSCALL(kill)
     19  SYSCALL(exec)
     20  SYSCALL(open)
────────────────────────────────────────────────────────────────────────────────────────────────────────────────── threads ────
[#0] Id 1, stopped 0x37b in read (), reason: BREAKPOINT
[#1] Id 2, stopped 0xfd0ac in ?? (), reason: BREAKPOINT
──────────────────────────────────────────────────────────────────────────────────────────────────────────────────── trace ────
[#0] 0x37b → read()
[#1] 0xca → cat(fd=0x3)
[#2] 0x54 → main(argc=0x2, argv=0x2fe8)
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
(remote) gef➤  
```

- You can now use gdb commands to step through the instructions. Some useful commands:
    * `stepi` (or `si`): this steps through the program one machine instruction at a time. 

    * `nexti` (or `ni`): this is similar to `si`, but it will treat any function call as one instruction, so will not step into the function. 

    * `next` (or `n`): this steps through the program one line of source code at a time. 

    * `continue` (or `c`): continue execution until the next breakpoint. 

    * `info break`: see all break points

  For more commands, type `help all` to see the list of all commands. The GEF extension does a pretty good job at displaying information about the states of the process, providing both the assembly view of the code around the program counter, and also the source code, in addition to the state of various registers and the stack region. You can also use gdb commands to inspect these states individually, e.g.,

  * `info registers` should provide information about contents of CPU registers.
  * `info locals` will display values of local variables in the current function.
  * `info frame` will display information about the current call frame. 
  * You can also use the `print` command and the `x` command to inspect the contents of memory addresses and registers. Again, use the `help` command to learn more about these, if you have not used gdb before. 
  * `backtrace` will display the call stack leading to the current function (but this information should already be part of GEF debugging output). You can navigate between frames by using the `frame n` command (substitute `n` with the frame number in the stack) and inspect their contents individually. 

- Step through the program using `stepi`, until you reach the instruction `int 0x40` at address `0x380`. This is where the switch from the user space to the kernel space will happen. 

- After you step through the `int 0x40` instruction at `0x380`, we transition to the kernel space, so to be able to use the debugging symbols for the kernel, we will need to load the kernel binary again: 

  ```
  (remote) gef➤  file kernel/kernel
  Reading symbols from kernel/kernel...
  Error in re-setting breakpoint 1: Function "read" not defined.
  ```

  Notice that we get an error that the function `read` is not defined as that was defined in the user program, but not in the kernel. 

- You can now experiment with different commands of gdb to step through the execution, to understand the call trace for a typical system call. You may want to consult various source files to gain a better understanding of the control flow of a syscall. This will be useful later when you write your own syscall.  

- **Exercise:** 
As a simple exercise, trace the `read` syscall for the command `wc README`, and intercept the read syscall just as the `wc` command is reading from the `README` file, and make the function `sys_read` return 0. The output of `wc README` would then show that there is 0 byte in `README` if you changed the value correctly. _Hint: the return value of a syscall is usually stored in the eax register, so you can use the `gdb` `set` command to change this register before returning (e.g., `set $eax=0`)._ 

