#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define TAM 20

// Run balcao:
/*
    gcc ./src/balcao.c -o ./dist/balcao && ./dist/balcao
*/

void trata_sinal(int s) {
    static int a; 
    a++;
    //SIGPIPE is the "broken pipe" signal, which is sent to a process when it attempts to write to a pipe whose read end has closed (or when it attempts to write to a socket that is no longer open for reading), but not vice versa. The default action is to terminate the process.
    printf("Recebido sinal SIGPIPE %d ",a); // fflush
}

int main(int argc, char **argv,  char *envp[]){
    setbuf(stdout, NULL);
    signal(SIGPIPE,trata_sinal);

    // Get environment variables
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

    printf("Ola eu sou o balcao\n");
    printf("My PID is: %d\n", getpid());
    printf("MAXCLIENTES:%d\n",maxclientes);
    printf("MAXMEDICOS:%d\n",maxmedicos);

    
    int fres;
    int fd[2]; // input, output
    pipe(fd);
    int df[2];
    pipe(df);
    fres = fork();
    if (fres == 1) {
        perror("erro fork: ");
        return 2;
    }
    if (fres == 0) { //CHILD
        // doesn't need writing side - close
        close(STDIN_FILENO); // stdin is a FILE *, thus you need to use STDIN_FILE_NUMBER instead of stdin
        /*Accesses first element of the following table:
            0      1       2
        ┌──────┬───────┬───────┬────┬────┐
        │stdin │stdout |stderr │ <rest is empty>
        └──────┴───────┴───────┴────┴────┘ */
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
    }
    else { // PARENT
        close(fd[0]); // has the duplicated, doesn't need this one
        close(df[1]);

        int tam=TAM;
        char phrase [TAM];
        char *b = phrase;
        size_t tam_phrase = TAM;
        int chars;
        chars = getline(&b,&tam_phrase,stdin);
        // fgets(phrase, sizeof(phrase),stdin);

        do{
            write(fd[1],phrase,strlen(phrase)+1);   // write is waiting for amount of characters +1 for \0
            // write uses strlen to not write unecessary data as we know all the data we want to write
            tam = read(df[0],phrase,sizeof(phrase));   // tam = sizeof(phrase if all is fine) | -1 i error while reading, 0 is unexpected EOF
            // read uses sizeof because we don't know what we'll recieve àpriori, so we send the max size of the string
            phrase[tam] = '\0';

            printf("%.*s",tam,phrase);

        } while (true);    
        return 4;
    }
}


// Perguntas ao Professor:
// what does %.*s do
// perguntar sobre o strlen + 1 ou o sizeof
// no write o string lenght + 1 é por causo do \n ou do \0 ou outra coisa?
// o scanf não retorna \n no fim
// o gets retorna \n sempre no fim
// nós queremos que retorne o \n para o classificador funcionar corretamente