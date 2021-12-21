#ifndef STRUCTS
#define STRUCTS

#include "globals.h"

#define WAIT_TIME 15

typedef struct fcli1{ // <from cliente>
    // use unsigned short
    unsigned short size;
    char nome[50];
    pid_t pid_cliente; // maneira de o identificar
    char sintomas[TAM_MAX_MSG]; //sintomas do cliente
}sint_fcli, *psint_fcli;

typedef struct fblc1{ // <from balcao>
    unsigned short size;
    char esp[20]; // <especialidade>
    int prio; // <prioridade>
    int num_peopleAhead; // num de especialistas à frente nessa especialidade
    int num_espOnline; // <numero especialistas online>
}info_fblc, *pinfo_fblc;

typedef struct fmed1{ // <from medico>
    unsigned short size;
    char nome[50];
    pid_t pid_medico; // maneira de o identificar
    char esp[20];
    bool disponivel; // perguntar se pode utilizar
}esp_fmed, *pesp_fmed;

typedef struct fblc2{
    unsigned short size;
    char nome[50];
}connectMed_fblc, *pconnectMed_fblc;

typedef struct fblc3{
    unsigned short size;
    char nome[50];
    pid_t pid_cliente;
}connectCli_fblc, *pconnectCli_fblc;

#endif