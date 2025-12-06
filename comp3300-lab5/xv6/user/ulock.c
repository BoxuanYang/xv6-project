#include "types.h"
#include "user.h"
#include "x86.h"

void lock_init(lock_t * lock) { 
    lock->locked = 0;
}

void lock_acquire(lock_t * lock) {  
    while(xchg(&lock->locked, 1) != 0);
}

void lock_release(lock_t* lock) { 
  asm volatile("movl $0, %0" : "+m" (lock->locked) : );
}

