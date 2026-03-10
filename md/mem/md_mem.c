#define _GNU_SOURCE
#define MB (1024*1024)

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     
#include <sys/mman.h>   

static void* alloc_mmap(size_t n) 
{
    void* p=mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p==MAP_FAILED) return NULL;
    return p;
}

static void* alloc_sbrk(size_t n) 
{
    void* old=sbrk(0);
    void* res=sbrk((intptr_t)n);
    if (res==(void*)-1) return NULL;
    return old;
}

int main(int argc, char** argv) 
{
    if ((argc<2)||!(strcmp(argv[1],"malloc")==0 || strcmp(argv[1],"mmap")==0 || strcmp(argv[1],"sbrk")==0)) {
        fprintf(stderr, "Usage: %s <malloc|mmap|sbrk>\n", argv[0]);
        return -1;
    }
    uint8_t method=0;
    long long count=0;
    if (strcmp(argv[1],"malloc")==0) method=0;
    else if (strcmp(argv[1],"mmap")==0) method=1;
    else method=2;
    size_t size,res;
    size=res=MB;
    for (;;size+=MB) {
        void* p=NULL;
        void* prev=NULL;
        if (method==0) p=malloc(size);
        else if (method==1) p=alloc_mmap(size);
        else { 
            prev=sbrk(0);
            p=alloc_sbrk(size);
        }

        if (!p) break;
        res=size;
        count++;
        /*
        volatile unsigned char* vp=p;
        memset((void*)vp,1,size);
        */
        if (method==0) free(p);
        else if (method==1) munmap(p,size);
        else brk(prev);

        if (count%256==0) fprintf(stderr,"Allocted MB thus far: %ld\n",size/MB);
    }

    printf("Results with method: %s\n",argv[1]);
    printf("Total memory allocated(bytes): %ld\n",res);
    printf("Total MB allocated: %ld\n",res/MB);
    return 0;
}
