// This is the | Operator

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>

// Experimenta correr: 
// gcc ./exec2exec.c -o exec2exec && ./exec2exec ls wc
// Tem que dar o mesmo output que: ls | wc

//primeiro o input depois o output
int main(int argc, char*argv[]){
    int res;
    int fd[2];
    if (argc < 3) {
        printf("\nIndicar programa e progama2 sff\n");
        return 1;
    }
    pipe(fd);
    res = fork();

    if (res == 1) {
        perror("erro fork: ");
        return 2;
    }

    if (res == 0) { //FILHO
        //fecha stdin: close(0); // OU
        close(STDIN_FILENO); // stdin é um FILE *, logo tens que usar STDOUT FILE NUMBER instead of 1
        /*Acede ao primeiro indice da seguinte tabela:
            0      1       2    
        ┌──────┬───────┬───────┬────┬────┐
        │stdin │stdout |stderr │ <o resto está vazio>
        └──────┴───────┴───────┴────┴────┘ */   
        dup(fd[0]); // duplica lado leitura no lugar stdin
        close(fd[0]); // ja tem o duplicado, podefechar este
        close(fd[1]); // não precisa do lado escrita - fecha
        execlp(argv[2], argv[2],NULL); // executa o prog2
        perror("erro exec prog2: ");
        return 3;
    }

    close(STDOUT_FILENO); // Fecha STDOUT
    dup(fd[1]); // duplica STDOUT para lugar recem liberto
    close(fd[1]); // ja tem o duplicado, não precisa este
    close(fd[0]); // não vai precisar de ler - fecha lado de leitura
    execlp(argv[1],argv[1],NULL); 
    perror("erro exec prog1: ");
    return 4;

}