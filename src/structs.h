#ifndef STRUCTS
#define STRUCTS

#define WAIT_TIME 15

# include "libraries.h"

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