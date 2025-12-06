#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int n;

  if(argc < 2){
    printf(2, "usage: schedlatency <number>\n");
    exit();
  }
  n = atoi(argv[1]); 
  if(schedlatency(n) == -1)
    printf(2, "failed setting scheduler latency\n");
  else printf(2, "scheduler latency set to %d ticks\n", n); 
  
  exit();
}
