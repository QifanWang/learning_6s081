#include "kernel/param.h"
#include "kernel/types.h"

#include "user/user.h"

#define MAXBUF 512

// char cmdarg[MAXARG][MAXBUF];

char* cmdargs[MAXARG];
char buf[MAXBUF];

static int readline(int fd);

int
main(int argc, char* argv[])
{
    int cnt_args = 0;
    int nbytes = 0, i;


    if(argc <= 1) {
        fprintf(2, "Usage: xargs command...\n");
        exit(1);
    }

    if(argc + 1 > MAXARG) {
        fprintf(2, "xargs: too many arguments, argc = %d \n", argc);
        exit(1);
    }

    // fprintf(1, "befor copy\n");
    // copy cmd original args
    for(i = 1; i < argc; ++i) {
        cmdargs[i-1] = argv[i];
        // fprintf(1, "ori args %s\n", cmdargs[i-1]);
    }
    cnt_args = argc - 1;

    // set

    while((nbytes = readline(0)) > 0) {
        // fprintf(1, "line is %s\n", buf);
        cmdargs[cnt_args] = buf;
        cmdargs[cnt_args+1] = 0;

        if(fork() == 0) {
            exec(cmdargs[0], cmdargs);
        } else {
            wait((int *) 0);
        }
    }
    exit(0);
}

static int readline(int fd) {
    char content;
    int i = 0;
    while(read(fd, &content, 1) == 1 && content != '\n') {
        buf[i++] = content;
        if(i + 1 == MAXBUF ) break;
    }
    buf[i] = 0;
    return i;
}