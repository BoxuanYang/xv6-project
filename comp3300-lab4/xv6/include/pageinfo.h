#ifndef _PAGEINFO_H
#define _PAGEINFO_H

struct pageinfo_t {
    unsigned int va; // virtual address
    int mapped;
    unsigned int pa; // physical address
    int flags;  // permission bits
}; 

#endif
