#include "structs.h"
#include "globals.h"

#define TAM 50

char* dicionario[7][2] = {
	{"memory\n", "memória"},
	{"computer\n","computador"},
	{"close\n","fechar"},
	{"open\n","abrir"},
	{"read\n","ler"},
	{"write\n","escrever"},
	{"file\n","ficheiro"}};

int	npb, npc;

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
	close(npb);
	unlink(BALCAO_FIFO);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv,  char *envp[]){

	// ======== Checking Arguments ======== //
	bool debugging = false;
	bool success = false;
	if (argc==1)
		success = true;
	else
		for (int i = 1; i < argc; i++) {
			if (!(strcmp(argv[i],"--debugging")) || !(strcmp(argv[i],"-D"))){
				printf("==Debugging mode activated==\n");
				success = true;
				debugging = true;
				break;
			}
		}
	if (!success){
		printf("Opcoes existentes: --debugging ou -D\n");
		printf("Exemplo ./balcao --debugging\n");
		exit(EXIT_SUCCESS);
	}
    // ======== ================== ======== //

    setbuf(stdout, NULL); // if the buffer argument is NULL, the stream is unbuffered
    // ============== Treat Signals ============== //
    if (signal(SIGPIPE,trata_SIGPIPE) == SIG_ERR)
        perror("\nNão foi possível configurar sinal SIGPIPE\n");
    if (debugging) fprintf(stderr, "==Sinal SIGPIPE configurado==\n");
    if (signal(SIGINT,trata_SIGINT) == SIG_ERR) {
        perror("\nNão foi possível configurar sinal SIGINT!\n");
        exit(EXIT_FAILURE); }
    if (debugging) fprintf(stderr, "==Sinal SIGINT configurado==\n");
    // ============== ============= ============== //

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
    sint_fcli sint_fcli; // <sintomas_fromClient> <sintomas_FromClient>
	info_fblc info_tcli; // <info_fromBalcao> <info_ToClient>
	sint_fcli.size = sizeof(sint_fcli);
	info_tcli.size = sizeof(info_fblc);

	int	res;
	int	i;
	char cFifoName[50];
	printf("Balcão!\n");

	res = mkfifo(BALCAO_FIFO, 0777);
	if (res == -1){	perror("\nmkfifo do FIFO do servidor deu erro"); exit(EXIT_FAILURE); }
	if (debugging) fprintf(stderr, "==FIFO servidor criado==\n");

	npb = open(BALCAO_FIFO, O_RDWR); // opens in Read/Write mode to prevent getting stuck in open, it's still blocking
	if (npb == -1){ perror("\nErro ao abrir o FIFO do servidor (RDWR/blocking)\n"); exit(EXIT_FAILURE);	}
	if (debugging) fprintf(stderr, "==FIFO aberto para READ (+WRITE) BLOQUEANTE==\n");
	
	// ciclo principal: read pedido -> write resposta -> repete
	while (1) {
		/* ---- OBTEM PERGUNTA ---- */
		res = read(npb, &sint_fcli, sizeof(sint_fcli));
		if (res < (int) sizeof(sint_fcli)){
			fprintf(stderr, "Recebida pergunta incompleta "
											"[bytes lidos: %d]\n", res);
			continue; // não responde a cliente (qual cliente?)
		}
		if (debugging) fprintf(stderr, "==Recebido [%s]==\n", sint_fcli.sintomas);

		// ---- PROCURA TRADUÇÃO ----
		strcpy(info_tcli.msg, "DESCONHECIDO"); // caso não encontre
		for (i = 0; i < (int)7; i++){
			if (!strcasecmp(sint_fcli.sintomas, dicionario[i][0])){
				strcpy(info_tcli.msg, dicionario[i][1]);
				break;
			}
		} 
		
		if (debugging) fprintf(stderr, "==Resposta = [%s]==\n", info_tcli.msg);

		// Get Filename of client's FIFO to send response
		sprintf(cFifoName, CLIENT_FIFO, sint_fcli.pid_cliente);

		// Opens clients FIFO for write
		npc = open(cFifoName, O_WRONLY);
		if (npc == -1) perror("Erro no open - Ninguém quis a resposta\n");
		else{
			if (debugging) fprintf(stderr, "==FIFO cliente aberto para WRITE==\n");

			// Send response
			res = write(npc, &info_tcli, sizeof(info_tcli));
			if (res == sizeof(info_tcli) && debugging) // if no error
				fprintf(stderr, "==escreveu a resposta com sucesso==\n");
			else
				perror("erro a escrever a resposta\n");

			close(npc); /* FECHA LOGO O FIFO DO CLIENTE! */
			if (debugging) fprintf(stderr, "==FIFO cliente fechado==\n");
		}
	} /* fim do ciclo principal do servidor */

	/* em princípio não chega a este ponto - sai via SIGINT */
	close(npb);
	unlink(BALCAO_FIFO);
	return (0);
}