#ifndef STRUCTS
#define STRUCTS

#include "globals.h"

#define WAIT_TIME 15

typedef struct cli{
    char nome[100];
    int nif[9]; // maneira de o identificar
}cliente, *pcliente;

typedef struct med{
    char nome[100];
    int nif;
    char especificacao[20];
    bool disponivel; // perguntar se se pode utilizar
}medico, *pmedico;

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