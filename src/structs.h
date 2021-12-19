#ifndef STRUCTS
#define STRUCTS

#include "globals.h"

#define WAIT_TIME 15

typedef struct fcli1{
    long long size;
    char nome[50];
    pid_t pid_cliente; // maneira de o identificar
    char sintomas[TAM_MAX_MSG]; //sintomas do cliente
}sint_fcli, *psint_fcli;

typedef struct fblc1{
    long long size;
    char esp[20]; // <especialidade>
    int prio; // <prioridade>
    int num_peopleAhead; // num de especialistas à frente nessa especialidade
    int num_espOnline; // <numero especialistas online>
}info_fblc, *pinfo_fblc;

typedef struct fmed{
    char nome[50];
    int nif;
    char especificacao[20];
    bool disponivel; // perguntar se pode utilizar
}medico, *pmedico;

#endif