#ifndef PROCSTAT_H
#define PROCSTAT_H

#include "param.h"

struct procstat_t {
    int inuse[NPROC]; 
    int pid[NPROC]; 
    int delay[NPROC];
    int tickets[NPROC]; 
    unsigned int rt[NPROC]; 
    int state[NPROC]; 
}; 

#endif

