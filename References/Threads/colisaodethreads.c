#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <limits.h>

//cliente
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#include <pthread.h>

#include <stdbool.h>

#include <locale.h>

/*
run with:
gcc colisaodethreads.c -o /tmp/colisaodethreads -pthread && /tmp/colisaodethreads <max> <threads>
Ex:
gcc colisaodethreads.c -o /tmp/colisaodethreads -pthread && time /tmp/colisaodethreads 1000000 4
wait (esperar)
release (assinalar)
*/

bool mutexworking = true;

void * identificaPares(void *);

typedef struct{
    pthread_t tid;
    int * current;
    int last;
    int found;
    int * allFound;
    pthread_mutex_t * pMutCurr;
    pthread_mutex_t * pMutFound;
} TDADOS;

#define MAXIMUMT 20

int main (int argc, char * argv[]){
    int NTHREADS, last;
    TDADOS workers[MAXIMUMT];

    pthread_mutex_t mutCurr = PTHREAD_MUTEX_INITIALIZER;  //mutexworking 
    pthread_mutex_t mutFound = PTHREAD_MUTEX_INITIALIZER;  //mutexworking

    int mainFound = 0;
    int current = 0;
    int allFound = 0;

    int i;
    char buffer[30];

    if(argc < 3){
        printf("\n%s <max> <threads>\n", argv[0]);
        return 1;
    }
    last = atoi(argv[1]);
    NTHREADS = atoi(argv[2]);
    
    printf("\nNum last = %d",last);
    printf("\nNum Threads = %d",NTHREADS);
    printf("\n");
    
    // if (mutexworking) mutCurr = PTHREAD_MUTEX_INITIALIZER;  //mutexworking
    // if (mutexworking) mutFound = PTHREAD_MUTEX_INITIALIZER;  //mutexworking
    // pthread_mutex_init(& mutCurr, Null); 
    // pthread_mutex_init(& mutFound, NULL);

    for (i = 0 ; i < NTHREADS ; i++) {
        workers[i].current = & current;
        workers[i].last = last;

        if (mutexworking) workers[i].pMutCurr = &mutCurr;  //mutexworking
        if (mutexworking) workers[i].pMutFound = &mutFound;  //mutexworking

        workers[i].allFound = & mainFound;
        pthread_create(& workers[i].tid, NULL, identificaPares, workers + i);
    }
    // aguarda N segundos
    printf("\nThreads iniciados\n");

    // aguarda threads terminarem
    for (i=0; i<NTHREADS; ++i) {
        pthread_join(workers[i].tid, NULL);
        printf("\n%d -> found = %d", i, workers[i].found);
        allFound += workers[i].found;
    }
    setlocale(LC_NUMERIC, "");



    printf("\n\n Threads sum all found = %'d",allFound);
    printf("\n\n main found = %d",mainFound);
    printf("\n\n real found should be = %d",(last-1+1)/2);
    printf("\n\n");
    return 0;

}

void * identificaPares(void * p){
    TDADOS * myDados = (TDADOS *) p;
    int i, myCurrent, myFound = 0;

    while (1) { // ñ é infinito porque tem condicao de paragem
        myCurrent = *(myDados->current);

        // obtem prox numero
        if (mutexworking) pthread_mutex_lock(myDados->pMutCurr);  //mutexworking
        if (myCurrent >= myDados->last){
            if (mutexworking) pthread_mutex_unlock(myDados->pMutCurr); //mutexworking
            break;
        }
        ++(*(myDados->current));
        if (mutexworking) pthread_mutex_unlock(myDados->pMutCurr); //mutexworking

        // verifica se é par
        if (myCurrent % 2 == 0){
            ++myFound;
            if (mutexworking) pthread_mutex_lock(myDados->pMutFound);  //mutexworking
            ++(*(myDados->allFound));
            if (mutexworking) pthread_mutex_unlock(myDados->pMutFound); //mutexworking
        }
    }
    myDados->found = myFound;
    return NULL;
}