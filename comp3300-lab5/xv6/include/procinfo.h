#ifndef PROCINFO_H
#define PROCINFO_H

struct procinfo_t {
    int pid; 
    char name[16];
    char st[3];     // process state, as a string.
                    // "UN" (UNUSED), "EM" (EMBRYO), "S" (SLEEPING)
                    // "RE" (RUNNABLE), "RN" (RUNNING), "Z" (ZOMBIE)
    struct procinfo_t * parent; 
}; 

#endif
