#include <stdio.h>

int main(){
    char a[40];
    int i = 0;
    while (1) {
        while (1) {
            scanf("%c", &a[i]);
            if (a[i] == '\n') {
                break;
            }
            else {
                i++;
            }
        }
        a[i] = '\0';
        printf("%s",a);
    }
}
