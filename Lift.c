#include <stdio.h>
#include "Lift.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; 

FILE* inFile;
FILE* outFile;

/* m is the size of the buffer, while t is the time it takes for each request 
to be done */
int m, t, end; 

int main( int argc, char** argv )
{
    
    /* lifts */
    pthread_t Lift_R, Lift1, Lift2, Lift3;
    Buffer *reqBuf;

    /* initialize all the conditions and mutex locks
    mutex = PTHREAD_MUTEX_INITIALIZER;
    full = PTHREAD_COND_INITIALIZER;
    empty = PTHREAD_COND_INITIALIZER; */

    sscanf( argv[1], "%d", &m );
    
    sscanf( argv[2], "%d", &t );

    end = 0;
   
    inFile = fopen( "sim_input", "r" );
    outFile = fopen( "sim_output", "w" );

    if( inFile == NULL )
    {
        perror( "Could not open selected file, program will now exit" );
    }
    else if( outFile == NULL )
    {
        perror( "Could not open file for writing, program will now exit" );
    }
    else if( m <= 1 )
    {
        printf( "Please enter a value of m that is larger than 1\n" );
    }
    else if( t <= 0 )
    {
        printf( "Please enter a value for t that is larger than 0\n" );
    }
    else
    {
        reqBuf = (Buffer*)calloc( 1, sizeof( Buffer ) );
        reqBuf->reQueue = (Request*)calloc( m, sizeof( Request ) );

        pthread_create( &Lift_R, NULL, request, reqBuf );

        pthread_create( &Lift1, NULL, processReq, reqBuf );
        pthread_create( &Lift2, NULL, processReq, reqBuf );
        pthread_create( &Lift3, NULL, processReq, reqBuf );
    
        pthread_join( Lift_R, NULL );
        pthread_join( Lift1, NULL );
        pthread_join( Lift2, NULL );
        pthread_join( Lift3, NULL );

        if( ferror( inFile ) )
        {
            perror( "Something went wrong while reading the file" );
        }

        if( ferror( outFile ) )
        {
            perror( "Something went wrong while writing the file" );
        }

        fclose( outFile );
        fclose( inFile );
        
        pthread_mutex_destroy( &mutex );
        pthread_cond_destroy( &empty );
        pthread_cond_destroy( &full );

        free( reqBuf->reQueue );
        free( reqBuf );
    }
}


void* request( void* inBuf )
{
    Buffer* reqBuf = (Buffer*)inBuf;

    Request nextReq;

    nextReq.from = 1;
    nextReq.to = 1;

    while( !feof( inFile ) )
    {
		fscanf( inFile, "%d %d\n", &( nextReq.from ), &( nextReq.to ) );
        
        pthread_mutex_lock( &mutex );

        if( reqBuf->count == m )
        {
            pthread_cond_wait( &empty, &mutex );
        }

	    (reqBuf->reQueue[reqBuf->count]).from  = nextReq.from;
        (reqBuf->reQueue[reqBuf->count]).to = nextReq.to;
    
        reqBuf->count++; 
    
        pthread_mutex_unlock( &mutex );
        pthread_cond_signal( &full ); 

        printf( "The following request was put into the buffer: %d %d\n", 
            nextReq.from, nextReq.to ); 
    }

    end = 1;

    return NULL;
}


void* processReq( void* inBuf )
{
    int current = 1;
    Request thisReq;
    Buffer* reqBuf = (Buffer*)inBuf;
    int processed = 0;
    int totalMove = 0;
    int thisMove = 0;

    thisReq.from = 1;
    thisReq.to = 1;

    while( end == 0 )
    {        
        pthread_mutex_lock( &mutex );

        if( reqBuf->count == 0 )
        {   
            pthread_cond_wait( &full, &mutex );
        }

        thisReq.from = (reqBuf->reQueue[reqBuf->count]).from;

        thisReq.to = (reqBuf->reQueue[reqBuf->count]).to;

        reqBuf->count--;

        thisMove = abs( thisReq.from - thisReq.to );

        totalMove += thisMove;

        processed++;

        pthread_cond_signal( &empty );

        fprintf( outFile, "Lift %ld Operation\nPrevious position: Floor %d\n"
            "Request: Floor %d to Floor%d\n Detail operations:\n      "
            "#movement for this request: %d\n       #request: %d\n      "
            "Total #movement: %d\nCurrent Position: %d",  
            pthread_self(), /* thread id/ the lift number */ 
            current, /* current floor */
            thisReq.from, thisReq.to, /* source and destination */
            thisMove, /* total moves done for current request */
            processed, /* the number of requests processed */
            totalMove, /* moves done in total for all requests */
            thisReq.to ); /* destination for current request will be the new 
                position for the lift */
           
        current = thisReq.to;

        pthread_mutex_unlock( &mutex );

        sleep ( t );
    }

    return NULL;
}

/* 
void request( char** fileName, Buffer* reqBuf, Request* nextReq )
{
	readInput( fileName, nextReq );
	
	pthread_mutex_lock( &mutex );

    if( count == 3 )
    {
        pthread_mutex_unlock( &mutex );
        pthread_mutex_lock( &full )
        pthread_mutex_lock( &mutex );
    }

	&( reqBuf->reQueue[ ( count * count ) / count] ) = nextReq;
    
    reqBuf->count++; 
    pthread_mutex_unlock( &mutex );
}

void readInput( char** fileName, Request* nextReq )
{
	FILE* inFile = fopen( (*fileName), "r" );
	
	if( inFile == NULL )
	{
		perror( "Error in opening file, exiting" );
	}
	else
	{
		if( !feof() )
		{
			fscanf( inFile, "%d %d\n", &( nextReq->from ), &( nextReq->to ) );
		}

		if( ferror( inFile ) )
		{
			perror( "Error in reading the file" );
		}

		fclose( inFile );
	}
}*/


        
