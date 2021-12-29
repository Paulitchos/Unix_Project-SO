#include "structs.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// Run balcao:
/*
    gcc ./src/balcao.c -o ./dist/balcao && ./dist/balcao
*/

typedef struct listaclientes lista_cli, *plista_cli;  

 struct listaclientes{ // <from balcao>
    pid_t pid_cliente;
    char nome[50];
    char esp[20];
    int prio;
    bool atendido; // perguntar se pode utilizar
    plista_cli prox;
};

typedef struct listamedicos lista_med, *plista_med;  

 struct listamedicos{ // <from balcao>
    pid_t pid_medico;
    char nome[50];
    char esp[20];
    bool disponivel; // perguntar se pode utilizar
    plista_med prox;
};

plista_cli insert_end(plista_cli p, plista_cli novo_cli);
void show_info(plista_cli p);
void free_list(plista_cli p);
plista_med insert_end_med(plista_med p, plista_med novo_med);
void show_info_med(plista_med p);
void free_list_med(plista_med p);
unsigned int calc_peepAhead(plista_cli cli, plista_cli cli_list);
bool medExists(pid_t med, plista_med p);
unsigned int calc_espOnline(plista_cli cli, plista_med p);
plista_med cleanDeadMed(plista_med p, pid_t pid);
plista_cli cleanDeadCli(plista_cli p, pid_t pid);
void terminate();
void tellClisToDie(plista_cli p);
void tellMedsToDie(plista_med p);
void changeFrqMeds(plista_med p, int freq);
void makeEmTalk(plista_med pm, plista_cli pc);
void sendConnect(pid_t pid, char * FifoName);
void sendConnectToCli(pid_t pidCli);
int makeMedAvailable(plista_med, pid_t);
void listaespera(plista_cli p);

typedef struct thread_global_info{
	bool debugging;
    plista_cli cli_list;
	plista_med med_list;
    int npb; // <named pipe balcao>
    int waitingdisplay;
	
    // Thread for user input
    bool t_die; // <Thread_Die>
    pthread_t tid; // <ThreadID>
    pthread_mutex_t *pMutAll;
}global_info, *pglobal_info;

typedef struct help{
    lista_cli * * cli_list;
	lista_med * * med_list;
    int * waitingdisplay;
	
    // Thread
    bool t_die; // <Thread_Die>
    // pthread_t tid; // <ThreadID> // You can't equal two thread IDs? wqual a global one to the one inside the struct
    pthread_mutex_t *pMutAll;
}thrds, *pthrds;

// declared globally to terminate thread when ^C recieved
// static so it can't be accessed by other source files
static global_info g_info; // <Global_Info>
pthread_t tid[2]; // <ThreadID>

void * waitingline(void * p){
	pthrds waitingline = (thrds *) p;
    do {
        sleep(*waitingline->waitingdisplay);
        listaespera(*waitingline->cli_list);
		if (g_info.debugging) { fprintf(stderr,"==display da lista de espera==\n");}
    }while (!waitingline->t_die);
    pthread_exit(NULL); // you can return something like a struct, (careful not to return a local variable)
}

void * adminCommands(void * p){
	pthrds thread_data = (thrds *) p;
    freq_tmed freq_tmed;
    freq_tmed.size = sizeof(freq_tmed);
    int ret_size,npm,res;
    pid_t pid_cli,pid_med;
    char usr_in[TAM_MAX_MSG],mFifoName[50]; // <User_Input>
    if (g_info.debugging) { fprintf(stderr,"==User input Thread running==\n");}
    do {
        ret_size = read(STDIN_FILENO,&usr_in,sizeof(usr_in));
		if (ret_size <= -1 ) { printf("Error Reading, output: %d\n",ret_size); terminate(); }
		usr_in[ret_size] = '\0';
        pthread_mutex_lock(thread_data->pMutAll);
		if (g_info.debugging) {fprintf(stderr, "==read: |"); debugString(usr_in); fprintf(stderr, "|==\n"); }

        if(!strcasecmp(usr_in, "UTENTES\n")){
            if (*thread_data->cli_list == NULL) {fprintf(stdout, "Não há utentes para mostrar\n");}
            show_info(*thread_data->cli_list);
        } else if(!strcasecmp(usr_in, "ESPECIALISTAS\n")){
            if (*thread_data->med_list == NULL) {fprintf(stdout, "Não há especialistas para mostrar\n");}
            show_info_med(*thread_data->med_list);
        } else if(sscanf(usr_in,"DELUT %d\n",&pid_cli) == 1 ){
            g_info.cli_list = cleanDeadCli(*thread_data->cli_list, pid_cli);
        }  else if(sscanf(usr_in,"DELESP %d\n",&pid_med) == 1 ){
            g_info.med_list = cleanDeadMed(*thread_data->med_list, pid_med);
        }else if(sscanf(usr_in,"FREQ MED %hd\n",&freq_tmed.freq) == 1 ){
            changeFrqMeds(*thread_data->med_list,freq_tmed.freq);
        }else if(sscanf(usr_in,"FREQ %d\n",&*thread_data->waitingdisplay) == 1 ){
        } else if (strcasecmp(usr_in,"ENCERRA\n")==0) {
            fprintf(stdout, "Exiting...\n");
            suicide Die_Blc;
            Die_Blc.size = sizeof(Die_Blc);
            // Opens balcao FIFO for write
            int npb = open(BALCAO_FIFO, O_WRONLY);
            if (npb == -1) perror("Erro no open - Ninguém quis a resposta\n");
            else{
                if (g_info.debugging) fprintf(stderr, "==FIFO cliente aberto para WRITE==\n");
                // Send response
                res = write(npb, &Die_Blc, sizeof(Die_Blc));
                if (res == sizeof(Die_Blc) && g_info.debugging) // if no error
                    fprintf(stderr, "==escreveu a ordem de morte com sucesso para BALCAO==\n");
                else
                    perror("erro a escrever a resposta\n");
                close(npb); // FECHA LOGO O FIFO DO CLIENTE!
                if (g_info.debugging) fprintf(stderr, "==FIFO cliente fechado==\n");
            }
        } else {
            fprintf(stdout, "Command not recognized, available commands are:\nUTENTES, ESPECIALISTAS, DELUT <PID>, DELESP <PID>, FREQ MED <Period>, FREQ <Period>, ENCERRA\n");
        }

		pthread_mutex_unlock(thread_data->pMutAll);
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
    if (g_info.debugging) fprintf(stderr, "==treat_SIGINT Called==\n");
    terminate();
}

void terminate(){
    if (pthread_equal(tid[1], pthread_self())){ // For User Input Thread
		if (g_info.debugging) fprintf(stderr, "==terminate Called for a thread==\n");
		if (g_info.debugging) fprintf(stderr, "==[User Input Thread] pthread_exit ing==\n");
		//pthread_mutex_destroy(tid.pMutAll); //? Should this be here
		pthread_exit(NULL);
		// [Never Reaches this Line] //
	} else { // For Main Thread
		if (g_info.debugging) fprintf(stderr, "==terminate Called for Main Thread==\n");

		// ===== Terminate Threads ===== //
		if (g_info.debugging) fprintf(stderr, "==[Main Thread] Sending SIGINT to User Input Thread==\n");
    	pthread_kill(tid[0], SIGINT); // maybe this? Mayyyyyybe
        pthread_kill(tid[1], SIGINT);
        //pthread_kill(g_info.tid, SIGINT);
		// ===== ================= ===== //

		// ==== Tell Everyone to die ==== //
        tellClisToDie(g_info.cli_list);
        tellMedsToDie(g_info.med_list);
		// ==== ==================== ==== //

		// ===== Close Pipes ===== // //?only need to close them once?
		close(g_info.npb);
		unlink(BALCAO_FIFO);
        if (g_info.debugging) fprintf(stderr, "==[Main Thread] Closing Pipes==\n");

        // ==== free memory ==== //
        free_list(g_info.cli_list);
        free_list_med(g_info.med_list);
		if (g_info.debugging) fprintf(stderr, "==[Main Thread] freeing memory==\n");
	}
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv,  char *envp[]){

	// ======== Checking Arguments ======== //
	g_info.debugging = false;
	bool success = false;
	if (argc==1)
		success = true;
	else
		for (int i = 1; i < argc; i++) {
			if (!(strcmp(argv[i],"--debugging")) || !(strcmp(argv[i],"-D"))){
				printf("==Debugging mode activated==\n");
				success = true;
			    g_info.debugging = true;
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
    if (g_info.debugging) fprintf(stderr, "==Sinal SIGPIPE configurado==\n");
    if (signal(SIGINT,trata_SIGINT) == SIG_ERR) {
        perror("\nNão foi possível configurar sinal SIGINT!\n");
        exit(EXIT_FAILURE); }
    if (g_info.debugging) fprintf(stderr, "==Sinal SIGINT configurado==\n");
    // ============== ============= ============== //

    if (g_info.debugging) { // get current working directory
        char cwd[500];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            fprintf(stderr, "==Current working dir: %s==\n", cwd);
        } else {
            perror("getcwd() error");
            return 1;
        }
    }
    
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

    int npc; // <named pipe cliente>
    

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
    
    if (f_res == 0) { //CHILD <Classificador>
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
    } else { // PARENT <Balcao>
        // ============== Communicate with Classificador (1) ============== //
        close(fd[0]); // has the duplicated, doesn't need this one
        close(df[1]);
        int tam = TAM_MAX_MSG;
        char msgClassificador[20]; // <mensagem Classificador>
        // ============== ================================== ============== //

        sint_fcli sint_fcli; // <sintomas_fromClient> <sintomas_FromClient>
        plista_cli novo_cli; // <Clients_List>
        plista_med novo_med; // <Medics_List>
        info_fblc info_tcli; // <info_fromBalcao> <info_ToClient>
        esp_fmed esp_fmed;  // <especialidade_fromMedico>
        info_tcli.size = sizeof(info_fblc);

        int	res;
        char cFifoName[50];
        unsigned short pipeMsgSize;
        int prioridade;

        res = mkfifo(BALCAO_FIFO, 0777);
        if (res == -1){	perror("\nmkfifo do FIFO do balcao deu erro"); exit(EXIT_FAILURE); }
        if (g_info.debugging) fprintf(stderr, "==FIFO servidor criado==\n");

        g_info.npb = open(BALCAO_FIFO, O_RDWR); // opens in Read/Write mode to prevent getting stuck in open, it's still blocking
        if (g_info.npb == -1){ perror("\nErro ao abrir o FIFO do servidor (RDWR/blocking)\n"); exit(EXIT_FAILURE);	}
        if (g_info.debugging) fprintf(stderr, "==FIFO aberto para READ (+WRITE) BLOQUEANTE==\n");        
        

        thrds td[2];
        lista_cli ** ppnovo_cli = &g_info.cli_list;
        lista_med ** ppnovo_med = &g_info.med_list;
        int waitingdisplay = 30;
        int * pwaitingdisplay =  &waitingdisplay;
        
        
        pthread_mutex_t MutAll = PTHREAD_MUTEX_INITIALIZER; // dont need to do pthread_mutex_init anymore
        //pthread_mutex_t MutCli = PTHREAD_MUTEX_INITIALIZER; // dont need to do pthread_mutex_init anymore
        //pthread_mutex_t MutMed = PTHREAD_MUTEX_INITIALIZER;
        //global_info g_info; //declared globally
        //g_info.debugging = g_info.debugging;

        // ================ waiting line ================ //
        td[1].t_die = 0;
        td[1].waitingdisplay = pwaitingdisplay;
        td[1].pMutAll = &MutAll;
        td[1].cli_list = ppnovo_cli;
        td[1].med_list = ppnovo_med;

        //tid[1] = td[1].tid;
        pthread_create(&tid[1], NULL, waitingline, &td[1]);
        
        // ================ ============ ================ //

        // ================ User Input ================ //
        td[0].t_die = false;
        td[0].waitingdisplay = pwaitingdisplay;
        td[0].pMutAll = &MutAll;
        td[0].cli_list = ppnovo_cli;
        td[0].med_list = ppnovo_med;

        //tid[0] = td[0].tid;
        pthread_create(&tid[0], NULL, adminCommands, &td[0]);
        
        // ================ ========== ================ //
        if (g_info.debugging) fprintf(stderr, "==Threads created==\n==Thread IDs: %lu %lu %lu==\n",tid[0], tid[1], pthread_self());

        do{
            // Recieve simptoms from cliente
            if (read(g_info.npb, &pipeMsgSize, sizeof(pipeMsgSize)) <= -1 ) { 
                printf("Error Reading, output\n"); trata_SIGINT(0); }
            if (pipeMsgSize == sizeof(sint_fcli)){
                if(g_info.debugging) fprintf(stderr,"==Recieved Msg Type \"sint_fcli\"==\n");
                sint_fcli.size = sizeof(sint_fcli);
                res = read(g_info.npb, &(sint_fcli.size)+1, (int)sizeof(sint_fcli)-sizeof(sint_fcli.size));
                if (res == (int)sizeof(sint_fcli)-sizeof(sint_fcli.size)) {

                    // ============== Communicate with Classificador (2) ============== //
                    sint_fcli.sintomas[strlen(sint_fcli.sintomas)-1] = '\n';
                    if(write(fd[1], sint_fcli.sintomas, strlen(sint_fcli.sintomas)) <= -1){ // write to pipe, write is waiting for amount of characters
                        printf("Error Writing\n") ; return 1; }
                    // write uses strlen to not write unecessary data as we know all the data we want to write
                    
                    if (g_info.debugging) {fprintf(stderr, "==Recebido do cliente & enviado para o classificador: |"); debugString(sint_fcli.sintomas); fprintf(stderr, "|==\n"); }

                    tam = read(df[0],msgClassificador,sizeof(msgClassificador)); // read from pipe, tam = sizeof(phrase if all is fine) | -1 i error while reading, 0 is unexpected EOF
                    if (tam <= -1 ) { printf("Error Reading, output: %d\n",tam); return 1; }
                    // read uses sizeof because we don't know what we'll recieve àpriori, so we send the max size of the string
                    msgClassificador[tam] = '\0';

                    if (g_info.debugging) {fprintf(stderr, "==Recebido do classificador: |"); debugString(msgClassificador); fprintf(stderr, "|==\n"); }
                    // ============== ================================== ============== //

                    // Get Filename of client's FIFO to send response
                    sprintf(cFifoName, CLIENT_FIFO, sint_fcli.pid_cliente);

                    // =========== Separar a especialidade e a prioridade ===========
                    sscanf(msgClassificador,"%s %d",info_tcli.esp,&info_tcli.prio);
                    if (g_info.debugging) { fprintf(stderr, "==Especialidade -> %s | Prioridade -> %d==\n",info_tcli.esp, info_tcli.prio); }
                    // =========== ====================================== ===========

                    // =========== Save in Linked List =========== //
                    novo_cli = malloc(sizeof(*novo_cli));
                    if (novo_cli==NULL) { fprintf(stderr,"==Malloc Error on new Client==\n"); terminate();}
                    novo_cli->pid_cliente = sint_fcli.pid_cliente;
                    strcpy(novo_cli->nome, sint_fcli.nome);
                    strcpy(novo_cli->esp, info_tcli.esp);
                    novo_cli->prio = info_tcli.prio;
                    novo_cli->prox = NULL;
                    novo_cli->atendido = false;
                    g_info.cli_list = insert_end(g_info.cli_list, novo_cli);
                    if (g_info.debugging) fprintf(stderr, "==Added New cli to Linked List, showing info:==\n");
                    if (g_info.debugging) show_info(g_info.cli_list);
                    if (g_info.debugging) fprintf(stderr, "====\n");
                    // =========== =================== =========== //

                    info_tcli.num_peopleAhead = calc_peepAhead(novo_cli, g_info.cli_list);
                    if (g_info.debugging) fprintf(stderr, "==people ahead: %d==\n", info_tcli.num_peopleAhead);

                    info_tcli.num_espOnline = calc_espOnline(novo_cli, g_info.med_list);
                    if (g_info.debugging) fprintf(stderr, "==especialistas online: %d==\n", info_tcli.num_espOnline);

                    // Opens clients FIFO for write
                    npc = open(cFifoName, O_WRONLY);
                    if (npc == -1) perror("Erro no open - Ninguém quis a resposta\n");
                    else{
                        if (g_info.debugging) fprintf(stderr, "==FIFO cliente aberto para WRITE==\n");

                        // Send response
                        res = write(npc, &info_tcli, sizeof(info_tcli));
                        if (res == sizeof(info_tcli) && g_info.debugging) // if no error
                            fprintf(stderr, "==escreveu a resposta com sucesso para cliente==\n");
                        else
                            perror("erro a escrever a resposta\n");

                        close(npc); // FECHA LOGO O FIFO DO CLIENTE!
                        if (g_info.debugging) fprintf(stderr, "==FIFO cliente fechado==\n");
                    }
                    makeEmTalk(g_info.med_list, g_info.cli_list);
                } else {
                    printf("incomprehensible response: bytes read [%d]\n", res);
                }

            } else if (pipeMsgSize == sizeof(esp_fmed)) {
			    if(g_info.debugging) fprintf(stderr,"==Recieved Msg Type \"esp_fmed\"==\n");
                res = read(g_info.npb, &(esp_fmed.size)+1, (int)sizeof(esp_fmed)-sizeof(esp_fmed.size));
                if (res == (int)sizeof(esp_fmed)-sizeof(esp_fmed.size)){
                    if (g_info.debugging) fprintf(stderr, "==nome medico: %s | especialidade: %s==\n",esp_fmed.nome, esp_fmed.esp);
                    
                    if(medExists(esp_fmed.pid_medico,g_info.med_list)){
                        printf("LIFE SIGNAL -> %s %s %d\n", esp_fmed.nome, esp_fmed.esp, esp_fmed.pid_medico);
                    } else {
                        // =========== Save in Linked List =========== //
                        novo_med = malloc(sizeof(*novo_med));
                        if (novo_med==NULL) { fprintf(stderr,"==Malloc Error on new Client==\n"); terminate(); }
                        novo_med->pid_medico = esp_fmed.pid_medico;
                        strcpy(novo_med->nome, esp_fmed.nome);
                        strcpy(novo_med->esp, esp_fmed.esp);
                        novo_med->disponivel = 1;
                        novo_med->prox = NULL;
                        g_info.med_list = insert_end_med(g_info.med_list, novo_med);
                        if (g_info.debugging) fprintf(stderr, "==Added New med to Linked List, showing info:==\n");
                        if (g_info.debugging) show_info_med(g_info.med_list);
                        if (g_info.debugging) fprintf(stderr, "====\n");
                        // =========== =================== =========== //
                        makeEmTalk(g_info.med_list, g_info.cli_list);
                    }
                } else {
                    printf("incomprehensible response: bytes read [%d]\n", res);
                }
            } else if (pipeMsgSize == sizeof(imDead)) {
                if(g_info.debugging) fprintf(stderr,"==Recieved Msg Type \"imDead\"==\n");
                imDead deadBodyFound;
                res = read(g_info.npb, &(deadBodyFound.size)+1, sizeof(deadBodyFound)-sizeof(deadBodyFound.size));
                if (res == sizeof(deadBodyFound)-sizeof(deadBodyFound.size)){
                    g_info.med_list = cleanDeadMed(g_info.med_list, deadBodyFound.pid);
                    g_info.cli_list = cleanDeadCli(g_info.cli_list, deadBodyFound.pid);
                    if (g_info.debugging) fprintf(stderr,"== Cleaned Dead Body of PID %d, showing  med List:==\n",deadBodyFound.pid);
                    if (g_info.debugging) show_info_med(g_info.med_list);
                    if (g_info.debugging) fprintf(stderr, "==Showing Cli List:==\n");
                    if (g_info.debugging) show_info(g_info.cli_list);
                    if (g_info.debugging) fprintf(stderr, "====\n");
                } else {
                    printf("incomprehensible response: bytes read [%d]\n", res);
                }
            } else if (pipeMsgSize == sizeof(suicide)) {
                if(g_info.debugging) fprintf(stderr,"==Recieved Msg Type \"suicide\"==\n");
                suicide goodbye;
                res = read(g_info.npb, &(goodbye.size)+1, sizeof(goodbye)-sizeof(goodbye.size));
                if (res == sizeof(goodbye)-sizeof(goodbye.size)){
                    terminate();
                } else {
                    printf("incomprehensible response: bytes read [%d]\n", res);
                }
            } else if (pipeMsgSize == 20000) { // ID for struct type available
                if(g_info.debugging) fprintf(stderr,"==Recieved Msg Type \"available\"==\n");
                available med_available;
                med_available.id = 20000;
                res = read(g_info.npb, &(med_available.id)+1, sizeof(med_available)-sizeof(med_available.id));
                if (res == sizeof(med_available)-sizeof(med_available.id)){
                    if (makeMedAvailable(g_info.med_list, med_available.pid) <= -1){
                        if (g_info.debugging) fprintf(stderr, "==Erro. Medico %d==\n", med_available.pid);
                    } else if (g_info.debugging) fprintf(stderr, "==Medico %d agora disponivel==\n", med_available.pid);
                    makeEmTalk(g_info.med_list, g_info.cli_list);
                } else {
                    printf("incomprehensible response: bytes read [%d]\n", res);
                }
            } else {
                fprintf(stderr,"No recognizable message recieved from pipe, aborting\n");
                terminate();
            }
        } while (true);    

        return 4;
    }
	// shouldn't reach this point
	close(g_info.npb);
	unlink(BALCAO_FIFO);
	return (0);
}

unsigned int calc_peepAhead(plista_cli cli, plista_cli p){ // only counts peeps for a certain esp
    unsigned int c = 0;
    if (p==NULL) 
        return 0;
    else if (cli->pid_cliente == p->pid_cliente)
        return 0;

    do {
        if ( strcmp(p->esp,cli->esp) == 0)
            c++;
        p = p->prox;
        if (p == NULL)
            break;
    } while (cli->pid_cliente != p->pid_cliente);
    return c;
}

unsigned int calc_espOnline(plista_cli cli, plista_med p){ // only counts peeps for a certain esp
    unsigned int c = 0;
    if (p==NULL || cli==NULL) 
        return 0;
            
    do {
        if ( strcmp(p->esp,cli->esp) == 0)
            c++;
        p = p->prox;
    } while (p != NULL);
    return c;
}

bool medExists(pid_t pid_med, plista_med p){
    if (p == NULL) return false;
    do {
        if ( p->pid_medico == pid_med)
            return true;
        p = p->prox;
    } while (p != NULL);
    return false;
}


plista_cli insert_end(plista_cli p, plista_cli novo_cli){
    plista_cli aux = NULL; 
    if(novo_cli == NULL) return p;
    
    // Inserir no fim da lista  
    if(p == NULL){  // Inserção à cabeça
       novo_cli->prox = p;
       p = novo_cli;
    }
    else{
        aux = p;
        while(aux->prox != NULL)
            aux = aux->prox;

        novo_cli->prox = NULL;
        aux->prox = novo_cli;
    }
    
    return p;   
}

void free_list(plista_cli p){
    plista_cli aux;
    fprintf(stderr, "Freeing cli list, PIDs:\n");
    while(p != NULL){
        aux = p;
        p = p->prox;
        fprintf(stderr, "%d\t", aux->pid_cliente);
        free(aux);
    }
    putchar('\n');
}

void free_list_med(plista_med p){
    plista_med aux;
    fprintf(stderr, "Freeing med list, PIDs:\n");
    while(p != NULL){
        aux = p;
        p = p->prox;
        fprintf(stderr, "%d\t", aux->pid_medico);
        free(aux);
    }
    putchar('\n');
}

plista_med insert_end_med(plista_med p, plista_med novo_med){
    plista_med aux = NULL; 
    if(novo_med == NULL) return p;
    
    // Inserir no fim da lista  
    if(p == NULL){  // Inserção à cabeça
       novo_med->prox = p;
       p = novo_med;
    }
    else{
        aux = p;
        while(aux->prox != NULL)
            aux = aux->prox;

        novo_med->prox = NULL;
        aux->prox = novo_med;
    }
    
    return p;   
}

void show_info(plista_cli p){
    while(p != NULL){
        fprintf(stderr, "%d\t%s\t%s\t%d\n", p->pid_cliente, p->nome, p->esp, p->prio);
        p = p->prox;
    }
}

void show_info_med(plista_med p){
    while(p != NULL){
        fprintf(stderr, "%d\t%s\t%s\t%d\n", p->pid_medico, p->nome, p->esp, p->disponivel);
        p = p->prox;
    }
}

void listaespera(plista_cli p){  
    plista_cli pauxc = p;
    if (p == NULL) return;
    // Go through clients
    for (int esp = 1; esp <= 5; esp++) {
        do {
            if (pauxc->atendido == false){
                if(esp == 1 && strcmp(pauxc->esp,"geral")==0){
                    printf("========== Geral ==========\n");printf("%d\t%s\t%d\n",pauxc->pid_cliente,pauxc->nome,pauxc->prio);
                } else if(esp == 2 && strcmp(pauxc->esp,"estomatologia")==0){
                    printf("========== Estomatologia ==========\n");printf("%d\t%s\t%d\n",pauxc->pid_cliente,pauxc->nome,pauxc->prio);
                } else if(esp == 3 && strcmp(pauxc->esp,"neurologia")==0){
                    printf("========== Neurologia ==========\n");printf("%d\t%s\t%d\n",pauxc->pid_cliente,pauxc->nome,pauxc->prio);
                } else if(esp == 4 && strcmp(pauxc->esp,"ortopedia")==0){
                    printf("========== Ortopedia ==========\n");printf("%d\t%s\t%d\n",pauxc->pid_cliente,pauxc->nome,pauxc->prio);
                } else if(esp == 5 && strcmp(pauxc->esp,"oftalmologia")==0){
                    printf("========== Oftalmologia ==========\n");printf("%d\t%s\t%d\n",pauxc->pid_cliente,pauxc->nome,pauxc->prio);}
            }
            pauxc = pauxc->prox;
        } while (pauxc != NULL);
        pauxc = p;
        
    }   
    p = p->prox;          
}

plista_med cleanDeadMed(plista_med p, pid_t med_pid){
    plista_med prev, curr; // <previous> <current>
    
    prev = NULL;
    curr = p;
    
    while(curr!=NULL && curr->pid_medico!=med_pid){
        prev = curr;
        curr = curr->prox;
    }

    if(curr == NULL){       // Livro nao existe
        fprintf(stderr, "Medico Não encontrado\n");
        return p;
    }
    
    if(prev == NULL)     // 1º nó da lista
        p = p->prox;
    else                            // outro nó
        prev->prox = curr->prox;
    
    free(curr);
    fprintf(stderr, "Medico expulso\n");

    return p;
}

plista_cli cleanDeadCli(plista_cli p, pid_t pid){
    plista_cli prev, curr; // <previous> <current>
    
    prev = NULL;
    curr = p;
    
    while(curr!=NULL && curr->pid_cliente!=pid){
        prev = curr;
        curr = curr->prox;
    }

    if(curr == NULL){       // Livro nao existe
        fprintf(stderr, "Cliente Não encontrado\n");
        return p;
    }
    
    if(prev == NULL)     // 1º nó da lista
        p = p->prox;
    else                            // outro nó
        prev->prox = curr->prox;
    
    free(curr);
    fprintf(stderr, "Cliente Expulso\n");

    return p;
}

void tellClisToDie(plista_cli p){
    if (p == NULL) return;
    suicide Die_Clis;
    Die_Clis.size=sizeof(Die_Clis);
    char cFifoName[50];
    int res;
    int npc;
    if (g_info.debugging) fprintf(stderr, "==Sending kill order to clis==\n");
    do {
        sprintf(cFifoName, CLIENT_FIFO, p->pid_cliente);

        // Opens clients FIFO for write
        npc = open(cFifoName, O_WRONLY);
        if (npc == -1) perror("Erro no open - Ninguém quis a resposta\n");
        else{
            // Send response
            res = write(npc, &Die_Clis, sizeof(Die_Clis));
            if (res == sizeof(Die_Clis) && g_info.debugging) // if no error
                fprintf(stderr, "==success writing kill order to cli of PID: %d==\n",p->pid_cliente);
            else
                perror("erro a escrever a resposta\n");
            close(npc); // FECHA LOGO O FIFO DO CLIENTE!
        }
        p = p->prox;
    } while (p != NULL);
}

void tellMedsToDie(plista_med p){
    if (p == NULL) return;
    suicide Die_Meds;
    Die_Meds.size=sizeof(Die_Meds);
    char cFifoName[50];
    int res;
    int npm;
    if (g_info.debugging) fprintf(stderr, "==Sending kill order to meds==\n");
    do {
        sprintf(cFifoName, MED_FIFO, p->pid_medico);

        // Opens clients FIFO for write
        npm = open(cFifoName, O_WRONLY);
        if (npm == -1) perror("Erro no open - Ninguém quis a resposta\n");
        else{
            // Send response
            res = write(npm, &Die_Meds, sizeof(Die_Meds));
            if (res == sizeof(Die_Meds) && g_info.debugging) // if no error
                fprintf(stderr, "==success writing kill order to med of PID: %d==\n",p->pid_medico);
            else
                perror("erro a escrever a resposta\n");
            close(npm); // FECHA LOGO O FIFO DO CLIENTE!
        }
        p = p->prox;
    } while (p != NULL);   
}

void changeFrqMeds(plista_med p, int freq){
    if (p == NULL) return;
    freq_tmed freq_tallmed;
    freq_tallmed.freq = freq;
    freq_tallmed.size=sizeof(freq_tallmed);
    char mFifoName[50];
    int res;
    int npm;
    if (g_info.debugging) fprintf(stderr, "==Sending freq to all meds==\n");
    do {
        sprintf(mFifoName, MED_FIFO, p->pid_medico);

        // Opens medics FIFO for write
        npm = open(mFifoName, O_WRONLY);
        if (npm == -1) perror("Erro no open - Ninguém quis a resposta\n");
        else{
            // Send response
            res = write(npm, &freq_tallmed, sizeof(freq_tallmed));
            if (res == sizeof(freq_tallmed) && g_info.debugging) // if no error
                fprintf(stderr, "==success writing freq period to med of PID: %d==\n",p->pid_medico);
            else
                perror("erro a escrever a resposta\n");
            close(npm); // FECHA LOGO O FIFO DO CLIENTE!
        }
        p = p->prox;
    } while (p != NULL);   
}

void makeEmTalk(plista_med pm, plista_cli pc){
    if (g_info.debugging) fprintf(stderr, "==makeEmTalk called==\n");
    if (pm == NULL || pc == NULL) return;
    plista_med pauxm = pm; 
    plista_cli pauxc = pc;
    // Go through clients
    for (int prio = 1; prio <= 3; prio++) {
        do {

            if (! (pauxc->prio!=prio || pauxc->atendido)){

                // Go through medics
                do{
                    if (pauxm->disponivel && strcmp(pauxm->esp,pauxc->esp)==0){
                        // If a match occurs:
                        if (g_info.debugging) fprintf(stderr, "==Match! med: %d, cli: %d==\n", pauxm->pid_medico, pauxc->pid_cliente);
                        pauxc->atendido = true;
                        pauxm->disponivel = false;
                        char mFifoName[50]; sprintf(mFifoName, MED_FIFO, pauxm->pid_medico);
                        char cFifoName[50]; sprintf(cFifoName, CLIENT_FIFO, pauxc->pid_cliente);
                        sendConnect(pauxm->pid_medico, cFifoName);
                        sendConnect(pauxc->pid_cliente, mFifoName);
                        break;
                    }

                    pauxm = pauxm->prox;
                } while (pauxm != NULL);
                pauxm = pm;

            }

            pauxc = pauxc->prox;
        } while (pauxc != NULL);
        pauxc = pc;
    }
}

// sendConnect(<PID of guy who he'll talk to>, <Fifo Name of who he have to send the connect message>)
void sendConnect(pid_t pid, char * FifoName){
    imDead connectToMed; // struct used to send connect order
    connectToMed.size = sizeof(connectToMed);
    connectToMed.pid = pid;

    // Opens medics FIFO for write
    int npm = open(FifoName, O_WRONLY);
    if (npm == -1) perror("Erro no open - Ninguém quis a resposta\n");
    else{
        // Send response
        int res = write(npm, &connectToMed, sizeof(connectToMed));
        if (res == sizeof(connectToMed) && g_info.debugging) // if no error
            fprintf(stderr, "==success writing connect order to: %d==\n",pid);
        else
            perror("erro a escrever a resposta\n");
        close(npm); // FECHA LOGO O FIFO DO CLIENTE!
    }
}

int makeMedAvailable(plista_med p, pid_t pid){
    if (p == NULL) return -2;
    do {
        if ( !p->disponivel )
            p->disponivel = true;
        else 
            return -1;
        p = p->prox;
    } while (p != NULL);
    return 1;
}