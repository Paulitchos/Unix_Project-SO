#include <stdio.h>
#define CLIENT_FIFO "/tmp/resp_%d_fifo"

int main(){
    int test = 100;
    char imok[100] = "Im not ok";
    // string
    sprintf(imok,CLIENT_FIFO,test);
    printf("%s", imok);
}
