#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    int ping_fds[2], pid;
    int pong_fds[2];


    char child_send_byte = '1';
    char parent_send_byte = '2';

    char *child_buf;
    char *parent_buf;

    // 创建pipe
    if(pipe(ping_fds) != 0){
        printf(1, "ping_fds() failed\n");
        exit();
    }
    if(pipe(pong_fds) != 0){
        printf(1, "pong_fds() failed\n");
        exit();
    }

    pid = fork();

    // 子进程
    if(pid == 0){
        // Child reads from parent
        read(ping_fds[0], child_buf, 1);
        int pidd = getpid();
        printf(1, "%d: received ping\n", pidd);
        close(ping_fds[0]);

        // Child then sends to parent
        write(pong_fds[1], &child_send_byte, 1);
        close(pong_fds[1]);

        exit();

    } 
    // 父进程
    else if(pid > 0){
        // Parent send to child
        write(ping_fds[1], &parent_send_byte, 1);
        close(ping_fds[1]);

        // Parent reads from child
        read(pong_fds[0], parent_buf, 1);
        int pidd = getpid();
        printf(1, "%d: received pong\n", pidd);

        wait();
        exit();
    }
}
