#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define MY_BUFFER_SIZE 4096
unsigned char mybuffer[MY_BUFFER_SIZE];

// Dienesta informācija katram blokam
typedef struct Header {
    size_t size;
    bool free;
} Header;

static Header* nextfit=NULL;

/*
Pārveido bloka izmēru tā, lai tas būtu 8 baitu liels
Piemēram 9->8, 3->8
*/
static size_t align(size_t s)
{
    return (s+7)&~(7);
}

static Header* get_start()
{
    return (Header*)mybuffer;
}

static Header* get_next_block(Header* blk)
{
    unsigned char* next=(unsigned char*)blk+sizeof(Header)+blk->size;
    if (next>=mybuffer+MY_BUFFER_SIZE) return NULL;
    return (Header*)next;
}

/*
Ja vajadzīgais izmērs nenosedz visu bloku, sadalām.
Šeit tad praktiski tiek izveidoti jauni bloki
*/
static void split_block(Header* blk, size_t s)
{
    size_t remaining=blk->size-s;
    if (remaining<=sizeof(Header)+8) return;
    unsigned char* new=(unsigned char*)blk+sizeof(Header)+s;

    Header* new_block=(Header*)new;
    new_block->size=remaining-sizeof(Header);
    new_block->free=true;
    blk->size=s;
}

static Header* get_header(void* ptr)
{
    return (Header*)((unsigned char*)ptr-sizeof(Header));
}

static void* mymalloc(size_t size)
{
    if (size==0) return NULL;
    size_t aligned_size=align(size);
    Header* start=get_start();
    if (!nextfit) {
        start->size=MY_BUFFER_SIZE-sizeof(Header);
        start->free=true;
        nextfit=start;
    }
    start=nextfit;
    Header* curr=start;
    do {
        if (curr->free&&curr->size>=aligned_size) {
            split_block(curr,aligned_size);
            curr->free=false;
            nextfit=get_next_block(curr);
            return (unsigned char*)curr+sizeof(Header);
        }
        curr=get_next_block(curr);
        if (!curr) curr=get_start();
    } while (curr!=start);
    return NULL;
}

int myfree(void* ptr)
{
    Header* h;
    if (!ptr||!(h=get_header(ptr))) return -1;
    h->free=true;
    /*
    TO DO
    Savienot iepriekšējo un nākamo
    */
    return 0;
}

int main(void)
{
    char* str="somethingsomething 234";
    char* p=(char*)mymalloc(strlen(str));
    int* a=(int*)mymalloc(sizeof(int)*4);
    strcpy(p,str);
    puts(p);
    for (int i=0;i<4;i++) a[i]=i;
    for (int i=0;i<4;i++) printf("%d\n",a[i]);
    myfree(p);
    myfree(a);
    return 0;
}
