#define _GNU_SOURCE
#define MB (1024*1024)

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>     
#include <sys/mman.h>   

int main(int argc, char **argv) 
{
    long long count=0;
    for (int i=0;i<100;i++) {
        void* p=NULL;
        p=malloc(MB);
        if (!p) break;
        volatile unsigned char* vp=p;
        memset((void*)vp,1,MB);
        count++;
    }

    printf("Total memory allocated(bytes): %ld\n",count*(long long)MB);
    printf("Total MB allocated: %lld\n",count);
    return 0;
}
