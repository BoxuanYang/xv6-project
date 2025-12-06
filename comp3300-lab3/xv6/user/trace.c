// A1 --

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
    int fd[2];
    pipe(fd); 

    if(pid == 0)
    {
        // Suppress the output of the child process. 
        close(fd[0]); // close the read end
        close(1); 
        dup(fd[1]);   // redirect standard output to a pipe that leads nowhere    
        exec(argv[2], argv+2); 
        exit();
    }
    else if(pid > 0) {
        wait();
    }
    exit();
}

// -- A1
