#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


void pp(char *k, size_t v) {
    printf("%-42s :%20zu bytes  %10.1f MB  %10.1f GB\n", \
        k, v, (float)v/1024/1024, (float)v/1024/1024/1024);
}


int main(int argc, char *argv[]) {
    struct stat st;
    int fd;
    off_t end, pos, next;
    size_t holes=0, datas=0;
    int what, current;

    if(argc!=2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }

    if(stat(argv[1], &st)!=0) {
        fprintf(stderr, "Unable to stat file %s\n", argv[1]);
        return 2;
    }

    fd = open(argv[1], O_RDONLY);
    if(!fd) {
        fprintf(stderr, "Uname to open file %s\n", argv[1]);
        return 0;
    }

    end = lseek(fd, 0, SEEK_END);
    what=SEEK_DATA;
    pos=lseek(fd, 0, SEEK_HOLE);

    if(pos==0) {
        what=SEEK_DATA;
    }
    else if(pos==end) {
        printf("DATA: %zu\nNot a sparse file\n", end);
        return 0;
    }
    else {
        what=SEEK_HOLE;
        pos=0;
    }

    while(pos<end) {
        if(what==SEEK_HOLE)
            current=SEEK_DATA;
        else
            current=SEEK_HOLE;

        next=lseek(fd, pos, what);

        if(next==-1) { 
            fprintf(stderr, "lseek error: %zu, %d, %s\n", next, errno, strerror(errno));
            break;
        }

        printf("%4s : %016ZX - %016ZX (%20zu bytes  %10.1f MB) \n", \
            (current==SEEK_DATA) ? "Data":"Hole" , pos, next, next-pos, (float)(next-pos)/1024/1024);

        if(current==SEEK_DATA)
            datas=datas+(next-pos);
        else
            holes=holes+(next-pos);

        pos=next;

        if(what==SEEK_HOLE)
            what=SEEK_DATA;
        else
            what=SEEK_HOLE;
    }

    close(fd);

    pp("Data Total", datas);
    pp("Allocated (st_blocks)", st.st_blocks*512);
    pp("Holes Total", holes);
    pp("Size (st_size)", st.st_size);

    return 0;
}
