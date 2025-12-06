#include "types.h"
#include "stat.h"
#include "user.h"

// the optimize("O0") attibute tells the compiler not to optmise this code. 
// Some compiler will by default apply an optimisation that replaces any
// null pointer dereference with an 'undefined' instruction, e.g., ud2. 
// This could generate an 'illegal operation' exception, which is not what
// we want to trigger with this example. 
int __attribute__((optimize("O0"))) 
main()
{
    int *x; 
    printf(1, "About to dereference a null pointer (should trigger an exception)...\n");
    x=(int *)0; 
    printf(1, "Error: dereferenced null pointer successfully, x=%d\n", *x); 
    exit();
}
