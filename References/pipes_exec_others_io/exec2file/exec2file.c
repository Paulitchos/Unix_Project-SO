# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>

// rm texto.txt primeiro, porque como está não dá override
// Experimenta correr: 
// if [ -f "test.txt" ] ; then rm "test.txt" ; fi && gcc ./exec2file.c -o exec2file && ./exec2file ls test.txt 

//primeiro o input depois o output
int main(int argc, char*argv[]){
    int res;
    if(argc<3){
        printf("\nindicar programa e ficheiro se faz favor\n");
        return 1;
    }
    //close(1); // OU
    close(STDOUT_FILENO); // stdout é um FILE *, logo tens que usar STDOUT FILE NUMBER instead of 1
    /*
    Acede ao primeiro indice da seguinte tabela:
	   0        1       2    
	┌──────┬───────┬───────┬────┬────┐
	│stdin │stdout |stderr │ <o resto está vazio>
	└──────┴───────┴───────┴────┴────┘
    */
    res = open(argv[2], O_WRONLY | O_CREAT, 00600);
    if (res == -1) {
        perror("erro ficheiro: ");
        return 2;
    }
    printf("writing to %s",argv[2]);
    execlp(argv[1], argv[1], NULL);
    perror("erro comando: ");
    return 0; // Não chega a esta linha
}