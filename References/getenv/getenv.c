#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void mostra(char *nome,int idade){
    printf("Eu sou %s e em 2022 %d anos\n",nome,idade);
    return;
}

int main(int argc, char*argv[],char *envp[]) {
    int idade;
    char *res,nome[30];

    res = getenv("NOME");

    if (res != NULL)
        strcpy(nome,res);
    else
        strcpy(nome,"---");

    res = getenv("IDADE");

    if (res != NULL)
        idade = atoi(res);
    else
        idade = 0;

    mostra(nome,idade +1);
    return (0);
} 
