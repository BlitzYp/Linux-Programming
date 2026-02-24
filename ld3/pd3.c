#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>
#define MAX_PATH 256

static void die_error(const char* msg)
{
    if (msg&&*msg) fprintf(stderr, "Error: %s\n", msg);
    else fprintf(stderr, "Unknown error!\n");
    exit(-1);
}

static bool is_dir(const char* path) 
{
    struct stat st;
    if (lstat(path, &st)!=0||S_ISLNK(st.st_mode)) return false;
    return S_ISDIR(st.st_mode);
}

static void dfs(char* file_name,char* path)
{
    char abs_path[MAX_PATH];
    DIR* dir=opendir(path);
    if (!dir) return;
    struct dirent* entry;
    while ((entry=readdir(dir))!=NULL) {
        if (strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0) continue;
        // /path/entry->d_name
        size_t l=strlen(path);
        if (l>0&&path[l-1]=='/') snprintf(abs_path,sizeof(abs_path),"%s%s",path,entry->d_name);
        else snprintf(abs_path,sizeof(abs_path),"%s/%s",path,entry->d_name);
        if (strcmp(entry->d_name,file_name)==0) puts(abs_path);
        if (is_dir(abs_path)) dfs(file_name,abs_path);
    }
    closedir(dir);
}

int main(int argc, char** argv)
{
    if (argc!=3) die_error("Usage: ./LSP_PD3_marcis_deguts <file> <directory>");
    char* file_name=argv[1],*dir=argv[2];
    if (!is_dir(dir)) die_error("The second argument is not a directory!");
    dfs(file_name,dir);
    return 0;
}