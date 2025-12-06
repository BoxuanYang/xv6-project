#include "types.h"
#include "stat.h"
#include "user.h"

__attribute__((aligned(4096))) char buf[4096 * 4]; // 16KB buffer for testing; aligned to page boundary

#define NULL 0
#define MMAPBASE 0x60000000

// Test mprotect syscall
void test1(void)
{
    // protect buffer to read-only
    if (mprotect(buf+2*4096, 4096) < 0) {
        printf(1, "Error: mprotect failed\n");
        return;
    }
    // try to write to the buffer
    printf(1, "Attempting to write to protected buffer (should cause a segfault)...\n");
    buf[2*4096] = 'A';
    printf(1, "Error: write to protected buffer succeeded\n");
}

// Test mprotect and munprotect 
void test2(void)
{
    // protect buffer to read-only
    printf(1, "Protecting buffer...\n");
    if (mprotect(buf, sizeof(buf)) < 0) {
        printf(1, "Error: mprotect failed\n");
        return;
    }

    printf(1, "Unprotecting buffer...\n");
    // unprotect the buffer to allow reading and writing
    if (munprotect(buf, sizeof(buf)) < 0) {
        printf(1, "Error: munprotect failed\n");
        return;
    }
    printf(1, "Writing to unprotected buffer (should succeed)...\n");
    buf[0] = 'B';
    printf(1, "OK: Wrote to unprotected buffer successfully\n");
}

void test3(void)
{
    // call mprotect to KERNBASE
    if (mprotect((void*)0x80000000, 4096) < 0) {
        printf(1, "OK: mprotect to KERNBASE failed as expected\n");
    } else {
        printf(1, "Error: mprotect to KERNBASE succeeded unexpectedly\n");
    }
    // call munprotect to KERNBASE
    if (munprotect((void*)0x80000000, 4096) < 0) {
        printf(1, "OK: munprotect to KERNBASE failed as expected\n");
    } else {
        printf(1, "Error: munprotect to KERNBASE succeeded unexpectedly\n");
    }
}

void test4(void)
{
    // call mprotect to an invalid address
    if (mprotect((void*)0x123, 4096) < 0) {
        printf(1, "OK: mprotect to invalid address failed as expected\n");
    } else {
        printf(1, "Error: mprotect to invalid address succeeded unexpectedly\n");
    }
    // call munprotect to an invalid address
    if (munprotect((void*)0x123, 4096) < 0) {
        printf(1, "OK: munprotect to invalid address failed as expected\n");
    } else {
        printf(1, "Error: munprotect to invalid address succeeded unexpectedly\n");
    }
}

void test5(void)
{
    // protect buffer to read-only
    if (mprotect(buf, sizeof(buf)) < 0) {
        printf(1, "Error: mprotect failed\n");
        return;
    }
    int fd[2];
    pipe(fd);
    write(fd[1], "foo", 3);
    // this should return -1
    if (read(fd[0], buf, 3) != -1) {
        printf(1, "Error: calling read() with a protected buffer succeeded unexpectedly\n");
    } else {
        printf(1, "OK: Calling read() with a protected (read-only) buffer failed as expected\n");
    }

    if (munprotect(buf, sizeof(buf)) < 0) {
        printf(1, "Error: munprotect failed\n");
        return;
    }
    if(read(fd[0], buf, 3) != 3) {
        printf(1, "Error: calling read() with an unprotected buffer failed\n");
    } else {
        printf(1, "OK: Calling read() with an unprotected buffer succeeded as expected\n");
    }
    close(fd[0]);
    close(fd[1]);
}

void test6(void)
{
    // protect buf partially
    if (mprotect(buf+4096, 4096) < 0) {
        printf(1, "Error: mprotect failed\n");
        return; 
    }
    // write to unprotected part
    buf[0] = 'X';
    buf[4095] = 'X';
    buf[8192] = 'X';
    printf(1, "OK: Wrote to unprotected part of buffer successfully\n");
    // write to protected part
    buf[4096] = 'Y';
    printf(1, "Error: wrote to protected part of buffer successfully\n");
}

void test7(void)
{
    // protect buf partially
    if (mprotect(buf+4096, 4096) < 0) {
        printf(1, "Error: mprotect failed\n");
        return; 
    }
    // calling read() with a partially protected buffer should fail
    int fd[2];
    pipe(fd);
    write(fd[1], "foobar", 6);
    if (read(fd[0], buf, sizeof(buf)) != -1) {
        printf(1, "Error: calling read() with a partially protected buffer succeeded unexpectedly\n");
    } else {
        printf(1, "OK: Calling read() with a partially protected buffer failed as expected\n");
    }
}

// proctet and unprotect mmapped memory
void test8(void)
{
    // protect and unprotect mmapped memory
    char *addr = mmap(NULL, 4096, 1);
    if ((int)addr == -1) {
        printf(1, "Error: mmap failed\n");
        return;
    }
    if (mprotect(addr, 4096) < 0) {
        printf(1, "Error: mprotect failed\n");
        return;
    }
    printf(1, "OK: Protected mmapped memory successfully\n");
    // writing to the protected mmapped memory should cause a segfault
    printf(1, "Attempting to write to protected mmapped memory (should cause a segfault)...\n");
    addr[0] = 'X';
    printf(1, "Error: wrote to protected mmapped memory successfully\n");
}

void test9(void)
{
    // unprotect mmapped memory
    char *addr = mmap(NULL, 4096, 1);
    if ((int)addr == -1) {
        printf(1, "Error: mmap failed\n");
        return;
    }

    if (mprotect(addr, 4096) < 0) {
        printf(1, "Error: mprotect failed\n");
        return;
    }
    printf(1, "OK: Protected mmapped memory successfully\n");

    if (munprotect(addr, 4096) < 0) {
        printf(1, "Error: munprotect failed\n");
        return;
    }

    printf(1, "OK: Unprotected mmapped memory successfully\n");

    addr[0] = 'Y';
    printf(1, "OK: Wrote to unprotected mmapped memory successfully\n");
}

void test10(void)
{
    // protect and unprotect mmapped memory partially
    char *addr = mmap(NULL, 3*4096, 1);
    if ((int)addr == -1) {
        printf(1, "Error: mmap failed\n");
        return;
    }
    if (mprotect(addr+4096, 4096) < 0) {
        printf(1, "Error: mprotect failed\n");
        return;
    }
    printf(1, "OK: Protected part of mmapped memory successfully\n");

    // writing to the protected part should cause a segfault
    printf(1, "Attempting to write to protected part of mmapped memory (should cause a segfault)...\n");
    addr[4096] = 'Z';
    printf(1, "Error: wrote to protected part of mmapped memory successfully\n");
}   


void test11(void)
{
    // unprotect part of mmapped memory
    char *addr = mmap(NULL, 3*4096, 1);
    if ((int)addr == -1) {
        printf(1, "Error: mmap failed\n");
        return;
    }
    if (mprotect(addr+4096, 4096) < 0) {
        printf(1, "Error: mprotect failed\n");
        return;
    }
    printf(1, "OK: Protected part of mmapped memory successfully\n");

    if (munprotect(addr+4096, 4096) < 0) {
        printf(1, "Error: munprotect failed\n");
        return;
    }
    printf(1, "OK: Unprotected part of mmapped memory successfully\n");

    addr[4096] = 'W';
    printf(1, "OK: Wrote to unprotected part of mmapped memory successfully\n");
}   

void test12(void)
{
    // mprotect with length not multiple of page size
    if (mprotect(buf, 5000) < 0) {
        printf(1, "Error: mprotect failed\n");
        return;
    }  
    printf(1, "OK: mprotect with length not multiple of page size succeeded\n");
    // writing to the protected part should cause a segfault
    printf(1, "Attempting to write to protected part of mmapped memory (should cause a segfault)...\n");
    buf[4096] = 'Z';
    printf(1, "Error: wrote to protected part of mmapped memory successfully\n");
}

void test13(void)
{
    // test that the protection is inherited by child processes after fork()
    if (mprotect(buf, sizeof(buf)) < 0) {
        printf(1, "Error: mprotect failed\n");
        return; 
    }
    // fork a child process
    int pid = fork();
    if (pid < 0) {
        printf(1, "Error: fork failed\n");
        return;
    }
    if (pid == 0) {
        // child process
        printf(1, "Child process: attempting to write to protected memory (should cause a segfault)...\n");
        buf[0] = 'C';
        printf(1, "Error: Child process wrote to protected memory successfully\n");
        exit();
    }
    // parent process
    wait();
    printf(1, "Parent process: done\n");
}

void test14(void)
{
    // test mprotect with length 0
    if (mprotect(buf, 0) < 0) {
        printf(1, "OK: mprotect with length 0 failed as expected\n");
    } else {
        printf(1, "Error: mprotect with length 0 succeeded unexpectedly\n");
    }
}

void test15(void)
{
    // test munprotect with length 0
    if (munprotect(buf, 0) < 0) {
        printf(1, "OK: munprotect with length 0 failed as expected\n");
    } else {
        printf(1, "Error: munprotect with length 0 succeeded unexpectedly\n");
    }  
}

void test16(void)
{
    // test mprotect with negative length
    if (mprotect(buf, -4096) < 0) {
        printf(1, "OK: mprotect with negative length failed as expected\n");
    } else {
        printf(1, "Error: mprotect with negative length succeeded unexpectedly\n");
    }
}

void test17(void)
{
    // test munprotect with negative length
    if (munprotect(buf, -4096) < 0) {
        printf(1, "OK: munprotect with negative length failed as expected\n");
    } else {
        printf(1, "Error: munprotect with negative length succeeded unexpectedly\n");
    }  
}   

void test18(void)
{
    // test mprotect with address not page-aligned
    if (mprotect(buf+1, 4096) < 0) {
        printf(1, "OK: mprotect with address not page-aligned failed as expected\n");
    } else {
        printf(1, "Error: mprotect with address not page-aligned succeeded unexpectedly\n");
    }
}   

void test19(void)
{
    // test munprotect with address not page-aligned
    if (munprotect(buf+1, 4096) < 0) {
        printf(1, "OK: munprotect with address not page-aligned failed as expected\n");
    } else {
        printf(1, "Error: munprotect with address not page-aligned succeeded unexpectedly\n");
    }  
}

void test20(void)
{
    // test mprotect of mmapped memory with a length that exceeds the mapped length
    char *addr = mmap(NULL, 4096, 1);
    if ((int)addr == -1) {
        printf(1, "Error: mmap failed\n");
        return;
    }

    if (mprotect(addr, 8192) < 0) {
        printf(1, "OK: mprotect with length exceeding mapped length failed as expected\n");
    } else {
        printf(1, "Error: mprotect with length exceeding mapped length succeeded unexpectedly\n");
    }

    printf(1, "Writing to mmapped memory (should succeed)...\n");
    // addr should still be writable
    addr[0] = 'A';
    printf(1, "OK: Wrote to mmapped memory successfully\n");
}

typedef void (*testfunc_t)(void);

testfunc_t tests[] = {
    test1,
    test2,
    test3,
    test4,
    test5,
    test6,
    test7,
    test8,
    test9,
    test10,
    test11,
    test12,
    test13,
    test14,
    test15,
    test16,
    test17,
    test18,
    test19,
    test20
};

int num_tests = sizeof(tests) / sizeof(tests[0]);

void run_test(int testnum)
{
    // run test in a child process
    int pid = fork();
    if (pid < 0) {
        printf(1, "Error: fork failed\n");
        return; 
    }
    if (pid == 0) {
        // child process
        tests[testnum - 1]();
        exit();
    }
    // parent process
    wait();
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        for (int i = 1; i <= num_tests; i++) {
            printf(1, "Running test %d...\n", i);
            run_test(i);
            printf(1, "Test %d completed.\n\n", i);
        }
        exit();
    }

    int testnum = atoi(argv[1]);
    if (testnum < 1 || testnum > num_tests) {
        printf(1, "Invalid test number %d. Must be between 1 and %d\n", testnum, num_tests);
        exit();
    }

    // run the selected test
    tests[testnum - 1]();

    exit();
}