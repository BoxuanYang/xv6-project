
// A1 --
#include "types.h"
#include "user.h"

int main()
{
    long t; 
    if(time(&t) != -1) 
        printf(1, "epoch time right now: %d\n", t); 

    t=0; 
    if(stime(&t) != -1)
        printf(1, "set the epoch time to 0\n");
    t=time((long *)0x80000000); 
    if(t != -1)
        printf(1, "not suppose to happen: %d\n", t); 
    else 
        printf(1, "you are trying to access kernel memory!\n"); 

    exit();
}

// -- A1
