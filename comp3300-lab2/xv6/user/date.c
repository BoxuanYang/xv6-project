// A1 --
#include "types.h"
#include "user.h"

int main()
{
    long t; 
    if(time(&t) != -1) 
        printf(1, "%d\n", t); 
    exit();
}

// -- A1