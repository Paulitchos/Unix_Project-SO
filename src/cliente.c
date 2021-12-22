#include "structs.h"
#include "globals.h"

// têm que ser global para ser tratadas no trata_SIGINT
static int	npc; // <named pipe cliente> FIFO's identifier for cliente
static char cFifoName[25];	// <client FIFO name> this client FIFO's name

void trata_SIGPIPE(int s) {
    static int a; 
    a++;
    //SIGPIPE is the "broken pipe" signal, which is sent to a process when it attempts to write to a pipe whose read end has closed (or when it attempts to write to a socket that is no longer open for reading), but not vice versa. The default action is to terminate the process.
    printf("Recebido sinal SIGPIPE %d ",a);
}

void trata_SIGINT(int i) { // CTRL + C
	(void) i;    //? necessary? Does what
	fprintf(stderr, "\nCliente a terminar\n");
	// ===== Close Pipes ===== //
	close(npc);
	unlink(cFifoName);

	exit(EXIT_SUCCESS);
}

void exceptionOcurred(){
    close(npc);
	unlink(cFifoName);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv){

    sint_fcli sint_toblc; // <sintomas_fromClient> <sintomas_toBalcao>
    info_fblc info_fblc; // <info_fromBalcao> <info_fromBalcao>
	sint_toblc.size = sizeof(sint_toblc);
	info_fblc.size = sizeof(info_fblc);
	unsigned short pipeMsgSize; // <pipe Message Size>
    int ret_size; //  <returned size>
	pinfo_fblc pinfo_fblc;

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

    // ============== Treat Signals ============== //
    if (signal(SIGPIPE,trata_SIGPIPE) == SIG_ERR)
        perror("\nNão foi possível configurar sinal SIGPIPE\n");
    if (debugging) fprintf(stderr, "==Sinal SIGPIPE configurado==\n");
    if (signal(SIGINT,trata_SIGINT) == SIG_ERR) {
        perror("\nNão foi possível configurar sinal SIGINT!\n");
        exit(EXIT_FAILURE); }
    if (debugging) fprintf(stderr, "==Sinal SIGINT configurado==\n");
    // ============== ============= ============== //

	if (debugging) fprintf(stderr, "==My PID is: %d==\n", getpid());
	printf("Bem vindo %s!\n",argv[1]);

    // ===========================
    int	npb;	// <named pipe balcao> FIFO's identifier for balcao
	int	read_res;

	strcpy(sint_toblc.nome, argv[1]); // save the client name in the sint_sint_toblc struct
    sint_toblc.pid_cliente = getpid(); // save the unique client PID in struct sint_sint_toblc

	// create this client's FIFO
	sprintf(cFifoName, CLIENT_FIFO, sint_toblc.pid_cliente); // CLIENT_FIFO = "/tmp/resp_%d_fifo"
	if (mkfifo(cFifoName, 0b111111111) == -1){ // with 0777 first zero means octal, get converted to 0b(binary) 111 111 111, (0x is hexadecimal), 7 is 111 for rwx (read/write/execute), three sevens for ugo (owner/group/others)
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

	memset(sint_toblc.sintomas, '\0', TAM_MAX_MSG); // guarantees memory zone to send  is clear

	printf("\nQuais sao os seus sintomas?\n");
	while (1){
		// read from user
		ret_size = read(STDIN_FILENO,&sint_toblc.sintomas,sizeof(sint_toblc.sintomas));
		if (ret_size <= -1 ) { printf("Error Reading, output: %d\n",ret_size); return 1; }
		sint_toblc.sintomas[ret_size] = '\0';
		if (debugging) {fprintf(stderr, "==read: |"); debugString(sint_toblc.sintomas); fprintf(stderr, "|==\n"); }
		fflush(stdout); // prevents keeping from sending all the information to the screen

		if (!strcasecmp(sint_toblc.sintomas, "adeus"))
			break;

		// Sends message
		write(npb, &sint_toblc, sizeof(sint_toblc));
		if (debugging) { fprintf(stderr,"==sent sint_toblc to npb==\n");}

		// Recieves message
		if (read(npc, &pipeMsgSize, sizeof(pipeMsgSize)) <= -1 ) { printf("Error Reading, output: %d\n",ret_size); exceptionOcurred(); }
		if (pipeMsgSize == sizeof(info_fblc)){
			if(debugging) fprintf(stderr,"==Recieved Msg Type \"info_fblc\"==\n");
			if(debugging) fprintf(stderr,"==sizeof(info_fblc) %d | sizeof(info_fblc)-sizeof(info_fblc.esp) %d==\n", (int)sizeof(info_fblc), (int)(sizeof(info_fblc)-sizeof(info_fblc.esp)));

			if(debugging) printf("==Endreços: &info_fblc: %p | old(&(info_fblc.msg)): %p | (&(info_fblc.size)+1): %p==\n",&info_fblc, &(info_fblc.esp), &(info_fblc.size)+1);

			read_res = read(npc, &(info_fblc.size)+1, (int)sizeof(info_fblc)-sizeof(info_fblc.size));
			if (read_res == (int)sizeof(info_fblc)-sizeof(info_fblc.size)) {
				printf("Especialidade -> %s\n", info_fblc.esp);
				printf("Prioridade -> %d\n", info_fblc.prio);
			} else
				printf("Resposta incompreensível: %d]\n", read_res);
		} else {
			printf("Not a info_fblc type msg\n");
		}
	}

	close(npc);
	close(npb);
	unlink(cFifoName);

	return (0);
}