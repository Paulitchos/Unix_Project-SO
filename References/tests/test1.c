#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char*argv[],char *envp[]) {
    char string[20] = "                   ";
    strcpy(string,"ola p\n");
    string[18] = '\0';
    string[6] = ' ';
    printf("%lu\t%lu", strlen(string), sizeof(string));
    // sizeof string retorna o tamanho alocado para a string
    // strlen retorna os bytes até chegar ao \0 (sem incluir o \0)
    return 0;
} 
