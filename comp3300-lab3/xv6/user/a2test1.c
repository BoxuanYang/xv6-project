#include "schedtest.h"

#define NWORKERS 50

int pid[NWORKERS];  
int pos[NWORKERS]; 

int nw;
struct procstat_t pstat[2]; 
int cur_stat;


int check_workers(int s, int z)
{
    int i; 
    int fail = 0; 
    int prev_stat=cur_stat; 
    cur_stat = (cur_stat+1) % 2; 
    struct procstat_t *cur = &pstat[cur_stat]; 
    struct procstat_t *prev = &pstat[prev_stat]; 

    if(procstat(cur) != 0)
        error("syscall procstat failed\n"); 

    for(i=0; i < nw; ++i)
    {
        printf(stdout, "Worker %d (pid=%d): %d -> %d", i, 
                pid[i], prev->tickets[pos[i]], 
                cur->tickets[pos[i]]);

        // tickets within the correct range
        if(cur->tickets[pos[i]] < MIN_TICKETS || cur->tickets[pos[i]] > MAX_TICKETS) {
            printf(stdout, " ** FAIL: Invalid tickets ** "); 
            fail=1;  
        } 
        // tickets don't go below minimum
        else if(prev->tickets[pos[i]] == MIN_TICKETS && cur->tickets[pos[i]] != MIN_TICKETS) {
            printf(stdout, " ** FAIL: MIN_TICKETS updated **");
            fail=1; 
        }
        // sleeping process or zombie process shouldn't have tickets updated
        else if(i == s || i == z ) { 
                
            if( (prev->state[pos[i]] == SLEEPING || prev->state[pos[i]] == ZOMBIE) 
               && (cur->tickets[pos[i]] != prev->tickets[pos[i]])) {
                printf(stdout, " ** FAIL: sleeper/zombie tickets updated **"); 
                fail=1; 
            }
        }
        // tickets for runnable processes should decrease overtime
        else if(cur->tickets[pos[i]] > MIN_TICKETS && prev->tickets[pos[i]] - cur->tickets[pos[i]] < PENALTY) {
            printf(stdout, " ** FAIL: Tickets not updated ** ");
            fail=1;
        }
        printf(stdout,"\n"); 
    }
    return fail; 
}

void init()
{
    int i;

    cur_stat = 0; 
    nw = 0;
    for(i=0; i < NWORKERS; ++i) {
        pid[i] = 0;
        pos[i] = -1;
    }
}

int create_zombie()
{
    int z; 

    z=fork(); 
    if(z == 0)
    {
        exit(); 
    }
    else if(z < 0) error("Failed creating processes\n"); 

    return z; 
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

int create_sleeper()
{
    int s; 

    s=fork(); 
    if(s == 0)
    {
        char *argv[] = {"cat", 0}; 
        run(argv[0], argv); 
    }
    else if(s < 0) error("Failed creating processes\n"); 

    return s; 
}

void start_measurement(void)
{
    int i, j; 

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
    nw = 7; 
    create_workers(pid, 5, num); 
    printf(stdout, "Create workers: pids = "); 
    for(i=0; i < 5; ++i)
    {
        printf(stdout, " %d ", pid[i]); 
    }
    printf(stdout, "\n"); 

    pid[5] = create_sleeper(); 
    printf(stdout, "Create sleeper: pid = %d\n", pid[5]); 
    pid[6] = create_zombie(); 
    printf(stdout, "Create zombie: pid = %d\n", pid[6]); 

    sleep(2 * nw * MAX_TS); // give the child processes a chance to run before starting measurement
    start_measurement(); 

    for(i=0; i < 10; ++i)
    {
        printf(stdout, "\nTest %d/10 ...\n", i+1); 
        sleep(2 * nw * MAX_TS); 
        fail+=check_workers(5, 6);
    }

    killall(pid, nw);
    if(fail == 0) {
        printf(stdout, "PASSED all tests\n"); 
    }
    else 
        printf(stdout, "FAILED %d out of 10 tests\n", fail); 

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
