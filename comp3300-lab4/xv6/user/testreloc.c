#include "types.h"
#include "stat.h"
#include "user.h"
#include "memlayout.h"

__attribute__((aligned(4096))) char buf[4096 * 4]; // 16KB buffer for testing; aligned to page boundary


// Test reading 0x0 (should cause a fault)
void __attribute__((optimize("O0"))) 
test1(void)
{
    char *p = (char *)0x0;
    printf(1, "Test 1: Attempting to write to address 0x0 (should cause a fault)...\n");
    *p = 'a';  // should cause a fault
}

void test2(void)
{
    char *p = (char *)0x300000; // below USERBASE
    printf(1, "Test 2: Attempting to write to address 0x300000 (should cause a fault)...\n");
    *p = 'a';  // should cause a fault
}

void test3(void)
{
    // using 0x0 as argument of read() should cause a fault
    printf(1, "Test 3: Attempting to read into address 0x0 (should return -1)\n");
    if(read(0, (void *)0x0, 1) == -1) {
        printf(1, "Test 3: read() returned -1 as expected\n");
    } else {
        printf(1, "Error: read() did not return -1\n");
    }
}

void test4(void)
{
    // using an address below USERBASE as argument of read() should cause a fault
    printf(1, "Test 4: Attempting to read into address 0x1000 (should return -1)\n");
    if(read(0, (void *)0x1000, 1) == -1) {
        printf(1, "Test 4: read() returned -1 as expected\n");
    } else {
        printf(1, "Error: read() did not return -1\n");
    }
}

void test5(void)
{
    // using an address below USERBASE as argument of write() should cause a fault
    printf(1, "Test 5: Attempting to write from address 0x1000 (should return -1)...\n");
    if(write(1, (void *)0x1000, 1) == -1) {
        printf(1, "Test 5: write() returned -1 as expected\n");
    } else {
        printf(1, "Error: write() did not return -1\n");
    }
}

void test6(void)
{
    // the address of buf should be above USERBASE
    if( (uint)buf < USERBASE ) {
        printf(1, "Error: buf is at address %x, which is below USERBASE (%x)\n", (uint)buf, USERBASE);
        exit();
    }
    printf(1, "Test 6: buf is at address %x, which is above USERBASE (%x)\n", (uint)buf, USERBASE);
}

void test7(void)
{
    // fork a child process and have it access an invalid address below USERBASE
    int pid = fork();
    if (pid < 0) {
        printf(1, "Fork failed\n");
        exit();
    } else if (pid == 0) {
        // child process
        char *p = (char *)0x200000; // below USERBASE
        printf(1, "Test 7 (child, pid %d): Attempting to write to address 0x200000 (should cause a fault)...\n", getpid());
        *p = 'a';  // should cause a fault
        exit(); // should not reach here
    } else {
        // parent process
        wait(); // wait for child to terminate
        printf(1, "Test 7 (parent, pid %d): Child process terminated (as expected if fault occurred)\n", getpid());
    }
}

typedef void (*testfunc_t)(void);

testfunc_t tests[] = {
    test1,
    test2,
    test3,
    test4,
    test5,
    test6,
    test7
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
        // run all tests
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