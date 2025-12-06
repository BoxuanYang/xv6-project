#include "types.h"
#include "stat.h"
#include "user.h"
#include "syscall.h"

int
main(int argc, char *argv[])
{
    int mask; 
    int pid; 

    if (argc < 3)
    {
        printf(2, "Usage: %s trace_mask command\n", argv[0]);
        exit();
    }

    mask = atoi(argv[1]); 
    if(trace(mask) < 0) {
        printf(1,"Error: sys_trace failed\n");
        exit(); 
    };
    
    pid = fork();

    if(pid == 0)
    {
        exec(argv[2], argv+2); 
        exit();
    }
    else if(pid > 0) {
        wait();
    }
    exit();
}