#include "schedtest.h"

#define NWORKERS 6

int pid[NWORKERS];  
int prev[NWORKERS]; 
int cur[NWORKERS]; 
int tks[NWORKERS]; 

struct procstat_t pstat; 

int check_workers()
{
    int i, j; 
    int fail = 0; 
    int delta = 0; 
    int s = NWORKERS-1; 
    int first_run=0; 

    if(procstat(&pstat) != 0)
        error("syscall procstat failed\n"); 

    if(cur[0] == 0) first_run=1;

    for(i=0; i < NPROC; ++i)
    {
        if(!pstat.inuse[i]) continue;
        for(j=0; j < NWORKERS; ++j) {
            if(pstat.pid[i] == pid[j]) {
                prev[j] = cur[j]; 
                cur[j] = pstat.rt[i];  
                tks[j] = pstat.tickets[i]; 
                break;
            }
        }
    }

    if(first_run) return 0; 

    delta = cur[s] - prev[s]; 

    printf(stdout, "Supervisor (pid=%d, tickets=%d): runtime %d -> %d\n", pid[s], tks[s], prev[s], cur[s]);
    
    for(i=0; i < NWORKERS-1; ++i)
    {
        printf(stdout, "Worker %d (pid=%d, tickets=%d): runtime %d -> %d", i, pid[i], tks[i], prev[i], cur[i]);

        if(cur[i] - prev[i] > delta-MAX_TS/2) {
            printf(stdout, " ** FAIL: supervisor runtime accumulates too slowly ** ");
            fail = 1; 
        }
        printf(stdout, "\n");  
    }
    
    
    return fail; 
}

void init()
{
    int i;
    for(i=0; i < NWORKERS; ++i) {
        pid[i] = 0;
        cur[i] = prev[i] = 0; 
    }
}

void check_tickets()
{
    int i; 
    printf(stdout, "Waiting for workers' tickets to degrade: \n"); 
    while(1)
    {
        if(procstat(&pstat) != 0)
            error("syscall procstat failed\n"); 
        i = get_pstat_idx(&pstat, pid[0]); 
        if(i == -1) error("worker process missing\n"); 

        printf(stdout, " %d ", pstat.tickets[i]); 
        if(pstat.tickets[i] <= 10) 
            break; 
        sleep(NWORKERS * MAX_TS); 
    }
    printf(stdout, "\n");
}

void testdaemon(long int num)
{
    int i; 
    int fail=0; 
    int round; 

    init(); 

    // create a bunch of worker processes and wait for their tickets to decrease
    printf(stdout, "Create workers: pids = ");
    create_workers(pid, NWORKERS-1, num); 
    for(i=0 ; i < NWORKERS - 1; ++i)
        printf(stdout, " %d ", pid[i]); 
    printf(stdout, "\n"); 
    check_tickets(); 

    // create a "supervisor" process that will have more tickets, hence higher priority.
    printf(stdout, "Create supervisor: pid = "); 
    create_workers(pid+NWORKERS-1, 1, num); 
    printf(stdout, " %d \n", pid[NWORKERS-1]); 

    check_workers(); 
    sleep(3 * MAX_TS); 

    for(round=0; round < 5; ++round)
    {
        printf(stdout, "Test %d/5 ...\n", round+1); 
        sleep(3 * MAX_TS); 
        fail+=check_workers();
    }

    killall(pid, NWORKERS);
    if(fail == 0) {
        printf(stdout, "PASSED all tests\n"); 
    }
    else 
        printf(stdout, "FAILED %d out of 5 tests\n", fail); 

    waitall(); 
    printf(stdout, "Press [Enter] to go back to shell "); 

    exit();
}

int main(int argc, char *argv[])
{
    long int num; 

    if(argc < 2)
    {
        printf(1, "Usage: %s <number of iterations>\n", argv[0]);
        exit(); 
    }

    num=atoi(argv[1]); 

    int pid;
    schedlatency(1); // set schedule latency to 1 tick
    pid=fork();
    if(pid == 0) testdaemon(num); 
    else if(pid < 0) error("fork failed\n"); 

    exit(); 
}
