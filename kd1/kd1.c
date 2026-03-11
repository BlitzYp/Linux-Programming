#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 4096 // 4KB ir praktisks bufera izmērs, pietiekami liels ātram IO.

typedef enum {
    XOR,
    CYPHER,
    TRANSLATE
} Mode;

// Programmas konfigurācija/dati
typedef struct {
    const char* t_file;
    const char* s_file;
    const char* o_file;
    const char* input_file;
    Mode m;
} Flags;

static unsigned char buff[BUF_SIZE];
static unsigned char map[256];
static unsigned char translate_buff[BUF_SIZE];
static unsigned char lieks;

static int parse(int argc, char** argv, Flags* opt);
static int open_input(const char* filename);
static int open_output(const char* filename);
static int init_map(Flags* opt);
static int write_out(int in_fd, int out_fd);
static int handle_translate_map(const char* filename);
static void print_usage(const char* progname);

/*
    1. Pārsē ievadu
    2. Inicializē map atbilstoši izvēlētajam režīmam
    3. Raksta iztulkoto uz izvadu
*/
int main(int argc, char** argv)
{
    Flags opt={NULL,NULL,NULL,NULL,XOR};
    int in_fd, out_fd;

    if (parse(argc, argv, &opt)!=0) {
        print_usage(argv[0]);
        return 1;
    }

    if (init_map(&opt)!=0) return 1;

    in_fd=open_input(opt.input_file);
    if (in_fd<0) return 1;
   
    out_fd=open_output(opt.o_file);
    if (out_fd<0) {
        if (in_fd!=STDIN_FILENO) close(in_fd);
        return 1;
    }

    if (write_out(in_fd, out_fd)!=0) {
        if (in_fd!=STDIN_FILENO) close(in_fd);
        if (out_fd!=STDOUT_FILENO) close(out_fd);
        return 1;
    }

    if (in_fd!=STDIN_FILENO) close(in_fd);
    if (out_fd!=STDOUT_FILENO) close(out_fd);
    return 0;
}

static void print_usage(const char* progname)
{
    fprintf(stderr,"Lietojums: %s [-t translation-file] [-s cypher-table] [-o output-file] [input-file]\n",progname);
}

static int parse(int argc, char** argv, Flags* opt)
{
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "-s")==0) {
            if (i+1>=argc) {
                fprintf(stderr, "Kluda: pec -s nav noradits fails\n");
                return -1;
            }
            opt->s_file=argv[++i];
            opt->m=CYPHER;
        }
        else if (strcmp(argv[i], "-t")==0) {
            if (i+1>=argc) {
                fprintf(stderr, "Kluda: pec -t nav noradits fails\n");
                return -1;
            }
            opt->t_file=argv[++i];
            opt->m=TRANSLATE;
        }
        else if (strcmp(argv[i], "-o")==0) {
            if (i+1>=argc) {
                fprintf(stderr, "Kluda: pec -o nav noradits fails\n");
                return -1;
            }
            opt->o_file=argv[++i];
        }
        else if (argv[i][0]=='-') {
            fprintf(stderr, "Kluda: nezinams parametrs %s\n", argv[i]);
            return -1;
        }
        else {
            if (opt->input_file!=NULL) {
                fprintf(stderr, "Kluda: parak daudz ievades failu\n");
                return -1;
            }
            opt->input_file=argv[i];
        }
    }

    if (opt->t_file!=NULL && opt->s_file!=NULL) {
        fprintf(stderr, "Kluda: -t un -s nedrikst lietot vienlaicigi\n");
        return -1;
    }

    return 0;
}

static int handle_translate_map(const char* filename) 
{
    int fd=open(filename,O_RDONLY);
    if (fd<0) {
        fprintf(stderr, "Kluda atverot tulkosanas failu '%s': %s\n", filename, strerror(errno));
        return -1;
    }

    // Nolasa visu translācijas failu statiskajā buferī.
    ssize_t total=0, n;
    while ((n=read(fd,translate_buff+total,BUF_SIZE-total))>0) {
        total+=n;
        if (total==BUF_SIZE) {
            ssize_t tmp=read(fd,&lieks,1);
            if (tmp>0) {
                fprintf(stderr,"Translacijas fails neietilpst buferi!\n");
                close(fd);
                return -1;
            }
            break;
        }
    }
    if (n<0) {
        fprintf(stderr, "Kluda lasot tulkosanas failu '%s': %s\n",filename, strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    if (total<=0) {
        fprintf(stderr,"Translacijas fails ir tukss!\n");
        return -1;
    }

    // Atdalītājs
    unsigned char atd=translate_buff[0];
    // Sākam ar identitātes karti un tad pārrakstām tikai vajadzīgos simbolus.
    for (int i=0;i<256;i++) map[i]=(unsigned char)i;

    // Meklējam pirmās virknes beigas, ko atdala tas pats pirmais baits.
    int from_start=1,from_end=-1;
    for (int i=from_start;i<total;i++) {
        if (translate_buff[i]==atd) {
            from_end=i;
            break;
        }
    }
    if (from_end<0) {
        fprintf(stderr,"Nav atrasts otrs atdalitajs\n");
        return -1;
    }
    int to_start=from_end+1;
    int to_len=total-to_start,from_len=from_end-from_start; 

    if (!(to_len==from_len||to_len==from_len+1)) {
        fprintf(stderr,"Formats translacijas failam nav pareizs!(nesakrit virknes garumi)\n");
        return -1;
    }

    // Pirmā virkne satur ko tulkot, otrā uz ko tulkot.
    for (int i=0;i<from_len;i++) map[translate_buff[from_start+i]]=translate_buff[to_start+i];

    // Papildu baits faila beigās nozīmē paša atdalītāja tulkojumu.
    if (from_len+1==to_len) map[atd]=translate_buff[to_start+from_len];
    return 0; 
}

static int init_map(Flags* opt)
{
    int fd;
    ssize_t total;
    if (opt->m==XOR) {
        // Noklusētais režīms apgriezts kreisais bits katram baitam.
        for (int i=0; i<256; i++) map[i]=(unsigned char)i^0x80;
        return 0;
    }
    if (opt->m==CYPHER) {
        fd=open(opt->s_file, O_RDONLY);
        if (fd<0) {
            fprintf(stderr, "Kluda atverot sifresanas failu '%s': %s\n", opt->s_file, strerror(errno));
            return -1;
        }

        // Šifrēšanas fails ir gatava 256 baitu tabula.
        total=read(fd,map,256);
        if (total<0) {
            fprintf(stderr, "Kluda lasot sifresanas failu '%s': %s\n", opt->s_file, strerror(errno));
            close(fd);
            return -1;
        }
        if (total!=256) { 
            fprintf(stderr, "Kluda! Cypher tabulai jabut 256 baitiem!\n");
            close(fd);
            return -1;
        }

        // Ja fails ir lielāks par 256, tad arī kļūda.
        total=read(fd,&lieks,1);
        if (total<0||total!=0) {
            fprintf(stderr, "Kluda! Cypher tabulai jabut 256 baitiem!\n");
            close(fd);
            return -1;
        }
        close(fd);
        return 0;
    }
    if (opt->m==TRANSLATE) {
        if (handle_translate_map(opt->t_file)!=0) return -1;
    } 
    return 0;
}

static int open_input(const char* filename)
{
    if (filename==NULL) return STDIN_FILENO;
    int fd=open(filename, O_RDONLY);
    if (fd<0) {
        fprintf(stderr, "Kluda atverot ievades failu '%s': %s\n", filename, strerror(errno));
        return -1;
    }

    return fd;
}

static int open_output(const char* filename)
{

    if (filename==NULL) return STDOUT_FILENO;
    int fd=open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd<0) {
        fprintf(stderr, "Kluda atverot izvades failu '%s': %s\n",filename, strerror(errno));
        return -1;
    }
    return fd;
}

static int write_out(int in_fd, int out_fd)
{
    ssize_t n,written,total;
    while ((n=read(in_fd, buff, BUF_SIZE))>0) {
        // Katru ievades baitu aizstāj ar atbilstošo vērtību no map.
        for (ssize_t i=0;i<n;i++) buff[i]=map[buff[i]];

        // write var izrakstīt tikai daļu bufera, tāpēc rakstām ciklā.
        total=0;
        while (total<n) {
            written=write(out_fd,buff+total,n-total);
            if (written<=0) {
                fprintf(stderr, "Kluda rakstot izvadi: %s\n", strerror(errno));
                return -1;
            }
            total+=written;
        }
    }

    if (n<0) {
        fprintf(stderr, "Kluda lasot ievadi: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}
