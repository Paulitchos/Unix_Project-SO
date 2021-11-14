#include "libraries.h"
#include "structs.h"

int main(int argc, char **argv){
    printf("Ola eu sou o cliente\n");
    printf("My PID is: %d\n", getpid());
    printf("Numero de Argumentos: %d\nArgumentos:\n", argc);
    for (int i = 0; i < argc; i++){
        printf(" %s ", argv[i]);
    }
    return 0;
}