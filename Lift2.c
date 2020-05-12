#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct{
    int** requests;
    int count;
} Buffer;

int main( int argc, char** argv )
{
    pid_t pid;

    int m, t, end, id = 0, x;

    Buffer *reqBuf;

    FILE* inFile;

    sscanf( argv[1], "%d", &m );
    sscanf( argv[2], "%d", &t );

    if( argc != 3 )
    {
        printf( "Please enter in....\n" );
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
         
        inFile = fopen( "sim_input", "r" );

        if( inFile == NULL )
        {
            perror( "Error opening the file for reading\n" );
        }
        else
        {      
            reqBuf = (Buffer*)mmap(NULL, sizeof(Buffer), PROT_READ | PROT_WRITE,
                                            MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
            reqBuf->requests = (int**)mmap(NULL, m * sizeof(int*), 
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );

            for( x = 0 ; x < m; x++ )
            {
                reqBuf->requests[x] = (int*)mmap(NULL, 2 * sizeof(int), 
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
            }
           
            for( x = -1; x < 3; x += 2 )
            {
                pid = fork();

                if( pid == 0 ) /* child process */
                {
                    id += x + 2;
                }
                else
                {
                    id += x + 1;   
                }
            }

            wait( 0 );
            sleep( 2 );
            printf( "I am process %d with id %d\n", (int)getpid(), id );
 
            for( x = 0; x < m; x++ )
            {
                munmap( reqBuf->requests[x], sizeof(int) * 2 );
            }   

            munmap( reqBuf->requests, sizeof(int*) * m );
            
            munmap( reqBuf, sizeof( Buffer ) );

            if( ferror( inFile ) )
            {
                perror( "Error while reading the file\n" );
            }

            fclose( inFile );
        }
    }
}



