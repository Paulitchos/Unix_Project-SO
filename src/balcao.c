#include "structs.h"
#include "globals.h"

#define TAM 50

char* dicionario[7][2] = {
	{"memory", "memória"},
	{"computer","computador"},
	{"close","fechar"},
	{"open","abrir"},
	{"read","ler"},
	{"write","escrever"},
	{"file","ficheiro"}};

int	s_fifo_fd, c_fifo_fd;

// Run balcao:
/*
    gcc ./src/balcao.c -o ./dist/balcao && ./dist/balcao
*/

void trata_SIGPIPE(int s) {
    static int a; 
    a++;
    //SIGPIPE is the "broken pipe" signal, which is sent to a process when it attempts to write to a pipe whose read end has closed (or when it attempts to write to a socket that is no longer open for reading), but not vice versa. The default action is to terminate the process.
    printf("Recebido sinal SIGPIPE %d ",a); // fflush
}

void trata_SIGINT(int i) { // CTRL + C
	(void) i;    //todo what?
	fprintf(stderr, "\nServidor de dicionário a terminar "
									"(interrompido via teclado)\n\n");
	close(s_fifo_fd);
	unlink(SERVER_FIFO);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv,  char *envp[]){
    setbuf(stdout, NULL);
    // ============== Treat Signals ==============
    if (signal(SIGPIPE,trata_SIGPIPE) == SIG_ERR)
        perror("\nWARNING: Não foi possível configurar sinal SIGPIPE\n");
    fprintf(stderr, "\nSinal SIGPIPE configurado");
    if (signal(SIGINT,trata_SIGINT) == SIG_ERR) {
        perror("\nNão foi possível configurar sinal SIGINT!\n");
        exit(EXIT_FAILURE); }
    fprintf(stderr, "\nSinal SIGINT configurado");
    // ============== ==============

    /*
    // ============== Get environment variables ==============
    char name[100];
    int maxclientes;
    int maxmedicos;
    char *res_maxclientes = getenv("MAXCLIENTES");
    char *res_maxmedicos = getenv("MAXMEDICOS");
    if (res_maxclientes != NULL)
        maxclientes = atoi(res_maxclientes);
    else
        maxclientes = 10;
    if (res_maxmedicos != NULL)
        maxmedicos = atoi(res_maxmedicos);
    else
        maxmedicos = 10;
    // ============== ============== ============== 

    printf("Ola eu sou o balcao\n");
    printf("My PID is: %d\n", getpid());
    printf("MAXCLIENTES:%d\n",maxclientes);
    printf("MAXMEDICOS:%d\n",maxmedicos);

    int f_res;
    int fd[2]; // file descriptor | input, output
    pipe(fd);
    int df[2];
    pipe(df);
    f_res = fork();
    if (f_res == 1) {
        perror("erro fork: ");
        return 2;
    }
    
    if (f_res == 0) { //CHILD
        // doesn't need writing side - close
        close(STDIN_FILENO); // stdin is a FILE *, thus you need to use STDIN_FILE_NUMBER instead of stdin
        // Accesses first element of the following table:
        //     0      1       2
        // ┌──────┬───────┬───────┬────┬────┐
        // │stdin │stdout |stderr │ <rest is empty>
        // └──────┴───────┴───────┴────┴────┘ 
        dup(fd[0]);
        close(fd[0]);
        close(fd[1]);

        close(STDOUT_FILENO);
        dup(df[1]);
        close(df[0]);
        close(df[1]);
        
        execlp("./classificador", "classificador", NULL); // executes calssificador
        perror("erro exec prog2: ");
        return 3;
    } else { // PARENT
        close(fd[0]); // has the duplicated, doesn't need this one
        close(df[1]);

        int tam = TAM;
        char phrase_client [TAM];

        do{
            tam = read(STDIN_FILENO,phrase_client,sizeof(phrase_client)); // read from user
            if (tam <= -1 ) { printf("Error Reading, output: %d\n",tam); return 1; }
            phrase_client[tam] = '\0';

            if(write(fd[1], phrase_client, strlen(phrase_client)) <= -1){ // write to pipe, write is waiting for amount of characters
                printf("Error Writing\n")
                ; return 1; }
            // write uses strlen to not write unecessary data as we know all the data we want to write
            
            tam = read(df[0],phrase_client,sizeof(phrase_client)); // read from pipe, tam = sizeof(phrase if all is fine) | -1 i error while reading, 0 is unexpected EOF
            if (tam <= -1 ) { printf("Error Reading, output: %d\n",tam); return 1; }
            // read uses sizeof because we don't know what we'll recieve àpriori, so we send the max size of the string
            phrase_client[tam] = '\0';

            printf("%.*s",tam,phrase_client);
        } while (true);    
        return 4;
    }
    */

    // ===========================
    pergunta_t	perg;
	resposta_t	resp;

	int	res;
	int	i;
	char	c_fifo_fname[50];
	printf("\nServidor de dicionário");

	res = mkfifo(SERVER_FIFO, 0777);
	if (res == -1)
	{
		perror("\nmkfifo do FIFO do servidor deu erro");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "\nFIFO servidor criado");

	/* prepara FIFO do servidor */
	/* abertura read+write -> evita o comportamento de ficar	*/
	/* bloqueado no open. a execução prossegue e as						*/
	/* operações read/write (neste caso apenas READ)					*/
	/* continuam bloqueantes (mais fácil de gerir)						*/
	s_fifo_fd = open(SERVER_FIFO, O_RDWR);
	if (s_fifo_fd == -1)
	{
		perror("\nErro ao abrir o FIFO do servidor (RDWR/blocking)");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "\nFIFO aberto para READ (+WRITE) bloqueante");
	
	/* ciclo principal: read pedido -> write resposta -> repete */
	while (1) /* sai via SIGINT */
	{
		/* ---- OBTEM PERGUNTA ---- */
		res = read(s_fifo_fd, &perg, sizeof(perg));
		if (res < (int) sizeof(perg))
		{
			fprintf(stderr, "\nRecebida pergunta incompleta "
											"[bytes lidos: %d]", res);
			continue; /* não responde a cliente (qual cliente?) */
		}
		fprintf(stderr, "\nRecebido [%s]", perg.palavra);

		/* ---- PROCURA TRADUÇÃO ---- */
		strcpy(resp.palavra, "DESCONHECIDO"); /* caso não encontre */
		for (i = 0; i < (int)7; i++)
		{
			if (!strcasecmp(perg.palavra, dicionario[i][0]))
			{
				strcpy(resp.palavra, dicionario[i][1]);
				break;
			}
		}
		fprintf(stderr, "\nResposta = [%s]", resp.palavra);

		/* ---- OBTÉM FILENAME DO FIFO PARA A RESPOSTA ---- */
		sprintf(c_fifo_fname, CLIENT_FIFO, perg.pid_cliente);

		/* ---- ABRE O FIFO do cliente p/ write ---- */
		c_fifo_fd = open(c_fifo_fname, O_WRONLY);
		if (c_fifo_fd == -1)
			perror("\nErro no open - Ninguém quis a resposta");
		else
		{
			fprintf(stderr, "\nFIFO cliente aberto para WRITE");

			/* ---- ENVIA RESPOSTA ---- */
			res = write(c_fifo_fd, &resp, sizeof(resp));
			if (res == sizeof(resp))
				fprintf(stderr, "\nescreveu a resposta");
			else
				perror("\nerro a escrever a resposta");

			close(c_fifo_fd); /* FECHA LOGO O FIFO DO CLIENTE! */
			fprintf(stderr, "\nFIFO cliente fechado");
		}
	} /* fim do ciclo principal do servidor */

	/* em princípio não chega a este ponto - sai via SIGINT */
	close(s_fifo_fd);
	unlink(SERVER_FIFO);
	return (0);
}