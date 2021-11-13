#ifndef STRUCTS
#define STRUCTS

#define WAIT_TIME 15

typedef struct holy{
    int l;
    int c;
    int playtype;
    struct holy *prox;
}data, *pdata;

#endif