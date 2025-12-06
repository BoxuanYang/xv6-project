
// A1 --

#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

int stdout = 1;


void error(char *msg)
{
    trace(0);
    printf(stdout, "Error: %s\n", msg);
    exit();
}

void mytrace(int mask)
{
    if(trace(mask) < 0) {
        printf(stdout, "Error: trace failed\n");
        exit();
    }
}

void
opentest1(void)
{
    int fd;

    fd = open("echo", 0);
    if(fd < 0){
        error("open echo failed!\n");
    }
    close(fd);
    fd = open("doesnotexist", 0);
    if(fd >= 0){
        error("open doesnotexist succeeded!\n");
    }

}

void
opentest2(int mode)
{
    int i, fd[10];
    char name[3];

    name[0] = 'a';
    name[2] = '\0';
    for(i = 0; i < 10; i++) {
        name[1] = '0' + i;
        fd[i] = open(name, mode);
    }

    for(i = 0; i < 10; i++) {
        close(fd[i]);
    }

    name[0] = 'a';
    name[2] = '\0';
    for(i = 0; i < 52; i++) {
        name[1] = '0' + i;
        unlink(name);
    }

}

void
writetest(char *str1, char *str2, char *buf)
{
    int fd;
    int i;


    fd = open("small", O_CREATE|O_RDWR);
    if(fd < 0) {
        error("creat small failed!\n");
    }

    if(write(fd, str1, 10) != 10)
        error("write str1 to new file failed\n");
    
    if(write(fd, str2, 10) != 10)
        error("write str2 to new file failed\n");

    close(fd);

    fd = open("small", O_RDONLY);
    if(fd < 0)
        error("open small failed!\n");
    
    i = read(fd, buf, 20);
    if(i != 20) error("read failed\n");
    
    close(fd);

    if(unlink("small") < 0){
        error("unlink small failed\n");
    }
}

void exectest1(char *cmd, char *argv[])
{
    int pid; 

    pid=fork();

    if(pid == 0) 
    {
        close(stdout);
        if(exec(cmd, argv) < 0)
            error("exec failed\n");
    }
    else if(pid > 0)
    {
        wait();
    }
    else {
        error("fork failed\n");
    }

}

void exectest2(char *cmd1, char *argv1[], char *cmd2, char *argv2[])
{
    int pid; 

    pid=fork(); 
    if(pid > 0)
    {
        pid=fork();
        if(pid == 0) {
            close(stdout); 
            exec(cmd1, argv1);
        }
        else if(pid < 0) 
            error("fork failed"); 

        wait(); 
    }
    else if(pid==0) {
        close(stdout); 
        exec(cmd2, argv2); 
    }
    else error("fork failed"); 
    wait();
}

// simple open and close tests
void test1(void)
{
    int mask = (1 << SYS_open) | (1 << SYS_close);

    printf(stdout, "\ntest 1\n");
    printf(stdout, "==================\n"); 
    printf(stdout, 
        "Description: \n"
        "opening and closing an existing file ('echo')\n" 
        "and a non-existing file ('doesnotexist') \n"); 
    
    mytrace(mask);
    opentest1(); 
    mytrace(0); 
    printf(stdout, "==================\n"); 
}

// testing creating a new file in O_CREATE|O_RDWR mode
void test2(void)
{
    int mask = (1 << SYS_open) | (1 << SYS_close);
    int mode = O_CREATE|O_RDWR; 

    printf(stdout, "\ntest 2\n");
    printf(stdout, "==================\n"); 
    printf(stdout, 
        "Description: \n"
        "creating 10 new files, named a0 to a11, with mode=514 (O_CREATE|O_RDWR)\n");

    mytrace(mask);
    opentest2(mode); 
    mytrace(0); 
    printf(stdout, "==================\n"); 
}

void test3(void)
{
    char *str1 = "aaaaaaaaaa";
    char *str2 = "bbbbbbbbbb";
    char buf[32]; 
    int mask; 

    mask  = (1 << SYS_open);
    mask |= (1 << SYS_close); 
    mask |= (1 << SYS_read); 
    mask |= (1 << SYS_write);

    printf(stdout, "\ntest 3\n");
    printf(stdout, "=======================\n"); 
    printf(stdout, 
        "Description:\n"
        "Writing two strings (str1, str2) to a file, and read the file to a buffer (buf).\n");

    printf(stdout, "str1 address: 0x%x\n", str1);
    printf(stdout, "str2 address: 0x%x\n", str2);
    printf(stdout, "buf  address: 0x%x\n", buf); 
    
    mytrace(mask); 
    writetest(str1, str2, buf); 
    mytrace(0);  
    printf(stdout, "=======================\n"); 

}

void test4(void)
{    
    char *cmd="echo";
    char *argv[] = {"echo", "one", "two", "three",  0}; 

    printf(stdout, "\ntest 4\n");
    printf(stdout, "=======================\n"); 
    printf(stdout, 
        "Description:\n"
        "exec an echo command ('echo one two three') in a child process.\n");

    mytrace((1 << SYS_exec) | (1 << SYS_fork) | (1 << SYS_wait)); 
    exectest1(cmd, argv); 
    mytrace(0); 
    printf(stdout, "=======================\n"); 
}

void test5(void)
{
    char *cmd="wc";
    char *argv[] = {"wc", "README", 0}; 
    int mask; 

    mask  = 1 << SYS_fork;
    mask |= 1 << SYS_exec;
    mask |= 1 << SYS_wait; 
    mask |= 1 << SYS_open;
    mask |= 1 << SYS_close;
    mask |= 1 << SYS_read;
    mask |= 1 << SYS_write; 

    printf(stdout, "\ntest 5\n");
    printf(stdout, "=======================\n"); 
    printf(stdout, 
        "Description:\n"
        "exec a command ('wc README') in a child process,\n"
        "and tracing open/close/read/write"
    );

    mytrace(mask); 
    exectest1(cmd, argv); 
    mytrace(0); 
    printf(stdout, "=======================\n"); 

}

void test6(void)
{
    char *cmd1="echo";
    char *argv1[] = {"echo", "first", 0}; 
    char *cmd2="badexec";
    char *argv2[] = {"badexec", "second", 0}; 

    int mask; 

    mask  = 1 << SYS_fork;
    mask |= 1 << SYS_exec;
    mask |= 1 << SYS_wait; 

    printf(stdout, "\ntest 6\n");
    printf(stdout, "=======================\n"); 
    printf(stdout, 
        "Description:\n"
        "exec two commands ('echo first' and 'badexec second') in child processes\n"); 

    mytrace(mask); 
    exectest2(cmd1, argv1, cmd2, argv2); 
    mytrace(0); 
    printf(stdout, "=======================\n"); 

}

static void (*tests[])(void) = {
0,
test1,
test2,
test3,
test4,
test5,
test6
};

int
main(int argc, char *argv[])
{
    int i,j; 

    if(argc < 2)
    {
        printf(stdout, "Running all tests...\n");
        for(i=1; i <= 6; ++i) 
            (*tests[i])();
        exit();
    }    

    for(i=1; i < argc; ++i)
    {
        j = atoi(argv[i]); 
        if(j >= 1 && j <= 6) {
            (*tests[j])(); 
        }
        else error("invalid test"); 
    }
    exit(); 
}

// -- A1
