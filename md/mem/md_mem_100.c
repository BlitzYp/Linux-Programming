#define _GNU_SOURCE
#define MB (1024*1024)

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
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
    void* r=sbrk((intptr_t)n);
    if (r==(void*)-1) return NULL;
    return old;
}

int main(int argc, char** argv) 
{
    // -t ir priekš touch (alokācija+rakstīt tajā lapā)
    if ((argc<2)||!(strcmp(argv[1],"malloc")==0 || strcmp(argv[1],"mmap")==0 || strcmp(argv[1],"sbrk")==0)) {
        fprintf(stderr, "Usage: %s <malloc|mmap|sbrk> [-t]\n", argv[0]);
        return -1;
    }
    
    uint8_t method=0,touch=0;
    if (argc==3&&strcmp(argv[2],"-t")==0) touch=1;

    long long count=0;
    if (strcmp(argv[1],"malloc")==0) method=0;
    else if (strcmp(argv[1],"mmap")==0) method=1;
    else method=2;
    for (int i=0;i<100;i++) {
        void* p=NULL;
        //void* prev=NULL;
        if (method==0) p=malloc(MB);
        else if (method==1) p=alloc_mmap(MB);
        else { 
            //prev=sbrk(0);
            p=alloc_sbrk(MB);
        }

        if (!p) break;
        if (touch) {
            volatile unsigned char* vp=p;
            memset((void*)vp,1,MB);
        }
        count++;
        /*
        if (method==0) free(p);
        else if (method==1) munmap(p,size);
        else brk(prev);
        */
    }

    printf("Results with method(100MB): %s\n",argv[1]);
    printf("With writing? (touch): %s\n", touch?"Enabled":"Disabled");
    printf("Total memory allocated(bytes): %ld\n",count*(long long)MB);
    printf("Total MB allocated: %lld\n",count);
    return 0;
}
