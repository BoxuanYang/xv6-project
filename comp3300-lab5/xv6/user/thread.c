#include "types.h"
#include "user.h"

#define PGSIZE 4096

struct stack_map{
  void *aligned_stack;
  void *raw_stack;
  struct stack_map *next;
};

static lock_t lock;
static int lock_initialized = 0;

static struct stack_map *head = 0;

void add_stack_map(void *raw, void *aligned){
  struct stack_map *m = malloc(sizeof(struct stack_map));

  m->raw_stack = raw;
  m->aligned_stack = aligned;


  m->next = head;
  head = m;
}


int thread_create(void (*start_routine)(void*,void*), void *arg1, void*arg2)
{
  // TODO: implement this for Assignment 4

  // Initialize the lock if not yet
  if(lock_initialized == 0){
    lock_init(&lock);
    lock_initialized = 1;
  }

  // allocate memory of 2 * PGSIZE
  lock_acquire(&lock);
  void *raw = malloc(2 * PGSIZE);
  //printf(1, "alloacted *raw: 0x%p\n", raw);
  lock_release(&lock);

  // Get page-aligned stack
  uint aligned_addr = ((uint) raw + PGSIZE - 1) & ~(PGSIZE - 1);
  void *aligned_stack = (void *)aligned_addr;

  int pid = clone(start_routine, arg1, arg2, aligned_stack);
  if (pid < 0) {
    free(raw);
    return -1;
  }

  // Record raw stack - aligned stack mapping
  lock_acquire(&lock);
  add_stack_map(raw, aligned_stack);
  lock_release(&lock);

  return pid; 
}

int thread_join()
{
  // TODO: implement this for Assignment 4
  void *stack_addr = 0;
  int pid = join(&stack_addr);

  if (pid < 0) {
    return -1;
  }

  // Look for the raw addr in our mapping
  lock_acquire(&lock);
  struct stack_map *prev = 0;
  struct stack_map *cur = head;
  struct stack_map *found = 0;
  while (cur){
    // Got the mapping, remove from linked list
    if (cur->aligned_stack == stack_addr){
      if (prev)
        prev->next = cur->next;
      else
        head = cur->next;

      found = cur;
      break;
    }

    prev = cur;
    cur = cur->next;
  }
  lock_release(&lock);

  // free the stack
  
  void *raw = found->raw_stack;

  lock_acquire(&lock);
  free(raw);
  free(found);
  lock_release(&lock);

  


  return pid; 
}
