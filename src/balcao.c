#include "structs.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>

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

// têm que ser global para ser tratadas no trata_SIGINT
static plista_cli cli_list = NULL;
static plista_med med_list = NULL;
static int	npb; // <named pipe balcao>

void trata_SIGPIPE(int s) {
    static int a; 
    a++;
    //SIGPIPE is the "broken pipe" signal, which is sent to a process when it attempts to write to a pipe whose read end has closed (or when it attempts to write to a socket that is no longer open for reading), but not vice versa. The default action is to terminate the process.
    printf("Recebido sinal SIGPIPE %d ",a);
}

void trata_SIGINT(int i) { // CTRL + C
	(void) i;    //? necessary? Does what
	fprintf(stderr, "\nBalcao a terminar\n");
	// ===== Close Pipes ===== //
    close(npb);
	unlink(BALCAO_FIFO);

    // ===== Free Linked Lists ===== //
    free_list(cli_list);

	exit(EXIT_SUCCESS);
}

void exceptionOcurred(){
    close(npb);
	unlink(BALCAO_FIFO);
    exit(EXIT_FAILURE);
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
        char mFifoName[50];
        unsigned short pipeMsgSize;
        int prioridade;

        res = mkfifo(BALCAO_FIFO, 0777);
        if (res == -1){	perror("\nmkfifo do FIFO do balcao deu erro"); exit(EXIT_FAILURE); }
        if (debugging) fprintf(stderr, "==FIFO servidor criado==\n");

        npb = open(BALCAO_FIFO, O_RDWR); // opens in Read/Write mode to prevent getting stuck in open, it's still blocking
        if (npb == -1){ perror("\nErro ao abrir o FIFO do servidor (RDWR/blocking)\n"); exit(EXIT_FAILURE);	}
        if (debugging) fprintf(stderr, "==FIFO aberto para READ (+WRITE) BLOQUEANTE==\n");        

        do{
            // Recieve simptoms from cliente
            if (read(npb, &pipeMsgSize, sizeof(pipeMsgSize)) <= -1 ) { 
                printf("Error Reading, output\n"); trata_SIGINT(0); }
            if (pipeMsgSize == sizeof(sint_fcli)){
                if(debugging) fprintf(stderr,"==Recieved Msg Type \"sint_fcli\"==\n");
                sint_fcli.size = sizeof(sint_fcli);
                res = read(npb, &(sint_fcli.size)+1, (int)sizeof(sint_fcli)-sizeof(sint_fcli.size));
                if (res == (int)sizeof(sint_fcli)-sizeof(sint_fcli.size)) {

                    // ============== Communicate with Classificador (2) ============== //
                    if(write(fd[1], sint_fcli.sintomas, strlen(sint_fcli.sintomas)) <= -1){ // write to pipe, write is waiting for amount of characters
                        printf("Error Writing\n") ; return 1; }
                    // write uses strlen to not write unecessary data as we know all the data we want to write
                    
                    if (debugging) {fprintf(stderr, "==Recebido do cliente & enviado para o classificador: |"); debugString(sint_fcli.sintomas); fprintf(stderr, "|==\n"); }

                    tam = read(df[0],msgClassificador,sizeof(msgClassificador)); // read from pipe, tam = sizeof(phrase if all is fine) | -1 i error while reading, 0 is unexpected EOF
                    if (tam <= -1 ) { printf("Error Reading, output: %d\n",tam); return 1; }
                    // read uses sizeof because we don't know what we'll recieve àpriori, so we send the max size of the string
                    msgClassificador[tam] = '\0';

                    if (debugging) {fprintf(stderr, "==Recebido do classificador: |"); debugString(msgClassificador); fprintf(stderr, "|==\n"); }
                    // ============== ================================== ============== //

                    // Get Filename of client's FIFO to send response
                    sprintf(cFifoName, CLIENT_FIFO, sint_fcli.pid_cliente);

                    // =========== Separar a especialidade e a prioridade ===========
                    sscanf(msgClassificador,"%s %d",info_tcli.esp,&info_tcli.prio);
                    if (debugging) { fprintf(stderr, "==Especialidade -> %s | Prioridade -> %d==\n",info_tcli.esp, info_tcli.prio); }
                    // =========== ====================================== ===========

                    info_tcli.num_peopleAhead = calc_peepAhead(novo_cli, cli_list);
                    if (debugging) fprintf(stderr, "==people ahead: %d==\n", info_tcli.num_peopleAhead);

                    // =========== Save in Linked List =========== //
                    novo_cli = malloc(sizeof(*novo_cli));
                    if (novo_cli==NULL) { fprintf(stderr,"==Malloc Error on new Client==\n"); exceptionOcurred();}
                    novo_cli->pid_cliente = sint_fcli.pid_cliente;
                    strcpy(novo_cli->nome, sint_fcli.nome);
                    strcpy(novo_cli->esp, info_tcli.esp);
                    novo_cli->prio = info_tcli.prio;
                    novo_cli->prox = NULL;
                    cli_list = insert_end(cli_list, novo_cli);
                    if (debugging) fprintf(stderr, "==Added New cli to Linked List, showing info:==\n");
                    if (debugging) show_info(cli_list);
                    if (debugging) fprintf(stderr, "====\n");
                    // =========== =================== =========== //

                    info_tcli.num_espOnline = calc_espOnline(novo_cli, med_list);
                    if (debugging) fprintf(stderr, "==especialistas online: %d==\n", info_tcli.num_espOnline);

                    // Opens clients FIFO for write
                    npc = open(cFifoName, O_WRONLY);
                    if (npc == -1) perror("Erro no open - Ninguém quis a resposta\n");
                    else{
                        if (debugging) fprintf(stderr, "==FIFO cliente aberto para WRITE==\n");

                        // Send response
                        res = write(npc, &info_tcli, sizeof(info_tcli));
                        if (res == sizeof(info_tcli) && debugging) // if no error
                            fprintf(stderr, "==escreveu a resposta com sucesso para cliente==\n");
                        else
                            perror("erro a escrever a resposta\n");

                        close(npc); // FECHA LOGO O FIFO DO CLIENTE!
                        if (debugging) fprintf(stderr, "==FIFO cliente fechado==\n");
                    }
                } else {
                    printf("Resposta incompreensível: %d]\n", res);
                }

            } else if (pipeMsgSize == sizeof(esp_fmed)) {
			    if(debugging) fprintf(stderr,"==Recieved Msg Type \"esp_fmed\"==\n");
                res = read(npb, &(esp_fmed.size)+1, (int)sizeof(esp_fmed)-sizeof(esp_fmed.size));
                if (res == (int)sizeof(esp_fmed)-sizeof(esp_fmed.size)){
                    if (debugging) fprintf(stderr, "==nome medico: %s | especialidade: %s==\n",esp_fmed.nome, esp_fmed.esp);
                    
                    if(medExists(esp_fmed.pid_medico,med_list)){
                        printf("LIFE SIGNAL -> %s %s %d\n", esp_fmed.nome, esp_fmed.esp, esp_fmed.pid_medico);
                    } else {
                        // =========== Save in Linked List =========== //
                        novo_med = malloc(sizeof(*novo_med));
                        if (novo_med==NULL) { fprintf(stderr,"==Malloc Error on new Client==\n"); exceptionOcurred();}
                        novo_med->pid_medico = esp_fmed.pid_medico;
                        strcpy(novo_med->nome, esp_fmed.nome);
                        strcpy(novo_med->esp, esp_fmed.esp);
                        novo_med->disponivel = 1;
                        novo_med->prox = NULL;
                        med_list = insert_end_med(med_list, novo_med);
                        if (debugging) fprintf(stderr, "==Added New med to Linked List, showing info:==\n");
                        if (debugging) show_info_med(med_list);
                        if (debugging) fprintf(stderr, "====\n");
                        // =========== =================== =========== //
                    }
                } else {
                    printf("Resposta incompreensível: %d]\n", res);
                }
            } else {
                fprintf(stderr,"No recognizable message recieved from pipe, aborting\n");
                exceptionOcurred();
            }
        } while (true);    

        return 4;
    }
	// shouldn't reach this point
	close(npb);
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
    } while (p != NULL || cli->pid_cliente != p->pid_cliente);
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
    fprintf(stderr, "Freeing PIDs:\n");
    while(p != NULL){
        aux = p;
        p = p->prox;
        fprintf(stderr, "%d\t", aux->pid_cliente);
        free(aux);
    }
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

void free_list_med(plista_med p){
    plista_med aux;
    fprintf(stderr, "Freeing PIDs:\n");
    while(p != NULL){
        aux = p;
        p = p->prox;
        fprintf(stderr, "%d\t", aux->pid_medico);
        free(aux);
    }
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