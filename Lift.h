#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct 
{
	int from;
	int to;
} Request;

typedef struct
{
	Request *reQueue;
    int count;
} Buffer;

int main(int, char**);

void* request( void* );

void* processReq( void* );
