#ifndef STRUCTS
#define STRUCTS

#include "globals.h"

#define WAIT_TIME 15

typedef struct fcli{
    char nome[50];
    pid_t pid_cliente; // maneira de o identificar
    char sintomas[100];
}fromCliente, *pfromCliente;

typedef struct tcli{
	char msg[50];
}toCliente, *ptoCliente;

typedef struct med{
    char nome[100];
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