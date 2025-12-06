#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    int sleep_time = atoi(argv[1]);
    printf(1, "sleep for: %d\n", sleep_time);
    sleep(sleep_time);
    exit();
}
