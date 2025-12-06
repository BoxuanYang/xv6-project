#include "types.h"
#include "user.h"
#include "x86.h"

// In this example, we have 5 threads updating a global counter concurrently.
// We will use a spinlock to ensure the critical section is executed by
// one thread at a time. 
// The counter should show the correct
// value (=50) after all threads complete execution. 
//  
// See the kernel implementation of spinlock to figure out how to implement
// a user space version. You will need the xchg() function (available in x86.h)


lock_t tlock; 

volatile int counter = 0;

char __attribute__ ((aligned (4096))) stack[10*4096]; 

void __attribute__((optimize("O0"))) foo(void *arg1, void *arg2)
{
    int i;
    int a; 
    int tid = (int)arg1; 
    int s = (int)arg2; 
    char buf[64]; 

    for(i=0; i < 10; ++i) {
        lock_acquire(&tlock);
        a = counter; 
        sleep(s);
        counter = a + 1;
        lock_release(&tlock);  
        PRINT(buf, 64, "thread %d: counter = %d\n", tid, counter); 
    }
    exit(); 
}

int main()
{
    int i; 
    char buf[64];

    lock_init(&tlock); 

    for(i=0; i<5; ++i)
    {
        if(clone(foo, (void*)i, (void*)(i%2), stack + i*4096) == -1) {
            PRINT(buf, 64, "clone failed\n");
            exit();
        }
    }
    PRINT(buf, 64, "Counter value: %d\n", counter); 
    
    char *ret_stack;  
    PRINT(buf, 64, "waiting for threads to finish\n"); 
    while(join((void**)&ret_stack) != -1) 
        ;

    PRINT(buf, 64, "Counter value in main process: %d\n", counter); 
    exit(); 
}