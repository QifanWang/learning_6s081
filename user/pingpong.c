#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char* argv[])
{   
    int pid;
    int fds[2];
    uchar buf[1];

    if (pipe(fds) < 0) {
        fprintf(2, "pingpong: pipe failed.\n");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        fprintf(2, "pingpong: fork failed.\n");
        exit(1);
    }

    if(0 == pid) {
        if( read(fds[0], buf, sizeof buf) < 0) {
            fprintf(2, "pingpong: read failed.\n");
            exit(1);
        }
        if( write(fds[1], "abc", 1) < 0) {
            fprintf(2, "pingpong: write failed.\n");
            exit(1);
        }

        fprintf(1, "%d: received ping\n", getpid());
    } else {
        if( write(fds[1], "abc", 1) < 0) {
            fprintf(2, "pingpong: write failed.\n");
            exit(1);
        }
        if( read(fds[0], buf, sizeof buf) < 0) {
            fprintf(2, "pingpong: read failed.\n");
            exit(1);
        }
        wait((int *)0);
        fprintf(1, "%d: received pong\n", getpid());
    }
    
    exit(0);
}