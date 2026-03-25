#include <stdio.h>
#include <setjmp.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{

   // tālāko kodu nemainīt!

    // taimeris uz 3 sekundeem
    alarm(3);

    // bezgalīgs cikls
    while (1);

    // atgriež kaut kādu nenulles kodu, lai kompilators nesūdzētos
    return -1;
   return 0;
}
