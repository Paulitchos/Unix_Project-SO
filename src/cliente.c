#include "structs.h"
#include "globals.h"

void debugString(char * str);

int main(int argc, char **argv){

    sint_fcli sint_toblc; // <sintomas_fromClient> <sintomas_toBalcao>
    info_fblc info_fblc; // <info_fromBalcao> <info_fromBalcao>
	sint_toblc.size = sizeof(sint_toblc);
	info_fblc.size = sizeof(info_fblc);
	long long pipeMsgSize; // <pipe Message Size>
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

	if (debugging) fprintf(stderr, "==My PID is: %d==\n", getpid());
	printf("Bem vindo %s!\n",argv[1]);

    // ===========================
    int	npb;	// <named pipe balcao> FIFO's identifier for balcao
	int	npc;	// <named pipe cliente> FIFO's identifier for cliente
	char cFifoName[25];	// <client FIFO name> this client FIFO's name
	int	read_res;

	strcpy(sint_toblc.nome, argv[1]); // save the client name in the sint_sint_toblc struct
    sint_toblc.pid_cliente = getpid(); // save the unique client PID in struct sint_sint_toblc

	// create this client's FIFO
	sprintf(cFifoName, CLIENT_FIFO, sint_toblc.pid_cliente); // CLIENT_FIFO = "/tmp/resp_%d_fifo"
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
		ret_size = read(npc, &pipeMsgSize, sizeof(pipeMsgSize));
		if (pipeMsgSize == sizeof(info_fblc)){
			if(debugging) fprintf(stderr,"==Recieved Msg Type \"info_fblc\"==\n");
			if(debugging) fprintf(stderr,"==sizeof(info_fblc) %d | sizeof(info_fblc)-sizeof(long long) %d==\n", (int)sizeof(info_fblc), (int)(sizeof(info_fblc)-sizeof(long long)));

			read_res = read(npc, &(info_fblc.msg), (int)sizeof(info_fblc)-sizeof(long long));
			if (read_res == (int)sizeof(info_fblc)-sizeof(long long))
				printf("Recebido -> %s\n", info_fblc.msg);
			else
				printf("Sem resposta ou resposta incompreensível"
							"[bytes lidos: %d]\n", read_res);
		} else {
			printf("Not a info_fblc type msg\n");
		}
	}

	close(npc);
	close(npb);
	unlink(cFifoName);

	return (0);
}