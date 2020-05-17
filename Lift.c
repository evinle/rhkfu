#include "Lift.h"

/* initializing all mutexes and conditions as global variables as they to be 
seen by all of the processes */
static pthread_mutex_t writeMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t full = PTHREAD_COND_INITIALIZER;
static pthread_cond_t empty = PTHREAD_COND_INITIALIZER; 

/* initializing both the file pointer that we're reading from and the file 
pointer that we're writing to as global variables because they also need to be
seen by all of the processes*/
static FILE* inFile;
static FILE* outFile;

/* m is the size of the buffer, while t is the time it takes for each request 
to be done, and liftID helps assigning an id for our processes */
static int m, t, liftID;

/* buffer also needs to be shared between threads and therefore is a global 
variable */
static Buffer* reqBuf; 

int main( int argc, char** argv )
{
   
    /* these variables are for the final total requests and movements of all 
        the lifts */ 
    int totalRequests = 0;
    int totalMovements = 0;

    /* declare the threads that will be used */
    pthread_t Lift_R, Lift1, Lift2, Lift3;
  
    /* this array is to fetch the return value from the threads */ 
    Request* results[3];
 
    sscanf( argv[1], "%d", &m );
    
    sscanf( argv[2], "%d", &t );
  
    liftID = 0;
 
    if( argc != 3 )
    {
        printf( "Please enter the details for the operation of the lifts as:\n"
            "./lift_sim_A m t \nwhere: \nm: the size of the buffer \n"
            "t: the time that each lift takes to complete its operations\n" );
    }
    else if( m < 1 )
    {
        printf( "Please enter a value of m that is larger than or equal to 1\n" );
    }
    else if( t < 0 )
    {
        printf( "Please enter a non negative value for t\n" );
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
            /*allocate memory for the buffer based on the size of the buffer*/
            reqBuf = (Buffer*)calloc( 1, sizeof( Buffer ) );
            reqBuf->reQueue = (Request*)calloc( m, sizeof( Request ) );

            /* create the threads and assign them to appropriate methods */
            pthread_create( &Lift_R, NULL, request, NULL );

            pthread_create( &Lift1, NULL, processReq, NULL );
            pthread_create( &Lift2, NULL, processReq, NULL );
            pthread_create( &Lift3, NULL, processReq, NULL );
            
            /* wait for all the threads to terminate and get their return value*/
            pthread_join( Lift_R, NULL );
            pthread_join( Lift1, (void**)&(results[0]) );
            pthread_join( Lift2, (void**)&(results[1]) );
            pthread_join( Lift3, (void**)&(results[2]) );

            /* output simple confirmation to the user */
            printf( "first thread %d %d\n", (results[0])->to, results[0]->from );
            printf( "second thread %d %d\n", (results[1])->to, results[1]->from );
            printf( "third thread %d %d\n", (results[2])->to, results[2]->from );

            /*add up total moves and requests */
            totalRequests = (results[0])->to + (results[1])->to 
                + (results[2])->to; 
            totalMovements = (results[0])->from + (results[1])->from
                + (results[2])->from;

            /* print to the final spot of outFile */
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

            /*clean up*/
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
    /* initialize place holder values */
    Request nextReq;

    nextReq.from = 1;
    nextReq.to = 1;

    while( !feof( inFile ) )
    { 
        pthread_mutex_lock( &mutex );
        
        /* grab input from buffer and assign it to appropriate place holders*/
		fscanf( inFile, "%d %d\n", &( nextReq.from ), &( nextReq.to ) );

        /* we cannot add more to the buffer once it is full, therefore we must
        wait for the buffer to be emptied out, at least partially, by the 
        consumers (processReq) in this case */
        while( reqBuf->count == m )
        {
            pthread_cond_wait( &empty, &mutex );
        }

        /* put the place holder values in the actual buffer */
	    (reqBuf->reQueue[reqBuf->count]).from  = nextReq.from;
        (reqBuf->reQueue[reqBuf->count]).to = nextReq.to;
    
        reqBuf->count++; 
    
        /* signal that the something is in the buffer, allowing the consumers 
        to start processing the info inside of the buffer */
        pthread_cond_signal( &full ); 
        pthread_mutex_unlock( &mutex );

        /*printf( "The following request was put into the buffer: %d %d\n", 
            nextReq.from, nextReq.to ); */
    }

    return NULL;
}


void* processReq( void* novalue )
{
    
    int current = 1; /* the current location, all start at 1 */
    Request thisReq; /* place holder */
    int processed = 0; /* number of requests processed */
    int totalMove = 0; /* the total number of moves done */
    int thisMove = 0; /* the moves done for the current request */
    int thisID; /* id of the process */
    Request returnValues; 

    returnValues.to = 0;
    returnValues.from =0;

    /* since liftID is a global variable shared amongst all of the threads, it
        is necessary to preserve mutual exclusion when altering its value */
    pthread_mutex_lock( &mutex );

    liftID++;

    thisID = liftID;

    pthread_mutex_unlock( &mutex );

    thisReq.from = 1;
    thisReq.to = 1;

    
    for(;;)
    {        
        pthread_mutex_lock( &mutex );
        
        /* if the buffer is empty, there are 2 possible scenarios, the first 
        one being one where the producer had not had enough time to put more 
        requests into the buffer, the second one being one where there's no 
        requests left to be put into the buffer. The second one only happens 
        when the end of the file that we're reading from (sim_input) is reached,
        hence it is what we check for. Otherwise, we should simply wait for the
        producer to put more requests into the buffer */
        while( reqBuf->count <= 0 )
        {  
            /* the reason why we need a while loop here is that sometimes when 
            the full signal is sent, the count is still 0 due to possible 
            context switch to other consumers */
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

        thisReq.from = (reqBuf->reQueue[0]).from;

        thisReq.to = (reqBuf->reQueue[0]).to;

        reqBuf->count--;

        /* in order to achieve the FIFO queue effect, memmove is used to 
        effectively move the entire array 1 spot to the left, overwriting
        the element at the start which we have already processed here */
        memmove( &( reqBuf->reQueue[0] ), &( reqBuf->reQueue[1] ), 
            sizeof( Request ) * reqBuf->count );

        /* signal that we've emptied out one of buffer slots so it can prepare 
        to put more into the buffer */
        pthread_cond_signal( &empty );
        
        pthread_mutex_unlock( &mutex );

        /* the moves done during this request is equal to the sum of the moves
        from the current location of the lift to the starting location of the 
        current request and the moves done from the starting location of the 
        current request to its end location */
        thisMove = abs( thisReq.from - current ) 
            + abs( thisReq.from - thisReq.to );

        totalMove += thisMove;

        processed++;

        /* since we're writing to a shared file, it is necessary to keep the 
        operation mutually exclusive between threads */
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
           
        /* the elevator now stays in the destination of this request until 
        further instructions */
        current = thisReq.to;

        sleep ( t );
    }

    return NULL;
}

