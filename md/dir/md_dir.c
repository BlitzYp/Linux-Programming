#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/md5.h>
#include <unistd.h>
#define MAX_PATH 4096

typedef struct Config {
    bool use_date;
    bool use_md5;
    bool use_help;
} Config;

typedef struct File {
    char* path;
    char* name;
    off_t size;
    time_t mtime;
    bool md5_ready;
    unsigned char md5[16];
} File;

typedef struct Vector {
    File** elements;
    size_t len;
    size_t cap;
} Vector;

// Each of the files in the group have the same attributes
typedef struct Group {
    Vector files;
    char* name;
    off_t size; // the size for each file
    time_t mtime;
    unsigned char md5[16];
    struct Group* next_group;
} Group;

typedef struct Vector_Group {
    Group** elements;
    size_t len;
    size_t cap;
} Vector_Group;

typedef struct HashTable {
    Group** groups;
    size_t count;
    size_t size;
} HashTable;

// Declerations
static void die_error(const char* msg);
static bool is_dir(const char* path); 
static void dfs(const char* path,HashTable* hash_table);
static int md5_file(File* f);
static void md5_hex(const unsigned char md5[16],char* o);
static File* create_file(const char* path, const struct stat* st);
static void free_file(File* f);
static Vector* create_vector(int n);
static void push_file_vector(Vector* v,File* f);
static void free_vector(Vector* v);
static HashTable* create_hash_table();
static void hash_table_insert_file(HashTable* hash_table,File* f);
static unsigned long get_file_hash(File* f);
static unsigned long get_group_hash(Group* g);
static unsigned long hash(const char* s);
static unsigned long mix_hash(unsigned long h, unsigned long x);
static void free_hash_table(HashTable* hash_table);
static Group* create_new_group(File* f);
static void print_group(Group* g);
static void free_group(Group* g);
static void format_time(time_t t,char* out); // "yyyy-mm-dd hh:mm"
static void sv_kopet(char *no, char *uz);
static Vector_Group* create_group_vector(size_t cap);
static void push_group_vector(Vector_Group* v,Group* g);
static void free_group_vector(Vector_Group* v);

// Global config for flags: date,md5,help
Config cfg={false,false,false};
Vector_Group* groups;

int main(int argc,char** argv) 
{
    for (int i=1;i<argc;i++) {
        if (!*(argv+i)) continue;
        if (strcmp(argv[i],"-d")==0) cfg.use_date=true;
        else if (strcmp(argv[i],"-m")==0) cfg.use_md5=true;
        else if (strcmp(argv[i],"-h")==0) cfg.use_help=true;
        else die_error("INVALID ARGUMENT");
    }
    char cwd[MAX_PATH];
    getcwd(cwd,MAX_PATH);
    HashTable* hash_table=create_hash_table();
    groups=create_group_vector(1024);
    dfs(cwd,hash_table);
    print_groups();
    free_group_vector(groups);
    free_hash_table(hash_table);
    return 0;
}

static void die_error(const char* msg)
{
    if (msg&&*msg) fprintf(stderr, "Error: %s\n", msg);
    else fprintf(stderr, "Unknown error!\n");
    exit(-1);
}

static void print_groups() 
{
    for (int i=0;i<groups->len;i++) {
        Group* g=groups->elements[i];
        printf("------------------------------------\n");
        fprintf(stdout,"Name for the group: %s, size: %jd\n",g->name,g->size);
        fprintf(stdout,"Nr. of files in group for group %s: %d\n",g->name,g->files.len);
    }
}

static void sv_kopet(char *no, char *uz)
{
    for (; *no != '\0'; no++, uz++) *uz = *no;
    *uz = '\0';
}

static unsigned long hash_bytes(const unsigned char* data, size_t len)
{
    unsigned long h = 1469598103934665603UL; // FNV-like start (fits unsigned long on 64-bit)
    for (size_t i = 0; i < len; i++) {
        h ^= (unsigned long)data[i];
        h *= 1099511628211UL;
    }
    return h;
}

static unsigned long hash(const char* s)
{
    if (!s) return 0;
    return hash_bytes((const unsigned char*)s, strlen(s));
}

static unsigned long mix_hash(unsigned long h, unsigned long x)
{
    h ^= x + 0x9e3779b97f4a7c15UL + (h << 6) + (h >> 2);
    return h;
}

static bool file_in_group(const Group* g, const File* f)
{
    if (!g || !f) return false;

    if (cfg.use_md5) {
        if (!f->md5_ready) return false;
        if (memcmp(g->md5, f->md5, sizeof(f->md5)) != 0) return false;
        if (cfg.use_date && g->mtime != f->mtime) return false;
        return true;
    }

    if (!g->name || !f->name) return false;
    if (strcmp(g->name, f->name) != 0) return false;
    if (g->size != f->size) return false;
    if (cfg.use_date && g->mtime != f->mtime) return false;
    return true;
}

static bool is_dir(const char* path) 
{
    struct stat st;
    if (lstat(path, &st)!=0||S_ISLNK(st.st_mode)) return false;
    return S_ISDIR(st.st_mode);
}

static Vector* create_vector(int n)
{
    Vector* v=malloc(sizeof(Vector));
    if (!v) die_error("malloc Vector");
    v->len=0;
    v->cap=(size_t)n;
    v->elements=NULL;
    if (v->cap>0) {
        v->elements=malloc(v->cap*sizeof(File*));
        if (!v->elements) {
            free_vector(v);
            die_error("malloc Vector elements");
        }
    }
    return v;
}

static void push_file_vector(Vector* v,File* f)
{
    if (!v) die_error("push_file_vector: NULL vector");
    if (v->len==v->cap) {
        size_t new_cap=(!v->cap) ? 64 : v->cap*2;
        File** new_elements=realloc(v->elements,new_cap*sizeof(File*));
        if (!new_elements) die_error("realloc Vector elements");
        v->elements=new_elements;
        v->cap=new_cap;
    }
    v->elements[v->len++]=f;
}

static Vector_Group* create_group_vector(size_t cap)
{
    Vector_Group* v=malloc(sizeof(Vector_Group));
    if (!v) die_error("malloc Vector_Group");
    v->len=0;
    v->cap=cap;
    v->elements=NULL;
    if (cap>0) {
        v->elements=malloc(cap * sizeof(Group*));
        if (!v->elements) {
            free(v);
            die_error("malloc Vector_Group elements");
        }
    }
    return v;
}

static void push_group_vector(Vector_Group* v, Group* g)
{
    if (!v) die_error("push_group_vector: NULL vector");
    if (v->len==v->cap) {
        size_t new_cap=v->cap ? v->cap * 2 : 64;
        Group** new_elements=realloc(v->elements, new_cap * sizeof(Group*));
        if (!new_elements) die_error("realloc Vector_Group elements");
        v->elements=new_elements;
        v->cap=new_cap;
    }
    v->elements[v->len++]=g;
}

static void free_group_vector(Vector_Group* v)
{
    if (!v) return;
    free(v->elements);
    v->elements = NULL;
    v->len = 0;
    v->cap = 0;
    free(v);
}

static void free_vector(Vector* v)
{
    if (!v) return;
    free(v->elements);
    v->elements=NULL;
    v->len = 0;
    v->cap = 0;
    free(v);
}

static File* create_file(const char* path, const struct stat* st)
{
    if (!path||!st) return NULL;
    File* f=malloc(sizeof(File));
    if (!f) die_error("Failed malloc file creation");
    f->path=NULL;
    f->name=NULL;
    f->size=st->st_size;
    f->mtime=st->st_mtime;
    f->md5_ready=false;
    memset(f->md5,0,sizeof(f->md5));
    f->path = strdup(path);
    if (!f->path) {
        free(f);
        die_error("Failed strdup file path");
    }
    const char* base=strrchr(path, '/');
    base=base ? base + 1 : path;
    f->name=strdup(base);
    if (!f->name) {
        free(f->path);
        free(f);
        die_error("Failed strdup file name");
    }
    return f;
}

static void free_file(File* f)
{
    if (!f) die_error("Null file pointer");
    free(f->path);
    free(f->name);
    f->path=NULL;
    f->name=NULL;
    free(f);
}

static HashTable* create_hash_table()
{
    const size_t group_count=1024;
    HashTable* ht=malloc(sizeof(HashTable));
    if (!ht) die_error("malloc HashTable");
    ht->groups=calloc(group_count, sizeof(Group*));
    if (!ht->groups) {
        free(ht);
        die_error("calloc hash buckets");
    }
    ht->count=0;
    ht->size=group_count;
    return ht;
}

static Group* create_new_group(File* f)
{
    if (!f) return NULL;
    Group* g=malloc(sizeof(Group));
    if (!g) die_error("malloc Group");

    g->files.elements=NULL;
    g->files.len=0;
    g->files.cap=0;
    g->name=NULL;
    g->size=f->size;
    g->mtime=f->mtime;
    memcpy(g->md5, f->md5, sizeof(g->md5));
    g->next_group = NULL;

    if (!cfg.use_md5&&f->name) {
        g->name=strdup(f->name);
        //sv_kopet(f->name,g->name);
        if (!g->name) {
            free(g);
            die_error("strdup group name");
        }
    }

    push_file_vector(&g->files, f);
    push_group_vector(groups,g);
    return g;
}

static void free_group(Group* g)
{
    if (!g) return;
    free(g->name);
    free(g->files.elements); // group does not own File objects
    g->files.elements = NULL;
    g->files.len = 0;
    g->files.cap = 0;
    free(g);
}

static void free_hash_table(HashTable* hash_table)
{
    if (!hash_table) return;

    for (size_t i = 0; i < hash_table->size; i++) {
        Group* g = hash_table->groups[i];
        while (g) {
            Group* next = g->next_group;
            free_group(g);
            g = next;
        }
    }
    free(hash_table->groups);
    hash_table->groups = NULL;
    hash_table->count = 0;
    hash_table->size = 0;
    free(hash_table);
}

static unsigned long get_file_hash(File* f)
{
    if (!f) return 0;

    unsigned long h=1469598103934665603UL;

    if (cfg.use_md5) {
        if (f->md5_ready) h=mix_hash(h,hash_bytes(f->md5, sizeof(f->md5)));
        if (cfg.use_date) h=mix_hash(h,(unsigned long)f->mtime);
        return h;
    }

    h=mix_hash(h,hash(f->name));
    h=mix_hash(h,(unsigned long)f->size);
    if (cfg.use_date) h=mix_hash(h,(unsigned long)f->mtime);
    return h;
}

static unsigned long get_group_hash(Group* g)
{
    if (!g) return 0;

    unsigned long h=1469598103934665603UL;

    if (cfg.use_md5) {
        h = mix_hash(h, hash_bytes(g->md5, sizeof(g->md5)));
        if (cfg.use_date) h = mix_hash(h,(unsigned long)g->mtime);
        return h;
    }

    h=mix_hash(h,hash(g->name));
    h=mix_hash(h,(unsigned long)g->size);
    if (cfg.use_date) h=mix_hash(h,(unsigned long)g->mtime);
    return h;
}

static void hash_table_insert_file(HashTable* hash_table,File* f)
{
    if (!hash_table||!f||hash_table->size==0) return;

    unsigned long h=get_file_hash(f);
    size_t idx=(size_t)(h % hash_table->size);

    Group* g=hash_table->groups[idx];
    while (g) {
        if (get_group_hash(g)==h && file_in_group(g, f)) {
            push_file_vector(&g->files, f);
            return;
        }
        g=g->next_group;
    }

    Group* new_group=create_new_group(f);
    new_group->next_group=hash_table->groups[idx];
    hash_table->groups[idx]=new_group;
    hash_table->count++;
}

// Koda fragments no ld3
static void dfs(const char* path,HashTable* hash_table)
{
    char abs_path[MAX_PATH];
    DIR* dir=opendir(path);
    if (!dir) return;

    struct dirent* entry;
    while ((entry=readdir(dir))!=NULL) {
        if (strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0) continue;

        size_t l=strlen(path);
        int n;
        if (l>0&&path[l-1]=='/') n = snprintf(abs_path,sizeof(abs_path),"%s%s",path,entry->d_name);
        else n = snprintf(abs_path,sizeof(abs_path),"%s/%s",path,entry->d_name);
        if (n < 0 || (size_t)n >= sizeof(abs_path)) continue;

        if (is_dir(abs_path)) {
            dfs(abs_path,hash_table);
            continue;
        }
        struct stat st;
        lstat(abs_path,&st);
        // Is file?
        if (S_ISREG(st.st_mode)) {
            File* f=create_file(abs_path, &st);
            puts(f->path);
            if (f) hash_table_insert_file(hash_table,f);
        }
    }
    closedir(dir);
}
