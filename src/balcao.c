#include "libraries.h"
#include "structs.h"
#include <stdio.h>

// Corre só balcãocom:
/*
    gcc balcao.c -o balcao && ./balcao
*/

void trata_sinal(int s) {
    static int a; 
    a++;
    printf("Recebido sinal SIGPIPE %d ",a); // fflush(stdout);
    /*
    if (a == 5) {
        printf("\naté logo ");
        exit(0);
    }
    */
}

int main(int argc, char **argv,  char *envp[]){
    setbuf(stdout, NULL);
    signal(SIGPIPE,trata_sinal);
    printf("Ola eu sou o balcao\n");
    printf("My PID is: %d\n", getpid());

    char input[100];
    //fgets(input, 100, stdin);
    //printf("%s",input);
    //sscanf()
    //execlp("./../classificador", "classificador", input, NULL);

    // To start another program without killing the current one
    // we need to start a fork before execlp as the latter will destroy our process


    // Obter environment variables
    char *pointer=envp[0];
    int i=0;
    while (pointer!=NULL){
        //printf("%s\n",envp[i]);
        pointer=envp[++i];
    }


    // Pipe from ls to wc
    int res;
    int fd[2];
    pipe(fd);
    res = fork();
    if (res == 1) {
        perror("erro fork: ");
        return 2;
    }
    if (res == 0) { //FILHO
        close(STDIN_FILENO); // stdin é um FILE *, logo tens que usar STDOUT FILE NUMBER instead of 1
        /*Acede ao primeiro indice da seguinte tabela:
            0      1       2    
        ┌──────┬───────┬───────┬────┬────┐
        │stdin │stdout |stderr │ <o resto está vazio>
        └──────┴───────┴───────┴────┴────┘ */   
        dup(fd[0]); // duplica lado leitura no lugar stdin
        close(fd[0]); // ja tem o duplicado, podefechar este
        close(fd[1]); // não precisa do lado escrita - fecha
        execlp("./../classificador", "classificador", NULL); // executa o prog2
        perror("erro exec prog2: ");
        return 3;
    }

    close(STDOUT_FILENO); // Fecha STDOUT
    dup(fd[1]); // duplica STDOUT para lugar recem liberto
    close(fd[1]); // ja tem o duplicado, não precisa este
    close(fd[0]); // não vai precisar de ler - fecha lado de leitura
    execlp("./cliente","cliente",NULL); 
    perror("erro exec prog1: ");
    return 4;
}