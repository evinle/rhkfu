#include "Lift2.h"

/* m is the size of the buffer, t is the time for sleeping in each function and
id is used to assign ids to the processes */
static int m, t, id = 1;

/* These are global variables that would be incremented by consummers each time
they process a move, therefore representing the total amount of moves and 
requests processed by the whole program */
static int *allMoves, *allRequests;

/* buffer and writing file is made global so we don't have to pass it around */
static Buffer *reqBuf;

static FILE* inFile;

/* signifies when to end a method */
static int* end;

int main( int argc, char** argv )
{
    /* used to identify processes */
    pid_t ID[3];

    /* delare the semaphores that will be used */
    sem_t *mutexsem, *fullsem, *emptysem, *writesem;

    sscanf( argv[1], "%d", &m );
    sscanf( argv[2], "%d", &t );
   
    /* since the method of writing to a file is appending, it is necessary to 
    clear out everything that was written on the file during the last 
    operation of this class */ 
    FILE* outFile = fopen( "sim_output", "w" );
    fclose( outFile );

    if( argc != 3 )
    {
        printf( "Please enter in the arguments " );
    }
    else if( m  < 1 )
    {
        printf( "Please enter a value of m that is larger than 1\n" );
    }
    else if( t < 0 )
    {
        printf( "Please enter a non negative value for n\n" );
    }
    else 
    {
        /* unlink any stray semaphores with the same name to avoid errors */ 
        sem_unlink( MUTEXSEM );

        sem_unlink( FULLSEM );
         
        sem_unlink( EMPTYSEM );

        sem_unlink( WRITESEM );

        /* create semaphores using sem open */

        /* mutex sem is our primary semaphore which is used to keep mutual 
        exclusion intached for global variables */
        mutexsem = sem_open( MUTEXSEM, O_CREAT, 0666, 1 ) ;
       
        /* fullsem is used to set a limit on how many elements the consumers 
        can take from the buffer before it runs out of requests, this needs to
        be set to 0 because at first the buffer is empty, therefore consumers 
        should not be able to take from the buffer. It is only when the producer
        has managed to put something into the buffer and incremented fullsem
        that consumers can start processing the info inside the buffer */
        fullsem = sem_open( FULLSEM, O_CREAT, 0666, 0 ) ;
        
        /* emptysem is used to set a limit on how many elements the producer 
        can put into the buffer before the buffer needs to be emptied out
        by the consumers, since the buffer is originally empty, this needs
        to be set to be the maximum size of the buffer. When it reaches 0, 
        the producer can no longer put items into the buffer */
        emptysem = sem_open( EMPTYSEM, O_CREAT, 0666, m ) ;

        /* writesem is used to ensure mutual exclusion in writing to files */
        writesem = sem_open( WRITESEM, O_CREAT, 0666, 1 );

        /* create regions of shared memory using mmap so that the variables are
        globally seen by all child processes */
        reqBuf = (Buffer*)mmap(NULL, sizeof(Buffer), 
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
        
        reqBuf->requests = (Request*)mmap(NULL, m * sizeof(Request), 
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );


        allRequests = (int*)mmap(NULL, sizeof(int), 
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
        
        allMoves = (int*)mmap(NULL, sizeof(int), 
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );

        end = (int*)mmap( NULL, sizeof(int), 
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
    
        *allMoves = 0;
        *allRequests = 0;
        *end = 0;
 
        ID[0] = fork(); 

        /* this structure is purely for ease of implementation, a for loop 
        structure can also be done to create similar results of 4 processes */

        /* the first child will create its own child */        
        if( ID[0] == 0 )
        {   
            ID[1] = fork();
            id++;
     
            /* the child of the first child now creates its own child */
            if( ID[1] == 0 )
            {
                ID[2] = fork();
                id++;

                if( ID[2] == 0 )
                {
                    id++;
                }
            }
       
            /* all child processes will end up running this function */ 
            processReq( id - 1 );
            
            wait( NULL );    /* sequentially wait for its only child */
        }
        else
        {
            /* we open the file here to reduce work load for the request func*/
            inFile = fopen( "sim_input", "r" );

            if( inFile == NULL )
            {
                perror( "Error opening the file for reading\n" );
            }
            else
            {      
                request();     
            }

            if( ferror( inFile ) )
            {
                perror( "Error while reading the file\n" );
            }

            fclose( inFile );
        }
        
        wait( 0 );
        
        /* write the total number of requests and moves at the end of the file*/
        outFile = fopen( "sim_output", "a" );
        
        if( outFile == NULL )
        {
            perror( "Cannot open file to print total values\n" );
        }
        else
        {
            fprintf( outFile, "\nTotal number of requests: %d\nTotal number of"
                " movements: %d\n", *allRequests, *allMoves );
        }
        
        /* clean up */
        fclose( outFile );

        sem_close( mutexsem );
    
        sem_close( fullsem );
            
        sem_close( emptysem );
       
        sem_close( writesem );
 
        sem_unlink( MUTEXSEM );

        sem_unlink( FULLSEM );
         
        sem_unlink( EMPTYSEM );

        sem_unlink( WRITESEM );

        munmap( reqBuf->requests, sizeof(int*) * m );
        
        munmap( reqBuf, sizeof( Buffer ) );

        munmap( end, sizeof( int ) );

        munmap( &allMoves, sizeof( int ) );

        munmap( &allRequests, sizeof( int ) );
        
        printf( "DONE\n" );
    }
    
}


void request()
{
    /* reopen the mutexes created in the parent process */
    sem_t *mutexsem = sem_open( MUTEXSEM, 0 );

    sem_t *fullsem = sem_open( FULLSEM, 0 );

    sem_t *emptysem = sem_open( EMPTYSEM, 0 );

    /* attenpt to read and put into buffer until end of file is reached */
    while( !feof( inFile ) )
    { 
        /* we need to wait for the emtpysem first before mutex locking to 
        avoid deadlock */
        sem_wait( emptysem );
 
        sem_wait( mutexsem );        
     
        fscanf( inFile, "%d %d\n", &( reqBuf->requests[reqBuf->count].from ),
            &( reqBuf->requests[reqBuf->count].to ) );

        reqBuf->count++;
                     
        sem_post( mutexsem ); 
        
        sem_post( fullsem );

    }   
    
    sem_wait( mutexsem );
    
    /* since end is a global shared variable between all child processes, 
    mutual exclusion needs to be kept */ 
    *end = 1;

    sem_post( mutexsem );

    if( sem_close( mutexsem ) < 0 )
    {
        perror( "Failed to close mutexsem\n" );
    }  
    if( sem_close( emptysem ) < 0 )
    {
        perror( "Failed to close emptysem\n" );
    }
    if( sem_close( fullsem ) < 0 )
    {
        perror( "Failed to close fullsem\n" );
    }
}


void processReq( int thisID )
{
    /* reopen the mutexes created in the parent process */
    sem_t *mutexsem = sem_open( MUTEXSEM, 0 );

    sem_t *fullsem = sem_open( FULLSEM, 0 );

    sem_t *emptysem = sem_open( EMPTYSEM, 0 );

    sem_t *writesem = sem_open( WRITESEM, 0 );

    /* initialize place holder values, which always start at 1 */
    int from = 1, to = 1;
    
    /* thisMove is the moves made for the current request,
    totalMove is the number of moves made in total BY THIS PROCESS,
    processed is the number of requests that have been processed BY THIS 
    PROCESS,
    current is the current location of the lift */
    int thisMove = 0, totalMove = 0, processed = 0, current = 1;

    /* although each process writing to this file will have a separate pointer,
    since we are appending our output to the file, they can be written one 
    after another with proper synchronization */

    FILE* outFile = fopen( "sim_output", "a" );

    for(;;)
    {
        sem_wait( mutexsem );
      
        /* always check if the end condition is met to avoid waiting for a
        signal that would never come or decrementing the count to a negative 
        number */
        if( *end == 1 )
        {
            /* even if the end of file is reached, that does not mean that the
            buffer is empty, therefore we need to also check if the buffer is
            empty, if it is, then we can safely assume that there's nothing left
            to process and procede to exit. If the count however, is larger than
            0, we need to process all of the requests that are left in the 
            buffer before exiting */
            if( reqBuf->count == 0 )
            {
                sem_post( mutexsem );
                fclose( outFile );
                printf( "\nPROCESS %d HAS FINISHED \n\n", thisID );
        
                if( sem_close( mutexsem ) < 0 )
                {
                    perror( "Failed to close mutexsem\n" );
                }  
                if( sem_close( emptysem ) < 0 )
                {
                    perror( "Failed to close emptysem\n" );
                }
                if( sem_close( fullsem ) < 0 )
                {
                    perror( "Failed to close fullsem\n" );
                }
                if( sem_close( writesem ) < 0 )
                {
                    perror( "Failed to close writesem\n" );
                }
                exit( 0 );
            }
            else
            {
                goto conproc; /* continue as normal */
            }
        }
        else 
        {
            /* we need to release the lock here in case theres nothing in the 
            buffer and the producer needs the lock to produce */
            conproc: sem_post( mutexsem );

            /* wait for the buffer to become populated */
            sem_wait( fullsem ); 

            /* start again once we know that there's something to process in
            the buffer */
            sem_wait( mutexsem );
           
            from = (reqBuf->requests[0].from);
            to = (reqBuf->requests[0].to);
     
            reqBuf->count--;

            /* no point moving 0 */
            if( reqBuf->count > 0 )
            {
                memmove( & ( reqBuf->requests[0] ), & ( reqBuf->requests[1] ), 
                    sizeof( Request ) * reqBuf->count );
            }
            
            sem_post( mutexsem );

            sem_post( emptysem );
            

            thisMove = abs( from - current ) + abs( from - to );

            totalMove += thisMove;

            processed++;

            /* since these are global variables shared between all processes, 
            it is important to preserve mutual exclusion */
            sem_wait( mutexsem );
            
            (*allMoves) += thisMove;
            
            (*allRequests)++;
        
            sem_post( mutexsem );


            sem_wait( writesem );
            
            fprintf( outFile, "Lift %d Operation\nPrevious position: Floor %d\n"
                "Request: Floor %d to Floor %d\nDetail operations:\n        "
                "#movement for this request: %d\n        #request: %d\n        "
                "Total #movement: %d\nCurrent Position: %d\n\n",  
                thisID, /* thread id/ the lift number */ 
                current, /* current floor */
                from, to, /* source and destination */
                thisMove, /* total moves done for current request */
                processed, /* the number of requests processed */
                totalMove, /* moves done in total for all requests */
                to ); /* destination for current request will be the new 
                    position for the lift */ 

            fflush( outFile ); /* flush so all thats in the writing buffer is
            printed. If this is not done it is possible to have them write 
            on top or in between each other (which is not good!) */
                
            sem_post( writesem );             
 
            /* the new current will be the destination of the current requests*/
            current = to;   

            sleep( t );
        }
    }
}

