#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MLEN 512

static char buf[MLEN];
static int recurfind(int dirFd, int len, const char* mode);

int
main(int argc, char* argv[])
{   
    int fd, len;
    char* path;
    char* filename;
    struct stat st;

    if(3 != argc) {
        fprintf(2, "Usage: find path filename\n");
        exit(1);
    }

    path = argv[1];
    filename = argv[2];

    if((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        exit(1);
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        exit(1);
    }

    if(st.type != T_DIR) {
        fprintf(2, "find: %s is not a direcotory\n", path);
        close(fd);
        exit(1);
    }

    len = strlen(path);
    if (len + 1 + DIRSIZ + 1 >= MLEN) {
        fprintf(2, "find: %s is too long\n", path);
        close(fd);
        exit(1);
    }

    if('/' != path[len-1]) {
        memmove(buf, path, len);
        buf[len++] = '/';
        buf[len] = 0;
    }
    
    exit(recurfind(fd, len, filename));
}

static int 
recurfind(int dirFd, int len, const char* mode)
{
    struct stat st;
    struct dirent de;
    int childLen, childFd;

    while(read(dirFd, &de, sizeof de) == sizeof de) {
        if(0 == de.inum || 0 == strcmp(".", de.name) || 0 == strcmp("..", de.name)) 
            continue;
        
        // iterate
        childLen = strlen(de.name);
        if(len + childLen >= MLEN) {
            fprintf(2, "find: %s%s too long", buf, de.name);
            return -1;
        }
        memmove(buf+len, de.name, childLen);
        buf[len+childLen] = 0;

        if(stat(buf, &st) < 0){
            fprintf(2, "ls: cannot stat %s\n", buf);
            continue;
        }

        if(st.type == T_DIR) {

            if (len + childLen + 1 >= MLEN) {
                fprintf(2, "find: %s%s/ too long", buf, de.name);
                return -1;
            }
            

            buf[len+childLen] = '/';
            buf[len+childLen+1] = 0;
            childFd = open(buf, O_RDONLY);
            if (recurfind(childFd, len+childLen+1, mode) < -1)
            {
                return -1;
            }
        }
        else if (0 == strcmp(de.name, mode)) {
            fprintf(1, "%s\n", buf);
        }
    }
    return 0;
}