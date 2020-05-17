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
    int** requests;
    int count;
} Buffer;

int m, t, id = 1, x;

Buffer *reqBuf;

FILE* inFile;

int* end;

void request();

void processReq( int );

int main( int argc, char** argv )
{
    pid_t ID[3];

    sem_t *mutexsem, *fullsem, *emptysem, *writesem;

    sscanf( argv[1], "%d", &m );
    sscanf( argv[2], "%d", &t );
        
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
        sem_unlink( MUTEXSEM );

        sem_unlink( FULLSEM );
         
        sem_unlink( EMPTYSEM );

        sem_unlink( WRITESEM );

        mutexsem = sem_open( MUTEXSEM, O_CREAT, 0666, 1 ) ;
       
        fullsem = sem_open( FULLSEM, O_CREAT, 0666, 0 ) ;
        
        emptysem = sem_open( EMPTYSEM, O_CREAT, 0666, m ) ;

        writesem = sem_open( WRITESEM, O_CREAT, 0666, 1 );

        reqBuf = (Buffer*)mmap(NULL, sizeof(Buffer), 
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
        
        reqBuf->requests = (int**)mmap(NULL, m * sizeof(int*), 
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );

        for( x = 0 ; x < m; x++ )
        {
            reqBuf->requests[x] = (int*)mmap(NULL, 2 * sizeof(int), 
                                PROT_READ | PROT_WRITE, 
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
        }

        end = (int*)mmap( NULL, sizeof(int), 
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
    
        *end = 0; 
        ID[0] = fork(); 
        
        if( ID[0] == 0 )
        {   
            /*printf( "2nd process\n" );*/
            ID[1] = fork();
            id++;
     
            if( ID[1] == 0 )
            {
                /*printf( "3rd process\n" );*/
                ID[2] = fork();
                id++;

                if( ID[2] == 0 )
                {
                    /*printf( "4th process\n" );*/
                    id++;
                }
            }
        
            processReq( id - 1 );
            wait( 0 );
        }
        else
        {
            /*printf( "1st process\n" );*/
        
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
        
        printf( "I am process %d with id %d\n", (int)getpid(), id );
        wait( 0 );
        /*exit( 0 );*/
        
        sem_close( mutexsem );
    
        sem_close( fullsem );
            
        sem_close( emptysem );
       
        sem_close( writesem );
 
        sem_unlink( MUTEXSEM );

        sem_unlink( FULLSEM );
         
        sem_unlink( EMPTYSEM );

        sem_unlink( WRITESEM );

        for( x = 0; x < m; x++ )
        {
            munmap( reqBuf->requests[x], sizeof(int) * 2 );
        }   

        munmap( reqBuf->requests, sizeof(int*) * m );
        
        munmap( reqBuf, sizeof( Buffer ) );

        munmap( end, sizeof( int ) );
        
        printf( "DONE\n" );
    }
    
}


void request()
{
    sem_t *mutexsem = sem_open( MUTEXSEM, 0 );

    sem_t *fullsem = sem_open( FULLSEM, 0 );

    sem_t *emptysem = sem_open( EMPTYSEM, 0 );

    while( !feof( inFile ) )
    {
        
        /*printf( "mutex locked for process request\n" );*/

        sem_wait( emptysem );
 
        sem_wait( mutexsem );        
     
        fscanf( inFile, "%d %d\n", &( reqBuf->requests[reqBuf->count][0] ),
            &( reqBuf->requests[reqBuf->count][1] ) );

        reqBuf->count++;

        /*printf( "count for request: %d\n", reqBuf->count ); */
         
        sem_post( mutexsem ); 
        
        sem_post( fullsem );
        /*printf( "mutex unlocked for process request\n" );*/

    }   
    
    sem_wait( mutexsem );
    
    *end = 1;

    sem_post( mutexsem );

    sem_close( mutexsem );
    sem_close( emptysem );
    sem_close( fullsem );
}


void processReq( int thisID )
{
    sem_t *mutexsem = sem_open( MUTEXSEM, 0 );

    sem_t *fullsem = sem_open( FULLSEM, 0 );

    sem_t *emptysem = sem_open( EMPTYSEM, 0 );

    sem_t *writesem = sem_open( WRITESEM, 0 );

    int from = 1, to = 1;

    int thisMove = 0, totalMove = 0, processed = 0, current = 1;

    FILE* outFile = fopen( "sim_output", "a" );
    int fsv;

    for(;;)
    {

        /*printf( "mutex locked for process %d 1st section\n",thisID );*/
        sem_wait( mutexsem );
       
        /*sem_getvalue( mutexsem, &fsv );
        printf( "MUTEX VALUE %d\n", fsv );*/
 
        if( *end == 1 )
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
            /*printf( "mutex unlocked for process %d 1st section\n",thisID );*/
            sem_post( mutexsem );

            sem_wait( fullsem );

            sem_wait( mutexsem );
            

            /*printf( "\npass 2nd mutex lock for process %d\n", thisID ); */
            from = (reqBuf->requests[0][0]);
            to = (reqBuf->requests[0][1]);
            
            /*printf( "\npass  from to for process %d\n", thisID ); */

            reqBuf->count--;

            /*printf( "\npass count dec for process %d\n", thisID ); */
            /*printf( "process %d takes out count\n", thisID );*/
            if( reqBuf->count > 0 )
            {
                memmove( & ( reqBuf->requests[0] ), reqBuf->requests[1], 
                    sizeof( sizeof( int ) * 2 * reqBuf->count ) );
            }
            /*printf( "\npass memory slide for process %d\n", thisID ); */

            sem_post( mutexsem );
            /*printf( "\npass post for sem for process %d\n", thisID ); */

            sem_post( emptysem );

            /*printf( "mutex unlocked for process %d 2nd section\n",thisID );*/
            
            thisMove = abs( from - current ) + abs( from - to );

            totalMove += thisMove;

            processed++;

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
            fflush( outFile );
            sem_post( writesem );             
            current = to;   

            sleep( t );
        }
    }
}

