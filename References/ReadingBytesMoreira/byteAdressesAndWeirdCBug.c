#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    int i;
    char ch1;
    short s;
    char ch2;
    char ch3[23];
    long long ll; 
} structA;

int main(){
    char pipe[200];
    
    structA* a = (structA*)malloc(sizeof(structA));
    a->i = 18;
    a->ch1 = 'B';
    a->s = 5;
    //...
    
    //simulating bytes on pipe
    memcpy(pipe, a, sizeof(structA));
    
    //reading first int from pipe to see if we can recreate the struct
    int* first_int = (int*)pipe;
    
    if(*first_int == 18){
        
        //recreating struct from pipe
        structA *recreated_struct = (structA*)malloc(sizeof(structA));
        memcpy(recreated_struct, pipe, sizeof(int));
        memcpy( &(recreated_struct->ch1) /* recreated_struct+sizeof(int) */ , pipe+sizeof(int), sizeof(structA)-sizeof(int));
        
        printf("%x \n", recreated_struct);
        printf("%x | %x\n", &(recreated_struct->ch1), &(recreated_struct->i)+1 /*&(recreated_struct)+1 diferent??*/);
        printf("%x | %x || %x | %x\n", (recreated_struct), &(recreated_struct->i), (recreated_struct)+1, &(recreated_struct->i)+1);

        printf("{i:%d, ch1:%c, s:%d}", recreated_struct->i, recreated_struct->ch1, recreated_struct->s);
        
    }

    return 0;
}