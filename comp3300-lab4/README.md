# COMP3300/COMP6330 Lab 4 (Coding Assignment 3) -- xv6 virtual memory

In this lab, we will learn how virtual memory in xv6 works through a number of exercises. By working through these exercises, you should gain a good understanding of the two-level page table used in Intel x86 32-bit architecture, memory protection and basic memory management (via mmap). There are four major tasks in this lab, to be attempted in Week 7 and 8. 

This lab also serves as Coding Assignment 3 -- you must submit your solution to the tasks below via CECS Teaching Gitlab, following the instructions below. 


## Deadline and submission instructions

**Deadline: Wednesday, 1 October 2025, 5pm Canberra time**

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


# Task 1 (15 marks) -- Relocating user space processes

Recall from the lectures that the xv6 virtual memory is arranged in a way that the address range from 0 to KERNBASE (which is 0x80000000) is reserved for the user space, and anything above KERNBASE is reserved for the kernel. 
The user programs in xv6 are by default loaded to memory address 0x0 -- this means that the null pointer in xv6 is always valid, i.e., it is always mapped. This also means that we don't have a null pointer exception in xv6 like in most real-world operating systems (and as a consequence, it's harder to detect memory corruption bugs that could lead to undefined behaviour in programs). 

In this task, you ae asked to modify xv6 so that each user space process (except for the `initcode` that launches the `init` process) is mapped to an address range that starts from 0x1000000 (which we shall refer to as `USERBASE`). This will leave the null pointer (0x0) unmapped, and hence a null pointer derefence will trigger a page fault. We will see how to handle such a fault in Task 2. For now, your task is to adjust the kernel code of xv6 so that addresses below `USERBASE` (including the null pointer) are not mapped. 

Some of the boilerplate code (such as syscalls definitions, various new constants, etc) has been prepared for you. This includes a change to the Makefile, to tell the linker to shift the program base to USERBASE (via the `-Ttext ${USERBASE}` option). However, you still need to adjust the kernel code to load the program to the intended USERBASE, and make sure that addresses below USERBASE are not mapped. 

Some hints to help you to get started:

- Study the `exec()` function in `kernel/exec.c` to understand how the address space gets filled with code and initialised. Go through all the important functions that `exec()` calls, in particular those in `kernel/vm.c` that perform various memory mapping and querying of page table entries. 

- The main loop in `exec()` iterates through the list of _program headers_ of the user binary, which is in ELF format. A program header contains information about a _segment_ of code that will be loaded to memory. (In this case, xv6 user binaries has a very simple structure -- it only has one segment). Program headers are stored in the data structure `struct proghdr`. The members of this structure that we will use are:
    - `vaddr`: this corresponds to the starting virtual address where this bytes of the segment of the code/data will be loaded. 
    - `memsz`: this corresponds to the size of the segment in memory.
    - `filesz`: this corresponds to the size of the segment (stored) in the file -- note that this can differ from `memsz`, as some data (such as uninitialised global variables) does not need to be stored in file, but space must be reserved in memory when it is loaded.

  Currently the code in exec() still assumes that the the address starts at 0x0 -- you will need to modify this so that code is loaded starting at `USERBASE`. 

- You also need to study how `fork()` works, in particular the part where the parent address space is copied to the child address space. 

You can test your kernel to see how it handles null pointer dereference. By default, xv6 has a handler that catches all traps and exceptions that have not been explicitly handled (see `kernel/trap.c`), so a null pointer dereference should trigger this default handler. The provided `user/testnull.c` is a simple program to test the null pointer dereference. Executing it should trigger a generic trap error to be printed by the kernel. For more test cases, see also `user/testreloc.c`, which also tests your solution for Task 2 (see below). 

# Task 2 (10 marks) -- page fault handling

As mentioned in Task 1, when an unknown fault occurs, the kernel currently handles it by printing a generic message detailing the trap number, the error number, the faulting address and other information. In this task, you will implement a specific handler for page fault (or better known as the dreaded "segmentation fault" in most systems), to output a more meaningful error message when it occurs.  

A page fault error can occur in at least a couple of situations:
- the requested page is not mapped, or
- the requested page is mapped, but it is protected, i.e., the faulting operation attempted an operation that is not permitted (e.g., trying to write to a read-only page, or trying to access a kernel page from userspace).

The source code to modify in this case is the function `trap()` in `kernel/trap.c`. Study the `default` handler in that function to see where the information about the trap and the error code stored. 

You may find the [Intel System Programming Guide](./IA32-3A.pdf) useful, see in particular  `Interrupt 14—Page-Fault Exception` (page 5-54) on the meaning of various error codes, to determine whether a page fault is due to request to an unmapped page, or a protected page.

You can use the program `user/memrw.c`, that allows the user to read/write arbitrary memory addresses, to test your page fault handling. Below are some of the expected output of running `memrw` on various addresses:

```
$ memrw
Enter an address to read from and write to (in hex, e.g., 0x0011aabb): 0x00
Segmentation fault: process 3 attempted to read from an unmapped address (0x0).

$ memrw
Enter an address to read from and write to (in hex, e.g., 0x0011aabb): 0x1234
Segmentation fault: process 4 attempted to read from an unmapped address (0x1234).

$ memrw
Enter an address to read from and write to (in hex, e.g., 0x0011aabb): 0x80000000
Segmentation fault: process 5 attempted to read from a protected address (0x80000000). 
```

The first and the second examples show the expected output when trying to access any addresses below USERBASE, which are not mapped. The last example shows an attempt to access a kernel address, which is mapped, but has the supervisor bit set. 

The program `user/testreloc.c` shows some additional test cases; see [this file](./reloc.txt) for a sample of the expected output.

# Task 3 (35 marks) -- mprotect and munprotect

Another issue with how xv6 maps the user process address space is that by default it is very permissive on what the process can do to any parts of its address space. By default, all userspace addresses are mapped with read and write permissions, including the code segment! Note that for the Intel 32 bit without PAE (which is what we use in the lab), the execute-disable bit capability is not supported, so we cannot mark a page as non-executable, and we are restricted to enable/disable write-protection. 

For this task, you will implement a system call `mprotect` to protect a segment of the user virtual address space so that it is readable, but not writeable, and another system call `munprotect` to remove the protection. These syscalls should have the following signatures:

- `int mprotect(void *addr, int len)`
- `int munprotect(void *addr, int len)`

where `addr` is the starting address of the memory region to (un)protect and `len` is the length of this region. 

Calling mprotect() should change the protection bits of the page range starting at addr and of len pages to be read only. Thus, the program could still read the pages in this range after mprotect() finishes, but a write to this region should cause a page fault trap (with error number 14), that should be handled by your page fault handler in Task 2. The munprotect() call does the opposite: sets the region back to both readable and writeable.

The page protections should be inherited on fork(). Thus, if a process has mprotected some of its pages, when the process calls fork, the OS should copy those protections to the child process.

There are some failure cases to consider: if addr is not page aligned, or addr points to a region that is not currently a part of the address space, or len is less than or equal to zero, return -1 and do not change anything. Otherwise, return 0 upon success.

An important consideration in implementing mprotect/munprotect is how they interact with other syscalls. For this assignment, we restrict to ensuring the correct interactions with the read() and the write() syscalls. In particular, if a buffer that is passed to read() is write-protected, then the expectation is that the syscall would return -1 and it should not cause a kernel panic.


Some hints: 

- The page table directory of each process is stored in the `pgdir` field in `struct proc`. You will need this to find the page table entry corresponding to a particular virtual address. 

- The permission bits in a page table entry are the lower 3 bits. Modify these bits to enforce the permission you want. 
  * Bit 0: this corresponds to the Present bit (PTE_P). It is set if the entry is mapped to main memory.
  
  * Bit 1: this corresponds to Read/Write (PTE_W). It is set if the entry is writeable; otherwise it is read-only. (Note: every entry is always executable -- there's no explicit executable flag in Intel 32-bit architecture). 

  * Bit 2: this corresponds to user/supervisor level access. It is set if the entry is accessible to the user (PTE_U); otherwise, it is accessible only in the kernel mode. 

- After changing a page-table entry, you need to update the CR3 register to let the hardware know of the change.  This guarantees that your PTE updates will be used upon subsequent accesses. You  can use `lcr3()` (see `kernel/vm.c` for an example use of this function) to update CR3 register. 

- The protection bits apply at the granulity of a page, so if an address to protect lands in the middle of a page, the entire page is protected. 


The provided `user/testmprotect.c` contains a (non-exhaustive) list of tests that you can use to test your implementation. (You should also try to come up with your own tests). Note that some of the tests assume that you have implemented mmap.
A sample of the expected output is given [here.](./mprotect.txt)


# Task 4 (40 marks) -- mmap and munmap

For this task you are asked the `mmap` and `munmap` system calls in xv6 to allocate/deallocate memory for given a range of virtual addresses. The `mmap` syscall is a (very much) simplified version of its counterpart in Linux -- it essentially corresponds to a what is called an anonymous mapping in Linux (that is not backed by the file system) that is private to the process that invokes it. 

The signatures of `mmap` and `munmap` for this assignment and their specifications (adapted from the `man` page of these calls in linux) are as follows:

- `void* mmap(void *addr, int length, int prot)` 
- `int munmap(void *addr, int length)` 

A failed call to mmap and munmap should always return -1. Note that -1 when interpreted as a (void *) pointer results in an invalid pointer so there is no confusion between valid pointers and the value of a failed allocation. 

## mmap

In `mmap(addr, length, prot)`, the `addr` argument is taken as a _hint_ of where to allocate the memory. If `addr` is NULL, then the kernel chooses the (page-aligned) address at which to create the mapping. If `addr` is not null then the kernel find **the next available address after `addr`.** Length must be greater than 0. The `prot` argument here is simply 0 or 1. When `prot=0` this tells the kernel to map the memory as read-only, and when `prot=1`, to map it for read and write. 

Note that for this assignment, we require that mmap works in a deterministic way when finding the address to allocate. It must always return the next available address available (if any). For example, let's say that we have the following memory map (shown at the page granularity,so each cell represents a page): 

```
Page #:  0  1  2  3  4  5  6  7  8
        [x][ ][ ][ ][x][ ][ ][ ][ ]
```

where page number shows the page number relative to MMAPBASE, so page #0 is at address MMAPBASE, page #1 at MMAPBASE+PAGESIZE, and so on, and where a `[x]` indicates that page is already mapped and `[ ]` indicates it is still available. 
If a request is made to, say, `mmap(MMAPBASE, 2 * PAGESIZE, 0)` then your implementation must return `MMAPBASE+PAGESIZE` (i.e., page 1), as the requested two pages will fit in the free space between page 0 and 4. If there are no available pages above the requested address, mmap should attempt to search for available pages starting from MMAPBASE, and return -1 if it fails to find sufficient available pages to fulfill the request. 

The mmap-ed pages must be located above a MMAPBASE, which is defined to be 0x60000000, and below the kernel base (KERNBASE=0x80000000) of xv6. A request to mmap non-NULL addresses below MMAPBASE or at or above KERNBASE must be rejected. 

While allocating memory pages for `mmap(addr, length)`, if any of the pages within the range fail to be allocated, mmap should fail (return -1), and all the pages allocated for this request so far must be freed. For example, suppose a process makes a request to `mmap(addr, 20 * PGSIZE)`, and in the process of allocating these pages (using `kalloc()`), the 5th allocation fails (e.g., kernel runs out of free physical pages to allocate), then all the previous four pages allocated already must be unmapped and freed (using `kfree()`). 

Just as is the case with mprotect, memory allocated using mmap should be compatible with the syscalls read() and write(), in the sense that the buffers allocated correctly using mmap() can be passed as arguments to read()/write() without triggering a segmentation fault or kernel panic.


## munmap

The `munmap(addr, len)` system call deletes the mappings for the specified address range (from `addr` to `addr+len-1`), and causes further references to addresses within the range  to generate  invalid  memory references.  The region must also be automatically unmapped when the process is terminated.  The  address `addr` must be a multiple of the page size (but length need not be).  All pages containing a part of the indicated  range  are  unmapped, and subsequent references to these pages will generate trap 14 error. It is not an error if the indicated range does not contain  any  mapped pages.

Note that `munmap` is allowed to unmap any address range outside the kernel. In particular, it should be allowed to unmapp addresses below MMAPBASE, such as code or data segment (with an obvious consequence that it will crash the program)! 

## Testing 

We have provided a test program `user/testmmap.c` to test your implementation of mmap/munmap. 
There are a total of 20 test cases -- see the file [testmmap.md](./testmmap.md) for the description of each test, and the source code of `user/testmmap.c` for further details.  

# Submission requirements

For this assignment, you are allowed to modify only the following files:

- `include/defs.h`
- `kernel/exec.c`
- `kernel/syscall.c`
- `kernel/sysfile.c`
- `kernel/sysproc.c`
- `kernel/trap.c`
- `kernel/vm.c`

Note that in particular, you are not allowed to add any data structures to `struct proc`. 
You must use the existing `pgdir` (page directory entry) of a process to get the necessary information to check available pages and the ranges of virtual addresses that have or have not been mapped. 

The necessary constants and definitions to patch the syscall from the user space to the kernel have already been done for you; all you have to do is to implement the actual functions that perform the mapping/unmapping.  

**IMPORTANT: DO NOT modify any other files than those listed above -- your assignment may not compile or may fail, and you may be given 0 mark.**


