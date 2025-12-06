#include "types.h"
#include "user.h"
#include "procstat.h"

#define MAX_NPROC 10

int nproc = 0; 
int pid[MAX_NPROC];  
int stdout=1; 

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

// turn off compiler optimisation for this function, as we want to
// use this as an artificial benchmark 
int __attribute__((optimize("O0"))) compute(int id, long int num)
{
    char buf[128]; 
    int i, j, k;
    int res=1;

    for(i=1; i < num; ++i)
        for(j=1; j < num; ++j)
            for(k=1; k < num; ++k)
                res = (res * i) % 1000 + (j*id) % 100 + k%10; 
    // do something with the result, 
    // so the above loop doesn't get optimised away by the compiler
    sprintf(buf, "%d", res); 
    sprintf(buf, "\nProcess %d completed\n", id); 
    write(stdout, buf, strlen(buf)); 
    exit(); 
}

void printstat(void)
{
    struct procstat_t stat; 
    int i,j; 

    printf(1,"Name\t\tPID\tTickets\tRuntime\tDelay\n"); 

    if(procstat(&stat) == 0)
    {
        for(i=0; i < NPROC; ++i)
        {
            if(!stat.inuse[i]) continue;
            for(j=0; j < nproc; ++j)
            {
                if(stat.pid[i] == pid[j]) {
                    printf(1, "Worker %d\t%d\t%d\t%d\t%d\n", 
                            j, pid[j], stat.tickets[i], stat.rt[i],
                            stat.delay[i]); 
                    break; 
                }                
            }
        }
    }
}

void killall(void)
{
    int i;

    printf(1, "Killing all subprocesses.."); 
    for(i=0; i < nproc; ++i)
    {
        kill(pid[i]); 
    }
    while(wait() != -1) ; 
    printf(1, "done\n"); 
}

void testdaemon(long int num)
{
    int i; 
    char print_buffer[128];
    char buf[8]; 

    for(i=0; i < nproc; ++i)
    {
        pid[i] = fork();
        if(pid[i] == 0) {
            compute(getpid(), num); 
        }
        else if(pid[i] < 0) {
            printf(1,"Error\n"); 
            exit();
        }
    }

    sprintf(print_buffer, "PIDs created: "); 
    for(i=0; i < nproc; ++i)
    {
        sprintf(buf, " %d ", pid[i]); 
        strcat(print_buffer, buf); 
    }
    strcat(print_buffer, "\n"); 
    write(stdout, print_buffer, strlen(print_buffer)); 
    waitall();
    exit();
}

int main(int argc, char *argv[])
{
    long int num; 

    if(argc < 3)
    {
        printf(1, "Usage: %s <number of workers> <number of iterations>\n", argv[0]);
        exit(); 
    }

    nproc=atoi(argv[1]);
    num=atoi(argv[2]); 

    if(nproc < 1 || nproc > MAX_NPROC) {
        printf(1, "Number of workers must be in [1,%d]\n", MAX_NPROC); 
        exit(); 
    }

    int pid;

    pid=fork();
    if(pid == 0) testdaemon(num); 
    else if(pid < 0) error("fork failed\n"); 

    exit(); 
}
