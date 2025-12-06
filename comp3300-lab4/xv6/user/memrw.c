#include "types.h"
#include "stat.h"
#include "user.h"

void testread(int * addr)
{
    int val = *addr;
    printf(1, "Value at address 0x%p: 0x%x\n", addr, val);
}

void testwrite(int * addr, int val)
{
    *addr = val;
    printf(1, "Wrote 0x%x to address 0x%p\n", val, addr);
}

int __attribute__((optimize("O0"))) 
main(int argc, char *argv[])
{
    // input an address to read from and write to
    int addr;
    char buf[32];
    printf(1, "Enter an address to read from and write to (in hex, e.g., 0x0011aabb): ");
    gets(buf, 16);
    addr = (int)strtol(buf, 0, 16);
    testread((int *)addr);
    testwrite((int *)addr, 0xdeadbeef);
    testread((int *)addr);
    exit();
}