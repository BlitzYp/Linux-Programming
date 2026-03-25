#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define MY_BUFFER_SIZE 4096
unsigned char mybuffer[MY_BUFFER_SIZE];

// Dienesta informācija katram blokam (Dubultsaišu saraksts)
typedef struct Header {
    size_t size;
    bool free;
    struct Header* next;
    struct Header* prev;
} Header;

static Header* nextfit=NULL;
static int search_count=0,alloc_count=0;

/*
Pārveido bloka izmēru uz augšu tā, lai tas dalītos ar 8 baitiem
Piemēram 1->8, 8- 8, 9->16, 13->16
Skaitļiem, kas dalās ar 8, bināri pēdējie 3 biti vienmēr ir 000, tāpēc izmanto ~7=11111000
*/
static size_t align(size_t s)
{
    return (s+7)&~(7);
}

static Header* get_start(void)
{
    return (Header*)mybuffer;
}

/*
Katrā solī iedodam vajadzīgo atmiņu s jaunajam blokam
un atdalām to no bufera atlikušās brīvās atmiņas
*/
static void split_block(Header* blk, size_t s)
{
    // Visa atlikusī brīvā vieta buferī
    size_t remaining=blk->size-s;
    if (remaining<=sizeof(Header)+8) return;
    unsigned char* new=(unsigned char*)blk+sizeof(Header)+s;

    // Nākamais bloks kurā liks
    Header* new_block=(Header*)new;
    new_block->size=remaining-sizeof(Header);
    new_block->free=true;
    new_block->next=blk->next;
    new_block->prev=blk;
    if (blk->next) blk->next->prev=new_block;
    blk->next=new_block;
    blk->size=s;
}

static Header* get_header(void* ptr)
{
    return (Header*)((unsigned char*)ptr-sizeof(Header));
}

static void merge_with_next(Header* blk)
{
    Header* next=blk->next;
    if (!next||!next->free) return;

    blk->size+=sizeof(Header)+next->size;
    blk->next=next->next;
    if (blk->next) blk->next->prev=blk;
}

static void* myalloc(size_t size)
{
    if (size==0) return NULL;
    Header* start;
    if (!nextfit) {
        start=get_start();
        start->size=MY_BUFFER_SIZE-sizeof(Header);
        start->free=true;
        start->next=NULL;
        start->prev=NULL;
        nextfit=start;
    }

    size_t aligned_size=align(size);
    start=nextfit;
    Header* curr=start;
    // Priekš testēšanas
    alloc_count++;
    do {
        search_count++;
        if (curr->free&&curr->size>=aligned_size) {
            split_block(curr,aligned_size);
            curr->free=false;
            nextfit=curr->next ? curr->next : get_start();
            return (unsigned char*)curr+sizeof(Header);
        }
        curr=curr->next;
        if (!curr) curr=get_start();
    } while (curr!=start);
    return NULL;
}

int myfree(void* ptr)
{
    unsigned char* p=(unsigned char*)ptr;
    if (!ptr || (p<mybuffer+sizeof(Header) || p>=mybuffer+MY_BUFFER_SIZE) || !nextfit) return -1;

    Header* h=get_header(ptr);
    h->free=true;

    // Sapludinām kopā ar nākamo
    merge_with_next(h);
    // Sapludinām kopā ar iepriekšējo
    if (h->prev&&h->prev->free) {
        h=h->prev;
        merge_with_next(h);
    }

    nextfit=h;
    return 0;
}

/*
Veiktspēju vērtēju pēc vidējā pārbaudīto bloku skaita vienā myalloc() izsaukumā ar
mainīgajiem search_count (skaita, cik bloki tika apskatīti meklēšanas laikā) un alloc_count
(skaita kopējo rezervāciju skaitu). Metrika search_count/alloc_count parāda,
cik dārga vidēji ir NextFit meklēšana dotajā testu scenārijā.
*/
void frag_test()
{
    void* blocks[10];
    for (int i=0;i<10;i++) {
        char* s="Hello";
        blocks[i]=myalloc(64);
        strcpy(blocks[i],s);
    }
    // Atbrīvo katru otru bloku, izprintē nepāra pozīcijas
    for (int i=0;i<10;i++) {
        if (i%2==0) { myfree(blocks[i]); continue; }
        puts(blocks[i]);
    }
    void* big=myalloc(200);
    char* s2="milzis 123 123 456";
    if (!big) printf("Lielais neizdevās...\n");
    else {
        strcpy((char*)big,s2);
        puts(big);
        myfree(big);
    }
}

int main(void)
{
    frag_test();
    printf("Malloc tika izsaukts %d reizes\n",alloc_count);
    printf("Vidējais atbilstošās vietas meklēšanas skaits priekš vienas rezervācijas: %.2f\n",(double)search_count/alloc_count);
    return 0;
}

/*
static void print_blocks(void)
{
    Header* curr=get_start();
    int i=0;
    while (curr) {
        printf("Block %d: addr=%p size=%zu free=%d next=%p prev=%p\n",i, (void*)curr, curr->size, curr->free, (void*)curr->next, (void*)curr->prev);
        curr=curr->next;
        i++;
    }
}
 */
