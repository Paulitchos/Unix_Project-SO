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
}