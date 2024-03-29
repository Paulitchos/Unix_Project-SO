#ifndef STRUCTS
#define STRUCTS

#include "globals.h"

#define WAIT_TIME 15

typedef struct fcli1 sint_fcli, *psint_fcli;   
struct fcli1{ // <from cliente>
    unsigned short size;
    char nome[50];
    pid_t pid_cliente; // maneira de o identificar
    char sintomas[TAM_MAX_MSG]; //sintomas do cliente
};

typedef struct fblc1{ // <from balcao>
    unsigned short size;
    char esp[20]; // <especialidade>
    int prio; // <prioridade>
    unsigned int num_peopleAhead; // num de especialistas à frente nessa especialidade
    int num_espOnline; // <numero especialistas online> da sua area
}info_fblc, *pinfo_fblc;

typedef struct fmed1{ // <from medico>
    unsigned short size;
    char nome[50];
    pid_t pid_medico; // maneira de o identificar
    char esp[20];
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

typedef struct death{
    unsigned short size;
    pid_t pid;
}imDead, *pimDead;

typedef struct frequencia{
    unsigned short size;
    short freq;
}freq_tmed, *pfreq_tmed;

typedef struct commitDie{ // struct ID: 20001
    unsigned short size;
    bool info;
}suicide, *psuicide;

typedef struct how_to_connect{
    unsigned short size;
}connect, *pconnect;

typedef struct msg_entre_cli_e_med{
    unsigned short size;
    char msg[TAM_MAX_MSG];
}msg, *pmsg;

typedef struct I_am_free{ // struct ID: 20000
    unsigned short id;
    pid_t pid;
}available, *pavailable;

#endif