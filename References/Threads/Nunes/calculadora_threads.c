#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>

#define FIFO_SRV "canal"
#define FIFO_CLI "cli%d"

/*
gcc ./calculadora_threads.c -o /tmp/calculadora_threads -Werror -Wextra -lm -pthread && /tmp/calculadora_threads
// -Wall -Werror -Wextra
*/

typedef struct {
   int num1, num2, pid;
   char op;
   float res;
} PEDIDO;

typedef struct {
   int continua;
   int *clientes;       // Partilhado
   pthread_mutex_t *m;  // Partilhado
} TDADOS;

// void acorda(int s, siginfo_t *info, void *c) { }

/* Thread B (#0) */
void *administrador(void *dados) {
   int i;
   char cmd[30];
   TDADOS *pdados = (TDADOS *) dados;

   do {
      printf("COMANDO?\n");
      scanf("%s", cmd);
      if (strcmp(cmd,"apaga")==0) {
         pthread_mutex_lock(pdados->m);     // Inicio de seccao critica...
         // sleep(30);
         for(i=0;i<5;i++)
            pdados->clientes[i] = -1;
         pthread_mutex_unlock(pdados->m);   // Fim de seccao critica!
      }
   } while (pdados->continua);
   pthread_exit(NULL);
}

/* Thread C (#1) */
void *temporizador(void *dados) {
   int i;
   TDADOS *pdados = (TDADOS *) dados;

   do {
      sleep(15);
      pthread_mutex_lock(pdados->m);     // Inicio de seccao critica...
      for(i=0;i<5;i++)
         printf("cli[%d] = %d\n", i, pdados->clientes[i]);
      pthread_mutex_unlock(pdados->m);   // Fim de seccao critica!
   } while (pdados->continua);
   pthread_exit(NULL);
}

/* Thread A (main) */
int main(int argc, char *argv[]) {
   int fd, fd_retorno, n, res, i;
   char str[30], cmd[30];
   PEDIDO p;
   int cli[5] = {-1, -1, -1, -1, -1};
   pthread_mutex_t mutex;
   pthread_t tid[2];
   TDADOS td[2];

   if (access(FIFO_SRV, F_OK) == 0) {
      printf("[ERRO] O fifo ja existe!\n");
      exit(1);
   }
   mkfifo(FIFO_SRV, 0600);
   printf("Criei o fifo...\n");
   fd = open(FIFO_SRV, O_RDWR);
   printf("Abri o fifo...\n");

   // struct sigaction act;
   // act.sa_sigaction = acorda;
   // act.sa_flags = SA_SIGINFO;
   // sigaction(SIGUSR1, &act, NULL);

   printf("Vou criar as thread...\n");
   pthread_mutex_init(&mutex, NULL);
   td[0].continua = 1;
   td[0].clientes = cli;
   td[0].m = &mutex;
   pthread_create(&tid[0], NULL, administrador, &td[0]);
   td[1].continua = 1;
   td[1].clientes = cli;
   td[1].m = &mutex;
   pthread_create(&tid[1], NULL, temporizador, &td[1]);

   i = 0;
   do {
      n = read(fd, &p, sizeof(PEDIDO));
      printf("Recebi %d bytes...\n", n);
      if (n == sizeof(PEDIDO)) {
         printf("%d %c %d = ? (%d)\n", p.num1, p.op, p.num2, p.pid);
         p.res = p.num1 + p.num2;
         sprintf(str, FIFO_CLI, p.pid);
         fd_retorno = open(str, O_WRONLY);
         n = write(fd_retorno, &p, sizeof(PEDIDO));
         close(fd_retorno);
         printf("Enviei %d bytes...\n", n);
     	 pthread_mutex_lock(&mutex);     // Inicio de seccao critica...
         cli[i++ % 5] = p.pid;
     	 pthread_mutex_unlock(&mutex);   // Fim de seccao critica!

      }
   } while (p.op != 'Q');

   printf("Vou terminar as thread...\n");
   td[0].continua = 0;
   // pthread_kill(tid[0], SIGUSR1);
   pthread_join(tid[0], NULL);
   td[1].continua = 0;
   pthread_join(tid[1], NULL);
   pthread_mutex_destroy(&mutex);

   close(fd);
   printf("Fechei o fifo...\n");
   unlink(FIFO_SRV);
   printf("Apaguei o fifo...\n");

   exit(0);
}
