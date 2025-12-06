
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define NTESTS 6

char *argv1[] = {"cat", 0};
char *argv2[] = {"wc", 0};
char *argv3[] = {"grep", "a", 0};
char *argv4[] = {"echo", "hello world", 0};

int stdout = 1;

void error(char *msg)
{
    printf(stdout, "Error: %s\n", msg);
    exit();
}

// wait for all children to terminate
void waitall()
{
    while(wait() != -1) ; 
}

void pid2str(int pid, char str[])
{
    if(pid >= 10) str[0] = '0' + pid/10; 
    else str[0] = ' '; 
    str[1] = '0' + pid%10; 
}

// a very hacky way to print logs. 
// use write() instead of printf().
// write() ensures atomic write of 512 chunks. 
void log(int ppid, int pid)
{
    char str[] = "fork: xx -> yy\n"; 
    pid2str(ppid, str+6);
    pid2str(pid, str+12); 
    write(stdout, str, 15); 
}

void run(char *cmd, char *argv[])
{
    int fd1[2];
    int fd2[2]; 

    if(pipe(fd1) < 0) error("pipe failed\n");
    if(pipe(fd2) < 0) error("pipe failed\n");

    // re-route standard input/output to pipes that are never read/written to,
    // to keep the command suspended waiting for I/O without contending with 
    // other processes for I/O.
    close(0);
    close(1);
    dup(fd1[0]);
    dup(fd2[1]); 

    if(exec(cmd, argv) < 0) error("exec failed\n");
}

void forkrun(char *cmd, char *argv[])
{
    int pid; 
    int ppid = getpid(); 
    pid=fork(); 
    if(pid == 0) run(cmd, argv); 
    else if(pid < 0) error("fork failed\n");
    
    log(ppid, pid); 
}

// exec three commands
void test1(void)
{
    forkrun(argv1[0], argv1); 
    forkrun(argv2[0], argv2); 
    forkrun(argv3[0], argv3); 
}

// create a zombie
void test2(void)
{
    int pid; 
    int ppid = getpid(); 

    pid=fork();
    if(pid == 0)
    {
        log(ppid, getpid()); 
        forkrun(argv4[0], argv4); 
        sleep(10000); 
        exit();
    }
    if(pid < 0) error("fork failed\n");
}


// create a zombie, and an infinite loop in the parent process
// this will show that the parent process is in a runnable state.
void test3(void)
{
    int pid; 
    int ppid=getpid(); 

    pid=fork();
    if(pid == 0)
    {
        log(ppid, getpid()); 
        forkrun(argv4[0], argv4);
        while(1);  // infinite loop; to keep the runnable state
    }
    if(pid < 0) error("fork failed\n");
}


// zombie apocalyse!

void test4(void)
{
    int pid[10]; 
    int i; 
    int ppid = getpid(); 

    for(i=0; i < 10; ++i) {
        pid[i]=fork();
        if(pid[i] == 0) exit();           
        if(pid[i] < 0) error("fork failed\n");
    }

    for(i=0; i < 10; ++i)
        log(ppid, pid[i]); 
    sleep(100000); 
}

// exec four commands, two nested under fork() 
void test5(void)
{
    int pid; 
    int ppid = getpid(); 

    pid=fork();
    if(pid == 0)
    {
        log(ppid, getpid()); 
        forkrun(argv1[0], argv1); 
        forkrun(argv2[0], argv2); 
        waitall(); 
        exit();
    }
    else if(pid < 0) error("fork failed\n");

    forkrun(argv3[0], argv3); 
}

// nested forks and execs
void test6(void)
{
    int pid; 
    int ppid = getpid(); 

    pid=fork();
    if(pid == 0)
    {
        pid = getpid();
        log(ppid, pid);
        ppid = pid; 
        pid=fork(); 
        if(pid==0) {
            test5();
            waitall(); 
        }
        else if(pid < 0) error("fork failed\n");
        
        log(ppid,pid); 

        pid=fork(); 
        if(pid==0) {
            test5();
            waitall();
        }
        else if(pid < 0) error("fork failed\n");

        log(ppid, pid); 
        waitall(); 
        exit();
    }

    if(pid < 0) error("fork failed\n");

    log(ppid, pid); 

    pid=fork(); 
    if(pid == 0)
    {
        test3(); 
        waitall(); 
        exit();
    }
    else if(pid < 0) error("fork failed\n");

    log(ppid, pid); 

    pid = fork();
    if(pid == 0)
        while(1);
    if(pid < 0) error("fork failed\n");

    log(ppid, pid); 
}

static void (*tests[])(void) = {
0,
test1,
test2,
test3,
test4,
test5,
test6,
};

void testdaemon(int n)
{
    write(stdout, "Testing procinfo syscall\n", 25); 
    (*tests[n])(); 
    sleep(100000); 
    waitall();
    exit();
}

int
main(int argc, char *argv[])
{
    int i; 

    if(argc < 2)
    {
        printf(stdout, "Usage: %s <test_number>\n", argv[0]);
        exit();
    }    

    i = atoi(argv[1]); 
    if(i >= 1 && i <= NTESTS) {
        int pid;

        pid=fork();
        if(pid == 0) testdaemon(i); 
        else if(pid < 0) error("fork failed\n"); 
    }
    else error("invalid test\n"); 
       
    exit(); 

}

