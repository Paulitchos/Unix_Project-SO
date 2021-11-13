#include "libraries.h"
#include "structs.h"

int main(int argc, char **argv,  char *envp[]){
    printf("Ola eu sou o balcao\n");
    printf("My PID is: %d\n", getpid());
    char input[100];
    fgets(input, 100, stdin);
    printf("%s",input);
    //sscanf()

    char *pointer=envp[0];
    int i=0;
    while (pointer!=NULL){
        printf("%s\n",envp[i]);
        pointer=envp[++i];
    }
    return 0;
}