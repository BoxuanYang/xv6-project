#include "types.h"
#include "user.h"

volatile int counter = 0;

// Make sure the stack is aligned to the page size (4096)
char __attribute__ ((aligned (4096))) thread_stack[4096]; 

// a simple thread to update a global counter
void foo(void *arg1, void *arg2)
{
    char buf[64];
    // PRINT is a wrapper macro for printf, but one that guarantees
    // the output is uninterrupted, as long as the it's less than 512 bytes
    PRINT(buf,64, "My first thread in xv6!\n");
    counter = 123; 
    exit();  // always calls exit() at the end of the execution of a thread
}

int main()
{
    int t1;
    char buf[64]; 
    printf(1, "Thread stack address: %p\n", thread_stack);     
    printf(1, "Counter value: %d\n", counter);

    // call clone() and sets execution to start at foo()
    // for now, the two arguments are just nulls as they are not used in foo().  
    t1 = clone(foo, 0, 0, (void *)thread_stack);
    if(t1 == -1) {
        PRINT(buf, 64, "thread clone failed\n");
        exit(); 
    }
    PRINT(buf, 64, "thread clone succeeded.\n"); 

    // for now, we don't use join() yet, so put the main thread to
    // sleep for a while until the thread finishes. 
    PRINT(buf, 64, "waiting for threads to finish\n"); 
    sleep(100); 
    PRINT(buf, 64, "Counter value: %d\n", counter); // This should show the updated counter. 
    exit(); 
}