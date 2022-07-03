#include "kernel/types.h"
#include "user/user.h"


static void tobuf(int X, char* buf);
static void tonum(char* buf, int* X);

int
main(int argc, char* argv[])
{
    int p, num;
    int pre, post;
    int fds[2];
    char buf[4];

    while(1) {
        fprintf(1, "prime 2\n");
        pipe(fds);

        if(fork() == 0) {
            close(fds[1]);
            break;
        }
        close(fds[0]);
        post = fds[1];
        num = 2;
        while(num < 35) {
            if(++num % 2) {
                tobuf(num, buf);
                write(post, buf, 4);
            }       
        }

        close(post);
        wait((int *)0);
        exit(0);
    }

    // not frist
    while (1)
    {
        pre = fds[0];
        if(0 == read(pre, buf, 4)) {
            // the last process
            close(pre);
            exit(0);
        }
        tonum(buf, &p);
        fprintf(1, "prime %d\n", p);

        pipe(fds);

        if(fork() == 0) {
            close(fds[1]);
            continue;
        }
        else{
            close(fds[0]);
            post = fds[1];
            while(read(pre, buf, 4)) {
                tonum(buf, &num);
                if(num % p) write(post, buf, 4);
            }
            close(pre);
            close(post);
            wait((int *) 0);
            exit(0);
        }
    }
}

static void 
tobuf(int X, char* buf) 
{
    buf[0] = (X & 0xf);
    buf[1] = ((X & 0xf0) >> 4);
    buf[2] = ((X & 0xf00) >> 8);
    buf[3] = ((X & 0xf000) >> 12);
}

static void 
tonum(char* buf, int* X)
{
    int ret = buf[0];
    ret |= (buf[1] << 4);
    ret |= (buf[2] << 8);
    ret |= (buf[3] << 12);
    *X = ret;
}
