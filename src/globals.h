#ifndef GLOBALS
#define GLOBALS

// balcao
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <limits.h>

//cliente
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#include <stdbool.h>
#include <pthread.h>

// Balcao FIFO's name
#define BALCAO_FIFO "/tmp/balcao_fifo"

// Client FIFO's name, %d will be substituted with sprintf
#define CLIENT_FIFO "/tmp/cliente_%d_fifo"

// MED FIFO's name, %d will be substituted with sprintf
#define MED_FIFO "/tmp/med_%d_fifo"

// max size of each message
#define TAM_MAX_MSG 100

#define WAIT_TIME 15

void debugString(char * str);

#endif