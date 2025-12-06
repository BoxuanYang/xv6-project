#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"


// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  if(addr >= curproc->sz || addr+4 > curproc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;
  struct proc *curproc = myproc();

  if(addr >= curproc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)curproc->sz;
  for(s = *pp; s < ep; s++){
    if(*s == 0)
      return s - *pp;
  }
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();
 
  if(argint(n, &i) < 0)
    return -1;
  if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_halt(void);
extern int sys_time(void);
extern int sys_stime(void);
extern int sys_trace(void);
extern int sys_procinfo(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_halt]    sys_halt,
[SYS_time]    sys_time,
[SYS_stime]   sys_stime,
[SYS_trace]   sys_trace,
[SYS_procinfo]   sys_procinfo
};

static char *syscall_names[] = {
  [SYS_fork]    "fork",
  [SYS_exit]    "exit",
  [SYS_wait]    "wait",
  [SYS_pipe]    "pipe",
  [SYS_read]    "read",
  [SYS_kill]    "kill",
  [SYS_exec]    "exec",
  [SYS_fstat]   "fstat",
  [SYS_chdir]   "chdir",
  [SYS_dup]     "dup",
  [SYS_getpid]  "getpid",
  [SYS_sbrk]    "sbrk",
  [SYS_sleep]   "sleep",
  [SYS_uptime]  "uptime",
  [SYS_open]    "open",
  [SYS_write]   "write",
  [SYS_mknod]   "mknod",
  [SYS_unlink]  "unlink",
  [SYS_link]    "link",
  [SYS_mkdir]   "mkdir",
  [SYS_close]   "close",
  [SYS_halt]    "halt",
  [SYS_time]    "time",
  [SYS_stime]   "stime",
  [SYS_trace]   "trace",
  [SYS_procinfo] "procinfo",
};

void escape_string(char *dst, char *src, int len) {
    int j = 0;
    for(int i = 0; i < len; i++){
        unsigned char c = src[i];
        if (c >= 32 && c <= 126) {   // 可打印 ASCII
            dst[j++] = c;
        } else if (c == '\n') {
            dst[j++] = '\\'; dst[j++] = 'n';
        } else if (c == '\t') {
            dst[j++] = '\\'; dst[j++] = 't';
        } else if (c == '\r') {
            dst[j++] = '\\'; dst[j++] = 'r';
        } 
    }
    dst[j] = '\0';
}

void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    
    // exec syscall
    char *path;
    char exec_program_name[128];
    if(num == 7){
      argstr(0, &path);
      safestrcpy(exec_program_name, path, sizeof(exec_program_name));
    }

    // read syscall
    // struct file *f;
    int n = -1;
    char *p;
    int fd = -1;
    char file_content[65];
    if(num == 5){
      argint(0, &fd);
      argint(2, &n);
      argptr(1, &p, n);
    }
    file_content[64] = '\0';

    // write syscall
    int write_n = -1;
    char *write_p;
    int write_fd = -1;
    char write_file_content[65];
    if(num == 16){
      argint(0, &write_fd);
      argint(2, &write_n);
      argptr(1, &write_p, write_n);
    }
    
    write_file_content[64] = '\0';

    // open syscall
    char *open_path;
    char open_file_content[65];
    int open_omode = -1;
    if(num == 15){
      argstr(0, &open_path);
      argint(1, &open_omode);
    }
    open_file_content[64] = '\0';

    // close syscall
    int close_fd = -1;
    if(num == 21){
      argint(0, &close_fd);
    }

    curproc->tf->eax = syscalls[num]();

    // print trace
    if(1 << num & curproc->tmask){

      
      // exec syscall
      if(num == 7){
        cprintf("%d: %s(\"%s\", ...) = %d\n", 
          curproc->pid, syscall_names[num], exec_program_name, curproc->tf->eax);
      }


      // read syscall
      else if(num == 5){
        int copy_len = curproc->tf->eax < sizeof(file_content)-1 ? curproc->tf->eax : sizeof(file_content)-1;
        // cprintf("copy_len: %d\n", copy_len);
        
        safestrcpy(file_content, p, sizeof(file_content));

        // convert all non-printable ASCII chars to escape form
        char escaped[256];            // 4 倍空间，防止 \xHH 过长
        escape_string(escaped, file_content, copy_len);
        
        if(curproc->tf->eax > copy_len){
          cprintf("%d: %s(%d, \"%s\"..., %d) = %d\n",
            curproc->pid, syscall_names[num], fd, escaped, n, curproc->tf->eax);
        }
        else{
          cprintf("%d: %s(%d, \"%s\", %d) = %d\n", 
            curproc->pid, syscall_names[num], fd, escaped, n, curproc->tf->eax);
        }
      }

      // write syscall
      else if(num == 16){
        int copy_len = -1;

        int ret = (int)curproc->tf->eax;
        if(ret > 0){
          copy_len = curproc->tf->eax < sizeof(write_file_content)-1 ? curproc->tf->eax : sizeof(write_file_content)-1;
        }else{
          copy_len = write_n;
        }

        safestrcpy(write_file_content, write_p, copy_len + 1);

        // convert all non-printable ASCII chars to escape form
        char escaped[256];            // 4 倍空间，防止 \xHH 过长
        escape_string(escaped, write_file_content, copy_len);
        
        if(ret > copy_len){
          cprintf("%d: %s(%d, \"%s\"..., %d) = %d\n",
            curproc->pid, syscall_names[num], write_fd, escaped, write_n, curproc->tf->eax);
        }
        else{
          cprintf("%d: %s(%d, \"%s\", %d) = %d\n", 
            curproc->pid, syscall_names[num], write_fd, escaped, write_n, curproc->tf->eax);
        }
      }

      // fork syscall
      else if(num == 1){
        cprintf("%d: %s() = %d\n", 
            curproc->pid, syscall_names[num], curproc->tf->eax);
      }

      // open syscall
      else if(num == 15){
        safestrcpy(open_file_content, open_path, sizeof(open_file_content));

        cprintf("%d: %s(\"%s\", %d) = %d\n", 
            curproc->pid, syscall_names[num], open_file_content, open_omode, curproc->tf->eax);

      }

      // close syscall
      else if(num == 21){
        cprintf("%d: %s(%d) = %d\n", 
            curproc->pid, syscall_names[num], close_fd, curproc->tf->eax);
      }


    }

    
    
    
    
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }



}
