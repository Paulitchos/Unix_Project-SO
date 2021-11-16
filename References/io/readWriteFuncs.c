# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <stdbool.h>
# include <signal.h>

int main(){
    char str_xpto[20];
    char frase [20] = "doi-me a barriga\n";
    write(STDOUT_FILENO,frase,strlen(frase)); // df[0]?
    read(STDIN_FILENO,frase,strlen(frase));
    write(STDOUT_FILENO,frase,strlen(frase)); // df[0]?
    return 0;
}
            