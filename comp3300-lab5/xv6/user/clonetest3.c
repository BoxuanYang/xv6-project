#include "types.h"
#include "user.h"

// In this example, we will have two threads updating a global counter.
// But one of the threads will run in a child process, so will have a separate
// address space from the other thread. Here we will see that the counter
// will be updated independently by each thread. 

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
    int pid; 
    char buf[64];
     
    PRINT(buf, 64, "Thread 1 stack: %p\nThread 2 stack: %p\n", stack1, stack2); 
    PRINT(buf, 64, "Counter value: %d\n", counter); 

    pid = fork();
    if(pid == 0)
    {
        t1 = clone(foo, (void *)1, (void *)1, (void *)stack1);
        if(t1 == -1)
            PRINT(buf, 64, "thread 1 clone failed\n");
        else 
            PRINT(buf, 64, "thread 1 clone succeeded.\n"); 
        char *ret_stack;  
        PRINT(buf, 64, "waiting for threads to finish\n"); 
        while(join((void**)&ret_stack) != -1) 
            ;
        PRINT(buf, 64, "Counter value in child process: %d\n", counter); 
        exit();  
    }
    else if(pid < 0) 
        {
            PRINT(buf, 64, "fork failed\n");
            exit(); 
        }

    t2 = clone(foo, (void *)2, (void *)0, (void *)stack2);
    if(t2 == -1)
        PRINT(buf, 64,"thread 2 clone failed\n");
    else {
        PRINT(buf, 64,"thread 2 clone succeeded.\n"); 
    }

    char *ret_stack;  
    PRINT(buf, 64,"waiting for threads to finish\n"); 
    while(join((void**)&ret_stack) != -1) 
        ;

    while(wait()!=-1)
        ;

    PRINT(buf, 64,"Counter value in main process: %d\n", counter); 
    exit(); 
}