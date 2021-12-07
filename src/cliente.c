#include "structs.h"
#include "globals.h"


int main(int argc, char **argv){

    fromCliente fcli;
    toCliente tcli;
    int tam;

    if (argc != 2){                                     //caso tenho mais que um argumento
        printf("Deve introduzir apenas o seu nome\n");
        printf("Exemplo ./cliente <nome>\n");
        exit(EXIT_SUCCESS);
    }
     
    sscanf(argv[1], "%s", &fcli.nome); // vai buscar ao argv[1] o nome do cliente

    printf("%s\n",fcli.nome);

    fcli.pid_cliente = getpid(); // guarda na struct fromCliente o PID único de casa cliente
    printf("My PID is: %d\n", fcli.pid_cliente);

    printf("Quais sao os seus sintomas?\n");

    tam = read(STDIN_FILENO,fcli.sintomas,sizeof(fcli.sintomas)); // read from user
    if (tam <= -1 ) { printf("Error Reading, output: %d\n",tam); return 1; }
    fcli.sintomas[tam] = '\0';

    printf("\n%s\n",fcli.sintomas);


    //sprintf(c_fifo_fname, CLIENT_FIFO, fcli.pid_cliente); // atualiza o nome do fifo deste cliente

    // ===========================
    int	npb;	/* identificador do FIFO do servidor */
	int	npc;	/* identificador do FIFO do cliente */
	//pergunta_t fcli;	/* mensagem do tipo "pergunta_t" */
	//resposta_t	resp;	/* mensagem do tipo "resposta_t" */
	char c_fifo_fname[25];	/* nome do FIFO deste cliente */
	int	read_res;

	/* cria o FIFO deste cliente */
	fcli.pid_cliente = getpid();
	sprintf(c_fifo_fname, CLIENT_FIFO, fcli.pid_cliente);
	if (mkfifo(c_fifo_fname, 0777) == -1){
		perror("\nmkfifo FIFO cliente deu erro");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "\nFIFO do cliente criado");

	/* abre o FIFO do servidor para escrita */
	npb = open(SERVER_FIFO, O_WRONLY); /* bloqueante */
	if (npb == -1){
		fprintf(stderr, "\nO servidor não está a correr\n");
		unlink(c_fifo_fname);
		exit(EXIT_FAILURE);
	}
    fprintf(stderr, "\nFIFO do servidor aberto WRITE / BLOCKING");

	/* abertura read+write -> evita o comportamento de ficar	*/
	/* bloqueado no open. a execução prossegue logo mas as		*/
	/* operações read/write (neste caso APENAS READ)			*/
	/* continuam bloqueantes (mais fácil)						*/
	npc = open(c_fifo_fname, O_RDWR);	/* bloqueante */
	if (npc == -1){
		perror("\nErro ao abrir o FIFO do cliente");
		close(npb);
		unlink(c_fifo_fname);
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "\nFIFO do cliente aberto para READ (+WRITE) BLOCK");

	memset(fcli.sintomas, '\0', TAM_MAX);

	while (1){
		/* "fim" para terminar cliente */
		/* ---- a) OBTÉM fcliUNTA ---- */
		printf("\nPalavra a traduzir -> ");
		scanf("%s", fcli.sintomas);
		if (!strcasecmp(fcli.sintomas, "fim"))
			break;

		/* ---- b) ENVIA A fcliUNTA ---- */
		write(npb, &fcli, sizeof(fcli));
		/* ---- c) OBTÉM A RESPOSTA ---- */
		read_res = read(npc, &tcli, sizeof(tcli));
		if (read_res == sizeof(tcli))
			printf("\nTradução -> %s", tcli.msg);
		else
			printf("\nSem resposta ou resposta incompreensível"
						"[bytes lidos: %d]", read_res);
	}

	close(npc);
	close(npb);
	unlink(c_fifo_fname);

	return (0);
    
}