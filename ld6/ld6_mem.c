#define _GNU_SOURCE
#define MB (1024*1024)

#include <alloca.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Paņēmu koda fragmentus no MD_MEM
int main(int argc, char** argv)
{
    if (argc<2||!(strcmp(argv[1],"one")==0||strcmp(argv[1],"many")==0||strcmp(argv[1],"100")==0||strcmp(argv[1],"stack")==0)) {
        fprintf(stderr,"Usage: %s <one|many|100|stack> [mb] [-t]\n",argv[0]);
        return -1;
    }

    uint8_t method=0,touch=0;
    long long count=0;
    if (strcmp(argv[1],"one")==0) method=0;
    else if (strcmp(argv[1],"many")==0) method=1;
    else if (strcmp(argv[1],"100")==0) method=2;
    else method=3;

    if ((argc>=3&&strcmp(argv[2],"-t")==0)||(argc>=4&&strcmp(argv[3],"-t")==0)) touch=1;

    printf("allocating...\n");

    if (method==0) {
        size_t size,res;
        size=res=MB;
        for (;;size+=MB) {
            void* p=malloc(size);
            if (!p) break;
            res=size;
            if (touch) memset((void*)p,0,size);
            free(p);
            count++;
            if (count%256==0) fprintf(stderr,"Allocated MB thus far: %ld\n",size/MB);
        }
        printf("Results with method: one\n");
        printf("Total memory allocated(bytes): %ld\n",res);
        printf("Total MB allocated: %ld\n",res/MB);
    }
    else if (method==1) {
        void** ptrs=NULL;
        long long cap=0;
        for (;;) {
            void* p=malloc(MB);
            if (!p) break;
            if (touch) memset((void*)p,0,MB);
            if (count==cap) {
                long long new_cap=cap?cap*2:1024;
                void** tmp=realloc(ptrs,new_cap*sizeof(void*));
                if (!tmp) {
                    free(p);
                    break;
                }
                ptrs=tmp;
                cap=new_cap;
            }
            ptrs[count++]=p;
            if (count%256==0) fprintf(stderr,"Allocated MB thus far: %lld\n",count);
        }
        printf("Results with method: many\n");
        printf("Total memory allocated(bytes): %lld\n",count*(long long)MB);
        printf("Total MB allocated: %lld\n",count);
        free(ptrs);
    }
    else if (method==2) {
        for (int i=0;i<100;i++) {
            void* p=malloc(MB);
            if (!p) break;
            if (touch) memset(p,0,MB);
            count++;
        }
        printf("Results with method: 100MB\n");
        printf("With writing? (touch): %s\n",touch?"Enabled":"Disabled");
        printf("Total memory allocated(bytes): %lld\n",count*(long long)MB);
        printf("Total MB allocated: %lld\n",count);
    }
    else {
        size_t size;
        void* p;
        if (argc<3) {
            fprintf(stderr,"Usage: %s stack <mb> [-t]\n",argv[0]);
            return -1;
        }
        size=(size_t)atoll(argv[2])*MB;
        p=alloca(size);
        if (touch) memset(p,0,size);
        printf("Results with method: stack\n");
        printf("Total memory allocated(bytes): %ld\n",size);
        printf("Total MB allocated: %ld\n",size/MB);
    }
    printf("done\n");
    return 0;
}
