#include <stdio.h>
#include "Lift.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

/* initializing all mutexes and conditions as global variables as they to be 
seen by all of the processes */
pthread_mutex_t writeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; 

/* initializing both the file pointer that we're reading from and the file 
pointer that we're writing to as global variables because they also need to be
seen by all of the processes*/
FILE* inFile;
FILE* outFile;

/* m is the size of the buffer, while t is the time it takes for each request 
to be done. end signifies the end of the while loops in the consumer, and 
liftID helps assigning an id for our processes */
int m, t, end, liftID;

Buffer* reqBuf; 

int main( int argc, char** argv )
{
    
    int totalRequests = 0;
    int totalMovements = 0;

    pthread_t Lift_R, Lift1, Lift2, Lift3;
   
    Request* results[3];
 
    sscanf( argv[1], "%d", &m );
    
    sscanf( argv[2], "%d", &t );

    end = 0;
  
    liftID = 0;
 
    if( argc != 3 )
    {
        printf( "Please enter the details for the operation of the lifts as:\n"
            "./lift_sim_A m t \nwhere: \nm: the size of the buffer \n"
            "t: the time that each lift takes to complete its operations\n" );
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
        else 
        {
            reqBuf = (Buffer*)calloc( 1, sizeof( Buffer ) );
            reqBuf->reQueue = (Request*)calloc( m, sizeof( Request ) );

            pthread_create( &Lift_R, NULL, request, NULL );

            pthread_create( &Lift1, NULL, processReq, NULL );
            pthread_create( &Lift2, NULL, processReq, NULL );
            pthread_create( &Lift3, NULL, processReq, NULL );
        
            pthread_join( Lift_R, NULL );
            pthread_join( Lift1, (void**)&(results[0]) );
            pthread_join( Lift2, (void**)&(results[1]) );
            pthread_join( Lift3, (void**)&(results[2]) );

            printf( "first thread %d %d\n", (results[0])->to, results[0]->from );
            printf( "first thread %d %d\n", (results[1])->to, results[1]->from );
            printf( "first thread %d %d\n", (results[2])->to, results[2]->from );

            totalRequests = (results[0])->to + (results[1])->to 
                + (results[2])->to; 
            totalMovements = (results[0])->from + (results[1])->from
                + (results[2])->from;

            fprintf( outFile, "\nTotal number of requests: %d\nTotal number"
                " of movements: %d\n", totalRequests, totalMovements );
            if( ferror( inFile ) )
            {
                perror( "Something went wrong while reading the file" );
            }

            if( ferror( outFile ) )
            {
                perror( "Something went wrong while writing the file" );
            }

            pthread_mutex_destroy( &writeMutex ); 
            pthread_mutex_destroy( &mutex );
            pthread_cond_destroy( &empty );
            pthread_cond_destroy( &full );

            free( reqBuf->reQueue );
            free( reqBuf );
        }

        fclose( outFile );
        fclose( inFile );   
    }
}


void* request( void* novalue )
{
    Request nextReq;

    nextReq.from = 1;
    nextReq.to = 1;

    while( !feof( inFile ) )
    { 
        pthread_mutex_lock( &mutex );

		fscanf( inFile, "%d %d\n", &( nextReq.from ), &( nextReq.to ) );

        if( reqBuf->count == m )
        {
            pthread_cond_wait( &empty, &mutex );
        }

	    (reqBuf->reQueue[reqBuf->count]).from  = nextReq.from;
        (reqBuf->reQueue[reqBuf->count]).to = nextReq.to;
    
        reqBuf->count++; 
    
        pthread_cond_signal( &full ); 
        pthread_mutex_unlock( &mutex );

        /*printf( "The following request was put into the buffer: %d %d\n", 
            nextReq.from, nextReq.to ); */
    }

    return NULL;
}


void* processReq( void* novalue )
{
    int current = 1;
    Request thisReq;
    int processed = 0;
    int totalMove = 0;
    int thisMove = 0;
    int thisID;
    Request returnValues;

    returnValues.to = 0;
    returnValues.from =0;

    pthread_mutex_lock( &mutex );

    liftID++;

    thisID = liftID;

    pthread_mutex_unlock( &mutex );

    thisReq.from = 1;
    thisReq.to = 1;

    
    for(;;)
    {        
        pthread_mutex_lock( &mutex );

        if( reqBuf->count == 0 )
        {  
            if( feof( inFile ) )
            {
                printf( "EXIT OF THREAD %d\n", thisID );
                returnValues.to = processed;
                returnValues.from = totalMove;
                pthread_mutex_unlock( &mutex );
                pthread_exit( (void*)(&returnValues) );
            }
            else
            {
                printf( "Thread %d waiting\n", thisID );
                pthread_cond_wait( &full, &mutex );
            }
        }

        thisReq.from = (reqBuf->reQueue[reqBuf->count - 1]).from;

        thisReq.to = (reqBuf->reQueue[reqBuf->count - 1]).to;

        reqBuf->count--;

        pthread_cond_signal( &empty );
        
        pthread_mutex_unlock( &mutex );

        /*printf( "Thread %d Received request: %d %d\n", thisID, thisReq.from, thisReq.to );*/

        thisMove = abs( thisReq.from - current ) 
            + abs( thisReq.from - thisReq.to );

        totalMove += thisMove;

        processed++;

        pthread_mutex_lock( &writeMutex );

        fprintf( outFile, "Lift %d Operation\nPrevious position: Floor %d\n"
            "Request: Floor %d to Floor %d\nDetail operations:\n        "
            "#movement for this request: %d\n        #request: %d\n        "
            "Total #movement: %d\nCurrent Position: %d\n\n",  
            thisID, /* thread id/ the lift number */ 
            current, /* current floor */
            thisReq.from, thisReq.to, /* source and destination */
            thisMove, /* total moves done for current request */
            processed, /* the number of requests processed */
            totalMove, /* moves done in total for all requests */
            thisReq.to ); /* destination for current request will be the new 
                position for the lift */
        
        pthread_mutex_unlock( &writeMutex );
           
        current = thisReq.to;

        sleep ( t );
    }

    return NULL;
}

