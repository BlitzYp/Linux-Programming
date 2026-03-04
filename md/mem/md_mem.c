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
static void touch_pages(volatile unsigned char *p, size_t nbytes, size_t pagesz) {
    // write 1 byte per page
    for (size_t off = 0; off < nbytes; off += pagesz) {
        p[off] = (unsigned char)(p[off] + 1u);
    }
    // also touch last byte (covers non-multiple-of-page cases; for 1MB it's fine anyway)
    if (nbytes > 0) {
        p[nbytes - 1] = (unsigned char)(p[nbytes - 1] + 1u);
    }
}


static void *alloc_mmap(size_t n) {
    void *p = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) return NULL;
    return p;
}

static void *alloc_sbrk(size_t n) {
    void *old = sbrk(0);
    void *res = sbrk((intptr_t)n);
    if (res == (void*)-1) return NULL;
    return old; // start of newly "allocated" region
}

int main(int argc, char **argv) {
    if (argc != 2) return -1;
    


    long ps = sysconf(_SC_PAGESIZE);
    size_t pagesz = (ps > 0) ? (size_t)ps : 4096u;

    // For malloc/mmap we store pointers so compiler can't optimize away and (optionally) could free later.
    // For sbrk we avoid malloc/realloc entirely to not mix allocators; we just keep counting.
    void **ptrs = NULL;
    size_t cap = 0;
    size_t cnt = 0;

    // We'll keep a volatile accumulator so touches can't be optimized away.
    volatile unsigned long long sink = 0;

    for (;;) {
    }

    /*
    // Report results
    printf("Method: %s\n", argv[1]);
    printf("Max allocated blocks: %zu\n", cnt);
    printf("Max allocated bytes : %" PRIu64 "\n", (uint64_t)cnt * (uint64_t)ONE_MB);
    printf("Max allocated MiB   : %zu\n", cnt);
    // print sink so compiler doesn't drop it
    printf("Sink: %llu\n", (unsigned long long)sink);
    */

    return 0;
}
