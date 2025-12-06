// #include "types.h"
// #include "user.h"
// #include "procstat.h"
#include "schedtest.h"

#define NWORKERS 50

int nw; 
int pid[NWORKERS];  
int pos[NWORKERS]; 

struct procstat_t pstat[2]; 
int cur_stat; 

int check_workers()
{
    int i, j; 
    int fail = 0; 
    int min = 0x7fffffff, max=0; 

    int prev_stat=cur_stat; 
    cur_stat = (cur_stat+1) % 2; 
    struct procstat_t *cur = &pstat[cur_stat]; 
    struct procstat_t *prev = &pstat[prev_stat]; 

    if(procstat(cur) != 0)
        error("syscall procstat failed\n"); 

    for(j=0; j < nw; ++j) {
        if(pos[j] != -1) {
            max = (cur->rt[pos[j]] > max)? cur->rt[pos[j]] : max;
            min = (cur->rt[pos[j]] < min)? cur->rt[pos[j]] : min;  
        }
    }
    

    for(i=0; i < nw; ++i)
    {
        printf(stdout, "Worker %d (pid=%d): %d -> %d", i, pid[i], prev->tickets[pos[i]], cur->tickets[pos[i]]);
        printf(stdout,"\n"); 
    }
    printf(stdout, "Delta = %d", max - min); 

    if(max - min > MAX_TS) {
        printf(stdout, " ** FAIL: Delta exceeds MAX_TS ** ");
        fail = 1; 
    }
    printf(stdout, "\n"); 

    return fail; 
}

void init()
{
    int i;
    nw = 0; 
    cur_stat = 0; 

    for(i=0; i < NWORKERS; ++i) {
        pid[i] = 0;
        pos[i] = -1;
    }
}

void start_measurement(void)
{
    int i,j;
    struct procstat_t * cur = &pstat[cur_stat]; 

    // query current process stats
    if(procstat(cur) != 0)
        error("syscall procstat failed\n"); 

    // maps worker to entries in process table
    for(i=0; i < NPROC; ++i)
    {
        if(!cur->inuse[i]) continue;

        for(j=0; j < nw; ++j) {
            if(cur->pid[i] == pid[j]) {
                pos[j] = i; 
                break;
            }
        }
    }
    for(j=0; j < nw; ++j) {
        // initially all processes must have maximum number of tickets
        if(cur->tickets[pos[j]] <= MIN_TICKETS) {
            killall(pid, nw); 
            error("Initial tickets too low; each process must be assigned MAX_TICKETS when run for the first time.\n");
        } 
    }

}

void testdaemon(long int num)
{
    int i; 
    int fail=0; 

    init(); 
    nw = 10; 
    create_workers(pid, nw, num); 
    printf(stdout, "Create workers: pids = "); 
    for(i=0; i < nw; ++i)
    {
        printf(stdout, " %d ", pid[i]); 
    }
    printf(stdout, "\n"); 

    sleep(2 * nw * MAX_TS); // give the child processes a chance to run before starting measurement
    start_measurement();

    for(i=0; i < 5; ++i)
    {
        printf(stdout, "\nTest %d/5 ...\n", i+1); 
        sleep(nw * MAX_TS); 
        fail+=check_workers();
    }
    killall(pid, nw);
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
