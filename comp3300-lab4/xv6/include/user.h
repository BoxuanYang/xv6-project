struct stat;
struct rtcdate;
struct procinfo_t;
struct procstat_t; 
struct pageinfo_t; 

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);

// A1 -- 
// new syscalls
int halt(void); 
int time(long *); 
int stime(long *); 
int trace(int); 
int procinfo(int, struct procinfo_t *); 
//  -- A1

// A2 --
int setpriority(int,int); 
int procstat(struct procstat_t *);
int schedlatency(int); 
// -- A2

// A3 --
int mprotect(void*, int);
int munprotect(void*, int); 
void* mmap(void *addr, int length, int prot); 
int munmap(void *addr, int length); 
int pageinfo(void *addr, struct pageinfo_t *pi); 
// -- A3

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
void sprintf(char *, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
char * strcat(char *, const char *);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
int strtol(const char *nptr, char **endptr, int base);
