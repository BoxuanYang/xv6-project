## Task 1 (Marks: 10/100) -- implementing a `poweroff` syscall

So far we have used the key sequence `Ctrl-a x` to power off the QEMU session running xv6. Now we will instead implement a syscall to issue the poweroff command to QEMU. To do this, you will implement a syscall, called `sys_halt` (with **syscall number 22**), and a user program `poweroff` that uses this syscall to poweroff the current xv6 session. 

To shutdown QEMU from xv6, we will send the value `0x2000` to the CPU port `0x604` (note: this is very specific to recent versions of QEMU, so it may not work on older versions or on a different virtualisation platform). The following code does the job. 

```C
// shutdown the system
int 
sys_halt(void)
{
  outw(0x604, 0x2000);

  return 0; // never reached if shutdown successful
}
```

There are several files you would need to modify; 

- `kernel/sysproc.c`: This is where you put the `sys_halt` function above that does the actual work. 

- `include/syscall.h`, `kernel/syscall.c`: You would need to add a new entry in the syscall table and assign a new  syscall number to your syscall.

- `user/usys.S`:  Add a macro `SYSCALL(halt)` to the list of syscalls (at the user level)

- `include/user.h`: Add a signature for the halt syscall `int halt(void)`. 


Once you are done with implementing the syscall, write a user level program `poweroff.c` that calls `halt()` syscall to shutdown the OS. Refer to Lab 01 on how to add and compile a user level program in xv6.
