#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// https://stackoverflow.com/questions/2748995/struct-memory-layout-in-c

typedef struct {
    long long i; // long long porque 64bit processors conseguem ler 8 bytes num ciclo e C tenta organizar os dados dentro de uma struct de forma a estarem no inicio destes 8 bytes, para não criar descrepância, ocupamos os primeiros 8 bytes com um lnog ong para quando reconstruir a estrutura não a organizar de forma diferente
    char ch1;
    short s;
    char ch2;
    char ch3[23];
    long long ll; 
} structA;

typedef struct {
    char ch1;
    short s;
    char ch2;
    char ch3[23];
    long long ll;   
} structAWithoutInt;

// -------------- To show that structs with same data can have diferent sizes ----------
typedef struct {
    char ch1;
    short s;
    char ch2;
    long long ll;
    int i;
} structC;

typedef struct {
    long long ll; // @ 0
    int i;        // @ 8
    short s;      // @ 12
    char ch1;     // @ 14
    char ch2;     // @ 15
} structD;
// -------------- To show that structs with same data can have diferent sizes ----------

int main() {
    char ptr[300];

    structA a;
    a.i = 2; a.ch1 = 'a'; a.s = 4; a.ch2 = 'b', a.ll = 5555; strcpy(a.ch3, "pepeguices hjhajahjshjs");

    memcpy(ptr,&a,sizeof(structA)); //copia a estrutura a para onde apontar a string

    printf("sizeof zone of memory: %lu\n",sizeof(a));

    structAWithoutInt* catchedStruct = (structAWithoutInt*)(ptr+sizeof(long long));
    printf("ch1->%c\n",catchedStruct->ch1);
    printf("s->%d\n",catchedStruct->s);
    printf("ch2->%c\n",catchedStruct->ch2);
    printf("ch3 ->%s\n",catchedStruct->ch3);
    printf("ll->%lld\n",catchedStruct->ll);

    printf("sizeof(structC)=%d\nsizeof(structD)=%d\n", (int)sizeof(structC),(int)sizeof(structD));
    return 0;
}