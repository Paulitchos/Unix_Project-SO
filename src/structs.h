#ifndef STRUCTS
#define STRUCTS

#include <stdbool.h>

#define WAIT_TIME 15

typedef struct cli{
    char nome[100];
    int nif; // maneira de o identificar
}cliente, *pcliente;

typedef struct med{
    char nome[100];
    int nif;
    char especificacao[20];
    bool disponivel; // perguntar se se pode utilizar
}medico, *pmedico;

#endif