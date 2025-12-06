#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int pid, n;

  if(argc < 3){
    printf(2, "usage: %s <pid> <tickets>\n", argv[0]);
    exit();
  }
  pid = atoi(argv[1]);
  n = atoi(argv[2]); 

  if(setpriority(pid, n) == -1)
    printf(2, "Failed setting process priority\n");
  else printf(2, "Process %d's tickets set to %d\n", pid, n); 
  
  exit();
}
