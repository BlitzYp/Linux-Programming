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

enum Method {
    M_MALLOC = 0,
    M_MMAP   = 1,
    M_SBRK   = 2
};

static const size_t ONE_MB = 1024u * 1024u;

static void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s <malloc|mmap|sbrk>\n"
            "Allocates memory in 1MB blocks until allocation fails; touches pages to force commit.\n",
            prog);
}

static enum Method parse_method(const char *s) {
    if (strcmp(s, "malloc") == 0) return M_MALLOC;
    if (strcmp(s, "mmap")   == 0) return M_MMAP;
    if (strcmp(s, "sbrk")   == 0) return M_SBRK;
    return (enum Method)-1;
}

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

static void *alloc_malloc(size_t n) {
    return malloc(n);
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
    if (argc != 2) {
        usage(argv[0]);
        return 2;
    }

    enum Method method = parse_method(argv[1]);
    if ((int)method < 0) {
        usage(argv[0]);
        return 2;
    }

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
        errno = 0;
        void *p = NULL;

        if (method == M_MALLOC) {
            p = alloc_malloc(ONE_MB);
        } else if (method == M_MMAP) {
            p = alloc_mmap(ONE_MB);
        } else { // M_SBRK
            p = alloc_sbrk(ONE_MB);
        }

        if (!p) {
            // Allocation failed (or mmap returned MAP_FAILED mapped to NULL)
            break;
        }

        // Force physical backing by touching pages
        touch_pages((volatile unsigned char*)p, ONE_MB, pagesz);

        // Update sink with something dependent on p to keep compiler honest
        sink ^= (unsigned long long)(uintptr_t)p;

        if (method != M_SBRK) {
            // grow pointer list
            if (cnt == cap) {
                size_t newcap = (cap == 0) ? 1024 : cap * 2;
                void **tmp = realloc(ptrs, newcap * sizeof(void*));
                if (!tmp) {
                    // If we can't even grow the list, stop; memory is likely near exhaustion anyway.
                    break;
                }
                ptrs = tmp;
                cap = newcap;
            }
            ptrs[cnt] = p;
        }

        cnt++;

        // Optional: print progress every 256MB so you see it's alive
        if ((cnt % 256) == 0) {
            fprintf(stderr, "allocated: %zu MB\n", cnt);
        }
    }

    // Report results
    printf("Method: %s\n", argv[1]);
    printf("Max allocated blocks: %zu\n", cnt);
    printf("Max allocated bytes : %" PRIu64 "\n", (uint64_t)cnt * (uint64_t)ONE_MB);
    printf("Max allocated MiB   : %zu\n", cnt);
    // print sink so compiler doesn't drop it
    printf("Sink: %llu\n", (unsigned long long)sink);
    
    // Note: freeing is not required by assignment.
    // If you want to be nice for malloc/mmap runs, you could free/munmap here,
    // but for "max" tests it can take time and isn't needed.

    return 0;
}
