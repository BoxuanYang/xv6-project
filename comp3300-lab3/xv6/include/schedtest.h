// A2 --

#ifndef _SCHEDTEST_
#define _SCHEDTEST_
#include "types.h"
#include "user.h"
#include "procstat.h"

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
#define NPROC 64
#define MIN_TICKETS  10
#define MAX_TICKETS 100
#define MAX_TS  100   // For adaptive lottery scheduler; update the ticket every MAX_TS ticks
#define PENALTY   5

int stdout=1; 

// turn off compiler optimisation for this function, as we want to
// use this as an artificial benchmark 
int __attribute__((optimize("O0"))) compute(int id, long int num)
{
    int i, j, k;
    int res=1;

    for(i=1; i < num; ++i)
        for(j=1; j < num; ++j)
            for(k=1; k < num; ++k)
                res = (res * i) % 1000 + (j*id) % 100 + k%10; 
    return res; 
}

void create_workers(int pid[], int nw, long int num)
{
    int i; 
    for(i=0; i < nw; ++i)
    {
        pid[i] = fork();
        if(pid[i] == 0) {
            compute(getpid(), num); 
            exit(); 
        }
        else if(pid[i] < 0) {
            printf(stdout,"Error\n"); 
            exit();
        }
    }
}

// wait for all children to terminate
void waitall()
{
    while(wait() != -1) ; 
}

void error(char *msg)
{
    printf(stdout, "Error: %s\n", msg);
    exit();
}

void killall(int pid[], int n)
{
    int i;

    printf(1, "Killing all subprocesses.."); 
    for(i=0; i < n; ++i)
    {
        kill(pid[i]); 
    }
    while(wait() != -1) ; 
    printf(1, "done\n"); 
}

int get_pstat_idx(struct procstat_t *pstat, int pid)
{
    int i; 

    for(i=0; i < NPROC; ++i)
        if(pstat->inuse[i] && pstat->pid[i] == pid) 
            return i;
    return -1;
}

#endif

// -- A2
