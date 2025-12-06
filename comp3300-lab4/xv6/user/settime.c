// A1 --
#include "types.h"
#include "user.h"

int main(int argc, char *argv[])
{
    long t; 
    if (argc < 2)
    {
        printf(2, "Usage: %s <epoch_time>\n", argv[0]);
        exit();
    }

    t = atoi(argv[1]); 

    if(stime(&t) != -1) 
        printf(1, "Current time set to %d\n", t); 
    else printf(1, "Failed setting time\n"); 

    exit();
}

// -- A1