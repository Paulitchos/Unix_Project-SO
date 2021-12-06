#include "structs.h"
#include "globals.h"


int main(int argc, char **argv){
    /*
    printf("Ola eu sou o cliente\n");
    printf("My PID is: %d\n", getpid());
    printf("Numero de Argumentos: %d\nArgumentos:\n", argc);
    for (int i = 0; i < argc; i++){
        printf(" %s ", argv[i]);
    }
    */

    cliente infor;
    int res1;
    int tam;
    
    
    if (argc != 2){
        printf("Deve introduzir apenas o seu nome\n");
        printf("Exemplo ./cliente <nome>\n");
        exit(EXIT_SUCCESS);
    }
     
    sscanf(argv[1], "%s", &infor.nome);
        
    printf("%s\n",infor.nome);
    
    printf("Qual o seu NIF?\n");
 
    tam = read(STDIN_FILENO,infor.nif,sizeof(infor.nif)); // read from user
    if (tam <= -1) { printf("Error Reading, output: %d\n",tam);return 1;}; 
    if(tam != 8) { printf("NIF contem 9 digitos\n");return 1;}

    printf("%d\n",infor.nif);
  
	return (0);
    
}