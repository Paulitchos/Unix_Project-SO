#ifndef STRUCTS
#define STRUCTS

#include "globals.h"

#define WAIT_TIME 15

typedef struct fcli1{
    int size;
    char nome[50];
    pid_t pid_cliente; // maneira de o identificar
    char sintomas[TAM_MAX_MSG];
}sint_fcli, *psint_fcli;

typedef struct fblc1{
    int size;
	char msg[TAM_MAX_MSG];
}info_fblc, *pinfo_fblc;

typedef struct fblc2{
	char msg[TAM_MAX_MSG];
}info_fblc2, *pinfo_fblc2;


typedef struct fmed{
    char nome[50];
    int nif;
    char especificacao[20];
    bool disponivel; // perguntar se pode utilizar
}medico, *pmedico;



// tmp
/* estrutura de mensagem correspondente a um pedido cliente -> servidor*/
typedef struct {
	pid_t	pid_cliente;
	char	palavra[50];
} pergunta_t;

/* estrutura da mensagem correspondente a uma resposta do servidor */
typedef struct {
	char	palavra[50];
} resposta_t;

#endif