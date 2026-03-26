#include <stdio.h>
#include <setjmp.h>
#include <unistd.h>
#include <signal.h>

static jmp_buf env;

static void handle_signal(int s)
{
    if (s==SIGALRM) longjmp(env,1);
}

int main(void)
{

    struct sigaction action={0};
    action.sa_handler=handle_signal;
    sigaction(SIGALRM,&action,NULL);

    // Pirmajā reizē atgriež 0, katrā nākamā atgriež 1
    if (setjmp(env)!=0) return 0;

    // tālāko kodu nemainīt!

    // taimeris uz 3 sekundeem
    alarm(3);

    // bezgalīgs cikls
    while (1);

    // atgriež kaut kādu nenulles kodu, lai kompilators nesūdzētos
    return -1;
}
