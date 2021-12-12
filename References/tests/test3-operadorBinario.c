#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <signal.h>
#include <string.h>

//bit wise and, bit wise or

int main(){
    printf("%d\n", O_RDWR | O_NONBLOCK);
    printf("binario: %d\n", 0b101); // binario
    printf("hexadecimal: %d\n", 0x101); // hexadecimal
    printf("octal: %d\n", 00101); // octal
    return 0;
}
