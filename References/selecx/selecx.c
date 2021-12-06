#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <signal.h>
#include <string.h>

/*
Run with:
    gcc selecx.c -o /tmp/selecx && /tmp/selecx
*/

void leEMostraPipes(char * quem, int fd);
void trataTeclado();
void sayThisAndExit(char * p);
int max(int a, int b);
void trataCC(int s);

int main(int arcg, char * argv[]){
    printf("\nStarting...\n");

    int fd_a, fd_b, fd_c; // file descriptor pipes
    int nfd; //valor retorno select()
    fd_set read_fds; //conjunto das flags para desc. ficheiros
    struct timeval tv; //timeout para select

    signal(SIGINT, trataCC); // para interroper via ^C

    //cria pipes
    mkfifo("pipe_a",00777);
    mkfifo("pipe_b",00777);

    //abre os pipes. RDRW vs RD -notar isto
    fd_a = open("pipe_a",O_RDWR | O_NONBLOCK);

    if(fd_a == -1)
        sayThisAndExit("ERRO no open pipe fd_a");

    fd_b = open("pipe_b",O_RDWR | O_NONBLOCK);

    if(fd_a == -1)
        sayThisAndExit("ERRO no open pipe fd_b");
        
    while (1){
        
        tv.tv_sec = 10; //segundos (10 = apenas um exemplo)
        tv.tv_usec = 0; //micro-segundos (se ambos a 0 então faz polling)
        
        FD_ZERO(& read_fds); //inicializa conjunto de fd (watch list)
        FD_SET(0,& read_fds); //adiciona stdin ao conj de fd a observar
        FD_SET(fd_a,& read_fds); //adiciona pipe_a ao conj de fd a observar
        FD_SET(fd_b,& read_fds); //adiciona pipe_b ao conj de fd a observar

        //vê se há dados em alguns dos fd (stdin,pipes) - modifica os sets
        //bloqueia ate: sinal, timeout, há dados para ler EOF, exception
        
        nfd = select(           //bloqueia até haver dados ou EOF no read-set
            max(fd_a,fd_b)+1,   //max valor dos vários fd + 1
            & read_fds,         //read fd set
            NULL,               //write fd set- (nenhum aqui)
            NULL,               //exeception fd set- (nenhum aqui)
            & tv);              //timeout- se ambos = 0-> returna logo (p/ polling)
            // actualiza tv -> quanto tempo faltava para o timeout
    
        if(nfd == 0) {
            printf("\n(Estou a espera....)\n"); fflush(stdout);
            continue;
        }

        if (nfd == -1) {
            perror("\nerro no select");
            close(fd_a); close(fd_b);
            unlink("pipe_a"); unlink("pipe_b");
            return EXIT_FAILURE;
        }
        
        if (FD_ISSET(0, & read_fds)) { // stdin tem algo para ler?
            trataTeclado();
            // sem "contenue" -> não vai logo para a próxima iterção
        }   // porque pode ser que pipe_a ou pipe_b tembém tenham algo

        if (FD_ISSET(fd_a, & read_fds)) { // fd_a tem algo para ler?
            leEMostraPipes("A", fd_a); // função auxiliar para ler pipes
            // sem "continue" -> não vai logo para a próxima interação
        }   // porque pode ser que pipe_b também tenha algo
        
        if (FD_ISSET(fd_b, & read_fds)) { // fd_b tem algo para ler?
            leEMostraPipes("B", fd_b);
            // ocódigo dentro do ciclo acaba jáa a seguir e volta a esperar por dados
        }
    }    

    // em prencípio, neste exemplo, não deve chegar aqui
    return EXIT_SUCCESS;
}

void leEMostraPipes(char * quem, int fd){
    char buffer[200];
    int bytes;
    bytes = read(fd, buffer, sizeof(buffer));
    buffer[bytes] = '\0';
    if ( (bytes > 0) && (buffer[strlen(buffer)-1] == '\n') )
        buffer[strlen(buffer)-1] = '\0';
    printf("\n%s: (%d bytes) [%s]\n", quem, bytes, buffer);
    if (strcmp(buffer, "sair")==0) {
        unlink("pipe_a"); unlink("pipe_b");
        exit(EXIT_SUCCESS);
    }
}

void trataTeclado() {
    char buffer[200];
    int bytes;
    fgets(buffer, sizeof(buffer), stdin); // scanf("%s", buffer);
    if ( (strlen(buffer)>0) && (buffer[strlen(buffer)-1] == '\n') )
        buffer[strlen(buffer)-1] = '\0';
    printf("\nKeyboard: [%s]\n", buffer);
    fflush(stdout);
    if (strcmp(buffer, "sair") == 0) {
        unlink("pipe_a"); unlink("pipe_b");
        exit(EXIT_SUCCESS);
    }
}

void sayThisAndExit(char * p){
    perror(p);
    exit(EXIT_FAILURE);
}

int max(int a, int b){
    return (a>b) ? a : b;
}

void trataCC(int s){
    unlink("pipee_a"); unlink("pipe_b");
    printf("\n ->CC<- \n\n");
    exit(EXIT_SUCCESS);
}