#include "types.h"
#include "user.h"
#include "procinfo.h"
#include "param.h"

int main(int argc, char *argv[])
{
    int max; 
    int n,i; 
    struct procinfo_t p[NPROC]; 

    if(argc < 2) max = NPROC; 
    else max = atoi(argv[1]);

    if(max > NPROC) {
        printf(2, "Number of entries to dispaly must be at most %d\n", NPROC);
        exit();
    }

    n=procinfo(max, p); 
    if(n < 0) {
        printf(1, "Error\n");
        exit();
    }

    printf(1,"PID\tPPID\tState\tName\n");
    for(i=0; i < n; ++i)
    {
        if(p[i].parent)
            printf(1,"%d\t%d\t%s\t%s\n", p[i].pid, p[i].parent->pid, p[i].st, p[i].name);
        else 
            printf(1,"%d\t-\t%s\t%s\n", p[i].pid, p[i].st, p[i].name);
    }
    exit(); 
}

