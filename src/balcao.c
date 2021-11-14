#include "libraries.h"
#include "structs.h"

int main(int argc, char **argv,  char *envp[]){
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
        printf("%s\n",envp[i]);
        pointer=envp[++i];
    }

    int res;
    close(1);
    res = open(texto.txt, O_WRONLY | O_CREAT, 00600);
    execlp("./../classificador", "classificador", NULL);
    printf("Erro na execução");
    return 0;
}