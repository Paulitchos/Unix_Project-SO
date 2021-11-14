# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>

/*
wc funcionamento:
wc sozinho, lê do stdin, podes mandar um EOF com CTRL + D para acabar

output
NºLinhas  NºPalavras  NºChars
*/

// Experimenta correr: gcc ./execFromFile.c -o execFromFile && ./execFromFile test.txt wc

// Primeiro argumento é input, segundo é output
int main(int argc, char*argv[]){
    int res;
    if(argc<3){
        printf("\nindicar ficheiro programa se faz favor\n");
        return 1;
    }
    // close(0); // OU
    close(STDIN_FILENO); // stdin é um FILE *, logo tens que usar STDIN FILE NUMBER instead of 0
    /*
    Acede ao primeiro indice da seguinte tabela:
	  0    1    2    
	┌──────┬───────┬───────┬────┬────┐
	│stdin │stdout |stderr │ <o resto está vazio>
	└──────┴───────┴───────┴────┴────┘
    */
    res = open(argv[1], O_RDONLY);
    if (res == -1) {
        perror("erro ficheiro: ");
        return 2;
    }
    execlp(argv[2], argv[2], NULL);
    perror("erro comando: ");
    return 0; // Não chega a esta linha
}