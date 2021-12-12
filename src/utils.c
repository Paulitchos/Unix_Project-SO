#include "globals.h"
#include <string.h>

void debugString(char * str){
	int i = 0;
	do {
		if (str[i] == '\n') {
			fprintf(stderr, "\\"); fprintf(stderr,"n"); 
		} else if (str[i] == '\0') {
			fprintf(stderr, "\\"); fprintf(stderr, "0"); break;
		} else {
			fprintf(stderr, "%c", str[i]);
		}
		i++;
	}while (i < strlen(str)+1);
}