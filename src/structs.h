#ifndef STRUCTS
#define STRUCTS

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

typedef struct holy{
    int l;
    int c;
    int playtype;
    struct holy *prox;
}data, *pdata;

#endif