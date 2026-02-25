#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define NAME_FIRST_LETTER     'M'
#define SURNAME_FIRST_LETTER  'D'

#define BUFFER_SIZE  100

// -----------------------------------------------------
// Hash function, using Jenkin's hash as base

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

// Length must be a multiple of 4 !
unsigned hashData(const void *x, unsigned length, unsigned seed) {
    const unsigned *k = (const unsigned *) x;
    unsigned a,b,c;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + length + seed;
    c += seed; // reuse the same seed

    length >>= 2;

    /*------------------------------------------------- handle most of the key */
    while (length > 3) {
      a += k[0];
      b += k[1];
      c += k[2];
      mix(a,b,c);
      length -= 3;
      k += 3;
    }

    /*------------------------------------------- handle the last 3 uint32_t's */
    switch(length) {    /* all the case statements fall through */
    case 3 : c+=k[2];
    case 2 : b+=k[1];
    case 1 : a+=k[0];
      final(a,b,c);
    case 0:     /* case 0: nothing left to add */
      break;
    }

    /*------------------------------------------------------ report the result */
    return a ^ b ^ c;
}

// -----------------------------------------------------

static char staticBuffer[BUFFER_SIZE];

void loopForever(void)
{
    for (;;);
}

int main(void)
{
    unsigned data[] = {NAME_FIRST_LETTER, SURNAME_FIRST_LETTER};
    unsigned hash, seed;
    char *buffer;

    const char *infilename;
    const char *outfilename;

    // decide what to do
    srand(0);
    seed = rand();
    hash = hashData(data, sizeof(data), seed);
    switch (hash % 3) {
    case 0:
      infilename = "/dev/random";
      buffer = malloc(BUFFER_SIZE);
      break;
    case 1:
      infilename = "/dev/urandom";
      buffer = malloc(BUFFER_SIZE);
      break;
    case 2:
    default:
      infilename = "/dev/zero";
      buffer = staticBuffer;
      break;
    }
    outfilename = "pd4-output.dat";

    // try to open files
    FILE *input = fopen(infilename, "r");
    FILE *output = fopen(outfilename, "w");
    if (!input || !output) {
      if (input) fclose(input);
      if (output) fclose(output);
      return -1;
    }

    // unblock input file
    if (hash == 0) {
      int fd = fileno(input);
      fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    }

    // read & write data
    size_t count = fread(buffer, 1, BUFFER_SIZE, input);
    if (count > 0) {
      fwrite(buffer, 1, count, output);
    }

    // free memory if it was allocated
    if (buffer != staticBuffer) free(buffer);
    // close files
    fclose(input);
    fclose(output);
    // never return
    loopForever();
    // but make compiler happy
    return 0;
}
