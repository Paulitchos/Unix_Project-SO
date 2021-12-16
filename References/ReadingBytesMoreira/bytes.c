#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int i;
    char ch1;
    short s;
    char ch2;
    long long ll;
} structB;

typedef struct {
    long long ll; // @ 0
    int i;        // @ 8
    short s;      // @ 12
    char ch1;     // @ 14
    char ch2;     // @ 15
} structC;

int main() {
    structB a;
    a.i = 2; a.ch1 = 'a'; a.s = 4; a.ch2 = 'b', a.ll = 5555;

    void * ptr = malloc(sizeof(a)); // lugar na memória pra meter bytes

    memcpy(ptr,&a,sizeof(structB)); //copia a estrutura a para onde apontar a string

    printf("sizeof zone of memory: %lu\n",sizeof(a));

    structB* b_val = (structB*)ptr;
    printf("i->%d\n",b_val->i);
    printf("ch1->%c\n",b_val->ch1);
    printf("s->%d\n",b_val->s);
    printf("ch2->%c\n",b_val->ch2);
    printf("ll->%lld\n",b_val->ll);

    printf("sizeof(structB)=%d\nsizeof(structC)=%d\n", (int)sizeof(structB),(int)sizeof(structC));
    return 0;
}