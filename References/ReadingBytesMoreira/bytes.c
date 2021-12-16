#include <stdio.h>
#include <string.h>

typedef struct {
    int i;
    int j;
} structA;

typedef struct {
   char ch1;
   short s;
   char ch2;
   long long ll;
   int i;
} structB;

typedef struct {
   long long ll; // @ 0
   int i;        // @ 8
   short s;      // @ 12
   char ch1;     // @ 14
   char ch2;     // @ 15
} structC;

int main() {
    char string[8];
    structA a;
    a.i = 5;
    a.j = 4;
    
    printf("sizeof(structA)=%d\n", (int)sizeof(structA));
    memcpy(string,&a,sizeof(structA)); //copia a estrutura a para onde apontar a string

    int* i_val = (int*)string;
    int* j_val = (int*)(string+sizeof(int));
    printf("i->%d\n",*i_val);
    printf("j->%d\n",*j_val);

    printf("sizeof(structB)=%d\nsizeof(structC)=%d\n", (int)sizeof(structB),(int)sizeof(structC));
    return 0;
}