#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

#define MUTEXSEM "/sempai"
#define FULLSEM "/fullsem"
#define EMPTYSEM "/emptysem"
#define WRITESEM "/writesem"

typedef struct{
    int from;
    int to;
} Request;
    
typedef struct{
    Request* requests;
    int count;
} Buffer;

void request();

void processReq( int );
