#define _GNU_SOURCE
#define MB (1024*1024)

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     
#include <sys/mman.h>   

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

// Touch each page so the OS actually backs it with physical memory (or swap).
static void touch_pages(volatile unsigned char *p, size_t nbytes, size_t pagesz) 
{
    // write 1 byte per page
    for (size_t off = 0; off < nbytes; off += pagesz) {
        p[off] = (unsigned char)(p[off] + 1u);
    }
    // also touch last byte (covers non-multiple-of-page cases; for 1MB it's fine anyway)
    if (nbytes > 0) {
        p[nbytes - 1] = (unsigned char)(p[nbytes - 1] + 1u);
    }
}

static void* alloc_mmap(size_t n) 
{
    void *p=mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p==MAP_FAILED) return NULL;
    return p;
}

static void* alloc_sbrk(size_t n) 
{
    void *old=sbrk(0);
    void *res=sbrk((intptr_t)n);
    if (res==(void*)-1) return NULL;
    return old; // start of newly "allocated" region
}

int main(int argc, char **argv) 
{
    // -t ir priekš touch (alokācija+rakstīt tajā lapā)
    if ((argc<2)||!(strcmp(argv[1],"malloc")==0 || strcmp(argv[1],"mmap")==0 || strcmp(argv[1],"sbrk")==0)) {
        fprintf(stderr, "Usage: %s <malloc|mmap|sbrk> t\n", argv[0]);
        return -1;
    }
    
    long ps=sysconf(_SC_PAGESIZE);
    size_t pagesz=(ps>0) ? (size_t)ps : 4096;
    uint8_t method=0,touch=0;
    if (argc==3&&strcmp(argv[2],"-t")==0) touch=1;

    long long count=0;
    if (strcmp(argv[1],"malloc")==0) method=0;
    else if (strcmp(argv[1],"mmap")==0) method=1;
    else if (strcmp(argv[1],"sbrk")==0) method=2;

    for (;;) {
        void* p=NULL;
        if (method==0) p=malloc(MB);
        else if (method==1) p=alloc_mmap(MB);
        else if (method==2) p=alloc_sbrk(MB);
        if (!p) break;
        count++;
        if (touch) touch_pages((volatile unsigned char*)p,MB,pagesz);
        if (count%256==0) fprintf(stderr,"Allocted MB thus far: %lld\n",count);
    }

    printf("Results with method %s\n",argv[1]);
    printf("Total memory allocated(bytes): %lld\n",count*MB);
    printf("Total MB allocated: %lld\n",count);
    return 0;
}
