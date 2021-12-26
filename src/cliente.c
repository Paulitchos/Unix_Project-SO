#include "structs.h"
#include "globals.h"

void terminate();

typedef struct thread_global_info{
	bool debugging;
    int npc; // <named pipe cliente>
	int npb;
	char cFifoName[25];	// <client FIFO name> this client FIFO's name
	
    // Thread for user input
    bool t_die; // <Thread_Die>
    pthread_t tid; // <ThreadID>
    pthread_mutex_t *pMutAll;
}global_info, *pglobal_info;

// declared globally to terminate thread when ^C recieved
// static so it can't be accessed by other source files
static global_info g_info; // <Global_Info>

// Thread #1
void * userInput(void * p){
	pglobal_info thread_data = (global_info *) p;
	int ret_size;
	char usr_in[TAM_MAX_MSG];
	if (thread_data->debugging) { fprintf(stderr,"==User input Thread running==\n");}
    do {
		//pthread_mutex_lock(thread_data->pMutAll);
		ret_size = read(STDIN_FILENO,&usr_in,sizeof(usr_in));
		if (ret_size <= -1 ) { printf("Error Reading, output: %d\n",ret_size); terminate(); }
		usr_in[ret_size] = '\0';
		usr_in[strlen(usr_in)-1] = '\n';
		if (thread_data->debugging) {fprintf(stderr, "==read: |"); debugString(usr_in); fprintf(stderr, "|==\n"); }
		if (!strcasecmp(usr_in, "adeus\n")){
			// ==== Terminate this client ==== //
			fprintf(stdout, "Exiting...\n");
            suicide Die_Cli;
            Die_Cli.size = sizeof(Die_Cli);
            // Opens clients FIFO for write
            int npc = open(thread_data->cFifoName, O_WRONLY);
            if (npc == -1) perror("Erro no open - Ninguém quis a resposta\n");
            else{
                if (g_info.debugging) fprintf(stderr, "==FIFO cliente aberto para WRITE==\n");
                // Send response
                ret_size = write(npc, &Die_Cli, sizeof(Die_Cli));
                if (ret_size == sizeof(Die_Cli) && g_info.debugging) // if no error
                    fprintf(stderr, "==escreveu a ordem de morte com sucesso para cliente==\n");
                else
                    perror("erro a escrever a resposta\n");
                close(npc); // FECHA LOGO O FIFO DO CLIENTE!
                if (g_info.debugging) fprintf(stderr, "==FIFO cliente fechado==\n");
            }
		}
			
	}while (!thread_data->t_die);
    pthread_exit(NULL); //podes devolver algo como uma estrutura (tem cuidade para não retornares uma variavel local)
}

void trata_SIGPIPE(int s) {
    static int a; 
    a++;
    //SIGPIPE is the "broken pipe" signal, which is sent to a process when it attempts to write to a pipe whose read end has closed (or when it attempts to write to a socket that is no longer open for reading), but not vice versa. The default action is to terminate the process.
    if (g_info.debugging) fprintf(stderr, "==Recebido sinal SIGPIPE %d==",a);
}

void trata_SIGINT(int i) { // CTRL + C
	//if (g_info.debugging) fprintf(stderr, "==treat_SIGINT Called==\n");
	terminate();
}

void terminate(){
	if (pthread_equal(g_info.tid, pthread_self())){ // For User Input Thread
		if (g_info.debugging) fprintf(stderr, "==terminate Called for User Input Thread==\n");
		if (g_info.debugging) fprintf(stderr, "==[User Input Thread] pthread_exit ing==\n");
		pthread_mutex_destroy(g_info.pMutAll); //? Should this be here
		pthread_exit(NULL);
		// [Never Reaches this Line] //
	} else { // For Main Thread
		if (g_info.debugging) fprintf(stderr, "==terminate Called for Main Thread==\n");

		// ===== Terminate Threads ===== //
		if (g_info.debugging) fprintf(stderr, "==[Main Thread] Sending SIGINT to User Input Thread==\n");
    	pthread_kill(g_info.tid, SIGINT);
		// ===== ================= ===== //

		// ===== Close Pipes ===== //
		close(g_info.npc);
		unlink(g_info.cFifoName);
		if (g_info.debugging) fprintf(stderr, "==[Main Thread] Closing Pipes==\n");

		// ==== Tell Balcao I'm dead ==== //
		imDead imDead_tblc;
		imDead_tblc.size=sizeof(imDead_tblc);
		imDead_tblc.pid=getpid();
		write(g_info.npb, &imDead_tblc, sizeof(imDead_tblc));
		if (g_info.debugging) { fprintf(stderr,"==sent imDead to npb==\n");}
		// ==== ==================== ==== //
	}
	exit(EXIT_SUCCESS);
}

void exceptionOcurred(){
    close(g_info.npc);
	unlink(g_info.cFifoName);
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
	pid_t pidMedicoAMeAtender = 0;
	char mFifoName[50];
	int npm;

	// ======== Checking Arguments ======== //
	g_info.debugging = false;
	bool success = false;
	if (argc==2)
		success = true;
	else if (argc > 2)
		for (int i = 2; i < argc; i++) {
			if (!(strcmp(argv[i],"--debugging")) || !(strcmp(argv[i],"-D"))){
				printf("==Debugging mode activated==\n");
				success = true;
				g_info.debugging = true;
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
    if (g_info.debugging) fprintf(stderr, "==Sinal SIGPIPE configurado==\n");
    if (signal(SIGINT,trata_SIGINT) == SIG_ERR) {
        perror("\nNão foi possível configurar sinal SIGINT!\n");
        exit(EXIT_FAILURE); }
    if (g_info.debugging) fprintf(stderr, "==Sinal SIGINT configurado==\n");
    // ============== ============= ============== //

	if (g_info.debugging) fprintf(stderr, "==My PID is: %d==\n", getpid());
	printf("Bem vindo %s!\n",argv[1]);

    // ===========================
    // int	npb;	// <named pipe balcao> FIFO's identifier for balcao
	int	read_res;

	strcpy(sint_toblc.nome, argv[1]); // save the client name in the sint_sint_toblc struct
    sint_toblc.pid_cliente = getpid(); // save the unique client PID in struct sint_sint_toblc

	// create this client's FIFO
	sprintf(g_info.cFifoName, CLIENT_FIFO, sint_toblc.pid_cliente); // CLIENT_FIFO = "/tmp/resp_%d_fifo"
	if (mkfifo(g_info.cFifoName, 0b111111111) == -1){ // with 0777 first zero means octal, get converted to 0b(binary) 111 111 111, (0x is hexadecimal), 7 is 111 for rwx (read/write/execute), three sevens for ugo (owner/group/others)
		perror("\nmkfifo FIFO cliente deu erro"); exit(EXIT_FAILURE); }
	if (g_info.debugging) fprintf(stderr, "==FIFO do cliente criado, cFifoName is: |%s|==\n", g_info.cFifoName);

	// open balcao's FIFO for writing
	g_info.npb = open(BALCAO_FIFO, O_WRONLY); // bloqueante
	if (g_info.npb == -1){ // if couldn't find balcao's fifo
		fprintf(stderr, "\nO balcao não está a correr\n"); 
		unlink(g_info.cFifoName); exit(EXIT_FAILURE); }
    if (g_info.debugging) fprintf(stderr, "==FIFO do balcao aberto WRITE / BLOCKING==\n");

	// abertura read+write evita o comportamento de ficar bloqueado no open. a execução prossegue logo mas as operações read/write (neste caso APENAS READ) continuam bloqueantes						
	g_info.npc = open(g_info.cFifoName, O_RDWR);	// bloqueante
	if (g_info.npc == -1){ perror("\nErro ao abrir o FIFO do cliente"); close(g_info.npb); unlink(g_info.cFifoName); exit(EXIT_FAILURE); }
	if (g_info.debugging) fprintf(stderr, "==FIFO do proprio cliente aberto READ(+WRITE) / BLOCKING==\n");

	memset(sint_toblc.sintomas, '\0', TAM_MAX_MSG); // guarantees memory zone to send  is clear

	printf("\nQuais sao os seus sintomas?\n");

	// ================ User Input ================ //
    pthread_mutex_t MutAll = PTHREAD_MUTEX_INITIALIZER; // dont need to do pthread_mutex_init anymore
    //pthread_mutex_t MutCli = PTHREAD_MUTEX_INITIALIZER; // dont need to do pthread_mutex_init anymore
    //pthread_mutex_t MutMed = PTHREAD_MUTEX_INITIALIZER;
    //global_info g_info; //declared globally
    //g_info.debugging = debugging;
    g_info.t_die = false;
    g_info.pMutAll = &MutAll;
    pthread_create(&g_info.tid, NULL, userInput, &g_info);
    // ================ ========== ================ //

	// read from user
	ret_size = read(STDIN_FILENO,&sint_toblc.sintomas,sizeof(sint_toblc.sintomas));
	if (ret_size <= -1 ) { printf("Error Reading, output: %d\n",ret_size); return 1; }
	sint_toblc.sintomas[ret_size] = '\0';
	if (g_info.debugging) { fprintf(stderr, "==read: |"); debugString(sint_toblc.sintomas); fprintf(stderr, "|==\n"); }
	fflush(stdout); // prevents keeping from sending all the information to the screen
		
	if (!strcasecmp(sint_toblc.sintomas, "adeus\n"))
		terminate();

	// Sends message
	if (write(g_info.npb, &sint_toblc, sizeof(sint_toblc)) <= -1){
		fprintf(stderr, "Error Writing, balcao disappeared before establishing a connection\n") ; terminate(); }
	if (g_info.debugging) { fprintf(stderr,"==sent sint_toblc to npb==\n");}

	while (1){
		// Recieves message
		if (read(g_info.npc, &pipeMsgSize, sizeof(pipeMsgSize)) <= -1 ) { printf("Error Reading, output: %d\n",ret_size); exceptionOcurred(); }
		if (pipeMsgSize == sizeof(info_fblc)){
			if(g_info.debugging) fprintf(stderr,"==Recieved Msg Type \"info_fblc\"==\n");
			if(g_info.debugging) fprintf(stderr,"==sizeof(info_fblc) %d | sizeof(info_fblc)-sizeof(info_fblc.esp) %d==\n", (int)sizeof(info_fblc), (int)(sizeof(info_fblc)-sizeof(info_fblc.esp)));

			if(g_info.debugging) printf("==Endreços: &info_fblc: %p | old(&(info_fblc.msg)): %p | (&(info_fblc.size)+1): %p==\n",&info_fblc, &(info_fblc.esp), &(info_fblc.size)+1);

			read_res = read(g_info.npc, &(info_fblc.size)+1, (int)sizeof(info_fblc)-sizeof(info_fblc.size));
			if (read_res == (int)sizeof(info_fblc)-sizeof(info_fblc.size)) {
				printf("Especialidade -> %s\n", info_fblc.esp);
				printf("Prioridade -> %d\n", info_fblc.prio);
				printf("Num pessoas a frente -> %d\n",info_fblc.num_peopleAhead);
				printf("Num medicos on-line -> %d\n",info_fblc.num_espOnline);
			} else
				printf("incomprehensible response: bytes read [%d]\n", read_res);
		}else if(pipeMsgSize == sizeof(suicide)){
			if(g_info.debugging) fprintf(stderr,"==Recieved Msg Type \"suicide\"==\n");
			suicide suicide;
			read_res = read(g_info.npc, &(suicide.size)+1, sizeof(suicide)-sizeof(suicide.size));
			if (read_res == sizeof(suicide)-sizeof(suicide.size)){
				terminate();
			} else {
				printf("Sem resposta ou resposta incompreensível [bytes lidos: %d]\n", read_res);
			}
		}else if(pipeMsgSize == sizeof(imDead)){ // msg with info to connect to medic
			imDead ConCli;
			if(g_info.debugging) fprintf(stderr,"==Recieved Msg Type \"imDEad(connect to cli)\"==\n");
            read_res = read(g_info.npc, &(ConCli.size)+1, sizeof(ConCli)-sizeof(ConCli.size));
            if (read_res == sizeof(ConCli)-sizeof(ConCli.size)){
				pidMedicoAMeAtender = ConCli.pid;
			} else {
				printf("Sem resposta ou resposta incompreensível [bytes lidos: %d]\n", read_res);
			}
		}else if(pipeMsgSize == sizeof(msg)){
			msg msgCli;
			if(g_info.debugging) fprintf(stderr,"==Recieved Msg Type \"msgCli\"==\n");
            read_res = read(g_info.npc, &(msgCli.size)+1, sizeof(msgCli)-sizeof(msgCli.size));
            if (read_res == sizeof(msgCli)-sizeof(msgCli.size)){

				if (pidMedicoAMeAtender == 0)
					continue; // Ainda não se recebeu nenhuma mensagem com quem deviamos falar

				// ==== Read user input ==== //
				msg msgToMed;
				msgToMed.size = sizeof(msgToMed);
				ret_size = read(STDIN_FILENO,&msgToMed.msg,sizeof(msgToMed.msg));
				if (ret_size <= -1 ) { printf("Error Reading, output: %d\n",ret_size); terminate(); }
				msgToMed.msg[ret_size] = '\0';
				msgToMed.msg[strlen(msgToMed.msg)-1] = '\n';
				if (g_info.debugging) {fprintf(stderr, "==read: |"); debugString(msgToMed.msg); fprintf(stderr, "|==\n"); }
				// ==== =============== ==== //

				// ======== msg para o cliente ======== //
				sprintf(mFifoName, CLIENT_FIFO, pidMedicoAMeAtender);
				// Opens medic FIFO for write
				npm = open(mFifoName, O_WRONLY);
				if (npm == -1) perror("Erro no open - Ninguém quis a resposta\n");
				else{
					// Send response
					ret_size = write(npm, &msgToMed, sizeof(msgToMed));
					if (ret_size == sizeof(msgToMed) && g_info.debugging) // if no error
						fprintf(stderr, "==success writing freq period to cli==\n");
					else
						perror("erro a escrever a resposta\n");
					close(npm); // FECHA LOGO O FIFO DO CLIENTE!
				}
				// ======== ================== ======== //

			} else {
				printf("Sem resposta ou resposta incompreensível [bytes lidos: %d]\n", ret_size);
			}
		} else {
			printf("Not a recognizable msg\n");
		}
	}

	close(g_info.npc);
	close(g_info.npb);
	unlink(g_info.cFifoName);

	return (0);
}