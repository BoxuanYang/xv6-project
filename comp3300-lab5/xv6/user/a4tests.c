// ANU COMP3300/COMP63300 Operating Systems Implementation
// Test programs for Coding Assignment 4
// (c) Alwen Tiu, 2024

#include "types.h"
#include "user.h"

struct list_t {
    int num;
    struct list_t *next; 
}; 

struct sequence_t {
    struct list_t *head; 
    lock_t lk; 
};  


lock_t tlock; 

volatile int counter = 0;

char __attribute__ ((aligned (4096))) thread_stack[10*4096]; 

unsigned long randstate = 1;
unsigned int
rand()
{
  randstate = randstate * 1664525 + 1013904223;
  return randstate;
}

int member(int n, struct sequence_t seq)
{
    struct list_t *l=seq.head;
    while(l)
    {
        if(n == l->num) {
            return 1; 
        }
        l=l->next;
    }
    return 0;
}

void fun1(void *arg1, void *arg2)
{
    char buf[64]; 
    sprintf(buf, "My first thread in xv6!\n");
    write(1, buf, strlen(buf)); 
    counter = 123; 
    exit();  // always calls exit() at the end of the execution of a thread
}

void __attribute__((optimize("O0"))) fun2(void *arg1, void *arg2)
{
    int i;
    int a; 
    char buf[64]; 
    int tid = (int)arg1; 
    int s = (int)arg2; 

    for(i=0; i < 100; ++i) {
        sprintf(buf, "[%d] thread %d, counter = %d\n", i, tid, counter); 
        write(1, buf, strlen(buf));
        a = counter; 
        sleep(s);
        counter = a+1;
    }
    exit(); 
}

void __attribute__((optimize("O0"))) fun3(void *arg1, void *arg2)
{
    int i;
    int a; 
    int tid = (int)arg1; 
    int s = (int)arg2; 
    int pid; 
    char buf[128]; 

    counter += s; 
    pid = fork();     
    if(pid == 0) {
        for(i=0; i < 100; ++i) {
            a = counter; 
            counter = a+1;
        }
        sprintf(buf, "child process in thread %d: counter = %d\n", tid, counter); 
        write(1, buf, strlen(buf)); 
        exit();
    } 
    else if(pid < 0) {
        printf(1, "fork failed\n");
        exit(); 
    }
    wait(); 
    sprintf(buf, "thread %d: counter = %d\n", tid, counter); 
    write(1, buf, strlen(buf));
    exit(); 
}

void __attribute__((optimize("O0"))) fun4(void *arg1, void *arg2)
{
    int i;
    int a; 
    char buf[64]; 
    int tid = (int)arg1; 
    int s = (int)arg2; 
    for(i=0; i < 10; ++i) {
        lock_acquire(&tlock);
        a = counter; 
        counter = a + 1;
        lock_release(&tlock);  
        sleep(s);
        sprintf(buf, "thread %d: counter = %d\n", tid, counter); 
        write(1, buf, strlen(buf)); 
    }
    exit(); 
}

void __attribute__((optimize("O0"))) fun5(void *arg1, void *arg2)
{
    int i;
    int a; 
    int tid = (int)arg1; 
    int s = (int)arg2; 
    char buf[64]; 
    int *t; 
    
    t = malloc(4); 
    if(t == 0) {
        printf(1, "malloc error\n");
        exit();
    }
    for(i=0; i < 10; ++i) {
        lock_acquire(&tlock);
        a = counter; 
        *t = counter; 
        counter = a + 1;
        lock_release(&tlock);  
        sleep(s);
        sprintf(buf, "thread %d: counter = %d\n", tid, *t); 
        write(1, buf, strlen(buf)); 
    }
    free(t);
    exit(); 
}

void __attribute__((optimize("O0"))) fun6(void *arg1, void *arg2)
{
    int i;
    int a; 
    int tid = (int)arg1; 
    int s = (int)arg2; 
    char buf[128];
    char *addr; 

    sbrk(4096); 
    addr = sbrk(-4096); 
    if(addr != (char *)-1) {
        printf(1, "error: sbrk() with a negative argument should fail in a child thread\n"); 
        exit();
    }

    for(i=0; i < 10; ++i) {
        lock_acquire(&tlock);
        a = counter; 
        counter = a + 1;
        lock_release(&tlock);  
        sleep(s);
        sprintf(buf, "thread %d: counter = %d\n", tid, counter); 
        write(1, buf, strlen(buf)); 
    }
    exit(); 
}

void __attribute__((optimize("O0"))) fun7(void *arg1, void *arg2)
{
    int i;
    int a; 
    int tid = (int)arg1; 
    int s = (int)arg2; 
    int *t; 
    char buf[128]; 
    
    t = malloc(4096); 
    if(t == 0) {
        printf(1, "malloc error\n");
        exit();
    }
    for(i=0; i < 10; ++i) {
        lock_acquire(&tlock);
        a = counter; 
        *t = counter; 
        counter = a + 1;
        lock_release(&tlock);  
        sleep(s);
        sprintf(buf, "thread %d: counter = %d\n", tid, *t); 
        write(1, buf, strlen(buf)); 
    }
    free(t);
    exit(); 
}

void __attribute__((optimize("O0"))) fun8(void *arg1, void *arg2)
{
    int i;
    int a; 
    int tid = (int)arg1; 
    int *t; 
    char buf[128]; 
    
    t = malloc(1+ (rand()%5) * 4096 + rand()%4096); 
    if(t == 0) {
        printf(1, "malloc error\n");
        exit();
    }
    for(i=0; i < 10; ++i) {
        a = counter; 
        *t = counter; 
        counter = a + 1;
        sprintf(buf, "thread %d: malloc'ed pointer = %p, counter = %d\n", tid, t, *t); 
        write(1, buf, strlen(buf)); 
    }
    free(t);
    exit(); 
}

void __attribute__((optimize("O0"))) fun9(void *s, void *v)
{
    int num = (int) v;
    struct sequence_t *seq = (struct sequence_t *) s; 
    
    if(seq == 0) {
        printf(1, "first argument must not be null\n");
        exit(); 
    }

    struct list_t *node = (struct list_t *) malloc(sizeof(struct list_t)); 

    if(node == 0) 
    {
        printf(1, "malloc error\n");
        exit(); 
    } 
    node->num = num;
    node->next = 0; 

    lock_acquire(&seq->lk);
    if(seq->head == 0) {
        seq->head = node; 
        node->next = 0; 
    }
    else {
        struct list_t * l = seq->head;
        struct list_t * prev = 0; 

        while(l)
        {
            if(l->num >= node->num) {
                node->next = l;
                break;  
            }
            prev = l; 
            l = l->next;
        }
        if(prev == 0) seq->head = node; 
        else prev->next = node; 
            
    }
    lock_release(&seq->lk);

    exit(); 
}

void __attribute__((optimize("O0"))) fun10(void *arg1, void *arg2)
{
    int i;
    int a; 
    int tid = (int)arg1; 
    int s = (int)arg2; 
    char buf[128];
    int *t; 

    int pid = getpid();

    lock_acquire(&tlock);
    if(pid == 7){
        printf(1, "pid7 gping to do malloc\n");
    }
    t = malloc(1+ (rand()%5) * 4096 + rand()%4096); 
    lock_release(&tlock);

    if(t == 0) {
        printf(1, "malloc error\n");
        exit();
    }

    for(i=0; i < 10; ++i) {
        lock_acquire(&tlock);
        a = counter; 
        counter = a + 1;
        lock_release(&tlock);  
        sleep(s);
        sprintf(buf, "thread %d: counter = %d\n", tid, counter); 
        write(1, buf, strlen(buf)); 
    }
    free(t); 
    exit(); 
}

void test1()
{
    int t1;
    char *stack; 
    char buf[64];

    stack = thread_stack + 1; // create an unaligned stack pointer
    printf(1, "Calling clone() with an unaligned stack address: %p\n", stack);     

    // call clone() and sets execution to start at foo()
    // for now, the two arguments are just nulls as they are not used in foo().  
    t1 = clone(fun1, 0, 0, (void *)stack);
    if(t1 != -1) {
        printf(1, "Error: clone() must check that the stack is aligned\n");
        exit(); 
    }
    sleep(100); 

    stack = thread_stack + 4096; 
    printf(1, "Calling clone() with a page-aligned stack address: %p\n", stack);     

    t1 = clone(fun1, 0, 0, stack);
    if(t1 == -1) {
        printf(1, "Error: thread clone failed\n");
        exit(); 
    }

    // for now, we don't use join() yet, so put the main thread to
    // sleep for a while until the thread finishes. 
    sprintf(buf, "waiting for threads to finish\n"); 
    write(1, buf, strlen(buf)); 
    sleep(100); 
    printf(1, "Counter value: %d\n", counter); // This should show the updated counter. 
    exit(); 
}

void test2()
{
    int t1;
    int t2; 
    char *stack1 = thread_stack;
    char *stack2 = thread_stack+4096;

    printf(1, "Thread 1 stack: %p\nThread 2 stack: %p\n", stack1, stack2); 
    
    printf(1, "Counter value: %d\n", counter); 

    // create two threads. The first argument of the thread is a number used to identify
    // the thread. The second argument is used to specify the amount of `ticks` the
    // thread should sleep in between counter updates. 
    t1 = clone(fun2, (void *)1, (void *)1, (void *)stack1);
    t2 = clone(fun2, (void *)2, (void *)0, (void *)stack2);
    if(t1 == -1) {
        printf(1, "Error: thread 1 clone failed\n");
        exit(); 
    }
    if(t2 == -1) {
        printf(1, "Error: thread 2 clone failed\n");
        exit(); 
    }

    char *ret_stack;  
    printf(1, "waiting for threads to finish\n"); 
    while(join((void**)&ret_stack) != -1) 
        ;

    printf(1, "Counter value: %d\n", counter); 
    exit(); 
}

void test3()
{
    int t1;
    int t2; 
    char buf[32]; 
    char *stack1 = thread_stack;
    char *stack2 = thread_stack+4096;

    printf(1, "Thread 1 stack: %p\nThread 2 stack: %p\n", stack1, stack2); 
    
    // create two threads. The first argument of the thread is a number used to identify
    // the thread. The second argument is used to specify the amount of `ticks` the
    // thread should sleep in between counter updates. 
    t1 = clone(fun2, (void *)1, (void *)1, (void *)stack1);
    t2 = clone(fun2, (void *)2, (void *)0, (void *)stack2);
    if(t1 == -1) {
        printf(1, "Error: thread 1 clone failed\n");
        exit();
    } 

    if(t2 == -1) {
        printf(1, "Error: thread 2 clone failed\n");
        exit();
    }

    // instead of join(), we execute wait(). 
    // This should cause the threads to be killed, so they won't
    // complete their iterations. 
    while(wait()!=-1)
        ;

    sprintf(buf, "Counter value: %d\n", counter); 
    write(1, buf, strlen(buf)); 
    exit(); 
}

void test4()
{
    int t1;
    int t2; 
    int pid; 
    char *stack1 = thread_stack;
    char *stack2 = thread_stack+4096;

    printf(1, "Thread 1 stack: %p\nThread 2 stack: %p\n", stack1, stack2); 
    printf(1, "Counter value: %d\n", counter); 

    pid = fork();
    if(pid == 0)
    {
        t1 = clone(fun2, (void *)1, (void *)1, (void *)stack1);
        if(t1 == -1) {
            printf(1, "thread 1 clone failed\n");
            exit();
        }
        char *ret_stack;  
        while(join((void**)&ret_stack) != -1) 
            ;
        printf(1, "Counter value in child process: %d\n", counter); 
        exit();  
    }
    else if(pid < 0) 
        {
            printf(1, "fork failed\n");
            exit(); 
        }

    t2 = clone(fun2, (void *)2, (void *)0, (void *)stack2);
    if(t2 == -1) {
        printf(1, "thread 2 clone failed\n");
        exit(); 
    }

    char *ret_stack;  
    while(join((void**)&ret_stack) != -1) 
        ;

    while(wait()!=-1)
        ;

    printf(1, "Counter value in main process: %d\n", counter); 
    exit(); 
}

void test5()
{
    int t1;
    int t2; 
    char *stack1 = thread_stack;
    char *stack2 = thread_stack+4096;

    t1 = clone(fun3, (void *)1, (void *)1, (void *)stack1);
    if(t1 == -1) {
        printf(1, "thread 1 clone failed\n");
        exit();
    }

    char *ret_stack; 
    join((void**)&ret_stack);

    t2 = clone(fun3, (void *)2, (void *)1, (void *)stack2);
    if(t2 == -1) {
        printf(1, "thread 2 clone failed\n");
        exit();
    }

    join((void**)&ret_stack);

    printf(1, "main thread: counter = %d\n", counter); 
    exit(); 
}


void test6()
{
    int i; 
    char *stack = thread_stack; 

    lock_init(&tlock); 

    for(i=0; i<5; ++i)
    {
        if(clone(fun4, (void*)i, (void*)(i%2), stack + i*4096) == -1) {
            printf(1, "clone failed\n");
            exit();
        }
    }

    char *ret_stack;  
    while(join((void**)&ret_stack) != -1) 
        ;

    printf(1, "Counter value in main process: %d\n", counter); 
    exit(); 
}

void test7()
{
    int i; 
    char *stack = thread_stack; 
    
    // malloc a huge chunk of memory and free it. 
    // This will provide sufficient free memory in the heap
    // so the process/threads should not need to call growproc() again
    // to grow the process memory after calls to clone()   
    free(malloc(100 * 4096));

    lock_init(&tlock); 

    for(i=0; i<10; ++i)
    {
        if(clone(fun5, (void*)i, (void*)(i%2), stack + i*4096) == -1) {
            printf(1, "clone failed\n");
            exit();
        }
    }
    
    char *ret_stack;  

    while(join((void**)&ret_stack) != -1) 
        ;

    printf(1, "Counter value in main process: %d\n", counter); 
    exit(); 
}

void test8()
{   
    // malloc a huge chunk of memory and free it. 
    // This will provide sufficient free memory in the heap
    // so the process/threads should not need to call growproc() again
    // to grow the process memory after calls to clone()
    free(malloc(100 * 4096));
    
    if(thread_create(fun2, (void*)0, (void*)1) == -1) {
            printf(1, "thread_create failed\n");
            exit();
    }
    
    thread_join(); 
    
    printf(1, "Counter value in main process: %d\n", counter); 
    exit(); 
}

void test9()
{
    int i; 
    
    // malloc a huge chunk of memory and free it. 
    // This will provide sufficient free memory in the heap
    // so the process/threads should not need to call growproc() again
    // to grow the process memory after calls to clone()
    free(malloc(100 * 4096));

    lock_init(&tlock); 

    for(i=0; i<10; ++i)
    {
        if(thread_create(fun4, (void*)i, (void*)(i%2)) == -1) {
            printf(1, "clone failed\n");
            exit();
        }
    }
    
    while(thread_join() != -1) 
        ;

    printf(1, "Counter value in main process: %d\n", counter); 
    exit(); 
}

void test10()
{
    int i; 
    
    // malloc a huge chunk of memory and free it. 
    // This will provide sufficient free memory in the heap
    // so the process/threads should not need to call growproc() again
    // to grow the process memory after calls to clone()
    free(malloc(100 * 4096));
    
    lock_init(&tlock); 

    for(i=0; i<10; ++i)
    {
        if(thread_create(fun5, (void*)i, (void*)(i%2)) == -1) {
            printf(1, "clone failed\n");
            exit();
        }
    }
    
    printf(1, "waiting for threads to finish\n"); 
    while(thread_join() != -1) 
        ;

    printf(1, "Counter value in main process: %d\n", counter); 
    exit(); 
}

void test11()
{
    int i,k; 
    char buf[128];
    
    sbrk(4096 * 20);
    // Shrinking sbrk should be allowed if no other threads running. 
    if(sbrk(-4096) == (void*)-1) {
        printf(1, "error: sbrk() returned -1\n");
        exit(); 
    }

    lock_init(&tlock); 

    for(k=0; k < 5; ++k) 
    {
        for(i=0; i<10; ++i)
        {
            if(thread_create(fun6, (void*)i, (void*)(i%2)) == -1) {
                printf(1, "clone failed\n");
                exit();
            }
        }
        sprintf(buf, "waiting for batch %d of threads to finish\n", k); 
        write(1,buf,strlen(buf));

        while(thread_join() != -1) 
            ;

    }


    printf(1, "Counter value in main process: %d\n", counter); 
    exit(); 
}


void test12()
{
    int i; 
    char buf[128];
    
    lock_init(&tlock); 

    printf(1, "creating thread\n"); 

    for(i=0; i<20; ++i)
    {
        if(thread_create(fun7, (void*)i, (void*)(i%2)) == -1) {
            printf(1, "clone failed\n");
            exit();
        }
    }
   
    sprintf(buf, "waiting for threads to finish\n"); 
    write(1, buf, strlen(buf)); 
    while(thread_join() != -1) {
    }

    printf(1, "Counter value in main process: %d\n", counter); 
    exit(); 
}

void test13()
{
    int i; 
    
    for(i=0; i<1000; ++i)
    {
        if(thread_create(fun8, (void*)i, (void*)(i%2)) == -1) {
            printf(1, "clone failed\n");
            exit();
        }
        thread_join(); 
    }
    printf(1, "Counter value: %d\n", counter); 
    
    exit(); 
}

void test14()
{
    struct sequence_t seq, sorted_seq; 
    int k;

    seq.head = 0;
    lock_init(&seq.lk); 
    sorted_seq.head = 0;
    lock_init(&sorted_seq.lk);

    struct list_t *node; 

    printf(1, "Random list: "); 
    for(k=0; k < 20; ++k)
    {
        node = (struct list_t *) malloc(sizeof(struct list_t));
        if(node == 0)
        {
            printf(1, "malloc error\n");
            exit();
        }
        node->num = rand() % 100;
        node->next = seq.head;
        seq.head = node; 
        printf(1, "%d ", node->num); 
    }

    struct list_t *l = seq.head; 

    while(l)
    {
        if(thread_create(fun9, (void *) &sorted_seq, (void *)l->num) == -1) 
        {
            printf(1, "clone failed\n");
            exit(); 
        }
        l=l->next; 
    }

    while(thread_join() != -1)
        ;
    
    l = sorted_seq.head; 
    printf(1, "\nSorted list: "); 
    while(l)
    {
        printf(1, "%d ", l->num);
        if(!member(l->num, seq)) {
            printf(1, "error\n"); 
            exit(); 
        }
        l = l->next;
    }
    printf(1,"\n");
}


void test15()
{
    int i,k; 
    char buf[128];
    
    lock_init(&tlock); 

    for(k=0; k < 25; ++k) 
    {
        for(i=0; i<40; ++i)
        {
            if(thread_create(fun10, (void*)i, (void*)(i%2)) == -1) {
                printf(1, "clone failed\n");
                exit();
            }
        }
        sprintf(buf, "waiting for batch %d of threads to finish\n", k); 
        write(1,buf,strlen(buf));

        while(thread_join() != -1) 
            ;

    }

    printf(1, "Counter value in main process: %d\n", counter); 
    exit(); 
}

void test16()
{
    int i;
    randstate = getpid();

    for(i=0; i < 500; ++i)
        test14();
    exit();

}

static void (*tests[])(void) = {
0,
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
};

int
main(int argc, char *argv[])
{
    int i; 
    randstate = getpid();

    if(argc < 2)
    {
        printf(1, "Usage: %s <num>\nwhere <num> in {1,..,16}\n", argv[0]);
        exit();
    }    

    i = atoi(argv[1]); 
    if(i >= 1 && i <= 16) 
        (*tests[i])();
    else {
        printf(1, "invalid test\n"); 
        exit();
    }

    exit(); 
}
