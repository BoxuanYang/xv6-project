#include "types.h"
#include "user.h"

#define MAX_NPROC 10

int compute(int id, long int num)
{
    char buf[128]; 
    int i, j, k;
    uint start, end;
    int res=1;

    start = uptime(); 
    for(i=1; i < num; ++i)
        for(j=1; j < num; ++j)
            for(k=1; k < num; ++k)
                res = (res * i) % 1000 + (j*id) % 100 + k%10; 

    end = uptime(); 

    // 
    sprintf(buf, "[Worker %d] Result: %d, time elapsed: %d\n", id, res, end-start); 
    write(1, buf, strlen(buf)); 
    exit(); 
}

int main(int argc, char *argv[])
{
    int pid[MAX_NPROC];  
    int i; 
    long int num; 
    int nproc; 

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

    for(i=0; i < nproc; ++i)
    {
        pid[i] = fork();
        if(pid[i] == 0) {
            // uncomment this line to test the setpriority syscall
            // setpriority((i+1)*10);   
            compute(i, num); 
        }
        else if(pid[i] < 0) {
            printf(1,"Error\n"); 
            exit();
        }
    }

    while(wait() != -1) ; 
    exit();  
}