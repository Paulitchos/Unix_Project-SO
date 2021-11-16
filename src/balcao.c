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
    // USAR getenv()
    char *pointer=envp[0];
    int i=0;
    while (pointer!=NULL){
        //printf("%s\n",envp[i]);
        pointer=envp[++i];
    }

    // Pipe from ls to wc
    int res;
    int fd[2]; // input, output
    pipe(fd);
    int df[2]; // input, output
    pipe(df);
    res = fork();
    if (res == 1) {
        perror("erro fork: ");
        return 2;
    }
    if (res == 0) { //FILHO
         // stdin é um FILE *, logo tens que usar STDOUT FILE NUMBER instead of 1
        /*Acede ao primeiro indice da seguinte tabela:
            0      1       2
        ┌──────┬───────┬───────┬────┬────┐
        │stdin │stdout |stderr │ <o resto está vazio>
        └──────┴───────┴───────┴────┴────┘ */   
        //dup(fd[0]); // duplica lado leitura no lugar stdin
        //close(fd[0]); // ja tem o duplicado, podefechar este
        close(STDIN_FILENO); // não precisa do lado escrita - fecha
        dup(fd[0]);
close(fd[0]);close(fd[1]);


close(STDoOUT_FILENO); // não precisa do lado escrita - fecha
        dup(df[1]);
close(df[0]);close(df[1]);



        //char foo[4096];
        //int nbytes = read(fd[0], foo, sizeof(foo));
        //printf("Output: (%.*s)\n", nbytes, foo);
        //write(STDIN_FILENO, foo, strlen(foo)+1);
        
        execlp("./../classificador", "classificador", NULL); // executa o prog2
        perror("erro exec prog2: ");
        return 3;
    }
   else{
        //close(STDOUT_FILENO); // Fecha STDOUT
        //dup(fd[1]); // duplica STDOUT para lugar recem liberto 

       
        close(fd[0)]; // ja tem o duplicado, não precisa este
        close (df[1]]);
do{       
        write(fd[0],"doi-me a barriga\n",11);
        tam = read(df[0],str_cpto,tam)

        str_cpto[tam]="/0";
}
        
        return 4;
    }
}