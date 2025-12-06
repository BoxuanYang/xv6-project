#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"
// #include "ulock.c"

static lock_t malloc_lock;
static int malloc_lock_init_done = 0;
static int malloc_lock_owner = -1;
static int malloc_lock_depth = 0;



// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

void
free(void *ap)
{
  int pid = getpid();

 

  if (!malloc_lock_init_done) {
    lock_init(&malloc_lock);
    malloc_lock_init_done = 1;
  }

  // ==== Add a lock ====
  if (malloc_lock_owner == pid) {
    // current thread already hols the lock
    malloc_lock_depth++;
  } else {
    lock_acquire(&malloc_lock);
    malloc_lock_owner = pid;
    malloc_lock_depth = 1;
  }

  
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr){
    // printf(1, "loop\n");
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  }

  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;

  // ==== release lock ====
  malloc_lock_depth--;
  if (malloc_lock_depth == 0) {
    malloc_lock_owner = -1;
    lock_release(&malloc_lock);
  }
}

void
free_without_lock(void *ap)
{
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}


static Header*
morecore(uint nu)
{
  char *p;
  Header *hp;  

  if(nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
  if(p == (char*)-1)
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
  
  // free -> free_without_lock
  free_without_lock((void*)(hp + 1));
  return freep;
}

void*
malloc(uint nbytes)
{
  int pid = getpid();

  // initialize malloc & free lock
  if (!malloc_lock_init_done) {
    lock_init(&malloc_lock);
    malloc_lock_init_done = 1;
  }

  // Guard malloc with recursive ulock
  if (malloc_lock_owner == pid) {
    malloc_lock_depth++;
  } else {
    lock_acquire(&malloc_lock);
    malloc_lock_owner = pid;
    malloc_lock_depth = 1;
  }
  

  Header *p, *prevp;
  uint nunits;

  

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      
      // ==== release lock ====
      malloc_lock_depth--;
      if (malloc_lock_depth == 0) {
        malloc_lock_owner = -1;
        lock_release(&malloc_lock);
      }
      return (void*)(p + 1);
    }
    if(p == freep)
      if((p = morecore(nunits)) == 0){
        malloc_lock_depth--;
        if (malloc_lock_depth == 0) {
          malloc_lock_owner = -1;
          lock_release(&malloc_lock);
        }
        return 0;
      }
  }
}
