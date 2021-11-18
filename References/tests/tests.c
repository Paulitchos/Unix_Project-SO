#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char*argv[],char *envp[]) {
    char string[20] = "                   ";
    strcpy(string,"ola p\n");
    string[18] = '\0';
    string[6] = ' ';
    printf("%lu\t%lu", strlen(string), sizeof(string));
    return 0;
} 
