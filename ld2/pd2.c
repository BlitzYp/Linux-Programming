#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// Es paņēmu bufera izmēru ~1MB
#define BUF_SIZE 1024*1024

static int ask(char** argv);
static int copy_file(int in_fd,int out_fd);
static int write_buffer(int fd,char* buff,size_t size);

int main(int argc,char** argv) 
{
    if (argc!=3) {
        fprintf(stderr,"Vajag šādā formā: %s <input file> <output file>\n",argv[0]);
        return -1;
    }
    char* in=argv[1],*out=argv[2];  
    //printf("%s %s\n",in,out);
    int in_fd=open(in,O_RDONLY);
    if (in_fd<0) {
        fprintf(stderr,"Nevar atvērt failu %s lasīšanai\n",in);
        return -1;
    }
    // Vai eksistē
    if (access(out,F_OK)==0) {
        // Apstiprinājums
        if (!ask(argv)) {
            printf("Pārrakstīšana atcelta!\n");
            close(in_fd);
            return -1;
        }
    }

    int out_fd=open(out,O_WRONLY | O_CREAT | O_TRUNC,0644);
    if (out_fd<0) {
        fprintf(stderr,"Nevar atvērt failu %s rakstīšanai\n",out);
        close(in_fd);
        return -1;
    }    
    // Kopēšana
    if (copy_file(in_fd,out_fd)<0) {
        fprintf(stderr,"Kļūda kopējot failu %s uz %s\n",in,out);
        close(in_fd);
        close(out_fd);
        return -1;
    }
    // Aizveram failus
    if (close(in_fd)!=0) {
        fprintf(stderr,"Nevar aizvērt ievades failu %s!\n",in);
        return -1;
    }
    if (close(out_fd)!=0) {
        fprintf(stderr,"Nevar aizvērt izvades failu %s!\n",out);
        return -1;
    }
    return 0;
}

static int write_buffer(int fd,char* buff,size_t size) 
{
    size_t off=0;
    while (off<size) {
        ssize_t b=write(fd,buff+off,size-off);
        if (b<0) {
            // Ja signāls pārtrauc lasīšanu, turpinām.
            if (errno==EINTR) continue;
            fprintf(stderr,"Kļūda rakstot uz failu: %s\n",strerror(errno));
            return -1;
        }
        if (b==0) return -1;
        
        off+=(size_t)b;
    }
    return 0;
}

static int copy_file(int in_fd,int out_fd) 
{
    char* buff=(char*)malloc(BUF_SIZE);
    if (!buff) {
        fprintf(stderr,"Nevar atvēlēt atmiņu buferim!\n");
        return -1;
    }
    for (;;) {
        ssize_t b=read(in_fd,buff,BUF_SIZE);
        // Nav ko lasīt, beidzam ciklu.
        if (b==0) break;
        if (b<0) {
            if (errno==EINTR) continue;
            fprintf(stderr,"Kļūda lasot no faila: %s\n",strerror(errno));
            free(buff);
            return -1;
        }
        if (write_buffer(out_fd,buff,b)<0) {
            free(buff);
            return -1;
        }
    }
    //printf("Dati veiksmīgi nokopēti!\n");
    free(buff);
    return 0;
}

static int ask(char** argv) 
{
    fprintf(stderr,"Vai tiešām gribat raksīt datus no %s uz %s? (y/n): ",argv[1],argv[2]);
    // Iztīra iepriekšējo ievadi.
    fflush(stderr);
    char c;
    //scanf("%c",&c);
    do {
        c=fgetc(stdin);
    } while (c=='\n' || c==' ');
    if (c=='y' || c=='Y') return 1;
    return 0;
}