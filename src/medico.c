#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char **argv){
    printf("Ola eu sou o servidor\n");
    printf("My PID is: %d\n", getpid());
    printf("Numero de Argumentos: %d\nArgumentos:\n", argc);
    for (int i = 0; i < argc; i++){
        printf(" %s ", argv[i]);
    }
    return 0;
}