# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>


//depois de receber 5 vezes o sinal CTRL + C, sai
void trata_sinal(int s) {
    static int a; 
    a++;
    printf("ouch %d ",a); // fflush(stdout);
    if (a == 5) {
        printf("\naté logo ");
        exit(0);
    }
}
//Catch SIGINT signal (CTRL+C)
int main(){
    setbuf(stdout, NULL);
    signal(SIGINT,trata_sinal);
    printf("Nao cliques CTRL+C\n");
    while(1) pause(); // unica maneira de quebrar o pause é recebendo um sinal
    return 0;
}