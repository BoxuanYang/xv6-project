#include "types.h"
#include "user.h"

// In this example, we will have two threads updating a global counter.
// The counter update is intentionally  split in two steps, with a sleep() 
// in between, to show the effect of race condition. 

volatile int counter = 0;

char __attribute__ ((aligned (4096))) stack1[4096]; 
char __attribute__ ((aligned (4096))) stack2[4096]; 

void __attribute__((optimize("O0"))) foo(void *arg1, void *arg2)
{
    int i;
    int a; 
    int tid = (int)arg1; 
    int s = (int)arg2; 
    char buf[64]; 
    for(i=0; i < 100; ++i) {
        PRINT(buf, 64, "[%d] thread %d, counter = %d\n", i, tid, counter); 
        a = counter; 
        sleep(s);
        counter = a+1;
    }
    exit(); 
}

int main()
{
    int t1;
    int t2; 
    char buf[64]; 

    printf(1, "Thread 1 stack: %p\nThread 2 stack: %p\n", stack1, stack2); 
    
    printf(1, "Counter value: %d\n", counter); 

    // create two threads. The first argument of the thread is a number used to identify
    // the thread. The second argument is used to specify the amount of `ticks` the
    // thread should sleep in between counter updates. 
    t1 = clone(foo, (void *)1, (void *)1, (void *)stack1);
    t2 = clone(foo, (void *)2, (void *)0, (void *)stack2);
    if(t1 == -1) {
        PRINT(buf, 64, "thread 1 clone failed\n");
    }
    else {
        PRINT(buf, 64, "thread 1 clone succeeded.\n"); 
    }

    if(t2 == -1) { 
        PRINT(buf, 64, "thread 2 clone failed\n");
    }
    else { 
        PRINT(buf, 64, "thread 2 clone succeeded.\n"); 
    }

    char *ret_stack;  
    PRINT(buf, 64, "waiting for threads to finish\n"); 
    while(join((void**)&ret_stack) != -1) 
        ;

    PRINT(buf, 64, "Counter value: %d\n", counter); 
    exit(); 
}