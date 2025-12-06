#include "types.h"
#include "user.h"
#include "procstat.h"

struct procstat_t pstat; 

int main(int argc, char *argv[])
{
    int i; 
    char *st[] = {"UN", "E", "S", "RE", "RN", "Z"}; 

    printf(1,"PID\tState\tTickets\tRuntime\tDelay\n"); 

    if(procstat(&pstat) == 0)
    {
        for(i=0; i < NPROC; ++i)
        {
            if(!pstat.inuse[i]) continue;
            printf(1, "%d\t%s\t%d\t%d\t%d\n", 
                    pstat.pid[i], st[pstat.state[i]], pstat.tickets[i], pstat.rt[i],
                    pstat.delay[i]); 
        }
    }
    exit(); 
}
