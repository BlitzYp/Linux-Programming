/*
Lai izpildītu uzdevumu es izveidoju grafa struktūru, kur katrs cilvēks ir mezgls, pēc tam veicu topoloģisko kārtošanu ar BFS Kahna algoritmu, 
lai izdrukātu cilvēkus pareizā secībā. Lai noteiktu, kuri cilvēki pieder pie kuras ģimenes, 
izmantoju Disjoint Set Union (DSU) datu struktūru. Lai efektīvi meklētu cilvēkus pēc vārda, izmantoju hash tabulu. 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define HASH_SIZE 1000001
#define MAX_LINE_LEN 126 // 2*63 

// Structures
typedef struct Person{
    int id;
    char* name;
    struct Person* mother;
    struct Person* father;
    struct Person** children;
    unsigned int num_children;
    bool defined; 
    unsigned int degree;
} Person;

// Priekš Hash tabulas
typedef struct Node {
    char* key;
    Person* value;
    struct Node* next;
} Node; 

typedef struct Record {
    bool has_current;
    char current_name[64];
    bool has_mother;
    char mother_name[64];
    bool has_father;
    char father_name[64];
} Record;

typedef struct DSU {
    int N;
    int* a;
    int* s;
} DSU;

typedef struct QueueNode {
    int id;
    int root;
} QueueNode;

Node* hash_table[HASH_SIZE];
static Person** people=NULL;
static int N=0;
static int people_cap=256;

// Function declerations
static Person* create_person(const char* name);
static void die_error(const char* msg);
static void trim(char* s);
static unsigned long hash(const char* str); // Izvēlējos šo hash funkciju (DJB2), jo tā ir diezgan vienkārša, bet arī efektīva
static Person* hash_get_person(const char* key);
static void create_record(Record* st);
static void record_reset(Record* st);
static void add_child(Person* parent, Person* child);
static DSU* init_dsu(int N);
static int find_dsu(DSU* dsu,int u);
static void merge_dsu(DSU* dsu,int u,int v); 
static bool is_connected(DSU* dsu,int u,int v);
static void free_dsu(DSU* dsu);
static void toposort_bfs(DSU* dsu);
void sv_kopet(const char *no, char *uz);

int main(int argc,char** argv)  
{
    char line[MAX_LINE_LEN];
    int n=0;
    Record st;
    record_reset(&st);
    people=(Person**)malloc(sizeof(Person*)*people_cap);
    while (fgets(line,MAX_LINE_LEN,stdin)) {
        trim(line);
        if (line[0]=='\0') continue; 
        char key[16];
        char value[64];
        if (sscanf(line, "%s %s", key, value)!=2) die_error("invalid input format");
        if (strcmp(key,"VARDS")==0) {
            create_record(&st);
            st.has_current=true;
            strcpy(st.current_name,value);
        }
        else if (strcmp(key,"MATE")==0) {
            if (!st.has_current) die_error("MATE bez VARDS");
            if (st.has_mother&&strcmp(st.mother_name,value)!=0) die_error("two different mothers for same person");
            st.has_mother=true;
            strcpy(st.mother_name,value);
        }
        else if (strcmp(key,"TEVS")==0) {
            if (!st.has_current) die_error("TEVS bez VARDS");
            if (st.has_father&&strcmp(st.father_name,value)!=0) die_error("two different fathers for same person");
            st.has_father=true;
            strcpy(st.father_name,value);
        } 
        //printf("Nolasīts: %s %s\n", key, value);
    }
    create_record(&st); // Apstrādā pēdējo ierakstu

    DSU* dsu=init_dsu(N);
    if (!dsu) die_error("dsu init fail");
    for (int i=0;i<N;i++) {
        Person* curr=people[i];
        for (int j=0;j<curr->num_children;j++) {
            Person* child=curr->children[j];
            merge_dsu(dsu,curr->id,child->id);
        }
        //printf("id for %s: %d, degree: %d, num_children: %d\n",curr->name,curr->id,curr->degree,curr->num_children);
    }
    toposort_bfs(dsu);
    free_dsu(dsu);
    for (int i=0;i<N;i++) { 
        free(people[i]->children);
        free(people[i]->name);
        free(people[i]); 
    }
    free(people);
    return 0;
}

void sv_kopet(const char *no, char *uz)
{
    for (;*no!='\0';no++,uz++)*uz=*no;
    *uz='\0';
}

static void people_push(Person *p) 
{
    if (N==people_cap) {
        people_cap*=2;
        Person** tmp=realloc(people,people_cap*sizeof(Person*));
        if (!tmp) die_error("out of memory (all_people)");
        people=tmp;
    }
    people[N++]=p;
}

static void trim(char* s) 
{
    // Noņem jaunās rindas simbolus
    size_t n=strlen(s);
    while (n>0&&(s[n-1]=='\n'||s[n-1]=='\r')) s[--n]='\0';
    // Noņem sākuma atstarpes
    size_t i=0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i>0) memmove(s,s+i,strlen(s+i)+1);
    // Noņem beigu atstarpes
    n=strlen(s);
    while (n>0 && isspace((unsigned char)s[n-1])) s[--n]='\0';
}

static Person* create_person(const char* name) 
{
    Person* p=(Person*)malloc(sizeof(Person));
    p->id=N;
    //p->name=strdup(name);
    sv_kopet(name,p->name=(char*)malloc(strlen(name)+1));
    p->mother=NULL;
    p->father=NULL;
    p->children=NULL;
    p->num_children=0;
    p->defined=false;
    p->degree=0;
    people_push(p);
    return p;
}

static void die_error(const char* msg) 
{
    if (msg&&*msg) fprintf(stderr, "Error!: %s\n", msg);
    else fprintf(stderr, "Unknown Error!\n");
    exit(1);
}

static unsigned long hash(const char* str) 
{
    unsigned long h=5381;
    int c;
    while ((c=(unsigned char)*str++)) h=((h<<5)+h)+(unsigned long)c;
    return h;
}

static Person* hash_get_person(const char* key) 
{
    unsigned long h=hash(key)%HASH_SIZE;
    Node* node=hash_table[h];
    while (node!=NULL) {
        if (strcmp(node->key,key)==0) return node->value;
        node=node->next;
    }
    Node* new_node=(Node*)malloc(sizeof(Node));
    //new_node->key=strdup(key);
    sv_kopet(key,new_node->key=(char*)malloc(strlen(key)+1));
    new_node->value=create_person(key);
    new_node->next=hash_table[h];
    hash_table[h]=new_node;
    return hash_table[h]->value;
}

static void record_reset(Record* st) 
{
    st->has_current=false;
    st->current_name[0]='\0';
    st->has_mother=false;
    st->mother_name[0]='\0';
    st->has_father=false;
    st->father_name[0]='\0';
}

// Priekš VARDS blokiem, izveido bērns, māte, tēvs struktūru
static void create_record(Record* st) 
{
    if (!st->has_current) return;
    Person* p=hash_get_person(st->current_name);

    // kļūda, ja divām personām vienāds vārds (atkārtots VARDS)
    if (p->defined) die_error("duplicate VARDS for same name");
    p->defined=true;

    if (st->has_mother) {
        Person*m=hash_get_person(st->mother_name);
        if (p->mother&&p->mother!=m) die_error("two different mothers");
        if (!p->mother) { // nepieaudzē indegree 2x, ja nejauši dubultā tas pats
            p->mother=m;
            add_child(m,p);
            p->degree++;
        }
    }
    if (st->has_father) {
        Person* f=hash_get_person(st->father_name);
        if (p->father&&p->father!=f) die_error("two different fathers");
        if (!p->father) {
            p->father=f;
            add_child(f,p);
            p->degree++;
        }
    }
    record_reset(st);
}

static void add_child(Person* parent, Person* child) 
{
    parent->children=(Person**)realloc(parent->children,(parent->num_children+1)*sizeof(Person*));
    if (parent->children==NULL) die_error("Memory allocation failed for children");
    parent->children[parent->num_children++]=child;
}

static DSU* init_dsu(int N) 
{
    DSU* dsu=(DSU*)malloc(sizeof(DSU));
    dsu->N=N;
    dsu->a=(int*)malloc(sizeof(int)*N);
    dsu->s=(int*)malloc(sizeof(int)*N);
    for (int i=0;i<N;i++) {
        dsu->s[i]=1;
        dsu->a[i]=i;
    }
    return dsu;
}

static int find_dsu(DSU* dsu,int u)
{
    if (u==dsu->a[u]) return u;
    dsu->a[u]=find_dsu(dsu,dsu->a[u]);
    return dsu->a[u];
}

static bool is_connected(DSU* dsu,int u,int v) 
{
    return find_dsu(dsu,u)==find_dsu(dsu,v);
}

static void merge_dsu(DSU* dsu,int u,int v) 
{
    if (!is_connected(dsu,u,v)) {
        int fu=find_dsu(dsu,u);
        int fv=find_dsu(dsu,v);
        if (dsu->s[fu]<dsu->s[fv]) {
            int tmp=fu;
            fu=fv;
            fv=tmp;
        }
        dsu->s[fu]+=dsu->s[fv];
        dsu->a[fv]=fu;
    }
}

static int cmp(const void* a,const void* b)
{
    const QueueNode* nodeA=(const QueueNode*)a;
    const QueueNode* nodeB=(const QueueNode*)b;
    if (nodeA->root!=nodeB->root) return nodeA->root-nodeB->root;
    return nodeA->id-nodeB->id;
}

static void toposort_bfs(DSU* dsu)
{
    QueueNode* items=(QueueNode*)malloc(sizeof(QueueNode)*N);
    if (!items) die_error("Memory allocation failed for BFS queue");
    for (int i=0;i<N;i++) {
        items[i].id=i;
        items[i].root=find_dsu(dsu,i);
    }
    qsort(items,N,sizeof(QueueNode),cmp);
    int* degrees=(int*)malloc(sizeof(int)*N);
    if (!degrees) die_error("Memory allocation failed for degrees array");
    for (int i=0;i<N;i++) degrees[i]=people[i]->degree;
    int pos=0;
    bool first=true;
    while (pos<N) {
        int root=items[pos].root; 
        int family_beg=pos,family_end;
        while (pos<N&&items[pos].root==root) pos++;
        family_end=pos;
        int family_size=family_end-family_beg;

        int* q=malloc(sizeof(int)*family_size);
        if (!q) die_error("Problem with queue");

        int qstart,qend,done;
        qstart=qend=done=0;
        for (int i=family_beg;i<family_end;i++) if (degrees[items[i].id]==0) q[qend++]=items[i].id;
        if (!first) fprintf(stdout,"\n");
        else first=false;
        while (qstart<qend) {
            int uid=q[qstart++];
            Person* u=people[uid];
            fprintf(stdout,"%s\n",u->name);
            done++;
            for (int j=0;j<u->num_children;j++) {
                int vid=u->children[j]->id;
                degrees[vid]--;
                if (degrees[vid]==0) q[qend++]=vid;
            }
        } 
        if (done!=family_size) die_error("cycle detected in family tree");
        free(q);
    }
    free(items);
    free(degrees);
}

static void free_dsu(DSU* dsu)
{
    free(dsu->a);
    free(dsu->s);
    free(dsu);
}