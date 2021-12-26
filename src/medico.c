#include "structs.h"
#include "globals.h"
#include <stdbool.h>
#include <unistd.h>

typedef struct thread_info{
	bool debugging;
	int npb;
	esp_fmed * pmed_toblc;
	int waitTime;
	
    // Thread for life signal
    bool t_die; // <Thread_Die>
    pthread_t tid; // <ThreadID>
    pthread_mutex_t *pMutVida;
}thread_info, *pthread_info;

void terminate();

//declared globally to terminate thread when ^C recieved
static thread_info t_info; // static so it can't be accessed by other source files

//Thread #1
void * sinalDeVida(void * p){
	pthread_info sinal_vida = (thread_info *) p;
	sinal_vida->waitTime = 20;
    do {
		pthread_mutex_lock(sinal_vida->pMutVida);
        sleep(sinal_vida->waitTime);
        write(sinal_vida->npb, sinal_vida->pmed_toblc, sizeof(*sinal_vida->pmed_toblc));
		if (sinal_vida->debugging) { fprintf(stderr,"==sent med_toblc to npb==\n");}
		pthread_mutex_unlock(sinal_vida->pMutVida);
    }while (!sinal_vida->t_die);
    pthread_exit(NULL); //podes devolver algo como uma estrutura (tem cuidade para não retornares uma variavel local)
}

// têm que ser global para ser tratadas no treat_SIGINT
int	npm; // <named pipe cliente> FIFO's identifier for cliente
char mFifoName[25];	// <client FIFO name> this client FIFO's name
int	npb; // <named pipe balcao> FIFO's identifier for balcao

void treat_SIGPIPE(int s) {
    static int a; 
    a++;
    //SIGPIPE is the "broken pipe" signal, which is sent to a process when it attempts to write to a pipe whose read end has closed (or when it attempts to write to a socket that is no longer open for reading), but not vice versa. The default action is to terminate the process.
    fprintf(stderr, "Recebido sinal SIGPIPE %d ",a);
}

void treat_SIGINT(int i) { // CTRL + C
	(void) i;    //? necessary? Does what
	terminate();
}

void terminate(){
	if (pthread_equal(t_info.tid, pthread_self())){ // For life signal Thread
		if (t_info.debugging) fprintf(stderr, "\n==treat_SIGINT Called for Life Signal Thread==\n");
		if (t_info.debugging) fprintf(stderr, "==[Life Signal Thread] pthread_exit ing==\n");
		pthread_mutex_destroy(t_info.pMutVida); //? Should this be here
		pthread_exit(NULL);
		// [Never Reaches this Line] //
	} else { // For Main Thread
		if (t_info.debugging) fprintf(stderr, "\n==treat_SIGINT Called for Main Thread==\n");

		// ===== Terminate Threads ===== //
		if (t_info.debugging) fprintf(stderr, "==[Main Thread] Sending SIGINT to Life Signal Thread==\n");
    	pthread_kill(t_info.tid, SIGINT);
		// ===== ================= ===== //

		// ==== Tell Balcao I'm dead ==== //
		imDead imDead_tblc;
		imDead_tblc.size=sizeof(imDead_tblc);
		imDead_tblc.pid=getpid();
		write(npb, &imDead_tblc, sizeof(imDead_tblc));
		if (t_info.debugging) { fprintf(stderr,"==sent imDead to npb==\n");}
		// ==== ==================== ==== //

		// ===== Close Pipes ===== // //?only need to close them once?
		close(npm);
		unlink(mFifoName);
		if (t_info.debugging) fprintf(stderr, "==[Main Thread] Closing Pipes==\n");
	}
	exit(EXIT_SUCCESS);
}

// Thread #0
int main(int argc, char **argv){

    esp_fmed med_toblc; // <sintomas_fromClient> <sintomas_toBalcao>
    connectCli_fblc connectCli_fblc; // <info_fromBalcao> <info_fromBalcao>
	med_toblc.size = sizeof(med_toblc);
	connectCli_fblc.size = sizeof(connectCli_fblc);
	unsigned short pipeMsgSize; // <pipe Message Size>
    int ret_size; //  <returned size>
	pinfo_fblc pinfo_fblc;
	freq_tmed freq_life_sig;
	freq_life_sig.size = sizeof(freq_life_sig);
	suicide suicide;
	suicide.size = sizeof(suicide);

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
    if (signal(SIGPIPE,treat_SIGPIPE) == SIG_ERR)
        perror("\nNão foi possível configurar sinal SIGPIPE\n");
    if (debugging) fprintf(stderr, "==Sinal SIGPIPE configurado==\n");
    if (signal(SIGINT,treat_SIGINT) == SIG_ERR) {
        perror("\nNão foi possível configurar sinal SIGINT!\n");
        exit(EXIT_FAILURE); }
    if (debugging) fprintf(stderr, "==Sinal SIGINT configurado==\n");
    // ============== ============= ============== //

	if (debugging) fprintf(stderr, "==My PID is: %d==\n", getpid());
	printf("Bem vindo %s!\n",argv[1]);

    //int	npb;	// declared globally // <named pipe balcao> FIFO's identifier for balcao
	int	read_res,res;

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

	// ================ Life Signal ================ //
	pthread_mutex_t MutVida = PTHREAD_MUTEX_INITIALIZER; // dont need to do pthread_mutex_init anymore
	//thread_info t_info; //declared globally
	t_info.t_die = false;
	t_info.npb = npb;
	t_info.debugging = debugging;
	t_info.pmed_toblc = &med_toblc;
	t_info.pMutVida = &MutVida;
	pthread_create(&t_info.tid, NULL, sinalDeVida, &t_info);
	// ================ =========== ================ //

	// Sends message
	write(npb, &med_toblc, sizeof(med_toblc));
	if (debugging) { fprintf(stderr,"==sent med_toblc to npb==\n");}

	while (1){

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
				printf("incomprehensible response: bytes read [%d]\n", read_res);
		}else if(pipeMsgSize == sizeof(freq_life_sig)){
			if(debugging) fprintf(stderr,"==Recieved Msg Type \"freq_life_sig\"==\n");
            res = read(npm, &(freq_life_sig.size)+1, sizeof(freq_life_sig)-sizeof(freq_life_sig.size));
            if (res == sizeof(freq_life_sig)-sizeof(freq_life_sig.size)){
				t_info.waitTime = freq_life_sig.freq; 
			} else {
				printf("Sem resposta ou resposta incompreensível [bytes lidos: %d]\n", res);
			}
		}else if(pipeMsgSize == sizeof(suicide)){
			if(debugging) fprintf(stderr,"==Recieved Msg Type \"suicide\"==\n");
            res = read(npm, &(suicide.size)+1, sizeof(suicide)-sizeof(suicide.size));
            if (res == sizeof(suicide)-sizeof(suicide.size)){
				terminate();
			} else {
				printf("Sem resposta ou resposta incompreensível [bytes lidos: %d]\n", res);
			}
		} else {
			printf("Not a recognizable msg\n");
		}
	}

	close(npm);
	close(npb);
	unlink(mFifoName);

	t_info.t_die = true;
	pthread_join(t_info.tid, NULL);  // Here you recieve the return value of the thread
	pthread_mutex_destroy(t_info.pMutVida);

	return (0);
}