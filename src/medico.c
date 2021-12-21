#include "structs.h"
#include "globals.h"

// têm que ser global para ser tratadas no trata_SIGINT
int	npm; // <named pipe cliente> FIFO's identifier for cliente
char mFifoName[25];	// <client FIFO name> this client FIFO's name

void trata_SIGPIPE(int s) {
    static int a; 
    a++;
    //SIGPIPE is the "broken pipe" signal, which is sent to a process when it attempts to write to a pipe whose read end has closed (or when it attempts to write to a socket that is no longer open for reading), but not vice versa. The default action is to terminate the process.
    printf("Recebido sinal SIGPIPE %d ",a); // fflush
}

void trata_SIGINT(int i) { // CTRL + C
	(void) i;    //todo what?
	fprintf(stderr, "\nMedico a terminar\n");
	close(npm);
	unlink(mFifoName);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){

    esp_fmed med_toblc; // <sintomas_fromClient> <sintomas_toBalcao>
    connectCli_fblc connectCli_fblc; // <info_fromBalcao> <info_fromBalcao>
	med_toblc.size = sizeof(med_toblc);
	connectCli_fblc.size = sizeof(connectCli_fblc);
	unsigned short pipeMsgSize; // <pipe Message Size>
    int ret_size; //  <returned size>
	pinfo_fblc pinfo_fblc;

	// ======== Checking Arguments ======== //
	bool debugging = false;
	bool success = false;
	if (argc==3)
		success = true;
	else if (argc > 3)
		for (int i = 3; i < argc; i++) {
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
		printf("Exemplo ./medico <nome> <especialidade> --debugging\n");
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

	strcpy(med_toblc.nome, argv[1]); // save the medic name in the med_toblc struct
    strcpy(med_toblc.esp, argv[2]); // save the medic esp in the med_toblc struct
    med_toblc.pid_medico = getpid(); // save the unique client PID in struct med_toblc

	// create this medic's FIFO
	sprintf(mFifoName, MED_FIFO, med_toblc.pid_medico); // MED_FIFO = "/tmp/resp_%d_fifo"
	if (mkfifo(mFifoName, 0b111111111) == -1){ // with 0777 first zero means octal, get converted to 0b(binary) 111 111 111, (0x is hexadecimal), 7 is 111 for rwx (read/write/execute), three sevens for ugo (owner/group/others)
		perror("\nmkfifo FIFO medico deu erro"); exit(EXIT_FAILURE); }
	if (debugging) fprintf(stderr, "==FIFO do medico criado, cFifoName is: |%s|==\n", mFifoName);

	// open balcao's FIFO for writing
	npb = open(BALCAO_FIFO, O_WRONLY); // bloqueante
	if (npb == -1){ // if couldn't find balcao's fifo
		fprintf(stderr, "\nO balcao não está a correr\n"); 
		unlink(mFifoName); exit(EXIT_FAILURE); }
    if (debugging) fprintf(stderr, "==FIFO do balcao aberto WRITE / BLOCKING==\n");

	// abertura read+write evita o comportamento de ficar bloqueado no open. a execução prossegue logo mas as operações read/write (neste caso APENAS READ) continuam bloqueantes						
	npm = open(mFifoName, O_RDWR);	// bloqueante
	if (npm == -1){ perror("\nErro ao abrir o FIFO do medico"); close(npb); unlink(mFifoName); exit(EXIT_FAILURE); }
	if (debugging) fprintf(stderr, "==FIFO do proprio medico aberto READ(+WRITE) / BLOCKING==\n");

	while (1){
		// Sends message
		write(npb, &med_toblc, sizeof(med_toblc));
		if (debugging) { fprintf(stderr,"==sent med_toblc to npb==\n");}

		// Recieves message
		ret_size = read(npm, &pipeMsgSize, sizeof(pipeMsgSize));
		if (pipeMsgSize == sizeof(connectCli_fblc)){
			if(debugging) fprintf(stderr,"==Recieved Msg Type \"connectCli_fblc\"==\n");
			if(debugging) printf("==Endereços: &connectCli_fblc: %p | old(&(connectCli_fblc.nome)): %p | (&(connectCli_fblc.size)+1): %p==\n",&connectCli_fblc, &(connectCli_fblc.nome), &(connectCli_fblc.size)+1);

			read_res = read(npm, &(connectCli_fblc.size)+1, (int)sizeof(connectCli_fblc)-sizeof(connectCli_fblc.size));
			if (read_res == (int)sizeof(connectCli_fblc)-sizeof(connectCli_fblc.size)) {
				printf("Nome do CLiente -> %s\n", connectCli_fblc.nome);
				printf("Prioridade -> %d\n", connectCli_fblc.pid_cliente);
			} else
				printf("Sem resposta ou resposta incompreensível"
							"[bytes lidos: %d]\n", read_res);
		} else {
			printf("Not a info_fblc type msg\n");
		}
	}

	close(npm);
	close(npb);
	unlink(mFifoName);

	return (0);
}