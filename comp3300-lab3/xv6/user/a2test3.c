// #include "types.h"
// #include "user.h"
// #include "procstat.h"
#include "schedtest.h"

#define NWORKERS 50
#define BATCH_SIZE 5

// Maximum delay -- each new process should be scheduled within 10 ticks
#define MAX_DELAY 10

int pid[NWORKERS];  
int cur[NWORKERS]; 

struct procstat_t pstat; 

int check_workers(int start, int end)
{
    int i, j; 
    int fail = 0; 

    if(procstat(&pstat) != 0)
        error("syscall procstat failed\n"); 


    for(i=0; i < NPROC; ++i)
    {
        if(!pstat.inuse[i]) continue;
        for(j=start; j < end; ++j) {
            if(pstat.pid[i] == pid[j]) {
                cur[j] = pstat.delay[i];  
                break;
            }
        }
    }

    for(i=start; i < end; ++i)
    {
        printf(stdout, "Worker %d (pid=%d): delay = %d", i, pid[i], cur[i]);
        if(cur[i] < 0) {
            printf(stdout, " ** FAIL: Invalid delay ** "); 
            fail=1;
        }
        if(cur[i] > MAX_DELAY) {
            printf(stdout, " ** FAIL: Delay too long **");
            fail=1; 
        }
        printf(stdout,"\n"); 

    }

    return fail; 
}


void testdaemon(long int num)
{
    int i; 
    int fail=0; 
    int round; 
    int start, end;

    for(round=0; round < 10; ++round)
    {
        printf(stdout, "\nTest %d/10 ...\n", round+1); 
        start = round*BATCH_SIZE;  
        end = start + 5; 
        printf(stdout, "Create workers: pids = ");
        create_workers(pid + start, BATCH_SIZE, num); 

        for(i=start ; i < end; ++i)
        {
            printf(stdout, " %d ", pid[i]); 
        }
        printf(stdout, "\n"); 
        sleep(2 * BATCH_SIZE * MAX_TS); 
        fail+=check_workers(start, end);
    }

    killall(pid, end);
    if(fail == 0) {
        printf(stdout, "PASSED all tests\n"); 
    }
    else 
        printf(stdout, "FAILED %d out of %d tests\n", fail, round); 

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

    schedlatency(1); // set schedule latency to 1 tick
    
    int pid;

    pid=fork();
    if(pid == 0) testdaemon(num); 
    else if(pid < 0) error("fork failed\n"); 

    exit(); 
}
