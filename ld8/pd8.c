#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

int n,m;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

void* print_letter(void* arg)
{
    for (int i=0;i<m;i++) {
        //pthread_mutex_lock(&mutex);
        //printf("Printing letter %c from thread %ld\n",*(char*)arg,(long)pthread_self());
        printf("%c\n",*(char*)arg);
        //pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(void)
{
    printf("Enter n and m: ");
    scanf("%d %d",&n,&m);
    if (n<0||m<0) {
        printf("N and M must be positive!");
        exit(-1);
    }
    pthread_t* threads=(pthread_t*)malloc(sizeof(pthread_t)*n);
    char* c=(char*)malloc(n);
    for (int i=0;i<n;i++) {
        c[i]='A'+i;
        pthread_create(&threads[i],NULL, &print_letter, (void*)&c[i]);
    }
    for (int i=0;i<n;i++) pthread_join(threads[i],NULL);
    free(threads);
    free(c);
}
