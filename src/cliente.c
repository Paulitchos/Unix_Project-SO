#include "structs.h"
#include "globals.h"

void debugString(char * str);

int main(int argc, char **argv){

    fromCliente fcli;
    toCliente tcli;
    int tam;

	// ======== Checking Arguments ======== //
	bool debugging = false;
	bool success = false;
	if (argc==2)
		success = true;
	else if (argc > 2)
		for (int i = 2; i < argc; i++) {
			if (!(strcmp(argv[i],"--debugging")) || !(strcmp(argv[i],"-D"))){
				printf("==Debugging mode activated==\n");
				success = true;
				debugging = true;
				break;
			}
		}
	if (!success){
		printf("Deve introduzir o seu nome como primeiro argumento\n");
		printf("Opcoes existentes: --debugging ou -D\n");
		printf("Exemplo ./cliente <nome> --debugging\n");
		exit(EXIT_SUCCESS);
	}
	// ======== ================== ======== //

	if (debugging) fprintf(stderr, "==My PID is: %d==\n", getpid());
	printf("Bem vindo %s!\n",argv[1]);

    // ===========================
    int	npb;	// <named pipe balcao> FIFO's identifier for balcao
	int	npc;	// <named pipe cliente> FIFO's identifier for cliente
	char cFifoName[25];	// <client FIFO name> this client FIFO's name
	int	read_res;

	strcpy(fcli.nome, argv[1]); // save the client name in the fromCliente struct
    fcli.pid_cliente = getpid(); // save the unique client PID in struct fromCliente

	// create this client's FIFO
	sprintf(cFifoName, CLIENT_FIFO, fcli.pid_cliente); // CLIENT_FIFO = "/tmp/resp_%d_fifo"
	if (mkfifo(cFifoName, 0b111111111) == -1){ //? with 0777 first zero means octal, get converted to 0b(binary) 111 111 111, (0x is hexadecimal), 7 is 111 for rwx (read/write/execute), three sevens for ugo (owner/group/others)
		perror("\nmkfifo FIFO cliente deu erro"); exit(EXIT_FAILURE); }
	if (debugging) fprintf(stderr, "==FIFO do cliente criado, cFifoName is: |%s|==\n", cFifoName);

	// open balcao's FIFO for writing
	npb = open(BALCAO_FIFO, O_WRONLY); // bloqueante
	if (npb == -1){ // if couldn't find balcao's fifo
		fprintf(stderr, "\nO balcao não está a correr\n"); 
		unlink(cFifoName); exit(EXIT_FAILURE); }
    if (debugging) fprintf(stderr, "==FIFO do balcao aberto WRITE / BLOCKING==\n");

	// abertura read+write evita o comportamento de ficar bloqueado no open. a execução prossegue logo mas as operações read/write (neste caso APENAS READ) continuam bloqueantes						
	npc = open(cFifoName, O_RDWR);	// bloqueante
	if (npc == -1){ perror("\nErro ao abrir o FIFO do cliente"); close(npb); unlink(cFifoName); exit(EXIT_FAILURE); }
	if (debugging) fprintf(stderr, "==FIFO do proprio cliente aberto READ(+WRITE) / BLOCKING==\n");

	memset(fcli.sintomas, '\0', TAM_MAX_MSG); // guarantees memory zone to send  is clear

	printf("\nQuais sao os seus sintomas?\n");
	while (1){
		// read from user
		//tam = read(STDIN_FILENO,&fcli.sintomas,sizeof(fcli.sintomas));
		//if (tam <= -1 ) { printf("Error Reading, output: %d\n",tam); return 1; }
		//fcli.sintomas[tam-1] = '\0';
		//if (debugging) {fprintf(stderr, "==read: |"); debugString(fcli.sintomas); fprintf(stderr, "|==\n"); }

		scanf("%s", fcli.sintomas);
		//printf("%ld\n", strlen(fcli.sintomas));
		if (debugging) {fprintf(stderr, "==read: |"); debugString(fcli.sintomas); fprintf(stderr, "|==\n"); }

		if (!strcasecmp(fcli.sintomas, "adeus"))
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
	unlink(cFifoName);

	return (0);
    
}