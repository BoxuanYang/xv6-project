// A1 -- 
#ifndef PROCINFO_H
#define PROCINFO_H

struct procinfo_t {
    int pid;        // process id
    char name[16];  // process name
    char st[3];     // process state, as a string.
                    // "UN" (UNUSED), "EM" (EMBRYO), "S" (SLEEPING)
                    // "RE" (RUNNABLE), "RN" (RUNNING), "Z" (ZOMBIE)
    long start_time;     // The process start time
    long elapsed_time; // How long the process has been running
}; 

#endif

// -- A1

